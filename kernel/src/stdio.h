#ifndef _STDIO_H_
#define _STDIO_H_

/**
 *  Classic C printf, but not fully functional.
 *
 *  @param  apFormatStr  Output format string.
 *  @param  ...          Arguments to be inserted in format string.
 */

/**
 *  Currently support formatting options:
 *  %d - int
 *  %x - int in hex format
 */

extern void __attribute__ ((stdcall))
printf(const char* apFormatStr, ...);

/**
 *  print out a character
 *
 *  @param  aC  character to be printed
 */

extern void KERNEL_CALL
putc(char aC);

#endif // _STDIO_H_
