#include "resmgr.h"
#include "global.h"
#include "stdio.h"
#include "mem.h"
#include "keyboard.h"
#include "idt.h"
#include "vga.h"
#include "sem.h"

// 2 is good enough for now as we have only STDIN and STDOUT
#define RESOURCE_LIST_SIZE    2

extern dword_t read_cr3();
extern dword_t write_cr3();

typedef struct {
    task_t* pTask;
    regs_t* pRegs;
    uint_t resource;
} waiting_task_t;

typedef size_t rm_descr_t;

typedef enum {
        READ,
        WRITE
    } rm_oper_t;

typedef struct {
        task_t*     mpTask;
        regs_t*     mpRegs;
        rm_descr_t  mDescr;
        rm_oper_t   mOper;
    } rm_waiting_task_t;

typedef bool_t  (*(rm_handler_t KERNEL_CALL))(task_t* apTask, regs_t* apRegs);

typedef struct {
        size_t          mLimit;
        size_t          mFree;
        rm_handler_t    mpRead;
        rm_handler_t    mpWrite;
    } rm_res_t;


    /*
bool_t KERNEL_CALL
rm_read_kbd(task_t* apTask, regs_t* apRegs);

bool_t KERNEL_CALL
rm_write_screen(task_t* apTask, regs_t* apRegs);


rm_res_t gspResourceList[RESOURCE_LIST_SIZE]    = {
        {   1,  1,  &rm_read_kbd,             NULL },
        {   1,  1,  NULL,                     rm_write_screen }
    };
    */

#define MAX_NUM_WAITING_TASKS 32
// can not use malloc here :-|
waiting_task_t gWaitingTasksList[MAX_NUM_WAITING_TASKS];
uint_t gNumWaitingTasks = 0;

void KERNEL_CALL
rm_install(void)
{
    semaphores_install();
    gNumWaitingTasks = 0;
}

void KERNEL_CALL
rm_start(void)
{
    int id = 0;
    while(1)
    {
        __asm__ __volatile__ ("cli");
        if(gNumWaitingTasks)
        {
            //int id = gNumWaitingTasks - 1;
            id %= gNumWaitingTasks;
            uint_t resource = gWaitingTasksList[id].resource;
            task_t* pTask = gWaitingTasksList[id].pTask;
            extern task_t* gpTopTask;
            if(resource == KEYBOARD_SEMAPHORE && pTask == gpTopTask)
            {
                if(semaphore_get(KEYBOARD_SEMAPHORE))
                {
                    semaphore_p(KEYBOARD_SEMAPHORE);
                    regs_t* pRegs = gWaitingTasksList[id].pRegs;
                    if (keyboard_had_input())
                    {
                        dword_t kernelPageDir = read_cr3();
                        dword_t taskPageDir = pTask->page_dir;

                        write_cr3(taskPageDir);

                        uint_t numbytes = pRegs->edx;
                        byte_t* pBuffer = (byte_t*) pRegs->ecx;
                        uint_t bytes_read = 0;

                        while (keyboard_had_input() && numbytes)
                        {
                            char c = keyboard_getchar();
                            numbytes--;
                            bytes_read++;
                            *pBuffer = c;
                            pBuffer++;
                        }

                        if (!numbytes)
                        {
                            pRegs->eax = bytes_read;
                            gNumWaitingTasks--;
                            if (gNumWaitingTasks != id)
                            {
                                memcpy(&gWaitingTasksList[id], &gWaitingTasksList[id + 1],
                                        (gNumWaitingTasks - id) * sizeof(waiting_task_t));
                            }
                            // BRAG("task %d unlocked\n", pTask->id);
                            pTask->locked = FALSE;
                        }
                        else
                        {
                            pRegs->ecx = (dword_t) pBuffer;
                        }
                        write_cr3(kernelPageDir);
                    }
                    semaphore_v(KEYBOARD_SEMAPHORE);
                }
            }

            ++id;
        }
        __asm__ __volatile__ ("sti");
    }
}

void KERNEL_CALL
rm_add_waiting_task(task_t* apTask, regs_t* apRegs, uint_t aResource)
{
    __asm__ __volatile__ ("cli");
    gWaitingTasksList[gNumWaitingTasks].pTask = apTask;
    gWaitingTasksList[gNumWaitingTasks].pRegs = apRegs;
    gWaitingTasksList[gNumWaitingTasks].resource = aResource;
    gNumWaitingTasks++;
    if(gNumWaitingTasks >= 32)
    {
        printf("Waiting tasks stack is full!");
        kernel_panic();
    }
}
