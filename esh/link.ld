OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
/*ENTRY(_start)           /* app entry point */

_gAppStart = 0x00;

/*

_gHeaderEnd     = _gCodeStart   - _gAppStart;
_gCodeEnd       = _gDataStart   - _gAppStart;
_gDataEnd       = _gBssStart    - _gAppStart;
_gBssEnd        = _gAppEnd      - _gAppStart;

*/

_gHeaderSize    = _gHeaderEnd   - _gAppStart;
_gCodeSize      = _gCodeEnd     - _gCodeStart;
_gDataSize      = _gDataEnd     - _gDataStart;
_gBssSize       = _gBssEnd      - _gBssStart;

/* this instructs the linker how to lay out sections in the output */
SECTIONS
{
    .header _gAppStart : AT(_gAppStart)
    {
        _gHeaderStart = .;
        *(.header)
            . = ALIGN(8);
        _gHeaderEnd = .;
    }
    .text : AT(_gAppStart + (_gCodeStart - _gAppStart))
    {
        _gCodeStart = .;
        *(.text)
            . = ALIGN(8);
        _gCodeEnd = .;
    }
    .data : AT(_gAppStart + (_gDataStart - _gAppStart))
    {
        _gDataStart = .;
        /* rodata is a special section made by GCC for read only data
           this section can have several parts like
                rodata.str1.4
                rodata.str1.1
           and maybe other, that is why i wrote .rodata*
         */
        *(.rodata*)
        *(.rdata*)
        *(.data)
            . = ALIGN(8);
        _gDataEnd = .;
    }
    .bss : AT(_gAppStart + (_gBssStart - _gAppStart))
    {
        _gBssStart = .;
        *(.bss*)
        *(COMMON)
            . = ALIGN(8);
        _gBssEnd = .;
    }
    /* this marks the end of all sections */
    _gAppEnd = .;

    /* ignore these gcc misc sections */
    /DISCARD/ : {
        *(.comment*)
        *(.note*)
    }
}

