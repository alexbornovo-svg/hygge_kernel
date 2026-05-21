CC      := gcc
AS      := nasm
LD      := gcc

CFLAGS  := -std=c11 \
            -ffreestanding \
            -O2 \
            -Wall \
            -Wextra \
            -Icode/include \
            -nostdlib \
            -nostdinc \
            -fno-builtin \
            -fno-stack-protector \
            -fno-pie \
            -no-pie \
            -m32

ASFLAGS := -f elf32

LDFLAGS := -ffreestanding \
            -nostdlib \
            -T linker.ld \
            -no-pie \
            -fno-pie \
            -m32 \
           
SRC_DIR   := code
BUILD_DIR := build
ISO_DIR   := $(BUILD_DIR)/iso
BOOT_DIR  := $(ISO_DIR)/boot
GRUB_DIR  := $(BOOT_DIR)/grub

KERNEL    := $(BUILD_DIR)/kernel.bin
ISO       := $(BUILD_DIR)/mykernel.iso

C_SRCS := $(wildcard $(SRC_DIR)/*.c) \
           $(wildcard $(SRC_DIR)/drivers/*.c) \
           $(wildcard $(SRC_DIR)/progfiles/*.c) \
           $(wildcard $(SRC_DIR)/libs/*.c)

ASM_SRCS  := $(wildcard $(SRC_DIR)/*.s)

C_OBJS    := $(patsubst $(SRC_DIR)/%.c,   $(BUILD_DIR)/%.o, $(C_SRCS))
ASM_OBJS  := $(patsubst $(SRC_DIR)/%.s,   $(BUILD_DIR)/%.o, $(ASM_SRCS))

OBJS      := $(ASM_OBJS) $(C_OBJS)

.PHONY: all clean iso run create help

all: $(KERNEL)

$(KERNEL): $(OBJS) linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo ""
	@echo "  [OK] Kernel: $@"
	@echo ""
	@-grub-file --is-x86-multiboot $@ && echo "  [OK] Multiboot header valido" \
	                                  || echo "  [!!] Multiboot header NON valido"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

iso: $(KERNEL)
	@echo "  Creazione struttura ISO..."
	mkdir -p $(GRUB_DIR)
	cp $(KERNEL) $(BOOT_DIR)/kernel.bin
	cp $(SRC_DIR)/grub_modules/grub.cfg $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR)
	@echo ""
	@echo "  [OK] ISO: $(ISO)"

run: iso
	qemu-system-i386 -cdrom $(ISO) -m 32M

run-bin: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -m 32M

create:
	@mkdir -p build
	@mkdir -p code/include
	@mkdir -p code/drivers
	@mkdir -p code/grub_modules
	@mkdir -p code/progfiles
	@mkdir -p code/misc
	@# Crea placeholder solo se non esistono già i file
	@[ -f code/main.c ]  || touch code/main.c
	@[ -f code/loader.s ] || touch code/loader.s
	@[ -f linker.ld ]     || touch linker.ld
	@[ -f code/grub_modules/grub.cfg ] || touch code/grub_modules/grub.cfg
	@echo ""
	@find . -not -path './.git/*' -not -name '.gitignore' \
	        | sort | sed 's|[^/]*/|  |g'
	@echo ""

clean:
	rm -rf $(BUILD_DIR)
	@echo "  [OK] Pulizia completata"

help:
	@echo ""
	@echo "  MyKernel — comandi disponibili:"
	@echo ""
	@grep -E '^## ' Makefile | sed 's/## /    make /'
	@echo ""