
// :TODO: gd 2007-02-28: documentation
// :TODO: gd 2007-02-28: vga_put_char() to put char on screen w/o moving cursor

#include "vga.h"
#include "ports.h"
#include "mem.h"


// :TODO: gx 2007-05-22: use gsVgaMem instead of this constant to avoid
//      arithmetics every time using VGA_MEM
#define VGA_MEM                             ((byte_t*)(0xB8000 + (dword_t)&gKernelBase))

#define VGA_LINE_WIDTH                      80
#define VGA_LINE_COUNT                      25
#define VGA_CHAR_SIZE                       2

#define VGA_TAB_WIDTH                       8
#define VGA_BLANK_CHAR                      0x20    // space

#define VGA_CURSOR_POS_HI                   0x0E
#define VGA_CURSOR_POS_LO                   0x0F
#define VGA_CURSOR_START                    0x0A
#define VGA_CURSOR_DISABLE_MASK             0x20    // 00100000b


extern char gKernelBase;

// we should use some kind of vga_init() to initialize cursor with real
// values and VGA ports with 0x3d4/0x3d5 or 0x3b4/0x3b5 values depending
// on misc output register 1st bit (0 - monochrome compatibility, 1 - color)

static struct {
        vga_pos_t   mPos;
        bool_t      mVisible;
        byte_t*     mpMemPos;
} gsCursor = { {0, 0}, TRUE, 0 };


static color_t      gsBgColor   = VGA_CL_BLACK;
static color_t      gsFgColor   = VGA_CL_LIGHT_GRAY;


static word_t       gsAddrPort  = 0x3d4;
static word_t       gsDataPort  = 0x3d5;



byte_t KERNEL_CALL
vga_get_reg(byte_t aRegIdx)
{
    outportb(gsAddrPort, aRegIdx);
    return inportb(gsDataPort);
}


void KERNEL_CALL
vga_set_reg(byte_t aRegIdx, byte_t aData)
{
    outportb(gsAddrPort, aRegIdx);
    outportb(gsDataPort, aData);
}


void KERNEL_CALL
vga_set_fg_color(color_t aColor)
{
    gsFgColor   = (color_t)(aColor & 0xF);     // clearing unused bits
}


void KERNEL_CALL
vga_set_bg_color(color_t aColor)
{
    gsBgColor   = aColor ;//& 0xF0;    // clearing unused bits
}


color_t KERNEL_CALL
vga_get_fg_color()
{
    return gsFgColor;
}


color_t KERNEL_CALL
vga_get_bg_color()
{
    return gsBgColor;
}


void KERNEL_CALL
vga_set_cursor_pos(byte_t aX, byte_t aY)
{
    // should we wrap cursor coordinates?
    if (aX >= VGA_LINE_WIDTH)
        aX  = VGA_LINE_WIDTH - 1;
    if (aY >= VGA_LINE_COUNT)
        aX  = VGA_LINE_COUNT - 1;

    word_t  pos = aY * VGA_LINE_WIDTH + aX;

    vga_set_reg(VGA_CURSOR_POS_HI, pos >> 8);
    vga_set_reg(VGA_CURSOR_POS_LO, pos);

    gsCursor.mPos.mX    = aX;
    gsCursor.mPos.mY    = aY;
    gsCursor.mpMemPos   = VGA_MEM + pos * 2;
}


vga_pos_t KERNEL_CALL
vga_get_cursor_pos()
{
    // should we check hardware cursor pos here?
    return gsCursor.mPos;
}


void KERNEL_CALL
vga_show_cursor(bool_t aShow)
{
    // :TODO: 2007-02-28 gd: check why this doesn't hide cursor

    byte_t val = vga_get_reg(VGA_CURSOR_START);
    if (aShow)
        vga_set_reg(VGA_CURSOR_START, val & ~VGA_CURSOR_DISABLE_MASK);
    else
        vga_set_reg(VGA_CURSOR_START, val | VGA_CURSOR_DISABLE_MASK);

    gsCursor.mVisible   = aShow;

    //gsCursor.mVisible = vga_get_reg(VGA_CURSOR_START) & VGA_CURSOR_ENABLE_MASK;
}


bool_t KERNEL_CALL
vga_cursor_visible()
{
    return gsCursor.mVisible;
}


void KERNEL_CALL
vga_clear()
{
    word_t blank    = VGA_BLANK_CHAR | gsFgColor << 8 | gsBgColor << 12;

    memsetw(
            (word_t*)VGA_MEM,
            blank,
            VGA_LINE_WIDTH * VGA_LINE_COUNT
        );

    vga_set_cursor_pos(0, 0);
}


void KERNEL_CALL
vga_print_char(char aChar)
{
    vga_pos_t   pos = vga_get_cursor_pos();

    switch (aChar)
    {

        case '\r':
        case '\n':
            pos.mX = 0;
            pos.mY++;
        break;

        case '\t':
            pos.mX += VGA_TAB_WIDTH - pos.mX % VGA_TAB_WIDTH;
            // should we fill memory with blanks?
        break;

        case '\b':
            if (pos.mX > 0)
                pos.mX--;
            gsCursor.mpMemPos           -= 2;
            *(gsCursor.mpMemPos)        = ' ';
            *(gsCursor.mpMemPos + 1)    = gsBgColor << 4 | gsFgColor;
        break;

        default:
            *(gsCursor.mpMemPos)        = aChar;
            *(gsCursor.mpMemPos + 1)    = gsBgColor << 4 | gsFgColor;
            pos.mX++;
        break;
    }

    if (pos.mX >= VGA_LINE_WIDTH)
    {
        pos.mX = pos.mX - VGA_LINE_WIDTH;
        pos.mY++;
    }
    if (pos.mY >= VGA_LINE_COUNT)
    {
        pos.mY = VGA_LINE_COUNT - 1;
        // copying all VGA memory one line up
        memcpy(
                VGA_MEM,
                VGA_MEM + VGA_LINE_WIDTH * VGA_CHAR_SIZE,
                VGA_LINE_WIDTH * (VGA_LINE_COUNT - 1) * VGA_CHAR_SIZE
            );
        // clearing bottom line
        memsetw(
                (word_t*)VGA_MEM + (VGA_LINE_COUNT - 1) * VGA_LINE_WIDTH,
                VGA_BLANK_CHAR | gsFgColor << 8 | gsBgColor << 12,
                VGA_LINE_WIDTH
            );
    }
    vga_set_cursor_pos(pos.mX, pos.mY);
}


void KERNEL_CALL
vga_print(const char* apStr)
{
    for (; *apStr; ++apStr)
        vga_print_char(*apStr);
}


void KERNEL_CALL
vga_cl_print(const char* apStr, color_t aFgColor, color_t aBgColor)
{
    color_t bg  = vga_get_bg_color();
    color_t fg  = vga_get_fg_color();

    vga_set_bg_color(aBgColor);
    vga_set_fg_color(aFgColor);

    vga_print(apStr);

    vga_set_bg_color(bg);
    vga_set_fg_color(fg);
}

void KERNEL_CALL
vga_install(void)
{
    //vga_clear();
    vga_show_cursor(TRUE);
    vga_set_cursor_pos(0, 5); // for not overwriting loader messages
    return;
}
