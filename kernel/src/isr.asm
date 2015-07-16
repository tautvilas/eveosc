;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Init                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; ISR references

global _isr0    ; division by zero exception
global _isr1    ; debug exception
global _isr2    ; non maskable interrupt exception
global _isr3    ; breakpoint exception
global _isr4    ; into detected overflow exception
global _isr5    ; out of bounds exception
global _isr6    ; invalid opcode exception
global _isr7    ; no coprocessor exception
global _isr8    ; double fault exception (pushes error code)
global _isr9    ; coprocessor segment overrun exception
global _isr10   ; bad TSS exception (pushes error code)
global _isr11   ; segment not present exception (pushes error code)
global _isr12   ; stack fault exception (pushes error code)
global _isr13   ; general protection fault exception (pushes error code)
global _isr14   ; page fault exception (pushes error code)
global _isr15   ; unknown interrupt exception
global _isr16   ; coprocessor fault exception
global _isr17   ; aligment check exception (486+)
global _isr18   ; machine check exception (pentium/586+)

; reserved exceptions

global _isr19
global _isr20
global _isr21
global _isr22
global _isr22
global _isr23
global _isr24
global _isr25
global _isr26
global _isr27
global _isr28
global _isr29
global _isr30
global _isr31

; IRQs begin here

global _irq0
global _irq1
global _irq2
global _irq3
global _irq4
global _irq5
global _irq6
global _irq7
global _irq8
global _irq9
global _irq10
global _irq11
global _irq12
global _irq13
global _irq14
global _irq15

; Kernel service interrupt

global _isr69

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; ISRS

_isr0:
    cli
    push byte 0    ; A normal ISR stub that pops a dummy error code to keep a
                   ; uniform stack frame
    push byte 0
    jmp isr_common

_isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_common

_isr2:
    cli
    push byte 0
    push byte 2
    jmp isr_common

_isr3:
    cli
    push byte 0
    push byte 3
    jmp isr_common

_isr4:
    cli
    push byte 0
    push byte 4
    jmp isr_common

_isr5:
    cli
    push byte 0
    push byte 5
    jmp isr_common

_isr6:
    cli
    push byte 0
    push byte 6
    jmp isr_common

_isr7:
    cli
    push byte 0
    push byte 7
    jmp isr_common

_isr8:
    cli
    push byte 8
    jmp isr_common

_isr9:
    cli
    push byte 0
    push byte 9
    jmp isr_common

_isr10:
    cli
    push byte 10
    jmp isr_common

_isr11:
    cli
    push byte 11
    jmp isr_common

_isr12:
    cli
    push byte 12
    jmp isr_common

_isr13:
    cli
    push byte 13
    jmp isr_common

_isr14:
    cli
    push byte 14
    jmp isr_common

_isr15:
    cli
    push byte 0
    push byte 15
    jmp isr_common

_isr16:
    cli
    push byte 0
    push byte 16
    jmp isr_common

_isr17:
    cli
    push byte 0
    push byte 17
    jmp isr_common

_isr18:
    cli
    push byte 0
    push byte 18
    jmp isr_common

_isr19:
    cli
    push byte 0
    push byte 19
    jmp isr_common

_isr20:
    cli
    push byte 0
    push byte 20
    jmp isr_common

_isr21:
    cli
    push byte 0
    push byte 21
    jmp isr_common

_isr22:
    cli
    push byte 0
    push byte 22
    jmp isr_common

_isr23:
    cli
    push byte 0
    push byte 23
    jmp isr_common

_isr24:
    cli
    push byte 0
    push byte 24
    jmp isr_common

_isr25:
    cli
    push byte 0
    push byte 25
    jmp isr_common

_isr26:
    cli
    push byte 0
    push byte 26
    jmp isr_common

_isr27:
    cli
    push byte 0
    push byte 27
    jmp isr_common

_isr28:
    cli
    push byte 0
    push byte 28
    jmp isr_common

_isr29:
    cli
    push byte 0
    push byte 29
    jmp isr_common

_isr30:
    cli
    push byte 0
    push byte 30
    jmp isr_common

_isr31:
    cli
    push byte 0
    push byte 31
    jmp isr_common

; IRQs begin here

_irq0:
    cli
    push byte 0 ; we need dummy byte here
    push byte 32
    jmp irq_common

_irq1:
    cli
    push byte 0 ; we need dummy byte here
    push byte 33
    jmp irq_common

_irq2:
    cli
    push byte 0 ; we need dummy byte here
    push byte 34
    jmp irq_common

_irq3:
    cli
    push byte 0 ; we need dummy byte here
    push byte 35
    jmp irq_common

_irq4:
    cli
    push byte 0 ; we need dummy byte here
    push byte 36
    jmp irq_common

_irq5:
    cli
    push byte 0 ; we need dummy byte here
    push byte 37
    jmp irq_common

_irq6:
    cli
    push byte 0 ; we need dummy byte here
    push byte 38
    jmp irq_common

_irq7:
    cli
    push byte 0 ; we need dummy byte here
    push byte 39
    jmp irq_common

_irq8:
    cli
    push byte 0 ; we need dummy byte here
    push byte 40
    jmp irq_common

_irq9:
    cli
    push byte 0 ; we need dummy byte here
    push byte 41
    jmp irq_common

_irq10:
    cli
    push byte 0 ; we need dummy byte here
    push byte 42
    jmp irq_common

_irq11:
    cli
    push byte 0 ; we need dummy byte here
    push byte 43
    jmp irq_common

_irq12:
    cli
    push byte 0 ; we need dummy byte here
    push byte 44
    jmp irq_common

_irq13:
    cli
    push byte 0 ; we need dummy byte here
    push byte 45
    jmp irq_common

_irq14:
    cli
    push byte 0 ; we need dummy byte here
    push byte 46
    jmp irq_common

_irq15:
    cli
    push byte 0 ; we need dummy byte here
    push byte 47
    jmp irq_common

; kernel service interrupt stub

_isr69:
    cli
    push byte 0
    push byte 69
    jmp isr_common
