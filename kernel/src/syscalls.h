#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "global.h"

extern void KERNEL_CALL
sys_read(regs_t* apRegs);

extern void KERNEL_CALL
sys_write(regs_t* apRegs);

extern void KERNEL_CALL
sys_exec(regs_t* apRegs);

extern void KERNEL_CALL
sys_kill(regs_t* apRegs);

enum syscalls_t {
    SYS_KILL = 1,
    SYS_FORK,
    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_WAITPID,
    SYS_CREAT,
    SYS_LINK,
    SYS_UNLINK,
    SYS_EXEC
};

#endif // _SYSCALLS_H_
