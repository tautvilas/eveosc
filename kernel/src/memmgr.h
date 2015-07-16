#ifndef _MEMMGR_H_
#define _MEMMGR_H_

#include "global.h"

/**
 *  Page size in bytes
 */
#define MM_PAGE_SIZE        4096

/**
 *  Values to build access_t value by or'ing
 */
#define ACC_SUPER           0
#define ACC_USER            4   // 0100b
#define ACC_READ            0
#define ACC_RW              2   // 0010b

typedef uint_t              mm_access_t;

typedef struct {
    dword_t start;          // task must be loaded at this point in memory
    dword_t entry;
    size_t header_size;
    size_t text_size;
    size_t data_size;
    size_t bss_size;
} mm_task_mem_t;

/**
 *  Installs memory manager.
 */
void KERNEL_CALL
mm_install();


pointer_t KERNEL_CALL
sbrk(int aIncrement);


int KERNEL_CALL
brk(pointer_t pEnd);

pointer_t KERNEL_CALL
mm_alloc_page();

uint_t KERNEL_CALL
mm_alloc_task(const mm_task_mem_t* apMem, const pointer_t apOffset, mm_access_t aAccess);

void KERNEL_CALL
mm_free_page_dir(uint_t apTaskPageDir);

/**
 *  Returns number of free pages.
 *
 *  @return Number of free pages.
 */
size_t KERNEL_CALL
mm_get_free_pages();


/**
 *  Prints memory manager info.
 */
void KERNEL_CALL
mm_print_info();


#endif  // _MEMMGR_H_
