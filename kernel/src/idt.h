#ifndef _IDT_H_
#define _IDT_H_

#include "global.h"

#define ISR_PAGE_FAULT 14

/**
 *  Installs interrupt descriptor table, ISRs and IRQs handling as well
 */

extern void KERNEL_CALL
idt_install(void);

/**
 *  Installs system call table
 */

extern void KERNEL_CALL
sys_call_table_install(void);

/**
 *  Sets IRQ routine handling function
 *
 *  @param  aIRQ      IRQ number
 *  @param  pHandler  pointer to handling function
 */

extern void KERNEL_CALL
irq_install_handler(int aIrq, void (*handler)(regs_t *apR));

/**
 *  Clears IRQ routine handling function
 *
 *  @param  aIRQ      IRQ number
 */

extern void KERNEL_CALL
irq_uninstall_handler(int aIrq);

/**
 *  Sets ISR routine handling function
 *
 *  @param  aISR      ISR number
 *  @param  pHandler  pointer to handling function
 */

extern void KERNEL_CALL
isr_install_handler(int aIsr, void (*apHandler)(regs_t *apR));

/**
 *  Clears ISR routine handling function
 *
 *  @param  aISR      ISR number
 */

extern void KERNEL_CALL
isr_uninstall_handler(int aIsr);

/**
 *  Kernel panic - prints out register information and halts
 */

extern void KERNEL_CALL
kernel_panic(void);

#endif  // _IDT_H_
