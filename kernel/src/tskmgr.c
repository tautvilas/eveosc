#include "tskmgr.h"
#include "memmgr.h"
#include "malloc.h"
#include "stdio.h"
#include "mem.h"

extern void         write_cr3(dword_t);
extern void         gKernelBase;
extern dword_t      read_cr3();
extern dword_t      gGdtKernelCsSel;
extern dword_t      gGdtUserCsSel;
extern dword_t      gGdtUserDataSel;
extern dword_t      gKernelEnd;

typedef struct {
    dword_t midmag;
    dword_t text;
    dword_t data;
    dword_t bss;
    dword_t syms;
    dword_t entry;
    dword_t trsize;
    dword_t drsize;
} aout_exec_t;

task_t* gpTopTask = NULL;
task_t* gpActiveTask = NULL;
task_ring_node_t* gpActiveTaskRingNode = NULL;

static size_t gsTaskCounter = 0;
static size_t gsTaskIdCounter = 0;

dword_t gPingTaskOffset;
dword_t gEshTaskOffset;
dword_t gKernelTaskOffset;

task_tree_node_t* gpTaskTreeTop;
dword_t gNextTaskOffset = 0;
dword_t gKernelCr3 = 0;

uint_t gpPriorityTimes[3] = {10, 20, 50};

static task_ring_node_t* gpKernelTaskRingNode;
static task_t* gpKernelTask;

task_ring_node_t* KERNEL_CALL
tm_load_task(void* apOffset, task_ring_node_t* apParent, mm_access_t aAccess, priority_t aPriority, bool_t aOnTop)
{
    write_cr3(gKernelCr3);

    aout_exec_t header;
    dword_t* pOffset;
    pOffset = apOffset;
    header.midmag = (dword_t) (*pOffset++);
    header.text = (dword_t) (*pOffset++);
    header.data = (dword_t) (*pOffset++);
    header.bss = (dword_t) (*pOffset++);
    header.syms = (dword_t) (*pOffset++);
    header.entry = (dword_t) (*pOffset++);
    header.trsize = (dword_t) (*pOffset++);
    header.drsize = (dword_t) (*pOffset);

    if(gNextTaskOffset == (dword_t) apOffset)
    {
        gNextTaskOffset += header.data + header.text + sizeof(aout_exec_t);
    }

    BRAG("*** Kernel is loading task... ***\n");
    BRAG("Bin start: %x, text size: %d, data size: %d, bss size: %d\n", apOffset, header.text, header.data, header.bss);

    task_t* pTask = malloc(sizeof(task_t));
    pTask->parent = apParent->pTask->id;
    pTask->vm_info.start = 0;
    pTask->vm_info.entry = header.entry;
    pTask->vm_info.text_size = header.text;
    pTask->vm_info.data_size = header.data;
    pTask->vm_info.bss_size = header.bss;
    pTask->vm_info.header_size = sizeof(aout_exec_t);
    pTask->id = gsTaskIdCounter;
    pTask->priority = aPriority;
    pTask->timetorun = gpPriorityTimes[aPriority];
    pTask->access = aAccess;

    //DUMP(aOnTop);
    if (aOnTop)
        gpTopTask   = pTask;

    gsTaskIdCounter++;
    gsTaskCounter++;


    // alloc task memory
    pTask->page_dir = mm_alloc_task(&pTask->vm_info, apOffset, aAccess);
    //DUMP(pTask->page_dir);

    pTask->ustack = 2U * GIGABYTE - 20 * sizeof(uint_t);
    // swapping page dirs might be dangerous
    if(aAccess == ACC_SUPER)
    {
        __asm__ __volatile__ ("cli");

        write_cr3(pTask->page_dir);

        // prepare RING 0 task stack
        // tribute to infinity
        dword_t* pStack = (dword_t*)pTask->ustack;
        pStack--;
        *(pStack)-- = 0x202;    //eflags: enable interrupts and set reserved bit to 1
        *(pStack)-- = gGdtKernelCsSel;
        *(pStack)-- = pTask->vm_info.entry;
        *(pStack)-- = 0x00;     // error code
        *(pStack)-- = 0x00;     // isr num
        *(pStack)-- = 0x00;
        //*(pStack)-- = (dword_t)((dword_t*) pTask->ustack) - 17;
        *(pStack)-- = 0x00;
        *(pStack)-- = 0x00;
        *(pStack)-- = 0x00;
        *(pStack)-- = 0x00;
        *(pStack)-- = 0x00;
        *(pStack)-- = 0x00;
        *(pStack)-- = 0x00;
        *(pStack)-- = 0x10;
        *(pStack)-- = 0x10;
        *(pStack)-- = 0x10;
        *(pStack)   = 0x10;

        write_cr3(gKernelCr3);
        __asm__ __volatile__ ("sti");
        pTask->esp = (dword_t)pStack;
    }
    else    // user privilege app
    {

        regs_t* pStack = malloc(sizeof(regs_t));
        memset(pStack, 0, sizeof(regs_t));

        pStack->eip     =   pTask->vm_info.entry;
        pStack->eflags  =   0x202;
        pStack->cs      =   gGdtUserCsSel   | 3;
        pStack->ss      =   gGdtUserDataSel | 3;
        pStack->useresp =   pTask->ustack;

        pStack->es      =   gGdtUserDataSel;
        pStack->ds      =   gGdtUserDataSel;
        pStack->fs      =   gGdtUserDataSel;
        pStack->gs      =   gGdtUserDataSel;

        pTask->esp      = (dword_t)pStack;
        pTask->kstack   = (dword_t)pStack + sizeof(regs_t);
        pTask->ss       = gGdtUserDataSel;
    }

    // put the task into task ring
    task_ring_node_t* pNode = malloc(sizeof(task_ring_node_t));
    pNode->pTask = pTask;
    pNode->pNext = gpActiveTaskRingNode->pNext;
    pNode->pNext->pPrev = pNode;
    pNode->pPrev = gpActiveTaskRingNode;
    gpActiveTaskRingNode->pNext = pNode;

    // put task into task tree node

    task_tree_node_t* pTreeNode = malloc(sizeof(task_tree_node_t));
    task_tree_node_t* pPrevChild = NULL;
    task_tree_node_t* pParentTreeNode = apParent->pTreeNode;
    task_tree_node_t* pChild = pParentTreeNode->pFirstChild;
    while (pChild != NULL)
    {
        pPrevChild = pChild;
        pChild = pChild->pNext;
    }
    if(pPrevChild) pPrevChild->pNext = pTreeNode;
    else pParentTreeNode->pFirstChild = pTreeNode;

    pTreeNode->pParent = pParentTreeNode;
    pTreeNode->pPrev = pPrevChild;
    pTreeNode->pNext = NULL;

    pTreeNode->pFirstChild = NULL;
    pTreeNode->pTask = pTask;

    pNode->pTreeNode = pTreeNode;

    BRAG("*** Kernel has ended loading task... Number of tasks running: %d ***\n", gsTaskCounter);

#ifdef EVE_DEBUG
    tm_print_task_tree();
    mm_print_info();
#endif

    return pNode;
}

void KERNEL_CALL
tm_unload_task(task_ring_node_t* apTaskRingNode, task_t* parentTask)
{
    task_t* pTask = apTaskRingNode->pTask;
    if (pTask == gpTopTask)
    {
        gpTopTask = parentTask;
    }

    // free task page dir

    DUMP(pTask->page_dir);
    mm_free_page_dir(pTask->page_dir);

    // free task kernel stack
    if (pTask->access == ACC_USER)
    {
        free((regs_t*)(pTask->kstack - sizeof(regs_t)));
    }

    // free task ring node

    apTaskRingNode->pPrev->pNext = apTaskRingNode->pNext;
    apTaskRingNode->pNext->pPrev = apTaskRingNode->pPrev;

    // free task struct
    free(pTask);

    gsTaskCounter--;

    // iterate through children
    task_tree_node_t* pTaskTreeNode = apTaskRingNode->pTreeNode;
    free(apTaskRingNode);
    task_tree_node_t* pChild = pTaskTreeNode->pFirstChild;
    while (pChild != NULL)
    {
        task_ring_node_t* pActiveTaskRingNode = gpActiveTaskRingNode;
        do
        {
            if(pActiveTaskRingNode->pTask == pChild->pTask)
            {
                tm_unload_task(pActiveTaskRingNode, parentTask);
                break;
            }
            pActiveTaskRingNode = pActiveTaskRingNode->pNext;
        }
        while (pActiveTaskRingNode != gpActiveTaskRingNode);
        pChild = pChild->pNext;
    }

    // free task tree node
    DUMP(pTaskTreeNode);
    free(pTaskTreeNode);
    BRAG("free ok\n");
    return;
}

int KERNEL_CALL
tm_kill_task(uint_t aTaskId)
{
    BRAG("*** Kernel is trying to kill task %d ***\n", aTaskId);

    if (aTaskId == 0)
    {
        BRAG("You can not kill root process!\n");
        return -1;
    }

    write_cr3((dword_t)gKernelCr3);
    //dword_t active_task_page_dir = gpActiveTask->page_dir;

    gpActiveTask = gpKernelTask;
    gpActiveTaskRingNode = gpKernelTaskRingNode;

    bool_t id_is_correct = FALSE;
    task_ring_node_t* pTaskRingNode = gpActiveTaskRingNode;
    do
    {
        if (pTaskRingNode->pTask->id == aTaskId)
        {
            task_tree_node_t* pTaskTreeNode = pTaskRingNode->pTreeNode;
            task_tree_node_t* pParent = pTaskTreeNode->pParent;
            task_tree_node_t* pChild = pParent->pFirstChild;
            task_tree_node_t* pPrevChild = NULL;
            DUMP(pParent);
            DUMP(pChild);
            while (pChild != NULL)
            {
                if (pChild == pTaskTreeNode)
                {
                    if (pPrevChild != NULL)
                    {
                        pPrevChild->pNext = pTaskTreeNode->pNext;
                    }
                    else
                    {
                        pParent->pFirstChild = pTaskTreeNode->pNext;
                    }

                    if (pTaskTreeNode->pNext != NULL)
                    {
                        pTaskTreeNode->pNext->pPrev = pPrevChild;
                    }
                    break;
                }
                pPrevChild = pChild;
                pChild = pChild->pNext;
            }

            tm_unload_task(pTaskRingNode, pParent->pTask);
            id_is_correct = TRUE;
            break;
        }
        pTaskRingNode = pTaskRingNode->pNext;
    }
    while (gpActiveTaskRingNode != pTaskRingNode);

    if (id_is_correct)
    {
        BRAG("*** Task %d and its children killed ***\n", aTaskId);
        tm_print_task_tree();
        mm_print_info();
        return 0;
    }
    else
    {
        BRAG("*** No such task! (%d) ***\n", aTaskId);
        return -1;
    }
}


task_ring_node_t* KERNEL_CALL
tm_install(void)
{
    task_t* pKernel = malloc(sizeof(task_t));
    pKernel->locked = FALSE;
    pKernel->id = gsTaskIdCounter;
    pKernel->parent = 0;
    pKernel->page_dir = read_cr3();
    pKernel->priority = PRIOR_HIGH;
    pKernel->access = ACC_SUPER;

    gsTaskIdCounter++;
    gsTaskCounter++;

    gKernelCr3 = pKernel->page_dir;
    // from here multitasking starts
    gpActiveTask = pKernel;
    gpTopTask = pKernel;

    task_ring_node_t* pNode = malloc(sizeof(task_ring_node_t));
    pNode->pTask = pKernel;
    pNode->pNext = pNode;
    pNode->pPrev = pNode;

    gpActiveTaskRingNode = pNode;

    gNextTaskOffset = (dword_t) &gKernelEnd;

    // tree setup
    task_tree_node_t* pTreeNode = malloc(sizeof(task_tree_node_t));
    pTreeNode->pTask = pKernel;
    pTreeNode->pNext = NULL;
    pTreeNode->pPrev = NULL;
    pTreeNode->pFirstChild = NULL;

    pNode->pTreeNode = pTreeNode;

    gpTaskTreeTop = pTreeNode;

    gpKernelTaskRingNode = pNode;
    gpKernelTask = pKernel;

    return pNode;
}

void KERNEL_CALL
tm_print_task_tree_node(int aDepth, task_tree_node_t* apNode)
{
    int i;

    task_t* pTask = apNode->pTask;
    printf("\t");
    for (i = 0; i < aDepth; i++)
    {
        printf(" ");
    }
    if (apNode->pNext)
    {
        char string[] = {195, 0};
        printf(string);
    }
    else
    {
        char string[] = {192, 0};
        printf(string);
    }
    if (apNode->pFirstChild)
    {
        char string2[] = {194, 0};
        printf(string2);
    }
    else
    {
        char string2[] = {196, 0};
        printf(string2);
    }
    printf(" id = %d", pTask->id);
    printf("\t");
    if (pTask->locked) printf(" LOCKED");
    if (pTask == gpTopTask) printf(" FG");
    printf("\n");
    task_tree_node_t* pChild = apNode->pFirstChild;
    while (pChild != NULL)
    {
        tm_print_task_tree_node(aDepth + 1, pChild);
        pChild = pChild->pNext;
    }
}

void KERNEL_CALL
tm_print_task_tree(void)
{
    printf("PS tree\n");
    tm_print_task_tree_node(0, gpTaskTreeTop);
    printf("\n");
}
