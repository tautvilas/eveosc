;;;;;;;;;;;;;;;;;;;;;;;;
; EveOS Bootloader 1.0 ;
;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Init                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../common/aliases.asm"
; PC starts booting in real mode
[bits 16]
; Location in mem where bootsector is about to be stored
[org 7C00h]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Main                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Set up the DS
    mov ax, SEG_BOOT
    mov ds, ax
; initialize the stack
    mov ax, SEG_STACK
    mov ss, ax
    mov sp, OFF_STACK ;stack top
; set default video mode 80x25:
    mov ah, F_SET_VIDEO_MODE
    mov al, VIDEO_MODE_80x25
    int S_VIDEO
; Print out OS title
    mov si, title
    call print_str
; Load kernel from floppy sector 2 to 0800h:0000h - es:bx
    mov si, loading_from_fd0
    call print_str
    call load_kernel
; kernel integrity check:
    mov ax, SEG_KERNEL
    mov es, ax
    cmp byte [es:OFF_KERNEL], FIRST_KERNEL_BYTE
    je integrity_check_ok
; integrity check error
    mov si, load_kern_err
    call print_str
; wait for any key...
    call wait_for_key
; store magic value at 0040h:0072h:
    mov ax, SEG_REBOOT
    mov ds, ax
    mov word [OFF_REBOOT], COLD_BOOT
; now reboot!
    jmp SEG_REBOOT_JMP:OFF_REBOOT_JMP
integrity_check_ok:
; pass control to kernel:
    jmp SEG_KERNEL:OFF_KERNEL

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Procedures                                            ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; load the kernel at 0800h:0000h
; BIOS passes drive number in dl,
; so it's not changed:

load_kernel:
    push ax
    push bx
    push cx
    push es

    push dx

    mov ah, F_READ_SECT_FROM_DRIVE
; al keeps how many sectors to read
    mov al, 64
    mov ch, 0 ;cylinder.
    mov cl, 2 ;sector.
    mov dh, 0 ;head.
; dl not changed! - drive number

; es:bx points to receiving
; data buffer:
    mov bx, SEG_KERNEL
    mov es, bx
    mov bx, OFF_KERNEL
; read!
    int S_DISK

    ; after 64 sectors appears segment overlap

    pop dx
    mov ah, F_READ_SECT_FROM_DRIVE
; al keeps how many sectors to read
    mov al, KERNEL_SIZE % 64
    mov ch, 1 ;cylinder.
    mov cl, 12 ;sector.
    mov dh, 1 ;head.
; dl not changed! - drive number

; es:bx points to receiving
; data buffer:
    mov bx, SEG_KERNEL2
    mov es, bx
    mov bx, 0000h
; read!
    int S_DISK

    ;mov ah, F_READ_SECT_FROM_DRIVE
; al keeps how many sectors to read
    ;mov al, KERNEL_SIZE - 62
    ;mov ch, 1 ;cylinder.
    ;mov cl, 1 ;sector.
    ;mov dh, 0 ;head.
; dl not changed! - drive number

; es:bx points to receiving
; data buffer:
    ;mov bx, SEG_KERNEL
    ;mov es, bx
    ;mov bx, OFF_KERNEL + 62 * 512
; read!
    ;int S_DISK

    pop es
    pop cx
    pop bx
    pop ax
ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

wait_for_key:
    mov ah, F_READ_CHARACTER
    int S_KEYBOARD
ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

debug:
    mov si, debug_string
    call print_str
ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

title            db 'K-loader started!', 13, 10, 0
debug_string     db '###### debug message ######', 13, 10, 0
loading_from_fd0 db 'Trying to read kernel from disc sector 2, cylinder 0, head 0...', 13, 10, 0
load_kern_err    db 'The kernel file is invalid!', 13, 10
                 db 'System will now reboot. Pres any key to continue...', 13, 10, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Ending                                                ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Fill all remaining of 510 bytes with 0
; $-$$ means [start of the instruction - start of the program]
    times 510-($-$$) db 0
; The magic value to comple 512b sector
    dw BOOTSECT_MAGIC_VALUE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; EOF
