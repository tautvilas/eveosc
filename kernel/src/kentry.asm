;;;;;;;;;;;;;;;;;;;;;;
; EveOS Kernel 0.0.1 ;
;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Init                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../common/aliases.asm"

bits 16

; global vars

global _start               ; entry symbol for linker
global _idt_load            ; function for loading IDT

global _gGdt

global _gGdtKernelCsSel     ; gdt kernel cs selector
global _gGdtKernelDataSel   ; gdt kernel data selector
global _gGdtUserCsSel       ; gdt user cs selector
global _gGdtUserDataSel     ; gdt user data selector

global _gKernelEsp

; extern vars

extern _gKernelStart        ; kernel vm start
extern _gBssStart           ; kernel bss section start
extern _gBssEnd             ; kernel bss section end

extern _os_main             ; OS main C function
extern _gIdtp               ; Pointer to IDT
extern _exception_handler   ; ISRs handler
extern _irq_handler         ; IRQs handler
extern _gpActiveTask        ; active task
extern _gKernelCr3          ; kernel page dir

extern _gTss                ; pointer to tss segment

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SECTION .text

    ; this jmp is needed for kernel loader integrity test
    jmp _start
_start:

    mov eax, title - KERNEL_BASE
    mov si, ax
    call print_str - KERNEL_BASE

    call enableA20 - KERNEL_BASE

    cmp eax, 0
    je a20_success
    mov eax, a20_failure_msg - KERNEL_BASE
    mov si, ax
    call print_str - KERNEL_BASE
    cli
    hlt
    call print_str - KERNEL_BASE

a20_success:
    mov eax, a20_success_msg - KERNEL_BASE
    mov si, ax
    call print_str - KERNEL_BASE


    cli             ; Disable external interrupts

    ; init gdt
    mov eax, gdtptr
    sub eax, KERNEL_BASE
    lgdt[eax]

    ; init p-mode
    mov eax, cr0    ; The lsb of cr0 is the protected mode bit
    or al, 1        ; Set protected mode bit
    mov cr0, eax    ; Mov modified word to the control register
    ; far jump is for setting code segment
    jmp dword CODE_SEL:go_pm - KERNEL_BASE

bits 32

go_pm:
    ; initialise all segments to data selector
    mov ax, DATA_SEL
    mov ds, eax
    mov fs, eax
    mov es, ax
    mov ss, ax
    mov gs, ax

    ; move user apps after bss
    mov ecx, KERNEL_SIZE * 512;
    mov eax, _gBssStart
    sub eax, _gKernelStart
    sub ecx, eax
    mov edi, _gBssStart - KERNEL_BASE
    mov esi, _gBssEnd - KERNEL_BASE
relocate_user_apps:
    mov dh, byte [edi + ecx - 1]
    mov byte [esi + ecx - 1], dh
    loop relocate_user_apps

    ; fill .bss with 0
    mov ecx, _gBssEnd
    sub ecx, _gBssStart
    mov eax, _gBssStart - KERNEL_BASE
set_bss_null:
    mov byte [eax], 0
    inc eax
    loop set_bss_null


    ; init paging
    mov esp, _sys_stack - KERNEL_BASE

    ; init kernel page table (over 2GB and first 8MB)
    ; TODO maybe do this with macros
    mov eax, _pd - KERNEL_BASE      ; eax  = &pd

    mov ebx, _pt1 - KERNEL_BASE + PAGE_RW_PRESENT   ; ebx  = &pt | 3
    mov [eax], ebx                  ; pd[0] = &pt
    add eax, 4
    mov ebx, _pt2 - KERNEL_BASE + PAGE_RW_PRESENT   ; ebx  = &pt | 3
    mov [eax], ebx

    mov eax,  _pd - KERNEL_BASE + PAGE_DIRECTORY_OFFSET ; eax = &pde[512h]

    mov ebx, _pt1 - KERNEL_BASE + PAGE_RW_PRESENT   ; ebx  = &pt | 3
    mov [eax], ebx                  ; pd[512] = &pt
    add eax, 4
    mov ebx, _pt2 - KERNEL_BASE + PAGE_RW_PRESENT   ; ebx  = &pt | 3
    mov [eax], ebx

    ; fill in the page tables

    mov edi, _pt1 - KERNEL_BASE     ; edi = &pt
    mov eax, PAGE_RW_PRESENT        ; Address 0, bit p & r/w set
    mov ecx, NUM_PAGE_ENTRIES       ; 1024 entries
init_pt1:
    stosd                           ; Write one entry
    add eax, PAGE_SIZE              ; Next page address
    loop init_pt1                   ; Loop

    mov edi, _pt2 - KERNEL_BASE     ; edi = &pt
    mov eax, PAGE_RW_PRESENT        ; Address 0, bit p & r/w set
    mov ecx, NUM_PAGE_ENTRIES       ; 1024 entries
init_pt2:
    stosd                           ; Write one entry
    add eax, PAGE_SIZE              ; Next page address
    loop init_pt2                   ; Loop

    ; set the page directory in cr3

    mov eax,  _pd - KERNEL_BASE     ; eax = &pd
    mov cr3, eax                    ; cr3 = &pd

    ; set CR0's PG bit.

    mov eax, cr0
    or eax, PAGING_BIT          ; Set PG bit
    mov cr0, eax                ; Paging is on!
    jmp $+2                     ; Flush the instruction queue.

    jmp CODE_SEL:paging_enabled

    ;push paging_enabled         ;Keep full address (8000xxxxh)
    ;ret                         ;Jump at Paging Mode (below)

paging_enabled:

    mov eax, CODE_SEL
    mov [_gGdtKernelCsSel], eax
    mov eax, DATA_SEL
    mov [_gGdtKernelDataSel], eax
    mov eax, USER_CODE_SEL
    mov [_gGdtUserCsSel], eax
    mov eax, USER_DATA_SEL
    mov [_gGdtUserDataSel], eax
    mov eax, TSS_SEL
    mov [_gTssSel], eax

    ; seting stack to use virual address
    mov eax, esp
    add eax, KERNEL_BASE
    mov esp, eax

    ; loading GDT using virtual address
    lgdt [vmgdtptr]

    ; here is the master call
    call _os_main

    cli
    hlt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Proc                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

bits 16

%include "enableA20.asm"

; Function to print out a string, which address is located in [SI]
print_str:
    push ax
    push bx
    push si

    mov ah, F_TELETYPE ; Function to display a chacter (teletype)
    mov bh, 0          ; Page number
    mov bl, CL_GRAY    ; Gray text color
.nextchar
    lodsb         ; Loads [SI] into AL and increases SI by one
    cmp al, 0     ; Check for end of string '0'
    jz .return
    int S_VIDEO
    jmp .nextchar ; Go to check next char
.return

    pop si
    pop bx
    pop ax
ret

bits 32

; Useful assembler functions, called from C code
%include "utils.asm"

; ISRs and IRQs function stubs
%include "isr.asm"

; Load the interrupt descriptor table
_idt_load:
    lidt [_gIdtp]
    ret

isr_common:
    pusha
    push ds
    push es
    push fs
    push gs

    ;;;;;;;;
    mov eax, [_gpActiveTask]
    cmp eax, 0          ; is mtasking enabled?
    je isr_no_multitasking1

    mov eax, [_gpActiveTask]
    mov [eax], esp      ; save active task esp
    mov ebx, [eax + 4]  ; load active task id
    cmp ebx, 0          ; is this task a root kernel task?
    jne isr_not_kernel
    mov [_gKernelEsp], esp   ; if this task is a kernel svave its esp in global

isr_not_kernel:
    mov eax, esp            ; prepare for pushinng pointer to regs
    mov esp, [_gKernelEsp]  ; change esp from task esp to kernel esp
    jmp isr_multitasking_init_end

isr_no_multitasking1:
    mov eax, esp
isr_multitasking_init_end:
    ;;;;;;;;

    push eax        ; pointer to regs struct

    mov ax, DATA_SEL
    mov ds, ax
    mov fs, ax
    mov es, ax
    mov gs, ax

    mov eax, _exception_handler
    call eax    ; a special call, preserves 'eip' register
    pop eax

    ;;;;;;;;
    mov eax, [_gpActiveTask]      ; is mtasking enabled?
    cmp eax, 0
    je isr_no_multitasking2

    ; read kstack top
    mov ebx, [eax + 16]
    ; update tss
    mov [_gTss + 4], ebx
    ; load stack pointer
    mov esp, [eax]
    ; load task page dir
    mov ebx, [eax + 8]
    mov cr3, ebx

isr_no_multitasking2:
    ;;;;;;;;

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8  ; cleans up pushed error code and ISR number
    iret        ; pops cs, eip, eflags (+ss and esp if interrupt was called at privilege lvl 3)

irq_common:
    pusha
    push ds
    push es
    push fs
    push gs

    ;;;;;;;;;
    mov eax, [_gpActiveTask]
    cmp eax, 0          ; is mtasking enabled?
    je irq_no_multitasking1

    mov eax, [_gpActiveTask]
    mov [eax], esp      ; save active task esp
    mov ebx, [eax + 4]  ; load active task id
    cmp ebx, 0          ; is this task a root kernel task?
    jne irq_not_kernel
    mov [_gKernelEsp], esp      ; if this task is a kernel svave its esp in global

irq_not_kernel:
    mov eax, esp            ; prepare for pushinng pointer to regs
    mov esp, [_gKernelEsp]  ; change esp from task esp to kernel esp
    jmp irq_multitasking_init_end

irq_no_multitasking1:
    mov eax, esp
irq_multitasking_init_end
    ;;;;;;;;;

    push eax                ; pointer to regs struct

    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, _irq_handler
    call eax    ; a special call, preserves 'eip' register
    pop eax

    ;;;;;;;;;
    mov eax, [_gpActiveTask]      ; is mtasking enabled?
    cmp eax, 0
    je irq_no_multitasking2

    ; patch tss

    ; read kstack top
    mov ebx, [eax + 16]
    ; update tss
    mov [_gTss + 4], ebx
    ; load task stack pointer
    mov esp, [eax]
    ; load task page dir
    mov ebx, [eax + 8]
    mov cr3, ebx

irq_no_multitasking2:
    ;;;;;;;;;

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8  ; cleans up pushed error code and ISR number
    iret        ; pops cs, eip, eflags, ss and esp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SECTION .data

title db 13, 10, "EveOS kernel v0.0.1 is starting, please fasten your seatbelts", 13, 10, 0
a20_success_msg db "A20 gate enabled", 13, 10, 0
a20_failure_msg db "Failded to enable A20 gate! Halting.", 13, 10, 0

; code selectors
_gGdtKernelCsSel    dd 0
_gGdtKernelDataSel  dd 0
_gGdtUserCsSel      dd 0
_gGdtUserDataSel    dd 0
_gTssSel dd 0

; kernel stack top
_gKernelEsp     dd 0

; main pointer to gdt
gdtptr :
    dw gdt_end- _gGdt-1     ; Length of the gdt - 1
    dd _gGdt - KERNEL_BASE  ; physical address of gdt

vmgdtptr :
    dw gdt_end-_gGdt-1      ; Length of the gdt - 1
    dd _gGdt                ; virtual address of gdt

; the mighty gdt itself
_gGdt
NULL_SEL equ $-_gGdt  ; $->current location,so nullsel = 0h
gdt0                   ; Null descriptor,as per convention gdt0 is 0
    dd 0               ; Each gdt entry is 8 bytes, so at 08h it is CS
    dd 0               ; In all the segment descriptor is 64 bits

; ATTENTION! If code segment position is about to be changed changes must be made to idt.c
; TODO externalize segment values

CODE_SEL equ $-_gGdt  ; This is 8h,ie 2nd descriptor in gdt
code_gd                ; Code descriptor 4Gb flat segment at 0000:0000h
    dw 0xffff          ; Limit 4Gb  bits 0-15 of segment descriptor
    dw 0x0000          ; BASE 0h bits 16-31 of segment descriptor (sd)
    db 0x00            ; BASE addr of seg 16-23 of 32bit addr,32-39 of sd
    db 10011010b       ; P,DPL(2),S,TYPE(3),A->Present bit 1,Descriptor
                       ; privilege level 0-3,Segment descriptor 1 ie code
                       ; or data seg descriptor,Type of seg,Accessed bit
    db 11001111b       ; Upper 4 bits G,D,0,AVL ->1 segment len is page
                       ; granular, 1 default operation size is 32bit seg
                       ; AVL : Available field for user or OS
                       ; Lower nibble bits 16-19 of segment limit
    db 0x00            ; BASE addr of seg 24-31 of 32bit addr,56-63 of sd
DATA_SEL equ $-_gGdt  ; ie 10h, beginning of next 8 bytes for data sd
data_gd                ; Data descriptor 4Gb flat seg at 0000:0000h
    dw 0xffff          ; Limit 4Gb
    dw 0x0000          ; BASE 0000:0000h
    db 0x00            ; Descriptor format same as above
    db 10010010b
    db 11001111b
    db 0x00
USER_CODE_SEL equ $-_gGdt
code_user_gd           ; Code descriptor 4Gb flat segment at 0000:0000h
    dw 0xffff          ; Limit 4Gb  bits 0-15 of segment descriptor
    dw 0x0000          ; BASE 0h bits 16-31 of segment descriptor (sd)
    db 0x00            ; BASE addr of seg 16-23 of 32bit addr,32-39 of sd
    db 11111010b       ; P,DPL(2),S,TYPE(3),A->Present bit 1,Descriptor
                       ; privilege level 0-3,Segment descriptor 1 ie code
                       ; or data seg descriptor,Type of seg,Accessed bit
    db 11001111b       ; Upper 4 bits G,D,0,AVL ->1 segment len is page
                       ; granular, 1 default operation size is 32bit seg
                       ; AVL : Available field for user or OS
                       ; Lower nibble bits 16-19 of segment limit
    db 0x00            ; BASE addr of seg 24-31 of 32bit addr,56-63 of sd
USER_DATA_SEL equ $-_gGdt
data_user_gd           ; Data descriptor 4Gb flat seg at 0000:0000h
    dw 0xffff          ; Limit 4Gb
    dw 0x0000          ; BASE 0000:0000h
    db 0x00            ; Descriptor format same as above
    db 11110010b
    db 11001111b
    db 0x00
TSS_SEL equ $-_gGdt
tss_gd
    dd 0
    dd 0
gdt_end

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; BSS                                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SECTION .bss
; page directory
_pd:
    resb PAGE_SIZE

; page tables
_pt1:
    resb PAGE_SIZE

_pt2:
    resb PAGE_SIZE
_pt2_end:

; system stack
    resb STACK_SIZE    ; This reserves 8KBytes of memory
_sys_stack:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; EOF
