section .multiboot
align 4
    dd 0x1BADB002          ; magic
    dd 0x00                ; flags
    dd -(0x1BADB002 + 0x00); checksum

section .text
global _start
extern kmain
_start:
    mov esp, stack_top
    call kmain
    cli
    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits