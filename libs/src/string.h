#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

/**
 *  Copies bytes from source to destination address.
 *
 *  @param  apDest  Destination address.
 *  @param  apSrc   Source address.
 *  @param  aCount  Number of bytes to copy.
 *  @return         Destination address.
 */
extern void*
memcpy(void* apDest, const void* apSrc, size_t aCount);


/**
 *  Fills memory bytes with specified value.
 *
 *  @param  apDest  Destination address.
 *  @param  aVal    Value to set.
 *  @param  aCount  Number of bytes to fill.
 *  @return         Destination address.
 */
extern void*
memset(void* apDest, int aVal, size_t aCount);


/**
 *  Fills memory words with specified value.
 *
 *  @param  apDest  Destination address.
 *  @param  aVal    Value to set.
 *  @param  aCount  Number of words to fill.
 *  @return         Destination address.
 */
//extern word_t*
//memsetw(word_t* apDest, word_t aVal, size_t aCount);


/**
 *  Fills memory double words with specified value.
 *
 *  @param  apDest  Destination address.
 *  @param  aVal    Value to set.
 *  @param  aCount  Number of double words to fill.
 *  @return         Destination address.
 */
//extern dword_t*
//memsetd(dword_t* apDest, dword_t aVal, size_t aCount);


/**
 *  Counts zero-terminated string length.
 *
 *  @param  apStr   String address.
 *  @return         Number of bytes from apStr to first zero byte not counting
 *                  zero byte.
 */
extern size_t
strlen(const char *apStr);


int
memcmp(const void* apSrc1, const void* apSrc2, size_t aCount);


int
strncmp(const char* apStr1, const char* apStr2, size_t aCount);


int
strcmp(const char* apStr1, const char* apStr2);


char*
strchr(char* apStr, int aChar);


char*
strpbrk(char* apStr, char* apDelims);


char*
strtok(char* apStr, char* apDelims);

#endif // _STRING_H_
