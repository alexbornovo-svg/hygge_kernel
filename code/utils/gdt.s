global gdt_flush
extern gp

section .text

gdt_flush:
    lgdt [gp]        
    jmp 0x08:.flush  
.flush:
    mov ax, 0x10     
    mov ds, ax       
    mov es, ax       
    mov ss, ax       
    mov fs, ax       
    mov gs, ax       
    ret

section .note.GNU-stack noalloc noexec nowrite progbits