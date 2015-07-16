#include "logo.h"
#include "vga.h"

#define ___ VGA_CL_BLACK
#define XXX VGA_CL_WHITE
#define YYY VGA_CL_RED

int gLogoImage[_LOGO_HEIGHT][_LOGO_WIDTH] = {
    {___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
    {___, XXX, XXX, XXX, ___, YYY, ___, ___, ___, YYY, ___, XXX, XXX, XXX, ___},
    {___, XXX, ___, ___, ___, YYY, YYY, ___, YYY, YYY, ___, XXX, ___, ___, ___},
    {___, XXX, XXX, XXX, ___, ___, YYY, ___, YYY, ___, ___, XXX, XXX, XXX, ___},
    {___, XXX, ___, ___, ___, ___, YYY, YYY, YYY, ___, ___, XXX, ___, ___, ___},
    {___, XXX, XXX, XXX, ___, ___, ___, YYY, ___, ___, ___, XXX, XXX, XXX, ___},
    {___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
};

/*
char* gpLogoBig =
        "�������������������������������������� ��\n"
        "�� � ������������������������������� � ��\n"
        "� ������������������������������������� �\n"
        " ��������������������������������������� \n"
        "���������������������������������������۱\n"
        "۲�� ��������������۱��۲����������� ����\n"
        " ������������������   ���߲������������� \n"
        "  ��  ������� ��� ��������� �������  ��  \n"
        "��   �� ���� ��������������� ���� ��  ���\n"
        "������ �    ���������������� ����� �� ���\n"
        "�۲����������������۱������������� ������\n"
        "�������� ����������� ����������� ���۲���\n"
        "�������۱ ���������� ���������� �۲������\n"
        "�������� ����������������������� ��������\n";
*/

void KERNEL_CALL
put_logo()
{
    int i, j;
    for(i = 0; i < _LOGO_HEIGHT; i++) {
        for(j = 0; j < _LOGO_WIDTH; j++) {
            vga_set_bg_color(gLogoImage[i][j]);
            vga_print_char(' ');
        }
        vga_print_char('\n');
    }
    /* explicit content :> */
    /*
        vga_set_bg_color(VGA_CL_BLACK);
        vga_set_fg_color(VGA_CL_LIGHT_GRAY);
        vga_print(gpLogoBig);
        vga_print("\n");
    */
    vga_set_bg_color(VGA_CL_BLACK);
    vga_set_fg_color(VGA_CL_LIGHT_GRAY);
}

