#include "global.h"
#include "sem.h"
#include "mem.h"

#define NUM_SEMAPHORES 16

uint_t gSemaphores[NUM_SEMAPHORES];

void KERNEL_CALL
semaphore_p(uint_t aSemaphore)
{
    __asm__ __volatile__ ("cli");
    if(gSemaphores[aSemaphore] > 0)
        gSemaphores[aSemaphore]--;
    return;
}

void KERNEL_CALL
semaphore_v(uint_t aSemaphore)
{
    __asm__ __volatile__ ("cli");
    gSemaphores[aSemaphore]++;
    return;
}

void KERNEL_CALL
semaphore_init(uint_t aSemaphore, uint_t aValue)
{
    __asm__ __volatile__ ("cli");
    gSemaphores[aSemaphore] = aValue;
    return;
}

uint_t KERNEL_CALL
semaphore_get(uint_t aSemaphore)
{
    __asm__ __volatile__ ("cli");
    return gSemaphores[aSemaphore];
}

void KERNEL_CALL
semaphores_install(void)
{
    memsetd(gSemaphores, 0, NUM_SEMAPHORES);
    semaphore_init(KEYBOARD_SEMAPHORE, 1);
}
