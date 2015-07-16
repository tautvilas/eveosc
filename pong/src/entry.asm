;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; aout format executable header ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

bits 32

global _start       ; entry for linker

extern _gCodeSize   ; code segment size (obtained from linker)
extern _gDataSize   ; data segment size (obtained from linker)
extern _gBssSize    ; bss segment size (obtained from linker)

extern _main

SECTION .header

;_start:

h_midmag    dd  00640107h   ; loading conventions
h_text      dd  _gCodeSize  ; code segmen size in bytes
h_data      dd  _gDataSize  ; data segment size in bytes
h_bss       dd  _gBssSize   ; bss segment size in bytes
h_syms      dd  00000000h   ; symbol table size in bytes
h_entry     dd  _main       ; executable entry point (loaded at runtime)
h_trsize    dd  00000000h   ; text relocation table size in bytes
h_drsize    dd  00000000h   ; data relocation table size in bytes
