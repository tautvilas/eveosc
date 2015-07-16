#ifndef _GDT_H_
#define _GDT_H_

#include "global.h"

typedef struct {
    word_t	backlink, __blh;
    dword_t	esp0;
    word_t	ss0, __ss0h;
    dword_t	esp1;
    word_t	ss1, __ss1h;
    dword_t	esp2;
    word_t	ss2, __ss2h;
    dword_t	cr3;
    dword_t	eip;
    dword_t	eflags;
    dword_t	eax, ecx, edx, ebx;
    dword_t	esp, ebp, esi, edi;
    word_t	es, __esh;
    word_t	cs, __csh;
    word_t	ss, __ssh;
    word_t	ds, __dsh;
    word_t	fs, __fsh;
    word_t	gs, __gsh;
    word_t	ldt, __ldth;
    word_t	trace, bitmap;
} __attribute__ ((packed)) tss_t;

extern void KERNEL_CALL
tss_install();

extern tss_t gTss;

#endif // _GDT_H_
