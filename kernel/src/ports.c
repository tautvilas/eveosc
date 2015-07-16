
#include "ports.h"

byte_t KERNEL_CALL
inportb(word_t aPort)
{
    byte_t result;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (result) : "dN" (aPort));
    return result;
}


void
outportb(word_t aPort, byte_t aByte)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (aPort), "a" (aByte));
}

