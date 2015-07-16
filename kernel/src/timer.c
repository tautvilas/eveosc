#include "timer.h"
#include "stdio.h"
#include "idt.h"
#include "ports.h"
#include "vga.h"
#include "tskmgr.h"

#define PIT_COMMAND  0x43
#define PIT_CHANNEL1 0x40 /* IRQ0 clock */
#define PIT_CHANNEL2 0x41 /* For system */
#define PIT_CHANNEL3 0x42 /* Beep */
#define PIT_RATE     1193182L /* Hz if divided by 1 (default rate = 18.222Hz) */

extern dword_t gKernelEsp;

extern dword_t gKernelCr3;

extern void write_cr3();

void KERNEL_CALL
timer_schedule(uint_t aSwapTaskNow)
{
    if (gpActiveTask->timetorun && !aSwapTaskNow) gpActiveTask->timetorun--;
    else
    {
        //int id = gpActiveTask->id;
        do
        {
            gpActiveTask->timetorun = gpPriorityTimes[gpActiveTask->priority];
            gpActiveTaskRingNode = gpActiveTaskRingNode->pNext;
            gpActiveTask = gpActiveTaskRingNode->pTask;
            // DUMP(gpActiveTaskRingNode);
            // DUMP(gpActiveTaskRingNode->pTask);
            // DUMP(gpActiveTaskRingNode->pNext);
            // DUMP(gpActiveTaskRingNode->pPrev);
            //printf("Switched to task %x (parent =  %d, id = %d)\n", gpActiveTask, gpActiveTask->parent, gpActiveTask->id);
        } while(gpActiveTask->locked == TRUE);
        /*
        if (id != gpActiveTask->id)
        {
            BRAG("switch to %d\n", gpActiveTask->id);
        }
        */
    }
}

/**
  * PIT IRQ0 handler
  *
  * @param apRegs register information
  */

unsigned static int gTimerTicks = 0U;

static void KERNEL_CALL
timer_handler(regs_t * apRegs)
{
    gTimerTicks++;

    vga_pos_t cursor = vga_get_cursor_pos();
    color_t bg = vga_get_bg_color();
    color_t fg = vga_get_fg_color();

    if(gTimerTicks % _TIMER_RATE == 0)
    {
        vga_set_fg_color(VGA_CL_BLACK);
        vga_set_bg_color(VGA_CL_WHITE);

        int numSecs = gTimerTicks / _TIMER_RATE;
        int length = 0;
        while(numSecs)
        {
            numSecs /= 10;
            length++;
        }
        vga_set_cursor_pos(71 - length, 0); /* TODO magic number to constant */
        printf("Uptime: %ds", gTimerTicks / _TIMER_RATE);
    }

    vga_set_cursor_pos(cursor.mX, cursor.mY);
    vga_set_bg_color(bg);
    vga_set_fg_color(fg);

    //if(gpActiveTask != NULL && gTimerTicks % 1000 == 0)
    //{

        //printf("Active task %x (parent =  %d, id = %d)\n", gpActiveTask, gpActiveTask->parent, gpActiveTask->id);
        // gpActiveTaskRingNode = gpActiveTaskRingNode;
        // DUMP(gpActiveTaskRingNode);
        // DUMP(gpActiveTaskRingNode->pTask);
        // DUMP(gpActiveTaskRingNode->pNext);
        // DUMP(gpActiveTaskRingNode->pPrev);
    //}
    if(gpActiveTask != NULL)
    {
        write_cr3(gKernelCr3);
        timer_schedule(0);
    }
    return;
}

/* set how many timer per second irq0 will fire */

static void KERNEL_CALL
timer_set_rate(int aHz)
{
    int divisor = PIT_RATE / aHz;            /* Calculate our divisor */
    outportb(PIT_COMMAND, 0x36);             /* Set our command byte 0x36 */
    outportb(PIT_CHANNEL1, divisor & 0xFF);  /* Set low byte of divisor */
    outportb(PIT_CHANNEL1, divisor >> 8);    /* Set high byte of divisor */
}

void KERNEL_CALL
timer_install()
{
    timer_set_rate(_TIMER_RATE);
    irq_install_handler(0, timer_handler);
}

unsigned int KERNEL_CALL
timer_get_num_ticks()
{
    return gTimerTicks;
}
