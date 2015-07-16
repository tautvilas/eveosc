#ifndef _VGA_H_
#define _VGA_H_

#include "global.h"

typedef enum {
        VGA_CL_BLACK                        = 0,
        VGA_CL_BLUE	                        = 1,
        VGA_CL_GREEN                        = 2,
        VGA_CL_CYAN                         = 3,
        VGA_CL_RED                          = 4,
        VGA_CL_MAGENTA                      = 5,
        VGA_CL_BROWN                        = 6,
        VGA_CL_LIGHT_GRAY                   = 7,
        VGA_CL_DARK_GRAY                    = 8,
        VGA_CL_LIGHT_BLUE                   = 9,
        VGA_CL_LIGHT_GREEN                  = 10,
        VGA_CL_LIGHT_CYAN                   = 11,
        VGA_CL_LIGHT_RED                    = 12,
        VGA_CL_LIGHT_MAGENTA                = 13,
        VGA_CL_YELLOW                       = 14,
        VGA_CL_WHITE                        = 15,
    }   color_t;

typedef struct {
        byte_t  mX;
        byte_t  mY;
    }   vga_pos_t;

extern void KERNEL_CALL
vga_print(const char* apStr);

extern void KERNEL_CALL
vga_cl_print(const char* apStr, color_t aFgColor, color_t aBgColor);

extern void KERNEL_CALL
vga_print_char(char aChar);

extern void KERNEL_CALL
vga_clear();

extern void KERNEL_CALL
vga_set_fg_color(color_t aColor);

extern void KERNEL_CALL
vga_set_bg_color(color_t aColor);

extern color_t KERNEL_CALL
vga_get_bg_color();

extern color_t KERNEL_CALL
vga_get_fg_color();

extern void KERNEL_CALL
vga_show_cursor(bool_t aShow);

extern bool_t KERNEL_CALL
vga_cursor_visible();

extern void KERNEL_CALL
vga_set_cursor_pos(byte_t aX, byte_t aY);

extern vga_pos_t KERNEL_CALL
vga_get_cursor_pos();

extern void KERNEL_CALL
vga_install();

#endif // _VGA_H_
