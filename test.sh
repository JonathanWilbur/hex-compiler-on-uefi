#!/bin/sh

set -e

dd if=/dev/zero of=fat.img bs=1M count=64
mkfs.vfat fat.img
sudo mkdir -p /mnt/fat
sudo mount -o loop fat.img /mnt/fat
sudo mkdir -p /mnt/fat/EFI/BOOT
sudo cp hexc.efi /mnt/fat/hexc.efi
sudo cp hello.hex /mnt/fat/hello.hex
sudo cp /usr/share/OVMF/DEBUGX64_Shell.efi /mnt/fat/EFI/BOOT/BOOTX64.EFI
sudo umount /mnt/fat
sudo qemu-system-x86_64 -m 1024 \
  -bios /usr/share/OVMF/DEBUGX64_OVMF.fd \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/DEBUGX64_OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=/usr/share/OVMF/DEBUGX64_OVMF_VARS.fd \
  -drive file=fat.img,if=virtio,format=raw
