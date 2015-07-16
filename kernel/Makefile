# setting variables depending on OS
# SYS   - OS identifier (win/nix)
# DS    - directory separator
# PS    - path separator
# RM    - command for deleting files
ifeq ($(OS),Windows_NT)
	SYS     := win
	DS      := \\# this is hack to set DS to `\`
	PS      := ;
	RM      := del /Q
	NUL     := NUL
else
	SYS     := nix
	DS      := /
	PS      := :
	RM      := rm -f
	NUL     := /dev/null
    SYS_CFLAGS  := -fno-stack-protector
endif

# this hides commands
# set E to empty string if you want to see commands used
# it may by done from command line: make E=
E := @

# directories
SRC_DIR := src
INC_DIR := $(SRC_DIR)$(DS)headers
OBJ_DIR := obj
BIN_DIR := bin

# directories with trailing directory separator
SRC := $(SRC_DIR)$(DS)
OBJ := $(OBJ_DIR)$(DS)
BIN := $(BIN_DIR)$(DS)

VPATH := $(SRC_DIR)$(PS)$(INC_DIR)$(PS)$(OBJ_DIR)$(PS)$(BIN_DIR)

# C compiler flags
#CFLAGS := -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions \
#		-ffreestanding -fleading-underscore -nostdinc -fno-builtin \
#       $(SYS_CFLAGS) -c -x c

CFLAGS := -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions \
		-fleading-underscore -nostdinc -fno-builtin \
        $(SYS_CFLAGS) -c -x c -ffreestanding

# nasm flags
# on nix nasm requires include directory with trailing slash
# on win nasm ignores trailing slash but can not work with trailing backslash!
ASFLAGS := -f aout -I$(SRC_DIR)/
LDFLAGS := -T link.ld -s -x $(MAP)

define C_COMPILE
		@echo ~~~ Compiling $(@F) from $(filter %.c, $(^F))
		$Eg++ $(CFLAGS) -o $(OBJ)$(@F) $(addprefix $(SRC), $(filter %.c, $(^F)))
endef

#paging.o
kernel.bin: kentry.o main.o vga.o mem.o ports.o idt.o stdio.o logo.o timer.o \
	keyboard.o memmgr.o syscalls.o tskmgr.o malloc.o errno.o sem.o resmgr.o \
	gdt.o ring.o
	@echo ~~~ Linking $(^F)
	$Eld $(LDFLAGS) -o $(BIN)kernel.bin $(addprefix $(OBJ), $(^F))
	@echo ~~~ Done

kentry.o: kentry.asm isr.asm utils.asm
	@echo ~~~ Compiling entry
	$Enasm $(ASFLAGS) $(SRC)kentry.asm -o $(OBJ)kentry.o

malloc.o: malloc.c mallocdefs.h
	$(C_COMPILE)

# default rule for compiling C source files
# used only if no rule is defined explictly
%.o: %.c %.h global.h malloc.h
	$(C_COMPILE)



.PHONY: clean vars
clean:
	@echo ~~~ Cleaning kernel
	-$E$(RM) $(OBJ)*.o > $(NUL)
	-$E$(RM) $(BIN)*.bin > $(NUL)
	@echo ~~~ Done
