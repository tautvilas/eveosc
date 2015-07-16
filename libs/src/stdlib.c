#include <stdlib.h>


long
atol(const char* apStr)
{
    const char* p   = apStr;
    int res         = 0;
    int negative    = 0;

    if ('-' == *p)
    {
        negative = 1;
        ++p;
    }

    for (; *p >= '0' && *p <= '9'; ++p)
        res = res * 10 + (*p - '0');

    if (negative)
        return -res;
    else
        return res;
}

int
atoi(const char* apStr)
{
    return (int)atol(apStr);
}

