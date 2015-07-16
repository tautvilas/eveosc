#ifndef _LOGO_H_
#define _LOGO_H_

#include "global.h"

#define _LOGO_WIDTH  15
#define _LOGO_HEIGHT 7

/* ASCII EveOS logo */

extern int gLogoImage[_LOGO_HEIGHT][_LOGO_WIDTH];

//extern char* gpLogoBig;

/**
 *  Print out eveos logo
 */

extern void KERNEL_CALL
put_logo();

#endif // _LOGO_H_
