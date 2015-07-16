#ifndef _MALLOCDEFS_H_
#define _MALLOCDEFS_H_

#include "memmgr.h"
#include "global.h"
#include "idt.h"
#include "mem.h"
#include "errno.h"


#define ABORT               (BRAG("malloc ERROR!\n"), kernel_panic())
#define HAVE_MMAP           0
#define HAVE_MREMAP         0
#define MMAP_CLEARS         0
#define malloc_getpagesize  MM_PAGE_SIZE
#define LACKS_UNISTD_H
#define LACKS_FCNTL_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRINGS_H
#define LACKS_STRING_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#define LACKS_STDLIB_H
#define DEFAULT_TRIM_THRESHOLD  MEGABYTE

#define NEW(type)   ((type*)malloc(sizeof(type)))

#endif  // _MALLOCDEFS_H_
