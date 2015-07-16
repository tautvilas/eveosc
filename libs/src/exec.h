#ifndef _EXEC_H_
#define _EXEC_H_


typedef enum {
    PRIOR_LOW = 0,
    PRIOR_NORMAL,
    PRIOR_HIGH
} priority_t;


int
exec(const char* apFileName, priority_t aPriority, int aAsync);


#endif // _EXEC_H_
