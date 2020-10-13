OUTPUT_ARCH(riscv)
ENTRY(_start)

BASE_ADDRESS = 0xffffffffc0200000;

SECTIONS
{
    /* Load the kernel at this address: "." means the current address */
    . = BASE_ADDRESS;
    start = .;

    .text : {
        stext = .;
        *(.text.entry)
        *(.text .text.*)
        . = ALIGN(4K);
        etext = .;
    }

    .data : {
        sdata = .;
        *(.data .data.*)
        edata = .;
    }

    .bss : {
        sbss = .;
        *(.bss .bss.*)
        ebss = .;
    }

    PROVIDE(end = .);
}