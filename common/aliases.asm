; kernel vitual memory base.
; !!! This value is hardcoded and must be synchronized with linker script base value !!!

    KERNEL_BASE equ 0x80000000;

; page directory 2GB table offset

    PAGE_DIRECTORY_OFFSET       equ KERNEL_BASE / 0x100000

; function aliases ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    F_TELETYPE                  equ 0Eh
    F_READ_CHARACTER            equ 00h
    F_SET_VIDEO_MODE            equ 00h
    F_READ_SECT_FROM_DRIVE      equ 02h

; interrupt (service) aliases
    S_DISK                      equ 13h
    S_VIDEO                     equ 10h
    S_KEYBOARD                  equ 16h

; colors
    CL_GRAY                     equ 07h

; memory addresses
    SEG_BOOT                    equ 0000h
    OFF_BOOT                    equ 7C00h
    SEG_KERNEL                  equ 0000h
    SEG_KERNEL2                 equ 1000h   ; segment for second section of kernel
    OFF_KERNEL                  equ 8000h
    SEG_REBOOT                  equ 0040h
    OFF_REBOOT                  equ 0072h
    SEG_REBOOT_JMP              equ 0ffffh
    OFF_REBOOT_JMP              equ 0000h
    SEG_STACK                   equ 07C0h   ; the same as 0000:7C00
    OFF_STACK                   equ 03feh

; sizes
    ; TODO:zv 2007 05 22: load at runtime
    KERNEL_SIZE                 equ 80      ; kernel size in sectors
    PAGE_SIZE                   equ 1000h   ; page size in bytes
    STACK_SIZE                  equ 8192    ; stack size in bytes
    NUM_PAGE_ENTRIES            equ 1024
    NUM_PAGE_TABLES             equ 2

; flags
    PAGE_RW_PRESENT             equ 3
    PAGING_BIT                  equ 80000000h

; other stuff
    FIRST_KERNEL_BYTE           equ 0E9h
    COLD_BOOT                   equ 0000h
    WARM_BOOT                   equ 1234h
    BOOTSECT_MAGIC_VALUE        equ 0AA55h  ; 55AAh
    VIDEO_MODE_80x25            equ 03h

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; EOF
