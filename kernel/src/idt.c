#include "mem.h"
#include "idt.h"
#include "stdio.h"
#include "ports.h"
#include "syscalls.h"

#define NUM_ISRS            32

extern dword_t gGdtKernelCsSel;
extern dword_t gGdtUserCsSel;
extern void idt_load(); /* assembler fuction with 'lidt' instruction */

#define KERNEL_CODE_SEGMENT gGdtKernelCsSel
// #define IDT_FLAGS           0x8E /* entry is present, ring 0 */
#define IDT_FLAGS           0xEE /* entry is present, ring 3 */

#define PIC1                0x20
#define PIC2                0xA0
/* PIC command ports */
#define PIC1_COMMAND        PIC1
#define PIC2_COMMAND        PIC2
/* PIC data ports */
#define PIC1_DATA           (PIC1 + 1)
#define PIC2_DATA           (PIC2 + 1)
/* End of interrupt signal */
#define PIC_EOI             0x20

/* number of system calls */
#define NUM_SYS_CALLS       20

/* ISR entries, see kentry.asm for more info */

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr69();

/* IRQ entries, see kentry.asm for more info */

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/* the structure of IDT entry */

typedef struct
{
    word_t base_lo;     /* ISR address low word */
    word_t sel;         /* kernel segment */
    byte_t always0;     /* always set this to 0 */
    byte_t flags;       /* 1 - present?1:0; 2 - ring; 01110 in the end */
    word_t base_hi;     /* ISR address high word */
} __attribute__ ((packed)) idt_entry_t;

/* special pointer to IDT, mentioning its size */

typedef struct
{
    word_t limit;
    dword_t base;
} __attribute__ ((packed)) idt_ptr_t;

idt_ptr_t gIdtp;    /* non static for asm to reach */

/* pointers to isr handling functions */
static void *gpIsrRoutines[70] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* pointers to kernel services handling functions */
static void *gpSysCallRoutines[NUM_SYS_CALLS] =
{
    0, sys_kill, 0, sys_read, sys_write, 0, 0, 0, 0, 0,
    0, sys_exec, 0, 0, 0, 0, 0, 0, 0, 0
};

static idt_entry_t gIdt[256];
/* ISR debug messages */
static  char* gpExceptionMessages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable interrupt",
    "Breakpoint",
    "Into  Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Aligment Check",
    "Machine Check",
    "Reserved 19",
    "Reserved 20",
    "Reserved 21",
    "Reserved 22",
    "Reserved 23",
    "Reserved 24",
    "Reserved 25",
    "Reserved 26",
    "Reserved 27",
    "Reserved 28",
    "Reserved 29",
    "Reserved 30",
    "Reserved 31"
};

/**
 *  Sets one IDT gate
 *
 *  @param  aNum   gate number
 *  @param  aBase  ISR base address
 *  @param  aSel   kernel segment
 *  @param  aFlags IDT gate flags
 */

static void KERNEL_CALL
idt_set_gate(byte_t aNum, dword_t aBase, byte_t aSel, byte_t aFlags)
{
    gIdt[aNum].base_lo = aBase & 0xFFFF;
    gIdt[aNum].base_hi = (aBase >> 16) & 0xFFFF;
    gIdt[aNum].sel = aSel;
    gIdt[aNum].flags = aFlags;
    gIdt[aNum].always0 = 0x00;
    return;
}

void KERNEL_CALL
isr_install_handler(int aIsr, void (*handler)(regs_t *apR))
{
    gpIsrRoutines[aIsr] = handler;
}

void KERNEL_CALL
irq_install_handler(int aIrq, void (*handler)(regs_t *apR))
{
    gpIsrRoutines[aIrq + 32] = handler;
}

void KERNEL_CALL
irq_uninstall_handler(int aIrq)
{
    gpIsrRoutines[aIrq + 32] = 0;
}

void KERNEL_CALL
isr_uninstall_handler(int aIsr)
{
    gpIsrRoutines[aIsr] = 0;
}

/* IRQs 0-7 are by default mapped to entries 8-15
   This conflicts with ISRs so we should remap all IRQs to 32-47 */
static void KERNEL_CALL
irq_remap(void)
{
    /* start sequence */
    outportb(PIC1_COMMAND, 0x11);
    outportb(PIC2_COMMAND, 0x11);
    /* vectors */
    outportb(PIC1_DATA, 0x20);
    outportb(PIC2_DATA, 0x28);
    /* continue initialization */
    outportb(PIC1_DATA, 0x04);
    outportb(PIC2_DATA, 0x02);
    /* processing mode */
    outportb(PIC1_DATA, 0x01);
    outportb(PIC2_DATA, 0x01);
    /* end */
    outportb(PIC1_DATA, 0x0);
    outportb(PIC2_DATA, 0x0);
}

void KERNEL_CALL
idt_install()
{
    gIdtp.limit = sizeof(idt_entry_t) * 256 - 1;
    gIdtp.base = (dword_t) &gIdt;

    memset((byte_t *) &gIdt, 0x00, sizeof(idt_entry_t) * 256);

    /* ISRs */
    idt_set_gate(0, (dword_t)isr0, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(1, (dword_t)isr1, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(2, (dword_t)isr2, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(3, (dword_t)isr3, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(4, (dword_t)isr4, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(5, (dword_t)isr5, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(6, (dword_t)isr6, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(7, (dword_t)isr7, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(8, (dword_t)isr8, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(9, (dword_t)isr9, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(10, (dword_t)isr10, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(11, (dword_t)isr11, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(12, (dword_t)isr12, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(13, (dword_t)isr13, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(14, (dword_t)isr14, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(15, (dword_t)isr15, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(16, (dword_t)isr16, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(17, (dword_t)isr17, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(18, (dword_t)isr18, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(19, (dword_t)isr19, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(20, (dword_t)isr20, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(21, (dword_t)isr21, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(22, (dword_t)isr22, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(23, (dword_t)isr23, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(24, (dword_t)isr24, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(25, (dword_t)isr25, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(26, (dword_t)isr26, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(27, (dword_t)isr27, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(28, (dword_t)isr28, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(29, (dword_t)isr29, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(30, (dword_t)isr30, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(31, (dword_t)isr31, KERNEL_CODE_SEGMENT, IDT_FLAGS);

    idt_set_gate(69, (dword_t)isr69, KERNEL_CODE_SEGMENT, IDT_FLAGS);

    /* IRQs */

    irq_remap();

    idt_set_gate(32, (dword_t)irq0, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(33, (dword_t)irq1, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(34, (dword_t)irq2, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(35, (dword_t)irq3, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(36, (dword_t)irq4, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(37, (dword_t)irq5, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(38, (dword_t)irq6, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(39, (dword_t)irq7, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(40, (dword_t)irq8, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(41, (dword_t)irq9, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(42, (dword_t)irq10, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(43, (dword_t)irq11, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(44, (dword_t)irq12, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(45, (dword_t)irq13, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(46, (dword_t)irq14, KERNEL_CODE_SEGMENT, IDT_FLAGS);
    idt_set_gate(47, (dword_t)irq15, KERNEL_CODE_SEGMENT, IDT_FLAGS);

    idt_load();

    __asm__ __volatile__ ("sti");

    return;
}

void KERNEL_CALL
sys_call_handler(regs_t * apRegs) {
    dword_t sys_call_id = apRegs->eax;
    if (sys_call_id < NUM_SYS_CALLS)
    {
        void (*handler)(regs_t *r);
        handler = gpSysCallRoutines[sys_call_id];
        if (handler)
        {
            handler(apRegs);
            return;
        }
    }
    printf("No such kernel service - %d\n", sys_call_id);
    return;
}

/** Install system call (int 69) handler **/

void KERNEL_CALL
sys_call_table_install() {
    isr_install_handler(69, sys_call_handler);
}

/* Kernel panic function, dumps system registers and halts */

void KERNEL_CALL
kernel_panic()
{
        regs_t regs;
        __asm__ __volatile__ ("movl %%eax, %0\n" :"=r" (regs.eax));
        __asm__ __volatile__ ("movl %%ebx, %0\n" :"=r" (regs.ebx));
        __asm__ __volatile__ ("movl %%ecx, %0\n" :"=r" (regs.ecx));
        __asm__ __volatile__ ("movl %%edx, %0\n" :"=r" (regs.edx));
        __asm__ __volatile__ ("movl %%esp, %0\n" :"=r" (regs.esp));
        __asm__ __volatile__ ("movl %%ebp, %0\n" :"=r" (regs.ebp));
        __asm__ __volatile__ ("movl %%edi, %0\n" :"=r" (regs.edi));
        __asm__ __volatile__ ("movl %%esi, %0\n" :"=r" (regs.esi));
        __asm__ __volatile__ ("movl %%ds, %0\n" :"=r" (regs.ds));
        __asm__ __volatile__ ("movl %%es, %0\n" :"=r" (regs.es));
        __asm__ __volatile__ ("movl %%fs, %0\n" :"=r" (regs.fs));
        __asm__ __volatile__ ("movl %%gs, %0\n" :"=r" (regs.gs));
        __asm__ __volatile__ ("movl %%ss, %0\n" :"=r" (regs.ss));
        __asm__ __volatile__ ("movl %%cs, %0\n" :"=r" (regs.cs));
        printf("eax = %x\t ebx = %x\t ecx = %x\t edx = %x\n", regs.eax, regs.ebx, regs.ecx, regs.edx);
        printf("esp = %x\t ebp = %x\t edi = %x\t esi = %x\n", regs.esp, regs.ebp, regs.edi, regs.esi);
        printf("ds = %x\t es = %x\t fs = %x\t gs = %x\n", regs.ds, regs.es, regs.fs, regs.gs);
        printf("cs = %x\t ss = %x\n", regs.cs, regs.ss);
        printf("Kernel panic!");
        for(;;);
}

/**
 *  Manages the exception after ISRs have been called
 *
 *  @param  apRegs pointer to a struct containing all register info
 */
void KERNEL_CALL
exception_handler(regs_t * apRegs)
{
    //BRAG("exception_handler\n");
    /* This is a blank function pointer */
    void (*handler)(regs_t *r);

    uint_t int_no = apRegs->int_no;
    //DUMP(int_no);
    handler = gpIsrRoutines[int_no];
    if ((int_no < 32 || int_no == 69) && handler)
    {
        handler(apRegs);
    }
    else if (int_no < 32)
    {
        printf(gpExceptionMessages[apRegs->int_no]);
        printf(" Exception caught\n");
        kernel_panic();
    }
    else
    {
        printf("Unhandled interrupt caught - %d\n", apRegs->int_no);
    }
    return;
}

/**
 *  Manages the IRQs
 *
 *  @param  apRegs pointer to a struct containing all register info
 */
void KERNEL_CALL
irq_handler(regs_t * apRegs)
{
    /* This is a blank function pointer */
    void (*handler)(regs_t *r);

    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = gpIsrRoutines[apRegs->int_no];
    if (handler)
    {
        handler(apRegs);
    }

    //if(apRegs->int_no != 32)
        //printf("We've got contact - %d\n", apRegs->int_no);

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (apRegs->int_no >= 40)
    {
        outportb(PIC2_COMMAND, PIC_EOI);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outportb(PIC1_COMMAND, PIC_EOI);
}
