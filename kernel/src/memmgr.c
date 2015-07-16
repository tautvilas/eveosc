#include "memmgr.h"
#include "stdio.h"
#include "idt.h"
#include "mem.h"

/**
 *  Mask to zero-out unused bits from address
 */
#define MM_PAGE_ADDR_MASK   0xFFFFF000

/**
 *  Number of bits to right-shift page number to get page address
 */
#define MM_PAGE_ADDR_SHIFT  12


/**
 *  Bit mask to mask out bits not used by access_t by and'ing
 */
#define ACC_MASK            6   // 0110b

/**
 *  Value to set page table/directory entry as present
 */
#define ENTRY_PRESENT       1


/**
 *  Bit in CR0 to enable paging
 */
#define MM_ENABLE_PAGING    0x80000000


#define MM_PAGE_TBL_SIZE    1024
#define MM_PAGE_DIR_SIZE    1024


#define MM_SYSTEM_LO_MEM    MEGABYTE


typedef pointer_t*          mm_page_tbl_t;
typedef mm_page_tbl_t*      mm_page_dir_t;


// Symbols from linker. Does not really matter the type of them as the
// address of them is what matters.
extern void                 gKernelStart;
extern void                 gKernelEnd;
extern void                 gKernelBase;


extern void write_cr0(dword_t);
extern void write_cr3(dword_t);
extern dword_t read_cr0();
extern dword_t read_cr3();
extern dword_t read_cr2();


/**
 *  RAM size in bytes
 *  TODO: gx 2007-05-15: load at run time
 */
static size_t       gsRamSize            = MEGABYTE * 8; // 8mb


static pointer_t*   gspFreePageStack;
static pointer_t*   gspFreePageStackTop;
static size_t       gsTotalPageCount;


/**
 *  Page directory.
 *
 *  Don't really know if we actually need this (gx)
 */
static mm_page_dir_t    gpPageDirectory;
static size_t           gsKernelVmSize;



pointer_t KERNEL_CALL
mm_page_to_pointer(size_t page);

bool_t KERNEL_CALL
mm_is_page_free(pointer_t aPage);

pointer_t KERNEL_CALL
mm_containing_page(const pointer_t aPointer);

size_t KERNEL_CALL
mm_install_paging(size_t aPagesPresent);

pointer_t KERNEL_CALL
mm_alloc_page();

void KERNEL_CALL
mm_free_page(const pointer_t aPage);



void KERNEL_CALL
mm_install()
{
    size_t      used_pages  = 0;
    size_t      stack_size;
    pointer_t   page_addr;          // used in loop
    pointer_t   first_free_addr;
    size_t      kernel_end  = (size_t)&gKernelEnd - (size_t)&gKernelBase;

    //printf("kernel end: %x\n", kernel_end);

    // assuming that all memory before kernel and kernel itself is used
    if (kernel_end > MM_SYSTEM_LO_MEM)
    {
        used_pages  = kernel_end >> MM_PAGE_ADDR_SHIFT;
        if (kernel_end & ~MM_PAGE_ADDR_MASK)
            used_pages  += 1;
    }
    else
    {
        used_pages  = MM_SYSTEM_LO_MEM / MM_PAGE_SIZE;
    }

    // putting free pages stack jus after kernel
    gspFreePageStack    = (pointer_t)(
            (size_t)mm_page_to_pointer(used_pages) + (size_t)&gKernelBase
        );
    //DUMP(gspFreePageStack);
    gspFreePageStackTop = gspFreePageStack;

    // calculating total number of pages that can fit in RAM
    // NOT using last bytes of RAM if its not a full page.
    // emm.. this is a geeky way to write (gsRamSize / MM_PAGE_SIZE) but
    // should be a little faster then that. Of course it's worth nothing
    // as this code is executed only once when kernel starts but...
    // let it be... ;-)
    gsTotalPageCount    = gsRamSize >> MM_PAGE_ADDR_SHIFT;

    // calculating space needed for free pages stack
    stack_size      = gsTotalPageCount * sizeof(pointer_t);
    //used_pages      += stack_size >> MM_PAGE_ADDR_SHIFT;
    used_pages      += stack_size / MM_PAGE_SIZE;
    //if (stack_size & ~MM_PAGE_ADDR_MASK)
    if (stack_size % MM_PAGE_SIZE)
        used_pages  += 1;

    // should we optimize this loop? (1GB is 262144 pages)
    first_free_addr = mm_page_to_pointer(used_pages);
    for (
            page_addr   = mm_page_to_pointer(gsTotalPageCount - 1);
            page_addr >= first_free_addr;
            page_addr -= MM_PAGE_SIZE
        )
    {
        mm_free_page(page_addr);
    }
    // done with memory management

    //mm_load_kernel_process();

    used_pages      = mm_install_paging(used_pages);
    gsKernelVmSize  = (size_t)&gKernelBase + used_pages * MM_PAGE_SIZE;
}


static void KERNEL_CALL
page_fault_handler(regs_t * apRegs)
{
    dword_t invalid_address = read_cr2();
    printf("\nPage fault exception caught at %x\n", invalid_address);
    kernel_panic();
    return;
}


size_t KERNEL_CALL
mm_install_paging(size_t aPagesPresent)
{
    size_t  dir;
    size_t  entry;
    size_t  page            = 0;
    size_t  pages_present   = aPagesPresent;

    const size_t ENTRY_FLAGS    = ACC_SUPER | ACC_RW | ENTRY_PRESENT;

    //page_table_t*   pPageTable;
    mm_page_tbl_t   page_table;

    // assuming that page directory is allocated immediately after
    // already used memory
    gpPageDirectory = (mm_page_dir_t)memsetd(mm_alloc_page(), 0, MM_PAGE_DIR_SIZE);
    pages_present   += 1;

    for (
            dir = (size_t)&gKernelBase / MEGABYTE / 4;
            page < pages_present;
            ++dir
        )
    {
        // assuming that page table is allocated immediately after
        // already used memory
        page_table      = mm_alloc_page();
        pages_present   += 1;

        // filling page table entries
        for (
                entry = 0;
                page < pages_present && entry < MM_PAGE_TBL_SIZE;
                ++entry, ++page
            )
        {
            page_table[entry]    = (pointer_t)(
                    (size_t)mm_page_to_pointer(page) | ENTRY_FLAGS
                );

        }

        for (; entry < MM_PAGE_TBL_SIZE; ++entry)
        {
            page_table[entry]   = NULL;
        }

        // adding page table to page directory
        gpPageDirectory[dir]  = (pointer_t)(
                (size_t)page_table | ENTRY_FLAGS
            );
    }

    gpPageDirectory[MM_PAGE_DIR_SIZE - 1]   = (pointer_t)(
            (size_t)gpPageDirectory | ENTRY_FLAGS
        );


    write_cr3((size_t)gpPageDirectory);   // put that page directory address into CR3
    gpPageDirectory = (pointer_t)((size_t)gpPageDirectory + (size_t)&gKernelBase);

    //install the page fault handler

    isr_install_handler(ISR_PAGE_FAULT, page_fault_handler);

    return pages_present;
}



/**
 *  Frees specified page.
 *
 *  Currently if specified memory address is not valid page address page that
 *  containts specified address is freed.
 *
 *  @param  aPage   Pointer of page to be freed.
 */
void KERNEL_CALL
mm_free_page(const pointer_t apPage)
{
    // NOTE: gx 2007-05-13: should memory be freed if page address is not valid?
    pointer_t page  = mm_containing_page(apPage);

    if (mm_is_page_free(page))
        return;

    *gspFreePageStackTop = page;
    gspFreePageStackTop++;

}

/**
 *  Allocates a page of memory.
 *
 *  @return Pointer to memory page.
 */
pointer_t KERNEL_CALL
mm_alloc_page()
{
    if (gspFreePageStackTop == gspFreePageStack)
    {
        BRAG("Huston. We have a problem. We are out of memory!\n");
        return NULL;
    }

    gspFreePageStackTop--;
    return *gspFreePageStackTop;
}


mm_page_dir_t KERNEL_CALL
mm_page_dir_addr()
{
    // last entry in page directory points to it self
    // this means we can use highest page below 4gb mark to access it
    return (mm_page_dir_t)((size_t)-1 - MM_PAGE_SIZE + 1);
}


mm_page_dir_t KERNEL_CALL
mm_page_dir_phys_addr()
{
    pointer_t*  pDirAddr    = (pointer_t*)((size_t)-1 - sizeof(pointer_t) + 1);
    return (mm_page_dir_t)((size_t)*pDirAddr & MM_PAGE_ADDR_MASK);
}


mm_page_tbl_t KERNEL_CALL
mm_page_tbl_addr(size_t aIndex)
{
    return (mm_page_tbl_t)(
            (size_t)-1
            - MM_PAGE_TBL_SIZE * MM_PAGE_SIZE + 1
            + aIndex * MM_PAGE_SIZE
        );
}


mm_page_tbl_t KERNEL_CALL
mm_page_tbl_phys_addr(size_t aIndex)
{
    mm_page_dir_t   pPageDir    = mm_page_dir_addr();
    return (mm_page_tbl_t)((size_t)pPageDir[aIndex] & MM_PAGE_ADDR_MASK);
}


size_t KERNEL_CALL
mm_paging_free_pages(size_t aIndex, size_t aCount)
{
    // frees backwards e.g. inf aIndex is 5 and aCount is 2 then pages
    // 5, 4 are freed
    size_t  tbl_i;
    size_t  page_i;
    size_t  count;
    mm_page_dir_t   pPageDir;
    mm_page_tbl_t   pTbl;

    BRAG("Freeing %d pages\n", aCount);

    tbl_i   = aIndex / MM_PAGE_TBL_SIZE;
    if (0 == aIndex % MM_PAGE_TBL_SIZE)
        tbl_i--;

    page_i  = aIndex % MM_PAGE_TBL_SIZE;
    if (0 == page_i)
        page_i  = MM_PAGE_TBL_SIZE - 1;

    pPageDir    = mm_page_dir_addr();
    pTbl        = mm_page_tbl_addr(tbl_i); //(mm_page_tbl_t)((size_t)pPageDir[tbl_i] & MM_PAGE_ADDR_MASK);

    for (count = 0; count < aCount; ++count)
    {
        mm_free_page((pointer_t)((size_t)pTbl[page_i] & MM_PAGE_ADDR_MASK));

        if (0 == page_i)
        {
            mm_free_page(mm_page_tbl_phys_addr(tbl_i));
            pPageDir[tbl_i] = NULL;
            tbl_i--;
            page_i  = MM_PAGE_TBL_SIZE - 1;
            pTbl    = mm_page_tbl_addr(tbl_i); //(mm_page_tbl_t)((size_t)pPageDir[tbl_i] & MM_PAGE_ADDR_MASK);
        }
        else
        {
            pTbl[page_i]    = NULL;
            page_i--;
        }
    }

    write_cr3((size_t)mm_page_dir_phys_addr());

    BRAG("\tdone");
    return count;
}


mm_page_tbl_t KERNEL_CALL
mm_paging_put_tbl(size_t aIndex, const pointer_t apTable, mm_access_t aAccess)
{
    mm_page_dir_addr()[aIndex] = (mm_page_tbl_t)(
            (size_t)apTable | (aAccess & ACC_MASK) | ENTRY_PRESENT
        );
    return mm_page_tbl_addr(aIndex);
}


mm_page_tbl_t KERNEL_CALL
mm_paging_alloc_tbl(size_t aIndex, mm_access_t aAccess)
{
    pointer_t   pPage   = mm_alloc_page();
    if (NULL == pPage)
    {
        return NULL;
    }
    else
    {
        mm_page_tbl_t   pTbl    = mm_paging_put_tbl(aIndex, pPage, aAccess);
        memsetd((dword_t*)pTbl, MM_PAGE_TBL_SIZE, 0);
        return pTbl;
    }
}


size_t KERNEL_CALL
mm_paging_alloc_pages(size_t aIndex, size_t aCount, mm_access_t aAccess)
{
    size_t  tbl_c;
    size_t  page_c;
    size_t  count;
    mm_page_dir_t   pPageDir;
    mm_page_tbl_t   pTbl;

    const size_t    ENTRY_FLAGS = (aAccess & ACC_MASK) | ENTRY_PRESENT;

    BRAG("Allocating %d pages.. ", aCount);
    if (0 == aCount)
    {
        BRAG("\tnothing to do\n");
        return 0;
    }

    if (aCount > mm_get_free_pages())
    {
        BRAG("\taborting only %d pages free\n", mm_get_free_pages());
        return 0;
    }

    tbl_c   = aIndex / MM_PAGE_TBL_SIZE;
    if (aIndex % MM_PAGE_TBL_SIZE)
        tbl_c++;

    page_c  = aIndex % MM_PAGE_TBL_SIZE;

    pPageDir    = mm_page_dir_addr();
    if ((0 != page_c) && FALSE == ((uint_t)pPageDir[tbl_c - 1] & ENTRY_PRESENT))
    {
        pTbl    = mm_paging_alloc_tbl(tbl_c - 1, aAccess);
        if (NULL == pTbl)
            return 0;
    }
    else
    {
        pTbl    = mm_page_tbl_addr(tbl_c - 1);
    }

    for (count = 0; count < aCount; ++count)
    {
        pointer_t       pPage;

        pPage   = mm_alloc_page();
        if (NULL == pPage)
        {
            mm_paging_free_pages(aIndex + count, count);
            return 0;
        }

        if (0 == page_c)
        {
            pTbl    = mm_paging_alloc_tbl(tbl_c, aAccess);
            if (NULL == pTbl)
            {
                mm_paging_free_pages(aIndex + count, count);
                return 0;
            }
            tbl_c++;
        }

        pTbl[page_c]    = (pointer_t)((size_t)pPage | ENTRY_FLAGS);
        // TODO: gx 2007-05-24: would be nice to fill new memory with zeros but
        //      we can not access it beacause page dir is not up to date
        page_c          = (page_c + 1) % MM_PAGE_TBL_SIZE;
    }

    write_cr3((size_t)mm_page_dir_phys_addr());

    BRAG("\tdone\n");
    return count;
}


int KERNEL_CALL
brk(pointer_t pEnd)
{
    // :TODO: gx 2007-05-24: adapt to work not only for kernel memory
    pointer_t   pCurrEnd    = (pointer_t)gsKernelVmSize;
    pointer_t   pCurrPageEnd;
    pointer_t   pCurrPageStart;

    // if address exceeds allowed range, return failure
    // :TODO: gx 2007-05-23:
    //      1) get values from somewhere reasonable
    //      2) set errno on error
    if ((pEnd < (pointer_t)&gKernelEnd) || (pEnd > (pointer_t)(3U * GIGABYTE)))
        return -1;

    // callculating the start and end addresses of the last actually allocated
    // memory page
    pCurrPageStart  = (pointer_t)((size_t)pCurrEnd & MM_PAGE_ADDR_MASK);
    pCurrPageEnd    = pCurrPageStart;
    if ((size_t)pCurrEnd & ~MM_PAGE_ADDR_MASK)
        pCurrPageEnd    += MM_PAGE_SIZE;
    else
        pCurrPageStart  -= MM_PAGE_SIZE;


    // check if pEnd fits in current page
    if (pEnd > pCurrPageStart && pEnd <= pCurrPageEnd)
    {
        gsKernelVmSize  = (size_t)pEnd;
        return 0;
    }

    if (pEnd > pCurrPageEnd)
    {
        // allocate needed pages
        size_t  allocated;
        size_t  delta   = pEnd - pCurrPageEnd;
        size_t  count   = delta / MM_PAGE_SIZE;
        if (delta % MM_PAGE_SIZE)
            count++;
        allocated   = mm_paging_alloc_pages(
                (size_t)pCurrPageEnd / MM_PAGE_SIZE,
                count,
                ACC_RW | ACC_SUPER
            );
        if (allocated < count)
            return -1;
    }
    else if (pEnd <= pCurrPageStart)
    {
        mm_paging_free_pages(
                (size_t)pCurrPageStart / MM_PAGE_SIZE,
                (pCurrPageStart - pEnd) / MM_PAGE_SIZE + 1
            );
    }

    gsKernelVmSize  = (size_t)pEnd;

    return 0;
}


pointer_t KERNEL_CALL
sbrk(int aIncrement)
{
    pointer_t   addr    = (pointer_t)gsKernelVmSize;

    if (aIncrement == 0)
        return addr;

    if (0 == brk((pointer_t)(gsKernelVmSize + aIncrement)))
        return addr;

    return (pointer_t)-1;
}


/**
 *  Returns number of free pages.
 *
 *  @return Number of free pages.
 */
size_t KERNEL_CALL
mm_get_free_pages()
{
    return gspFreePageStackTop - gspFreePageStack;
}


void KERNEL_CALL
mm_print_info()
{
    size_t  free_pages  = mm_get_free_pages();

    printf("Memory manager\n");
    printf("\taddress:  \t%x\n", gspFreePageStack);
    printf("\tsize:     \t%d bytes\n", gsTotalPageCount * sizeof(pointer_t));
    printf("\tfree:     \t%d pages (%d bytes)\n", free_pages,
            free_pages * MM_PAGE_SIZE);
}


/**
 *  Converts page number to page address in memory.
 *
 *  @param  aPage   Page number.
 *  @return Pointer to specified page.
 */
pointer_t KERNEL_CALL
mm_page_to_pointer(size_t aPage)
{
    return (pointer_t)(aPage << MM_PAGE_ADDR_SHIFT);
}


/**
 *  Returns pointer to page containing specified pointer.
 *
 *  @param  aPointer    Pointer in memory.
 *  @return Pointer to page containing specified pointer.
 */
pointer_t KERNEL_CALL
mm_containing_page(const pointer_t aPointer)
{
    return (pointer_t)((size_t)aPointer & MM_PAGE_ADDR_MASK);
}


/**
 *  Checks if page is free.
 *
 *  @param  aPage Pointer to page.
 *  @return TRUE if page is free, FALSE otherwise.
 */
bool_t KERNEL_CALL
mm_is_page_free(const pointer_t aPage)
{
    // TODO: gx 2007-05-13: optimize this! (may be using bitmap of free pages)
    pointer_t* page;
    for (page = gspFreePageStackTop - 1; page >= gspFreePageStack; --page)
    {
        if (aPage == *page)
            return TRUE;
    }
    return FALSE;
}

mm_page_dir_t KERNEL_CALL
mm_duplicate_page_dir(void)
{
    const pointer_t TMP_PAGE_FRAME = (pointer_t)((size_t)-1 - 2 * MM_PAGE_SIZE + 1);

    mm_page_dir_t pKernelPageDir = mm_page_dir_addr();
    mm_page_dir_t pTaskPageDir;

    pTaskPageDir = mm_alloc_page();
    pKernelPageDir[MM_PAGE_DIR_SIZE - 2] = (mm_page_tbl_t)(
            (uint_t)pTaskPageDir | ACC_SUPER | ACC_RW | ENTRY_PRESENT
        );

    memcpy(
            (byte_t*)TMP_PAGE_FRAME,
            (byte_t*)pKernelPageDir,
            MM_PAGE_DIR_SIZE * sizeof(mm_page_tbl_t)
        );
    ((mm_page_dir_t)TMP_PAGE_FRAME)[MM_PAGE_DIR_SIZE - 1] = (mm_page_tbl_t)(
            (uint_t)pTaskPageDir | ACC_SUPER | ACC_RW | ENTRY_PRESENT
        );
    pKernelPageDir[MM_PAGE_DIR_SIZE - 2] = NULL;
    return pTaskPageDir;
}

uint_t KERNEL_CALL
mm_alloc_task(const mm_task_mem_t* apMem, const pointer_t apOffset, mm_access_t aAccess)
{
    mm_page_dir_t   pTaskPageDir    = mm_duplicate_page_dir();
    mm_page_dir_t   pKernelPageDir  = mm_page_dir_phys_addr();

    // set page dir to the task page dir
    // swapping page dirs might be dangerous
    __asm__ __volatile__ ("cli");
    write_cr3((dword_t)pTaskPageDir);

    size_t  task_start_page = apMem->start / MM_PAGE_SIZE;
    size_t  task_size       = apMem->header_size + apMem->text_size
            + apMem->data_size + apMem->bss_size;
    size_t task_page_count = task_size / MM_PAGE_SIZE;
    if (task_size % MM_PAGE_SIZE)
        task_page_count++;

    // allocate task code and data
    mm_paging_alloc_pages(task_start_page, task_page_count, aAccess | ACC_RW);

    // copy task code and data
    memcpy((byte_t*)apMem->start, apOffset, task_size - apMem->bss_size);
    /*BRAG("Code, data %x-%x (%db)\n",
            apMem->start,
            apMem->start + task_size - apMem->bss_size,
            task_size - apMem->bss_size
        );*/

    /*BRAG("Bss %x-%x (%db)\n",
            apMem->start + task_size - apMem->bss_size,
            apMem->start + task_size - apMem->bss_size + apMem->bss_size,
            apMem->bss_size
        );*/

    // bss memset 0
    memset(
            (byte_t*)(apMem->start + task_size - apMem->bss_size), 0,
            apMem->bss_size
        );

    // allocate task stack
    mm_paging_alloc_pages(
            (2U * GIGABYTE - 2 * MM_PAGE_SIZE) / MM_PAGE_SIZE,
            2,
            (aAccess & ACC_MASK) | ACC_RW
        );

    // memdump((pointer_t)(apMem->start + task_size - apMem->bss_size), apMem->bss_size);

    write_cr3((dword_t)pKernelPageDir);
    // __asm__ __volatile__ ("sti");
    return (uint_t)pTaskPageDir;
}


void KERNEL_CALL
mm_free_page_dir(uint_t apTaskPageDir)
{
    const pointer_t TMP_PAGE_FRAME  = (pointer_t)((size_t)-1 - 2 * MM_PAGE_SIZE + 1);

    mm_page_dir_t   pKernelPageDir  = mm_page_dir_addr();
    mm_page_dir_t   pTaskPageDir    = (mm_page_dir_t)TMP_PAGE_FRAME;

    pKernelPageDir[MM_PAGE_DIR_SIZE - 2] = (mm_page_tbl_t)(
            (uint_t)apTaskPageDir | ACC_SUPER | ACC_RW | ENTRY_PRESENT
        );

    size_t  tbl_i   = 0;
    for (tbl_i = 0; tbl_i < 512; ++tbl_i)
    {
        if ((size_t)pTaskPageDir[tbl_i] & ENTRY_PRESENT)
        {
            // some black magic to calculate virtual address of page table
            // :TODO: gx 2007-06-05: replace the magic with science ;-)
            mm_page_tbl_t   pTbl    = (mm_page_tbl_t)(
                    (size_t)-1 - 8 * MEGABYTE + 1
                    + MM_PAGE_SIZE * tbl_i
                );
//DUMP(pTaskPageDir[tbl_i]);
//DUMP(tbl_i);
//DUMP(pTbl);

            size_t  page_i  = 0;
            for (page_i = 0; page_i < MM_PAGE_TBL_SIZE; ++page_i)
            {
                if ((uint_t)pTbl[page_i] & ENTRY_PRESENT)
                {
//DUMP(page_i);
//DUMP(pTbl[page_i]);
                    mm_free_page(
                            (pointer_t)((uint_t)pTbl[page_i] & MM_PAGE_ADDR_MASK)
                        );
                }
            }
            mm_free_page(
                    (pointer_t)((size_t)pTaskPageDir[tbl_i] & MM_PAGE_ADDR_MASK)
                );
        }
    }
    pKernelPageDir[MM_PAGE_DIR_SIZE - 2] = NULL;
    mm_free_page((pointer_t)apTaskPageDir);
}
