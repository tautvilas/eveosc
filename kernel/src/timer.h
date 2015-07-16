#ifndef _TIMER_H_
#define _TIMER_H_

#include "global.h"

#define _TIMER_RATE   1000 /* IRQ0 will be ticking 1000 times per second */

/* install PIR IRQ0 handler and set PIT firing rate */

extern void KERNEL_CALL
timer_install(void);

/* Return, how many ticks passed since PIT initialization */

extern unsigned int KERNEL_CALL
timer_get_num_ticks(void);

/* change the running task */

extern void KERNEL_CALL
timer_schedule();

#endif // _TIMER_H_
