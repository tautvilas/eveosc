OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(_start)                  /* kernel entry point */

_gKernelBase        = 0x80000000;                       /* mark of kernel space */
_gKernelOffset      = 0x00008000;                       /* stands for physical address of kernel and as offset from kernel base */
_gKernelStart       = _gKernelBase + _gKernelOffset;    /* virtual address of kernel */

/* this instructs the linker how to lay out sections in the output */
SECTIONS
{
    .text _gKernelStart : AT(_gKernelStart) {
        /*_gKernelStart = .;*/
        code = .;
        *(.text)
            . = ALIGN(4096); /* align every section on a separate page */
    }
    .data : AT(_gKernelStart + (data - code))
    {
        data = .;
        /* rodata is a special section made by GCC for read only data
           this section can have several parts like
                rodata.str1.4
                rodata.str1.1
           and maybe other, that is why i wrote .rodata*
         */
        *(.rodata*)
        *(.rdata*)
        *(.data)
            . = ALIGN(4096);
    }
    .bss : AT(_gKernelStart + (bss - code))
    {
        _gBssStart = .;
        bss = .;
        *(.bss*)
        *(COMMON)
            . = ALIGN(4096);
        _gBssEnd = .;
    }
    /*end = .; /* this marks the end of all sections */
    _gKernelEnd = .;

    /* ignore these gcc misc sections */
    /DISCARD/ : {
        *(.comment*)
        *(.note*)
    }
}

