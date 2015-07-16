#include "syscalls.h"
#include "keyboard.h"
#include "global.h"
#include "stdio.h"
#include "sem.h"
#include "resmgr.h"
#include "vga.h"
#include "tskmgr.h"
#include "timer.h"
#include "mem.h"

#define STDIN  0
#define STDOUT 1

void KERNEL_CALL
sys_read(regs_t* apRegs)
{
    //DUMP(apRegs->ecx);
    //DUMP(apRegs->ebx);
    //DUMP(apRegs->edx);
    dword_t file_descriptor = apRegs->ebx;
    //char * offset = (char*) apRegs->ecx;
    //dword_t numbytes = apRegs->edx;
    //BRAG("task %d asked res (fd:%x, l:%d)\n", gpActiveTask->id, file_descriptor, gpActiveTask->locked);
    if (file_descriptor == STDIN && !gpActiveTask->locked)
    {
        rm_add_waiting_task(gpActiveTask, apRegs, KEYBOARD_SEMAPHORE);
        gpActiveTask->locked = TRUE;
        //BRAG("task %d locked\n", gpActiveTask->id);
        timer_schedule(TRUE);
    }
    else
    {
        printf("bad file descriptor specified! (%x)\n", file_descriptor);
        gpActiveTask->locked = TRUE;
        timer_schedule(TRUE);
    }
    return;
}

void KERNEL_CALL
sys_write(regs_t* apRegs)
{
    dword_t file_descriptor = apRegs->ebx;
    char * offset = (char*) apRegs->ecx;
    dword_t length = apRegs->edx;

    if (file_descriptor == STDOUT)
    {
        int i;
        for(i = 0; i < length; i++, offset++)
        {
            vga_print_char(*offset);
        }
    }
    return;
}

void KERNEL_CALL
sys_exec(regs_t* apRegs)
{
    bool_t on_top = apRegs->ebx; //
    char * name = (char*) apRegs->ecx; // buffer
    priority_t priority = apRegs->edx;   // priviledge
    if (strcmp(name, "ping") == 0)
    {
        tm_load_task((pointer_t)gPingTaskOffset, gpActiveTaskRingNode, ACC_USER, priority, on_top);
        apRegs->eax = 0; // success
    }
    else if (strcmp(name, "esh") == 0)
    {
        tm_load_task((pointer_t)gEshTaskOffset, gpActiveTaskRingNode, ACC_USER, priority, on_top);
        apRegs->eax = 0; // success
    }
    else
    {
        apRegs->eax = -1;
    }
    //DUMP(apRegs->ebx);
    //DUMP(apRegs->ecx);
    //DUMP(apRegs->edx);
    return;
}

void KERNEL_CALL
sys_kill(regs_t* apRegs)
{
    uint_t id = apRegs->edx;
    timer_schedule(TRUE);
    tm_kill_task(id);
    return;
}
