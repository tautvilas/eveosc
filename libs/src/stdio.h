#ifndef _STDIO_H_
#define _STDIO_H_

#include <stddef.h>

#define EOF     -1

typedef struct {
        int mFd;
    } FILE;


extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

/**
 *  Classic C printf, but not fully functional.
 *
 *  Currently supported formatting options:
 *  %d - int
 *  %x - int in hex format
 */

extern void /*__attribute__ ((cdecl))*/
printf(const char* apFormatStr, ...);

/**
 *  print out a character
 *
 *  @param  aC  character to be printed
 */

extern void
putc(char aC);

extern char
getchar(void);

char*
fgets(char* apBuffer, int aSize, FILE* apStream);

int
fgetc(FILE* apStream);

#endif // _STDIO_H_
