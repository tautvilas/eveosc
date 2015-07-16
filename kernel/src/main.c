#include "main.h"

extern int gKernelEnd;

void KERNEL_CALL
os_main()
{
    vga_install();
    BRAG("Eve successfully switched to P-mode with paging enabled\n");
    put_logo();

    idt_install();
    BRAG("ISRs & IRQs are on-line\n");

    sys_call_table_install();
    BRAG("System call table installed\n");

    timer_install();
    BRAG("PIT firing rate is %d Hz\n", _TIMER_RATE);

    mm_install();
    BRAG("Memory manager is installed\n", _TIMER_RATE);
    //mm_print_info();

    /*{
        char* p;

        SEPARATE
        DBG_DUMP(p = sbrk(2));
        DBG_DUMP(sbrk(0));
        DBG_DUMP(*p);
        DBG_DUMP(p[1]);
        DBG_DUMP(p[4095]); // possable page fault
        //DBG_DUMP(p[4096]); // page fault for sure

        SEPARATE
        DBG_DUMP(p = sbrk(4 * MEGABYTE));
        DBG_DUMP(sbrk(0));
        DBG_DUMP(p[0]);
        DBG_DUMP(p[MEGABYTE]);
        DBG_DUMP(p[4 * MEGABYTE]); // possable page fault
        //DBG_DUMP(p[5 * MEGABYTE]); // page fault for sure

        SEPARATE
        DBG_DUMP(sbrk(-4 * MEGABYTE));
        DBG_DUMP(p = sbrk(0));
        DBG_DUMP(p[0]);     // possable page fault
        //DBG_DUMP(p[MEGABYTE]);  // page fault for sure

        DBG_DUMP(sbrk(-2));
        DBG_DUMP(p = sbrk(0));
        //DBG_DUMP(p[0]); // page fault for sure
    }*/

    /*{
        char* p;
        SEPARATE;
        DUMP(sbrk(0));
        DUMP(p = malloc(2));
        DUMP(sbrk(0));
        DUMP(*p);
        DUMP(p[1]);
        //DUMP(p[4095]); // possable page fault
        //DUMP(p[4096]); // page fault for sure

        char* q;
        SEPARATE;
        DUMP(q = malloc(4 * MEGABYTE));
        DUMP(sbrk(0));
        DUMP(q[0]);
        DUMP(q[MEGABYTE]);
        DUMP(q[4 * MEGABYTE]); // possable page fault
        //DUMP(p[5 * MEGABYTE]); // page fault for sure

        SEPARATE;
        MARK(free(q));
        DUMP(sbrk(0));
        //DUMP(q[0]);     // possable page fault
        //DUMP(p[MEGABYTE]);  // page fault for sure

        MARK(free(p));
        DUMP(sbrk(0));
        //DUMP(p[0]); // page fault for sure
    }*/

	keyboard_install();
    BRAG("Keyboard is on-line (US layout)\n");

    // prepare tss segment
    tss_install();
    //BRAG("TSS is loaded\n");
    // install resource manager before starting multitasking
    rm_install();
    // task manager
    task_ring_node_t* pKernelNode = tm_install();
    BRAG("Multitasking is enabled\n");
    //print_task_tree();

    // load shell app
    gEshTaskOffset = (dword_t)&gKernelEnd;
    tm_load_task((pointer_t) gEshTaskOffset, pKernelNode, ACC_USER, PRIOR_LOW, TRUE);

    extern dword_t gNextTaskOffset;
    gPingTaskOffset = gNextTaskOffset;

    /* from this point on Kernel process will serve as a resources manager */

    rm_start();

    for (;;);
}
