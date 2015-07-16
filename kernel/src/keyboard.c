#include "keyboard.h"
#include "stdio.h"
#include "ports.h"
#include "vga.h"
#include "idt.h"

#define KEYBOARD_BUFFER_SIZE 256

#define KEYBOARD_COMMAND 0x64
#define KEYBOARD_DATA    0x60

#define LIGHT_ON         1
#define LIGHT_OFF        0

#define LIGHT_SCROLL     0x1
#define LIGHT_NUM        0x2
#define LIGHT_CAPS       0x4

static bool_t gShiftPressed = FALSE;
static bool_t gCtrlPressed = FALSE;
static bool_t gAltPressed = FALSE;
static bool_t gCapsPressed = FALSE;
static bool_t gNumPressed = FALSE;
static bool_t gScrollPressed = FALSE;

static byte_t lights = 0;

static char gKeyboardBuffer[KEYBOARD_BUFFER_SIZE];
static char gNewlineBuffer[KEYBOARD_BUFFER_SIZE];
static uint_t gKeyboardBufferPos = 0;
static size_t gKeyboardBufferSize = 0;     //how much kbd buffer is filled?
static uint_t gNewlineBufferPos = 0;
static uint_t gNewlineBufferStart = 0;
static uint_t gNewlineBufferSize = 0;

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. */
static unsigned char gKbdLayoutUs[2][128] =
{
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
        '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,	/* 29   - Control */
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 39 */
        '\'', '`',
        0,  /* Left shift - 42 */
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
        0,	/* Right shift - 54 */
        0,  /* print screen? */
        0,	/* Alt */
        ' ',/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,	0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
        '-',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
        '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0, 0, 0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0,	/* All other keys are undefined */
    },
    /* if shift is pressed */
    {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
        '_', '+', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0,	/* 29   - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',   /* 39 */
        '"', '~',
        0,  /* Left shift */
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
        0,	/* Right shift */
        0,  /* print screen? */
        0,	/* Alt */
        ' ',/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,	0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
        '_',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
        '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0, 0, 0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0,	/* All other keys are undefined */
    }
};

/* wait till keyboard is ready; */

static void KERNEL_CALL
keyboard_wait()
{
    while(1)
        if ((inportb(KEYBOARD_COMMAND) & 2) == 0) break;
    return;
}

/**
  * manage keyboard lights
  *
  * @param aState  light on or off
  * @param aLight  caps, scroll or num lock
  */

static void KERNEL_CALL
keyboard_manage_lights(bool_t aState, byte_t aLight)
{
    keyboard_wait();

    /* tell the keyboard that we want to manage lights */
    outportb(KEYBOARD_DATA, 0xED);
    if(aState == LIGHT_ON)
    {
        lights |= aLight;
    }
    else
    {
        lights &= ~aLight;
    }
    outportb(KEYBOARD_DATA, lights);
    return;
}

static void KERNEL_CALL
newline_buf_pos_dec(void)
{
    if (gNewlineBufferPos)
    {
        gNewlineBufferPos--;
    }
    else
    {
        gNewlineBufferPos = KEYBOARD_BUFFER_SIZE - 1;
    }
}

/* handler keyboard IRQ1 */

static void KERNEL_CALL
keyboard_handler(regs_t * apRegs)
{
	unsigned char scancode;
    char c = 0;

	scancode = inportb(KEYBOARD_DATA);
    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
        scancode &= 0x7F;
        switch (scancode)
        {
            case 29:
                gCtrlPressed = FALSE;
            break;

            case 42:
                gShiftPressed = FALSE;
            break;

            case 54:
                gShiftPressed = FALSE;
            break;

            case 56:
                gAltPressed = FALSE;
            break;

            default:
            break;
        }
    }
    else
    {
        switch (scancode)
        {
            case 29:
                gCtrlPressed = TRUE;
            break;

            case 42:
                gShiftPressed = TRUE;
            break;

            case 54:
                gShiftPressed = TRUE;
            break;

            case 56:
                gAltPressed = TRUE;
            break;

            case 58:
                if(!gCapsPressed)
                {
                    gCapsPressed = TRUE;
                    keyboard_manage_lights(LIGHT_ON, LIGHT_CAPS);
                }
                else
                {
                    gCapsPressed = FALSE;
                    keyboard_manage_lights(LIGHT_OFF, LIGHT_CAPS);
                }
            break;

            case 69:
                if(!gNumPressed)
                {
                    gNumPressed = TRUE;
                    keyboard_manage_lights(LIGHT_ON, LIGHT_NUM);
                }
                else
                {
                    gNumPressed = FALSE;
                    keyboard_manage_lights(LIGHT_OFF, LIGHT_NUM);
                }
            break;

            case 70:
                if(!gScrollPressed)
                {
                    gScrollPressed = TRUE;
                    keyboard_manage_lights(LIGHT_ON, LIGHT_SCROLL);
                }
                else
                {
                    gScrollPressed = FALSE;
                    keyboard_manage_lights(LIGHT_OFF, LIGHT_SCROLL);
                }
            break;

            case 83:    /* ctrl + alt + del */
                if(gCtrlPressed & gAltPressed)
                {
                    printf("\n");
                    kernel_panic();
                }
            break;

            default:
                if(gKbdLayoutUs[0][scancode] == 0)
                {
                    BRAG("\nUnrecognized scancode - %d\n", scancode);
                    break;
                }
                else if(gShiftPressed)
                {
                    c = gKbdLayoutUs[1][scancode];
                }
                else
                {
                    c = gKbdLayoutUs[0][scancode];
                }

                gNewlineBufferPos %= KEYBOARD_BUFFER_SIZE;
                gKeyboardBufferPos %= KEYBOARD_BUFFER_SIZE;

                if (c == '\b')
                {
                    if (gNewlineBufferSize)
                    {
                        gNewlineBufferSize--;
                        vga_print_char(c);
                        newline_buf_pos_dec();
                    }
                } else if (c == '\n') {
                    int i;
                    int keyboardCopyPos = gKeyboardBufferPos;
                    //DUMP(gNewlineBufferSize);
                    for (i = 0; i < gNewlineBufferSize; i++)
                    {
                        keyboardCopyPos = gKeyboardBufferPos + i;
                        keyboardCopyPos %= KEYBOARD_BUFFER_SIZE;
                        gKeyboardBuffer[keyboardCopyPos] = gNewlineBuffer[gNewlineBufferStart + i];
                        if(gKeyboardBufferSize < KEYBOARD_BUFFER_SIZE)
                        {
                            gKeyboardBufferSize++;
                        }
                    }
                    keyboardCopyPos++;
                    keyboardCopyPos %= KEYBOARD_BUFFER_SIZE;
                    gKeyboardBuffer[keyboardCopyPos] = '\n';
                    if(gKeyboardBufferSize < KEYBOARD_BUFFER_SIZE)
                    {
                        gKeyboardBufferSize++;
                    }
                    // flush newline buf
                    gNewlineBufferSize = 0;
                    gNewlineBufferStart = gNewlineBufferPos;
                    vga_print_char(c);
                } else
                {
                    gNewlineBuffer[gNewlineBufferPos] = c;
                    gNewlineBufferPos++;
                    if(gNewlineBufferSize < KEYBOARD_BUFFER_SIZE)
                    {
                        gNewlineBufferSize++;
                    }
                    vga_print_char(c);
                }
            break;
        }
    }
}

void KERNEL_CALL
keyboard_install(void)
{
	irq_install_handler(1, keyboard_handler);
	return;
}

char KERNEL_CALL
keyboard_getchar(void)
{
    int size = gKeyboardBufferSize;
    if(!size) return 0;
    gKeyboardBufferPos %= KEYBOARD_BUFFER_SIZE;
    char c = gKeyboardBuffer[gKeyboardBufferPos];
    gKeyboardBufferPos++;
    gKeyboardBufferSize--;
    return c;
}

bool_t KERNEL_CALL
keyboard_had_input(void)
{
    if(gKeyboardBufferSize) return TRUE;
    else return FALSE;
}

void KERNEL_CALL
keyboard_flush_buffer(void)
{
    gKeyboardBufferSize = 0;
    gNewlineBufferSize = 0;
}

void KERNEL_CALL
a20_enable(void)
{
    /* TODO test A20 gate, maybe remove cli and sti by enabling A20 before ISR's */
    byte_t a;
    __asm__ __volatile__ ("cli");

    keyboard_wait();
    outportb(KEYBOARD_COMMAND, 0xAD); // disable keyboard

    keyboard_wait();
    outportb(KEYBOARD_COMMAND, 0xD0); // Read from input

    keyboard_wait();
    a = inportb(KEYBOARD_DATA);

    keyboard_wait();
    outportb(KEYBOARD_COMMAND, 0xD1); // Write to output

    keyboard_wait();
    outportb(KEYBOARD_DATA, a|2);     // set A20 gate enabled

    keyboard_wait();
    outportb(KEYBOARD_COMMAND, 0xAE); // enable keyboard

    __asm__ __volatile__ ("sti");
	return;
}
