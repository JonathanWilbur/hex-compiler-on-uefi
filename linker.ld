OUTPUT_FORMAT(pei-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(efi_main)

SECTIONS {
  . = 0x1000;

  .text : ALIGN(4096) {
    *(.text)
  }

  .data : ALIGN(4096) {
    *(.data)
  }

  .rodata : ALIGN(4096) {
    *(.rodata)
  }

  .bss : ALIGN(4096) {
    __bss_start = .;
    *(.bss)
    __bss_end = .;
  }

  .dynamic : ALIGN(4096) { *(.dynamic) }
  .dynsym  : ALIGN(4096) { *(.dynsym)  }
  .rela    : ALIGN(4096) { *(.rela*)   }
  .reloc   : ALIGN(4096) { KEEP(*(.reloc)) }
}
