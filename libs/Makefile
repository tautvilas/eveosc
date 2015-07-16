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

# set the compiler
CC := gcc

# this hides commands
# set E to empty string if you want to see commands used
# it may by done from command line: make E=
E := @

# directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# directories with trailing directory separator
SRC := $(SRC_DIR)$(DS)
OBJ := $(OBJ_DIR)$(DS)
BIN := $(BIN_DIR)$(DS)

VPATH := $(SRC_DIR)$(PS)$(OBJ_DIR)$(PS)$(BIN_DIR)

# C compiler flags
CFLAGS := -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions \
		-ffreestanding -fleading-underscore -nostdinc -fno-builtin \
        -I$(SRC_DIR) $(SYS_CFLAGS) -c -x c

# nasm flags
# on nix nasm requires include directory with trailing slash
# on win nasm ignores trailing slash but can not work with trailing backslash!
ASFLAGS := -f elf -I$(SRC_DIR)/
LDFLAGS := -T link.ld -s -x $(MAP)

define C_COMPILE
		@echo ~~~ Compiling $(@F) from $(filter %.c, $(^F))
        $(E)$(CC) $(CFLAGS) -o $(OBJ)$(@F) $(addprefix $(SRC), $(filter %.c, $(^F)))
endef

all: stdio.o string.o stdlib.o syscalls.o
	$(E)date +"%Y-%m-%d %H:%M:%S" > $(OBJ)version

syscalls.o: syscalls.asm
	@echo ~~~ Compiling syscalls
	$Enasm $(ASFLAGS) $(SRC)syscalls.asm -o $(OBJ)syscalls.o

# default rule for compiling C source files
# used only if no rule is defined explictly
%.o: %.c %.h
	$(C_COMPILE)

.PHONY: clean vars
clean:
	@echo ~~~ Cleaning libs
	-$(E)$(RM) $(OBJ)*.o > $(NUL)
	-$(E)$(RM) $(BIN)*.bin > $(NUL)
	@echo ~~~ Done
