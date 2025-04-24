# Hex Compiler that Runs on UEFI

I will document this more later. This is really just a part of a larger project.

## Dependencies

All of the scripts in this repo were written on and for a Linux distro, so
you'll probably want to use one.

For building:

- `x86_64-w64-mingw32-gcc`

For testing (running in QEMU):

- `qemu-system-x86_64`
- Whatever is needed to run `mkfs.vfat`
- The QEMU / OVMF Shell, Vars, and Code. I don't remember where I downloaded these.

You might need to tweak my scripts for your system. Sorry this isn't more
thorough. It will be later.

## Setup

Run `./compile.sh` to compile `hexc.efi`.
Run `./test.sh` to put it in a tiny FAT file system, boot it in QEMU.

## Usage

In the UEFI shell, run

```
fs0:
```

to select the first filesystem, which I presume is where your FAT filesystem
shows up. This might be different for you.

Then run:

```
hexc.efi hello.hex out.bin
```

This takes the hex file at `hello.hex` and compiles it to `out.bin`.

Now you can run `out.bin`:

```
out.bin
```

If you see `Hello World!`, it worked.

## Tips

Use `echo %lasterror%` in the UEFI shell to see the last return code.

## To Do

- [ ] Labels
- [ ] Refactoring into functions
- [ ] Simplify `StrTok`
