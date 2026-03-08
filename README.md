# UNIX Shell Project (mCertiKOS)

A teaching operating-systems project that extends mCertiKOS with a more UNIX-like shell, unified file-descriptor I/O, pipe support, redirection, and process I/O channel wiring.

This repository contains:
- the kernel and user-space source (`mcertikos/`)
- project helper scripts (for build/test/image generation)

## Overview

This project evolves a basic shell into a more realistic one by implementing:
- file descriptor abstraction for regular files and pipes
- unified `read`/`write` based I/O path
- `pipe()` syscall and kernel pipe backend
- shell parsing for redirection and pipelines
- user command set for file-system and process interaction

## Main Features Implemented

### 1. File Descriptor Abstraction
- `FD_INODE` for files/directories
- `FD_PIPE` for pipes
- unified operations through `file_read`, `file_write`, `file_close`

Key files:
- `mcertikos/kern/fs/file.h`
- `mcertikos/kern/fs/file.c`

### 2. Pipe Support
- syscall: `SYS_pipe` / `sys_pipe()`
- fixed-size circular pipe buffer
- read/write/close operations for both ends

Key files:
- `mcertikos/kern/fs/pipe.h`
- `mcertikos/kern/fs/pipe.c`
- `mcertikos/kern/fs/sysfile.c`
- `mcertikos/kern/lib/syscall.h`
- `mcertikos/kern/trap/TDispatch/TDispatch.c`

### 3. Unified Read/Write I/O
- shell and user programs use `read(fd, ...)` and `write(fd, ...)`
- kernel validates user buffers and routes I/O to file/pipe handlers

Key files:
- `mcertikos/kern/fs/sysfile.c`
- `mcertikos/user/include/syscall.h`
- `mcertikos/user/include/file.h`

### 4. Shell Improvements
- command parsing with support for:
  - input redirection: `<`
  - output redirection: `>` and `>>`
  - pipelines: `|`
- visible line input editing behavior (echo + Enter handling)
- built-in command execution and selected fd-aware execution path

Key file:
- `mcertikos/user/shell/shell.c`

### 5. Spawn I/O Channel Wiring
- process spawn path supports explicit input/output fd channels
- wrapper `spawn_io(...)` added at user library layer

Key files:
- `mcertikos/user/include/proc.h`
- `mcertikos/user/lib/proc.c`
- `mcertikos/user/include/syscall.h`
- `mcertikos/kern/trap/TSyscall/TSyscall.c`

## Shell Command Reference

Implemented built-ins in current shell:

- `help`
  - show command descriptions
- `ls [path]`
  - list directory entries
- `pwd`
  - print current working directory
- `cd [path]`
  - change directory (`cd` alone goes to `/`)
- `mkdir <dir> [dir ...]`
  - create one or multiple directories
- `touch <file> [file ...]`
  - create empty file(s)
- `echo <text...>`
  - print text (works with redirection/pipeline in shell parser)
- `cat [-n|-b] [file ...]`
  - print/concatenate files or stdin
- `cp [-r] <src> <dst>`
  - copy file or directory (`-r` for recursive)
- `mv <src> <dst>`
  - move/rename files or directories
- `rm [-r] <path> [path ...]`
  - remove file(s), recursive directory removal with `-r`
- `write <text> <file>`
  - write a string to file
- `append <text> <file>`
  - append string to file
- `rot13 [file ...]`
  - transform stream or files with ROT13
- `spawn <elf_id>`
  - create process from predefined user binaries
- `kill <signal> <pid>`
  - send signal to process
- `trap <signum>`
  - register signal handler for current shell process

## Build and Run

## Prerequisites
- Linux environment with `gcc`, `ld`, `make`, Python 3
- QEMU (`qemu-system-x86_64` used by Makefile target)
- toolchain compatible with this mCertiKOS setup (`misc/gccprefix.sh` auto-detects)

### Build
From repository root:

```bash
cd mcertikos
make clean
make -j2
```

This compiles kernel + user-space and generates disk images via `make_image.py`.

### Run (QEMU target from Makefile)

```bash
cd mcertikos
make qemu
```

For non-graphical run:

```bash
make qemu-nox
```

## Quick Functional Test Plan

Inside the shell prompt:

```sh
help
pwd
ls

mkdir demo
cd demo
pwd

echo hello > a.txt
cat a.txt
echo world >> a.txt
cat -n a.txt

cp a.txt b.txt
cat b.txt

mv b.txt c.txt
ls
rm c.txt
ls

echo abcXYZ | rot13
cat < a.txt | rot13 > enc.txt
cat enc.txt
cat enc.txt | rot13 > dec.txt
cat dec.txt

cd ..
rm -r demo
ls
```

Expected:
- path changes correctly with `cd`/`pwd`
- redirection and append work
- copy/move/remove operations behave correctly
- pipeline and transform (`rot13`) work through shell parser

## Code Layout

- `mcertikos/kern/`
  - kernel subsystems (trap, fs, proc, devices, memory)
- `mcertikos/user/`
  - user programs and shell implementation
- `mcertikos/boot/`
  - bootloader stages
- `mcertikos/newfs/`
  - filesystem image resources
- `BUILD_AND_TEST_GUIDE.sh`
  - helper notes/script output for manual test guidance

## Notes and Limitations

- This is a teaching OS shell, not full POSIX shell behavior.
- Pipeline execution is implemented in shell command staging, not full job-control/fork model.
- Some syscalls and helpers are project-specific and may differ from Linux behavior.
- Signal and process behavior depends on mCertiKOS runtime constraints.

## Troubleshooting

### Shell accepts input but command does not run
- Ensure Enter key handling is correct in current image (rebuild/reboot after code change).
- Rebuild from clean state:

```bash
cd mcertikos
make clean
make -j2
make qemu
```

### `pwd` or directory commands behave unexpectedly
- Confirm newest kernel image is booted after rebuild.
- Use minimal test sequence: `pwd`, `mkdir t`, `cd t`, `pwd`, `cd /`, `pwd`.

### Command seems missing
- Run `help` to list built-ins compiled into `user/shell/shell.c`.

## License / Academic Use

This project is part of an operating systems coursework-style codebase. Use and sharing should follow your course/institution policy.
