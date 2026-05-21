bits 32
section .multiboot
align 4
    dd 0x1BADB002
    dd 0x0
    dd -(0x1BADB002 + 0x0)

section .bss
resb 4096
stack_top:

section .text
global _start
extern kmain
_start:
    mov esp, stack_top
    call kmain
.hang:
    cli
    hlt
    jmp .hang