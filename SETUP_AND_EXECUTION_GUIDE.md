# Setup and Execution Guide for mCertiKOS UNIX Shell Project

## Table of Contents

1. [Required Software & Dependencies](#required-software--dependencies)
2. [System Requirements](#system-requirements)
3. [Step-by-Step Setup Instructions](#step-by-step-setup-instructions)
4. [Building the Project](#building-the-project)
5. [Running the Project](#running-the-project)
6. [Verification & Testing](#verification--testing)
7. [Troubleshooting](#troubleshooting)
8. [Additional Resources](#additional-resources)

---

## Required Software & Dependencies

### Essential Tools

1. **C Compiler Toolchain** (32-bit x86 target)
   - GCC cross-compiler: `gcc` (with 32-bit support)
   - GNU Binutils: `ld`, `objdump`, `objcopy`, `nm`
   - Recommendation: Install `i386-elf` cross-compiler or `pios-gcc`

2. **Build System**
   - GNU Make (version 3.81 or later)

3. **Virtualization/Emulator**
   - QEMU x86_64 emulator (`qemu-system-x86_64`)
   - Optional: Bochs emulator for advanced debugging
   - Optional: GDB debugger for debugging kernel code

4. **Disk Image Tools**
   - `parted` - partition editor for creating disk images
   - `dd` - Unix disk utility (usually pre-installed)
   - `fdisk` - partition table manipulator

5. **Python 3**
   - Python 3.6+ for running `make_image.py` build script

6. **Additional Utilities**
   - `perl` - Perl interpreter for build scripts
   - `grub-mkrescue` - (optional) for creating ISO images
   - `cscope` - (optional) for source code browsing

### Linux/macOS Installation Examples

#### Ubuntu/Debian

```bash
# Install GCC cross-compiler and build tools
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    gcc-multilib \
    g++-multilib \
    binutils \
    perl \
    python3 \
    qemu-system-x86 \
    parted \
    gdb

# Install GCC i386-elf cross-compiler (if not available)
sudo apt-get install -y gcc-i386-linux-gnu binutils-i386-linux-gnu

# Verify installations
gcc --version
make --version
qemu-system-x86_64 --version
python3 --version
```

#### macOS (Homebrew)

```bash
# Install build tools
brew install gcc make binutils qemu parted python3

# Install cross-compiler if needed
# Follow instructions at: https://wiki.osdev.org/GCC_Cross-Compiler
```

#### Windows (WSL2 Recommended)

```bash
# Use WSL2 (Windows Subsystem for Linux 2) with Ubuntu
# Then follow Ubuntu installation steps above
```

### Verify Installation

```bash
# Check GCC version
gcc --version

# Check Make version
make --version

# Check Python version
python3 --version

# Check QEMU
which qemu-system-x86_64
qemu-system-x86_64 --version

# Check required tools
which perl
which parted
which dd
which fdisk
```

---

## System Requirements

### Minimum System Specifications

- **OS**: Linux (Ubuntu 18.04+), macOS 10.13+, or Windows with WSL2
- **CPU**: Any x86_64 processor (Intel or AMD)
- **RAM**: 2 GB minimum (4 GB recommended for smooth emulation)
- **Disk Space**: 500 MB for source code and build artifacts
- **Network**: Internet connection for installing dependencies (one-time setup)

### Supported Environments

- Linux (RECOMMENDED): Ubuntu 18.04 LTS, 20.04 LTS, 22.04 LTS; Debian 10+; Fedora 32+
- macOS: 10.13 (High Sierra) and later
- Windows: Windows 10/11 with WSL2 (using Linux subsystem)

---

## Step-by-Step Setup Instructions

### Step 1: Clone or Obtain the Project

If cloning from repository:

```bash
git clone https://github.com/ariyan91-cmd/UNIX_SHELL.git
cd UNIX_SHELL
```

Or, if you have the project directory already:

```bash
cd /path/to/UNIX_SHELL
```

### Step 2: Verify Project Structure

Ensure the project has the following directory structure:

```
UNIX_SHELL/
├── README.md
├── VISUAL_DIAGRAMS.md
├── SETUP_AND_EXECUTION_GUIDE.md
├── mcertikos/
│   ├── Makefile
│   ├── make_image.py
│   ├── boot/          # Bootloader code
│   ├── kern/          # Kernel source code
│   ├── user/          # User-space programs and shell
│   ├── misc/          # Build utilities
│   ├── newfs/         # Pre-made file system image
│   └── samples/       # Sample code
├── presentation_slides/
└── ...
```

### Step 3: Install Dependencies

**For Ubuntu/Debian Users:**

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    gcc-multilib \
    binutils \
    perl \
    python3 \
    qemu-system-x86 \
    parted \
    gdb \
    cscope
```

**For macOS Users:**

```bash
brew install gcc make binutils qemu parted python3 perl
```

**For Windows Users (WSL2):**

1. Enable WSL2 in Windows Settings
2. Install Ubuntu from Microsoft Store
3. Run the Ubuntu installation commands above

### Step 4: Configure the Cross-Compiler (if needed)

The build system automatically detects the cross-compiler. If it fails, manually set the compiler:

```bash
cd mcertikos/

# Check available toolchains
sh misc/gccprefix.sh

# If auto-detection fails, set GCCPREFIX manually
export GCCPREFIX=i386-elf-
# or
export GCCPREFIX=i386-linux-gnu-
```

To verify the compiler works:

```bash
# Test 32-bit compilation
${GCCPREFIX}gcc --version
${GCCPREFIX}ld --version
```

### Step 5: Navigate to Project Directory

```bash
cd mcertikos/
```

All subsequent commands assume you are in the `mcertikos/` directory.

---

## Building the Project

### Build All Components

**Complete build (recommended for first time):**

```bash
make clean        # Remove old build artifacts (optional on first run)
make all          # Build bootloader, kernel, user programs, and create disk image
```

This command will:

1. Compile bootloader (`boot/`)
2. Compile kernel (`kern/`)
3. Compile user-space programs including shell (`user/`)
4. Link all components
5. Generate the disk image (`certikos.img`)

**Expected build time**: 30 seconds to 2 minutes (depending on system)

### Build Individual Components

If you only modify specific parts:

```bash
# Build only bootloader
make boot

# Build only kernel
make kern

# Build only user programs
make user

# Build and link
make link

# Create disk image (after building)
python3 make_image.py
```

### Build Output

Successful compilation should show:

```
Building Certikos Image...
creating disk...
done.
writing mbr...
done.
copying kernel files...
kernel starts at sector XXXX
All done.
```

**Build artifacts** are created in the `obj/` directory:

```
obj/
├── boot/          # boot0, boot1, boot2, loader
├── kern/          # kernel (final kernel executable)
└── user/          # user-space binaries
```

**Final disk image**: `certikos.img` (created in project root)

---

## Running the Project

### Option 1: Run with QEMU (Recommended)

**Basic execution (without graphics):**

```bash
make qemu-nox
```

This starts the kernel in QEMU with:

- 4 virtual CPUs
- 2 GB RAM
- Serial console output
- 64 MB primary disk (bootable)
- Secondary disk for file system

**Expected output**:

```
[Boot message...]
[Kernel initialization messages...]
CertiKOS>
```

### Option 2: Run with QEMU GUI

**Graphical mode** (if X11/display available):

```bash
make qemu
```

### Option 3: Run with GDB Debugging

**For kernel debugging with breakpoints:**

Terminal 1 - Start QEMU with GDB server:

```bash
make qemu-nox-gdb
```

Terminal 2 - Connect GDB debugger:

```bash
make gdb
```

Inside GDB:

```
(gdb) break main
(gdb) continue
(gdb) step
(gdb) info registers
(gdb) quit
```

### Option 4: KVM Acceleration (Linux only)

For faster execution on Linux with KVM support:

```bash
make qemu-kvm
```

### Exit QEMU

To exit QEMU, press:

- **Ctrl+a**, then **x** (in `-nographic` mode)
- Or close the window (in GUI mode)

---

## Verification & Testing

### Immediate Verification (Post-Boot)

Once the kernel boots and shows the prompt (`CertiKOS> `), the system is running correctly.

### Test Case 1: Basic Commands

Execute these commands to verify basic functionality:

```bash
CertiKOS> pwd
/

CertiKOS> ls
.
..
bin
boot
dev
etc
lib
mnt
proc
root
sys
tmp
usr
var
home
```

**Expected result**: Directory listing should appear

---

### Test Case 2: File Operations

**Create and verify files:**

```bash
CertiKOS> mkdir mydir
CertiKOS> cd mydir
CertiKOS> pwd
/mydir

CertiKOS> touch myfile.txt
CertiKOS> ls
.
..
myfile.txt

CertiKOS> echo "Hello, mCertiKOS!" > myfile.txt
CertiKOS> cat myfile.txt
Hello, mCertiKOS!
```

**Expected output**:

- Directory created successfully
- File created and written to
- Content readable via `cat`

---

### Test Case 3: Pipe Operations

**Test command piping:**

```bash
CertiKOS> echo "one
two
three" | cat
one
two
three

CertiKOS> ls | cat
.
..
[directory listing]
```

**Expected result**: Commands connected via pipes execute correctly

---

### Test Case 4: Redirection

**Test input/output redirection:**

```bash
CertiKOS> echo "Test data" > output.txt
CertiKOS> cat output.txt
Test data

CertiKOS> cat output.txt > copy.txt
CertiKOS> cat copy.txt
Test data
```

**Expected result**: Files created and data redirected correctly

---

### Test Case 5: File Manipulation

**Test copy, move, and remove:**

```bash
CertiKOS> cp output.txt backup.txt
CertiKOS> ls
.
..
output.txt
copy.txt
backup.txt

CertiKOS> mv backup.txt renamed.txt
CertiKOS> rm output.txt copy.txt
CertiKOS> ls
.
..
renamed.txt
```

**Expected result**: Files copied, moved, and deleted as expected

---

### Test Case 6: Multi-Level Directories

```bash
CertiKOS> mkdir -p a/b/c
CertiKOS> cd a/b/c
CertiKOS> touch deep_file.txt
CertiKOS> pwd
/a/b/c

CertiKOS> ls
.
..
deep_file.txt
```

**Expected result**: Nested directories created and file operations work correctly

---

### Comprehensive Test Script

For automated testing, create `test.sh` in the project:

```bash
#!/bin/bash

# mCertiKOS Automated Test Script
# This script tests core shell functionality

echo "=== Test 1: Directory Navigation ==="
pwd
ls
mkdir testdir
cd testdir
pwd
cd ..

echo "=== Test 2: File Creation ==="
touch test1.txt test2.txt
ls

echo "=== Test 3: File Writing ==="
echo "Line 1" > test1.txt
echo "Line 2" >> test1.txt

echo "=== Test 4: File Reading ==="
cat test1.txt

echo "=== Test 5: Piping ==="
echo -e "one\ntwo\nthree" | cat

echo "=== Test 6: Redirection ==="
cat test1.txt > test1_copy.txt
cat test1_copy.txt

echo "=== Test 7: Cleanup ==="
rm test1.txt test2.txt test1_copy.txt
rmdir testdir

echo "=== All Tests Completed ==="
```

---

## Troubleshooting

### Problem 1: Cross-Compiler Not Found

**Error message**:

```
GCCPREFIX detection failed
```

**Solutions**:

1. Install cross-compiler:

   ```bash
   sudo apt-get install gcc-i386-linux-gnu binutils-i386-linux-gnu
   ```

2. Manually specify compiler:

   ```bash
   export GCCPREFIX=i386-linux-gnu-
   make clean
   make all
   ```

3. Check available compilers:
   ```bash
   which i386-elf-gcc
   which i386-linux-gnu-gcc
   ```

---

### Problem 2: QEMU Not Installed

**Error message**:

```
command not found: qemu-system-x86_64
```

**Solution**:

```bash
# Ubuntu/Debian
sudo apt-get install qemu-system-x86

# macOS
brew install qemu

# Verify installation
qemu-system-x86_64 --version
```

---

### Problem 3: Disk Image Creation Fails

**Error message**:

```
parted: command not found
```

**Solution**:

```bash
# Ubuntu/Debian
sudo apt-get install parted

# macOS
brew install parted
```

---

### Problem 4: Build Compilation Errors

**Error message**:

```
undefined reference to 'function_name'
```

**Solutions**:

1. Ensure build is clean:

   ```bash
   make clean
   make all
   ```

2. Check for incomplete file modifications:

   ```bash
   git status
   git diff
   ```

3. Verify all source files are present:
   ```bash
   ls -la kern/*.[ch]
   ls -la user/*.[ch]
   ```

---

### Problem 5: QEMU Crashes on Startup

**Error message**:

```
Could not boot from hard disk
```

**Solutions**:

1. Ensure disk image is built:

   ```bash
   ls -la certikos.img
   ```

2. Rebuild disk image:

   ```bash
   make clean
   python3 make_image.py
   ```

3. Use MBR boot (default):
   ```bash
   # Ensure bootloader is properly installed
   make boot
   ```

---

### Problem 6: Shell Prompt Not Appearing

**Symptom**: System hangs after boot

**Solutions**:

1. Check boot messages (may need scrollback)
2. Restart QEMU with increased verbosity:

   ```bash
   qemu-system-x86_64 -d int,cpu_reset -hda certikos.img
   ```

3. Check kernel compilation:
   ```bash
   make clean kern
   ```

---

### Problem 7: Commands Not Executing in Shell

**Symptom**: Shell prompt appears but commands don't execute

**Solutions**:

1. Verify user-space programs are compiled:

   ```bash
   ls -la obj/user/
   ```

2. Rebuild user space:

   ```bash
   make user
   make all
   ```

3. Check `/bin` directory exists:
   ```
   CertiKOS> ls /bin
   ```

---

### Problem 8: File Operations Not Working

**Symptom**: Can't create files or directories

**Likely cause**: File system not properly initialized

**Solution**:

```bash
# Restore clean file system
make mkfs

# Rebuild and run
make all
make qemu-nox
```

---

## Additional Resources

### Documentation Files

- [README.md](README.md) - Project overview and feature list
- [VISUAL_DIAGRAMS.md](VISUAL_DIAGRAMS.md) - Command flow explanations and diagrams
- Source code documentation in individual source files

### Key Source Files to Explore

**Shell Implementation**:

- `user/shell/shell.c` - Main shell interpreter with command parsing

**File System**:

- `kern/fs/file.c/.h` - File descriptor abstraction
- `kern/fs/pipe.c/.h` - Pipe implementation
- `kern/fs/sysfile.c` - File system syscalls

**Kernel Components**:

- `kern/init/init.c` - Kernel initialization
- `kern/trap/` - Interrupt handling
- `kern/dev/` - Device drivers

**User Space**:

- `user/lib/` - User-space library functions
- `user/include/` - User-space header files
- `user/shell/` - Shell program

### External Resources

- **QEMU Documentation**: https://wiki.qemu.org/Documentation
- **GDB Documentation**: https://sourceware.org/gdb/documentation/
- **x86 Assembly**: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
- **C Programming**: https://en.cppreference.com/w/c

### Getting Help

If you encounter issues not covered here:

1. **Check for common issues** in this guide's Troubleshooting section
2. **Review build logs** for specific error messages:
   ```bash
   make all 2>&1 | tee build.log
   ```
3. **Inspect kernel output** with verbose debugging:
   ```bash
   make qemu-nox | tee qemu.log
   ```
4. **Use GDB for debugging**:
   ```bash
   make qemu-nox-gdb
   # In a separate terminal:
   make gdb
   ```

---

## Quick Reference

### Build Commands

| Command      | Purpose                                |
| ------------ | -------------------------------------- |
| `make all`   | Build everything and create disk image |
| `make clean` | Remove all build artifacts             |
| `make boot`  | Build only bootloader                  |
| `make kern`  | Build only kernel                      |
| `make user`  | Build only user-space programs         |

### Execution Commands

| Command             | Purpose                                                          |
| ------------------- | ---------------------------------------------------------------- |
| `make qemu-nox`     | Run in emulator (no graphics)                                    |
| `make qemu`         | Run in emulator with graphics                                    |
| `make qemu-nox-gdb` | Run with GDB debugging enabled                                   |
| `make qemu-kvm`     | Run with KVM acceleration (Linux)                                |
| `make gdb`          | Connect GDB debugger (requires qemu-nox-gdb in another terminal) |

### Useful Shell Commands

| Command          | Usage                           |
| ---------------- | ------------------------------- |
| `pwd`            | Print current working directory |
| `ls [path]`      | List directory contents         |
| `cd [path]`      | Change directory                |
| `mkdir <dir>`    | Create directory                |
| `touch <file>`   | Create empty file               |
| `cat <file>`     | Display file contents           |
| `echo <text>`    | Print text                      |
| `cp <src> <dst>` | Copy files                      |
| `mv <src> <dst>` | Move/rename files               |
| `rm <path>`      | Remove files                    |

---

## Summary

This guide provides complete instructions for setting up and running the mCertiKOS UNIX Shell project. Follow the step-by-step instructions to:

1. ✓ Install all required dependencies
2. ✓ Download/locate the project
3. ✓ Configure the build environment
4. ✓ Build all components
5. ✓ Run the kernel in QEMU
6. ✓ Test shell functionality
7. ✓ Troubleshoot any issues

The project should now be fully operational on your machine!

---

**Last Updated**: April 2026  
**Project Repository**: https://github.com/ariyan91-cmd/UNIX_SHELL  
**License**: Refer to project LICENSE file
