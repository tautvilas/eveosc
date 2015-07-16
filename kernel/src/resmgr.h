#ifndef _RESMGR_H_
#define _RESMGR_H_

#include "global.h"
#include "tskmgr.h"

extern KERNEL_CALL void
rm_start(void);

extern KERNEL_CALL void
rm_install(void);

extern void KERNEL_CALL
rm_add_waiting_task(task_t* apTask, regs_t* apRegs, uint_t aResource);

#endif // _RESMGR_H_
