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
endif

# this hides commands
# set E to empty string if you want to see commands used
# it may by done from command line: make E=
E := @

# directories
SRC_DIR := src
BIN_DIR := bin

# directories with trailing directory separator
SRC := $(SRC_DIR)$(DS)
OBJ := $(OBJ_DIR)$(DS)
BIN := $(BIN_DIR)$(DS)

VPATH := $(SRC_DIR)$(PS)$(BIN_DIR)

# nasm flags
# on nix nasm requires include directory with trailing slash
# on win nasm ignores trailing slash but can not work with trailing backslash!
ASFLAGS := -f bin -I$(SRC_DIR)/


loader.bin: loader.asm
	@echo ~~~ Compiling loader
	$Enasm $(ASFLAGS) $(SRC)loader.asm -o $(BIN)loader.bin
	@echo ~~~ Done

.PHONY: clean
clean:
	@echo ~~~ Cleaning loader
	-$E$(RM) $(BIN)*.bin > $(NUL)
	@echo ~~~ Done
