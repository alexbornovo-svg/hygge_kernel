# ==============================================================================
#  Hygge Kernel — Makefile
# ==============================================================================

CC      := gcc
AS      := nasm
LD      := gcc

# GCC ships its own freestanding headers (stddef.h, stdint.h, stdarg.h …).
# We need their path so -isystem can find them even under -nostdinc.
GCC_INCLUDES := $(shell $(CC) -m32 -print-file-name=include)

CFLAGS  := -std=c99              \
            -ffreestanding        \
            -O2                   \
            -Wall                 \
            -Wextra               \
            -Icode/include        \
			-Icode 				  \
            -nostdlib             \
            -nostdinc             \
            -isystem $(GCC_INCLUDES) \
            -fno-builtin          \
            -fno-stack-protector  \
            -fno-pie              \
            -no-pie               \
            -m32

ASFLAGS := -f elf32

LDFLAGS := -ffreestanding \
            -nostdlib      \
            -T linker.ld   \
            -no-pie        \
            -fno-pie       \
            -m32

# ------------------------------------------------------------------------------
#  Directories
# ------------------------------------------------------------------------------
SRC_DIR   := code
BUILD_DIR := build
ISO_DIR   := $(BUILD_DIR)/iso
BOOT_DIR  := $(ISO_DIR)/boot
GRUB_DIR  := $(BOOT_DIR)/grub
KERNEL    := $(BUILD_DIR)/kernel.bin
ISO       := $(BUILD_DIR)/hygge.iso
DISK_IMG  := $(BUILD_DIR)/disk.img

# ------------------------------------------------------------------------------
#  Sources & objects
# ------------------------------------------------------------------------------
C_SRCS := $(wildcard $(SRC_DIR)/kernel/*.c)      \
           $(wildcard $(SRC_DIR)/drivers/*.c)     \
           $(wildcard $(SRC_DIR)/mm/*.c)          \
           $(wildcard $(SRC_DIR)/utils/*.c)       \
           $(wildcard $(SRC_DIR)/libs/*.c)        \
           $(wildcard $(SRC_DIR)/progfiles/*.c)

ASM_SRCS := $(wildcard $(SRC_DIR)/boot/*.s)      \
            $(wildcard $(SRC_DIR)/utils/*.s)

# IMPORTANTE: Cambiamo l'estensione finale degli oggetti Assembly in .s.o per non sovrascrivere i .o del C
C_OBJS   := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.s.o, $(ASM_SRCS))
OBJS     := $(ASM_OBJS) $(C_OBJS)

# ------------------------------------------------------------------------------
#  Phony targets
# ------------------------------------------------------------------------------
.PHONY: all clean iso run run-bin run-debug create info help

# ------------------------------------------------------------------------------
#  Default target
# ------------------------------------------------------------------------------
all: $(KERNEL)

# ------------------------------------------------------------------------------
#  Link
# ------------------------------------------------------------------------------
$(KERNEL): $(OBJS) linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo ""
	@echo "  [OK] Kernel: $@"
	@echo ""
	@-grub-file --is-x86-multiboot $@ \
		&& echo "  [OK] Multiboot header valid" \
		|| echo "  [!!] Multiboot header NOT valid"

# ------------------------------------------------------------------------------
#  Compile C
# ------------------------------------------------------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ------------------------------------------------------------------------------
#  Assemble
# ------------------------------------------------------------------------------
$(BUILD_DIR)/%.s.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# ------------------------------------------------------------------------------
#  Build directory
# ------------------------------------------------------------------------------
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ------------------------------------------------------------------------------
#  ISO image
# ------------------------------------------------------------------------------
iso: $(KERNEL)
	@echo "  Building ISO..."
	@mkdir -p $(GRUB_DIR)
	@cp $(KERNEL) $(BOOT_DIR)/kernel.bin
	@cp $(SRC_DIR)/grub_modules/grub.cfg $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR)
	@echo ""
	@echo "  [OK] ISO: $(ISO)"

# ------------------------------------------------------------------------------
#  Hard Disk Generation (1MB)
# ------------------------------------------------------------------------------
$(DISK_IMG): | $(BUILD_DIR)
	@echo "  Creating 1MB virtual hard drive..."
	@dd if=/dev/zero of=$(DISK_IMG) bs=1M count=1 2>/dev/null
	@echo "  [OK] Hard disk image created: $(DISK_IMG)"

# ------------------------------------------------------------------------------
#  Run targets
# ------------------------------------------------------------------------------

## run         — boot ISO in QEMU with 1MB ATA Drive
run: iso $(DISK_IMG)
	qemu-system-i386 -cdrom $(ISO) -m 32M -drive file=$(DISK_IMG),format=raw,index=0,media=disk

## run-bin      — boot kernel binary directly with 1MB ATA Drive
run-bin: $(KERNEL) $(DISK_IMG)
	qemu-system-i386 -kernel $(KERNEL) -m 32M -drive file=$(DISK_IMG),format=raw,index=0,media=disk

## run-debug    — boot kernel with GDB stub and ATA Drive
run-debug: $(KERNEL) $(DISK_IMG)
	qemu-system-i386 -kernel $(KERNEL) -m 32M -drive file=$(DISK_IMG),format=raw,index=0,media=disk -s -S &
	@echo ""
	@echo "  QEMU paused — attach GDB with:"
	@echo "      gdb $(KERNEL)"
	@echo "      (gdb) target remote :1234"
	@echo "      (gdb) continue"
	@echo ""

# ------------------------------------------------------------------------------
#  Utilities
# ------------------------------------------------------------------------------

## size         — show section sizes for each object file
size:
	@echo ""
	@size $(OBJS) 2>/dev/null || echo "  (no objects built yet)"
	@echo ""

## nm           — list all kernel symbols (sorted by address)
nm: $(KERNEL)
	@echo ""
	@nm -n $(KERNEL)
	@echo ""

## objdump      — disassemble the kernel binary
objdump: $(KERNEL)
	objdump -d -M intel $(KERNEL) | less

## syms         — list only global function symbols
syms: $(KERNEL)
	@echo ""
	@nm $(KERNEL) | grep " T " | sort
	@echo ""

## check        — run static analysis with cppcheck (if installed)
check:
	@command -v cppcheck >/dev/null 2>&1 || { echo "  cppcheck not found"; exit 0; }
	cppcheck --enable=all --std=c99 \
	         --suppress=missingIncludeSystem \
	         -I code/include \
	         $(C_SRCS)

## lines        — count lines of C and assembly source
lines:
	@echo ""
	@wc -l $(C_SRCS) $(ASM_SRCS) 2>/dev/null | sort -rn | head -20
	@echo ""

## todo         — grep TODO / FIXME / HACK in source files
todo:
	@echo ""
	@grep -rn --color=always -E "TODO|FIXME|HACK" $(SRC_DIR) || echo "  (none found)"
	@echo ""

## watch        — rebuild automatically on file change (requires inotifywait)
watch:
	@command -v inotifywait >/dev/null 2>&1 || { echo "  inotifywait not found (install inotify-tools)"; exit 1; }
	@echo "  Watching $(SRC_DIR) for changes…"
	@while inotifywait -rqe modify,create,delete $(SRC_DIR); do \
		echo ""; \
		echo "  [change detected] rebuilding…"; \
		$(MAKE) all; \
	done

# ------------------------------------------------------------------------------
#  Scaffold new project layout
# ------------------------------------------------------------------------------

## create       — create directory structure and placeholder files
create:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p code/include
	@mkdir -p code/drivers
	@mkdir -p code/grub_modules
	@mkdir -p code/progfiles
	@mkdir -p code/libs
	@mkdir -p code/misc
	@[ -f code/main.c ]                     || touch code/main.c
	@[ -f code/loader.s ]                   || touch code/loader.s
	@[ -f linker.ld ]                       || touch linker.ld
	@[ -f code/grub_modules/grub.cfg ]      || touch code/grub_modules/grub.cfg
	@echo ""
	@find . -not -path './.git/*' -not -name '.gitignore' \
	        | sort | sed 's|[^/]*/|  |g'
	@echo ""

# ------------------------------------------------------------------------------
#  Info — print build configuration
# ------------------------------------------------------------------------------

## info         — print compiler flags and source list
info:
	@echo ""
	@echo "  CC        : $(CC)"
	@echo "  AS        : $(AS)"
	@echo "  LD        : $(LD)"
	@echo "  GCC hdrs  : $(GCC_INCLUDES)"
	@echo "  CFLAGS    : $(CFLAGS)"
	@echo "  LDFLAGS   : $(LDFLAGS)"
	@echo "  ASFLAGS   : $(ASFLAGS)"
	@echo ""
	@echo "  C sources :"
	@for f in $(C_SRCS);   do echo "    $$f"; done
	@echo "  ASM sources:"
	@for f in $(ASM_SRCS); do echo "    $$f"; done
	@echo ""

# ------------------------------------------------------------------------------
#  Clean
# ------------------------------------------------------------------------------

## clean        — remove all build artifacts
clean:
	rm -rf $(BUILD_DIR)
	@echo "  [OK] Clean done"

# ------------------------------------------------------------------------------
#  Help
# ------------------------------------------------------------------------------

## help         — show this message
help:
	@echo ""
	@echo "  Hygge Kernel — available targets:"
	@echo ""
	@grep -E '^## ' Makefile | sed 's/## /    make /'
	@echo ""