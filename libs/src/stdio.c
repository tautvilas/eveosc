//#include "global.h"
#include <stdio.h>
#include <string.h>
#include <syscalls.h>

#define PRINT_STRING    sys_print_string
#define PRINT_CHAR      sys_print_char

#define STDIN           0
#define STDOUT          1
#define STDERR          1   // :TODO: gx 2007-05-31: use normal stderr when it's implemented in kernel

static char gHexTable[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
static char number[256];

static FILE gsStdIn     = { STDIN };
static FILE gsStdOut    = { STDOUT };
static FILE gsStdErr    = { STDERR };

FILE* stdin     = &gsStdIn;
FILE* stdout    = &gsStdOut;
FILE* stderr    = &gsStdErr;

static void
sys_print_string(char* apString)
{
    sys_write(apString, strlen(apString));
    return;
}

static void
sys_print_char(char aC)
{
    sys_write(&aC, 1);
    return;
}

char
getchar()
{
    //char buffer[1];
    //sys_read(0, buffer, 1);
    //return buffer[0];
    return fgetc(stdin);
}

int
fgetc(FILE* apStream)
{
    //printf("fgetC fd:%x\n", apStream->mFd);
    char c;
    if (1 == sys_read(apStream->mFd, &c, 1))
        return c;
    else
        return EOF;
}


char*
fgets(char* apBuffer, int aSize, FILE* apStream)
{
    //printf("fgetS fd:%x\n", apStream->mFd);
    int c;
    int i       = 0;
    int len     = aSize - 1;
    while (i < len)
    {
        c = fgetc(apStream);
        if (EOF == c)
            return NULL;

        apBuffer[i++]   = c;
        if ('\n' == c)
            break;
    }
    apBuffer[i] = 0;
    return apBuffer;
}

void
print_int_dec(const int aInt)
{
    int x = aInt;

    if (x == 0)
    {
        PRINT_CHAR('0');
        return;
    }

    int y = 0;
    int count = 0;

    if(x < 0) {
        PRINT_CHAR('-');
        x *= -1;
    }
    while(x) {
        y *= 10;
        y += x%10;
        x /= 10;
        count++;
    }

    while(count) {
        count--;
        PRINT_CHAR(y%10 + '0');
        y /= 10;
    }
    return;
}

static void
print_int_hex(const unsigned int aInt)
{
    unsigned int x = aInt;
    int i;
    int count = 0;
    while(x)
    {
        number[count] = gHexTable[x % 16];
        x /= 16;
        count++;
    }
    PRINT_STRING("0x");
    if (count % 2) PRINT_CHAR('0');
    if(count == 0) PRINT_STRING("00");
    for(i = count-1; i >= 0; i--)
    {
        PRINT_CHAR(number[i]);
    }
    return;
}

void /*__attribute__((stdcall))*/
printf(const char * apFormatStr, ...)
{
    const char *pS;
    void * pArgs;
    int * int_val;
    pArgs = &apFormatStr;
    //DUMP(apFormatStr);
    //void *pArgs = apFormatStr + ;
    for(pS = apFormatStr; *pS; pS++)
    {
        if(*pS != '%')
        {
            PRINT_CHAR(*pS);
            continue;
        }
        switch (*++pS) {
            case 'd':
                pArgs = (int*)pArgs + 1;
                int_val = (int*)pArgs;
                print_int_dec(*int_val);
                break;
            case 'x':
                pArgs = (int*)pArgs + 1;
                int_val = (int*)pArgs;
                print_int_hex(*int_val);
                break;
            case 's':
                pArgs = (int*)pArgs + 1;
                if (*(char**)pArgs)
                    PRINT_STRING(*(char**)pArgs);
                break;
            default:
                PRINT_CHAR(*pS);
                break;
        }
    }
    /* TODO stack cleanup? */
    return;
}

void
putc(char aChar)
{
	PRINT_CHAR(aChar);
	return;
}
