OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    /* Scarta sezioni inutili/dannose per il kernel */
    /DISCARD/ :
    {
        *(.note.gnu.build-id)
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.comment)
        *(.note.*)
    }

    /* .text inizia ESATTAMENTE a 0x100000 */
    .text :
    {
        *(.multiboot)
        *(.text)
        *(.text.*)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data : ALIGN(4096)
    {
        *(.data)
        *(.data.*)
    }

    .bss : ALIGN(4096)
    {
        *(.COMMON)
        *(.bss)
        *(.bss.*)
    }

    . = ALIGN(4096);
    _kernel_end = .;
}