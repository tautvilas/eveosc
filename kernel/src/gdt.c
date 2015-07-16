#include "gdt.h"
#include "global.h"
#include "mem.h"
#include "stdio.h"

#define SEG_PRESENT     0x80
#define SEG_DPL_0       0
#define SEG_DPL_1       0x20
#define SEG_DPL_2       0x40
#define SEG_DPL_3       0x60
#define SEG_HAS_TYPE    16
#define SEG_TYPE_CODE   10
#define SEG_TYPE_DATA   2
#define SEG_TYPE_TSS    9

typedef struct {
    word_t limit_low;
    word_t base_low;
    byte_t base_middle;
    byte_t access;
    byte_t granularity;
    byte_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef enum {
    RESERVED,
    KERNEL_CODE_SEG,
    KERNEL_DATA_SEG,
    USER_CODE_SEG,
    USER_DATA_SEG,
    TSS_SEG
} segments_t;

extern void gGdt;

extern dword_t gGdtKernelDataSel;

extern void load_tr();

extern dword_t read_tr();

extern dword_t read_tss_limit();

extern void gKernelBase;

static gdt_entry_t* gpGdt = &gGdt;

tss_t gTss;

/* Setup a descriptor in the Global Descriptor Table */
    static void KERNEL_CALL
gdt_set_gate(uint_t aNum, dword_t aBase, dword_t aLimit, byte_t aAccess, byte_t aGran)
{
    /* Setup the descriptor base address */
    gpGdt[aNum].base_low = (aBase & 0xFFFF);
    gpGdt[aNum].base_middle = (aBase >> 16) & 0xFF;
    gpGdt[aNum].base_high = (aBase >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gpGdt[aNum].limit_low = (aLimit & 0xFFFF);
    gpGdt[aNum].granularity = ((aLimit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gpGdt[aNum].granularity |= (aGran & 0xF0);
    gpGdt[aNum].access = aAccess;
}

void KERNEL_CALL
tss_install()
{
    memset(&gTss, 0, sizeof(tss_t));
    gTss.ss0 = gGdtKernelDataSel;
    gdt_set_gate(TSS_SEG, (dword_t) &gTss, 150, SEG_PRESENT | SEG_DPL_0 | SEG_TYPE_TSS , 0xCF);
    load_tr();
    // printf("%x\n", read_tr());
    // dword_t tss_limit = read_tss_limit();
    // DUMP(tss_limit);
    return;
}
