#ifndef _PORTS_H_
#define _PORTS_H_

#include "global.h"

/**
 *  Returns byte of data from specified hardware port.
 *
 *  @param  aPort   Port to get byte from.
 *  @return         Byte of data from specified port.
 */
extern byte_t KERNEL_CALL
inportb(word_t aPort);

/**
 *  Sends byte to specified hardware port.
 *
 *  @param  aPort   Port to send byte to.
 *  @param  aByte   Byte to send.
 */
extern void
outportb(word_t aPort, byte_t aByte);


#endif  // _PORTS_H_
