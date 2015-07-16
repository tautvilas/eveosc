#ifndef _SEM_H_
#define _SEM_H_

#include "global.h"

#define KEYBOARD_SEMAPHORE 0

extern uint_t gSemaphores[];

extern void KERNEL_CALL
semaphore_p(uint_t aSemaphore);

extern void KERNEL_CALL
semaphore_v(uint_t aSemaphore);

extern void KERNEL_CALL
semaphore_init(uint_t aSemaphore, uint_t aValue);

extern void KERNEL_CALL
semaphores_install(void);

extern uint_t KERNEL_CALL
semaphore_get(uint_t aSemaphore);
#endif // _SEM_H_
