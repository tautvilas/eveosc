#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_


typedef enum {
    PRIOR_LOW = 0,
    PRIOR_NORMAL,
    PRIOR_HIGH
} priority_t;

extern void __attribute__((cdecl)) sys_write(const char* apStr, int aLen);
extern int __attribute__((cdecl)) sys_read(int aFd, char* apBuffer, int aLen);
extern int __attribute__((cdecl)) exec(const char* apFileName, priority_t aPriority, int aOnTop);
extern int __attribute__((cdecl)) kill(int aTaskId);

#endif // _SYSCALLS_H_
