#ifndef _GLOBAL_H_
#define _GLOBAL_H_

// compile time options
#define EVE_DEBUG
#define EVE_DEMO

// some useful macros
#define STOP                for(;;)
#ifdef EVE_DEBUG
#define DUMP(expr)          (printf(#expr ": "), printf("%x\n", (uint_t)(expr)))
#define MARK(expr)          (printf(#expr "\n"), (expr))
#define SEPARATE            (printf("---------\n"))
#else
#define DUMP(expr)
#define MARK(expr)
#define SEPARATE
#endif

#ifdef EVE_DEMO
#define BRAG(f, ...)        printf(f, ##__VA_ARGS__)
#else
#define BRAG(f, ...)
#endif

#define KERNEL_CALL         //__attribute__((cdecl))

#define NULL                0

#define MEGABYTE            1048576
#define GIGABYTE            (MEGABYTE * 1024)

typedef unsigned char       byte_t;     // unsigned 8bit integer
typedef unsigned short      word_t;     // unsigned 16bit integer
typedef unsigned int        dword_t;    // unsigned 32bit integer
typedef unsigned long long  qword_t;    // unsigned 64bit integer

typedef int                 int_t;      // signed pointer width integer
typedef unsigned int        uint_t;     // unsigned pointer width integer
typedef uint_t              size_t;     // unsigned pointer width integer
typedef void*               pointer_t;  // void pointer
typedef void                label_t;

typedef enum {
        FALSE,
        TRUE
    }   bool_t;

/**
  * This defines what the stack looks like after an ISR was running
  */
typedef struct
{
    dword_t gs, fs, es, ds;                          /* pushed the segs last */
    dword_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    dword_t int_no, err_code;                        /* push byteX and push ecode */
    dword_t eip, cs, eflags, useresp, ss;            /* pushed by the processor automatically */
} regs_t;

#endif  // _GLOBAL_H_
