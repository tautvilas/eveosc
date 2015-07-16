#ifndef _GLOBAL_H_
#define _GLOBAL_H_

// compile time options
#define EVE_DEBUG
#define EVE_DEMO

// some useful macros
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

//#define KERNEL_CALL         //__attribute__((cdecl))

//#define NULL                0

#define MEGABYTE            1048576
#define GIGABYTE            (MEGABYTE * 1024)

#include <stddef.h>

typedef unsigned char       byte_t;     // unsigned 8bit integer
typedef unsigned short      word_t;     // unsigned 16bit integer
typedef unsigned int        dword_t;    // unsigned 32bit integer
typedef unsigned long long  qword_t;    // unsigned 64bit integer

typedef int                 int_t;      // signed pointer width integer
typedef unsigned int        uint_t;     // unsigned pointer width integer
//typedef uint_t              size_t;     // unsigned pointer width integer
typedef void*               pointer_t;  // void pointer

typedef enum {
        FALSE,
        TRUE
    }   bool_t;

#endif  // _GLOBAL_H_
