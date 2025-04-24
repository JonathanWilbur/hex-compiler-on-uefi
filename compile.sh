#!/bin/sh

set -e

# TODO: Could these be combined into a single command?
x86_64-w64-mingw32-gcc -ffreestanding -I/usr/include/efi/x86_64 -I/usr/include/efi -fno-stack-protector \
    -fpic -fshort-wchar -mno-red-zone -Wall -c hexc.c -o hexc.o

x86_64-w64-mingw32-gcc -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o hexc.efi hexc.o
