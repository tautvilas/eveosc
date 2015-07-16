#include <string.h>


void*
memcpy(void* apDest, const void* apSrc, size_t aCount)
{
    char* pDest   = (char*)apDest;
    char* pSrc    = (char*)apSrc;
    for(; aCount; aCount--)
    {
        *pDest++ = *pSrc++;
    }
    return apDest;
}


void*
memset(void* apDest, int aVal, size_t aCount)
{
    char*   pDest   = (char*)apDest;
    char    c       = (char)aVal;
    for(; aCount; aCount--)
    {
        *pDest++    = c;
    }
    return apDest;
}


/*
extern word_t*
memsetw(word_t* apDest, word_t aVal, size_t aCount)
{
    word_t* pDest   = apDest;
    for(; aCount; aCount--)
    {
        *pDest++ = aVal;
    }
    return apDest;
}


extern dword_t*
memsetd(dword_t* apDest, dword_t aVal, size_t aCount)
{
    dword_t* pDest  = apDest;
    for(; aCount; aCount--)
    {
        *pDest++ = aVal;
    }
    return apDest;
} */

size_t
strlen(const char *apStr)
{
    const char* pPos = apStr;
    for (; *pPos; ++pPos);    // NO BODY
	return pPos - apStr;
}


int
memcmp(const void* apSrc1, const void* apSrc2, size_t aCount)
{
    int i;
    for (i = 0; i < aCount; ++i, ++apSrc1, ++apSrc2)
    {
        int diff    = *(char*)apSrc1 - *(char*)apSrc2;
        if (diff)
            return diff;
    }
    return 0;
}


int
strncmp(const char* apStr1, const char* apStr2, size_t aCount)
{
    int diff;
    int i;
    for (i = 0; i < aCount && *apStr1 && *apStr2; ++i, ++apStr1, ++apStr2)
    {
        diff    = *apStr1 - *apStr2;
        if (diff)
            return diff;
    }
    if (i < aCount)
        return *apStr1 - *apStr2;
    else
        return 0;
}


int
strcmp(const char* apStr1, const char* apStr2)
{
    return strncmp(apStr1, apStr2, strlen(apStr1) + 1);
}


char*
strchr(char* apStr, int aChar)
{
    for (; *apStr; ++apStr)
        if (aChar == *apStr)
            return apStr;

    return NULL;
}


char*
strpbrk(char* apStr, char* apDelims)
{
    for (; *apStr; ++apStr)
        if (strchr(apDelims, *apStr))
            return apStr;

    return NULL;
}


char*
strtok(char* apStr, char* apDelims)
{
    static char* spStr  = NULL;

    //DUMP(spStr);
    if (apStr)
        spStr   = apStr;

    if (NULL == spStr)
        return NULL;

    //DUMP(spStr);

    char* pRes  = spStr;
    char* pPos  = strpbrk(spStr, apDelims);
    //DUMP(pPos);
    if (pPos)
    {
        *pPos   = 0;
        spStr   = pPos + 1;
    }
    else
    {
        spStr   = NULL;
    }
    return pRes;
}
