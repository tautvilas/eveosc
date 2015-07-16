#ifndef _MEM_H_
#define _MEM_H_

#include "global.h"


/**
 *  Copies bytes from source to destination address.
 *
 *  @param  apDest  Destination address.
 *  @param  apSrc   Source address.
 *  @param  aCount  Number of bytes to copy.
 *  @return         Destination address.
 */
extern pointer_t KERNEL_CALL
memcpy(pointer_t apDest, const pointer_t apSrc, size_t aCount);

/**
 *  Fills memory bytes with specified value.
 *
 *  @param  apDest  Destination address.
 *  @param  aVal    Value to set.
 *  @param  aCount  Number of bytes to fill.
 *  @return         Destination address.
 */
extern pointer_t KERNEL_CALL
memset(pointer_t apDest, byte_t aVal, size_t aCount);


/**
 *  Fills memory words with specified value.
 *
 *  @param  apDest  Destination address.
 *  @param  aVal    Value to set.
 *  @param  aCount  Number of words to fill.
 *  @return         Destination address.
 */
extern word_t* KERNEL_CALL
memsetw(word_t* apDest, word_t aVal, size_t aCount);


/**
 *  Fills memory double words with specified value.
 *
 *  @param  apDest  Destination address.
 *  @param  aVal    Value to set.
 *  @param  aCount  Number of double words to fill.
 *  @return         Destination address.
 */
extern dword_t* KERNEL_CALL
memsetd(dword_t* apDest, dword_t aVal, size_t aCount);


/**
 *  Counts zero-terminated string length.
 *
 *  @param  apStr   String address.
 *  @return         Number of bytes from apStr to first zero byte not counting
 *                  zero byte.
 */
extern size_t KERNEL_CALL
strlen(const char *apStr);

extern void KERNEL_CALL
memdump(byte_t* apSource, size_t aCount);

int
strncmp(const char* apStr1, const char* apStr2, size_t aCount);

int
strcmp(const char* apStr1, const char* apStr2);

#endif // _MEM_H_
