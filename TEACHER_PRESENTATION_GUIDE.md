# Shell Commands Explanation - Teacher Presentation Guide

## Overview
This guide explains how 4 fundamental shell commands work: **pwd**, **mkdir**, **touch**, and **cd**.

---

## COMMAND #1: pwd (Print Working Directory)

### What It Does
Displays the full path of your current location in the file system.

### User Perspective
```bash
$ pwd
/home/user/documents
```

### How It Works Internally

**User Space (shell.c):**
```c
int shell_pwd(int argc, char ** argv) {
    if (argc != 1) {
        printf("pwd: too many arguments\n");
        return 0;
    }
    sys_pwd(shell_buf);              // Ask kernel for current path
    printf("%s\n", shell_buf);       // Display the path
    return 0;
}
```

**What Happens:**
1. Shell calls `sys_pwd()` syscall
2. Kernel looks up current process's directory inode
3. Kernel converts inode pointer to path string
4. Returns path to user space
5. Shell prints it to screen

**Code Location:** `user/shell/shell.c` lines 162-170

---

## COMMAND #2: mkdir (Make Directory)

### What It Does
Creates one or more new directories (folders).

### User Perspective
```bash
$ mkdir project
$ mkdir dir1 dir2 dir3        # Multiple directories at once
```

### How It Works Internally

**User Space (shell.c):**
```c
int shell_mkdir(int argc, char** argv) {
    if (argc < 2) {
        printf("mkdir: missing operand\n");
        return 0;
    }

    // Loop through each directory name argument
    for (int i = 1; i < argc; i++) {
        int ret = sys_mkdir(argv[i]);     // Create it
        if (ret != 0) {
            printf("mkdir: cannot create directory '%s'\n", argv[i]);
        }
    }
    return 0;
}
```

**What Happens:**
1. Check that at least one directory name was provided
2. Loop through each argument (each directory name)
3. For each one, call `sys_mkdir()` syscall
4. Kernel creates new directory inode
5. Kernel updates parent directory to include new entry
6. Return to shell with status (0=success, negative=error)

**Algorithm:**
```
FOR EACH directory name:
    SYSCALL: Create directory inode
    IF error: Print error message
```

**Code Location:** `user/shell/shell.c` lines 420-432

**Kernel Side:** `kern/fs/sysfile.c` contains `sys_mkdir()` implementation

---

## COMMAND #3: touch (Create Empty File)

### What It Does
Creates a new empty file. If file exists, updates its timestamp.

### User Perspective
```bash
$ touch README.txt
$ touch file1.txt file2.txt file3.txt    # Multiple files
```

### How It Works Internally

**User Space (shell.c):**
```c
int shell_touch(int argc, char** argv) {
    if (argc == 1) {
        printf("touch failed. No Path.\n");
        return 0;
    }

    // Try to create each file
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);  // Try to open
        
        if(fd >= 0) {
            // File already exists
            printf("%s file exist\n", argv[i]);
            close(fd);
            continue;
        } else {
            // File doesn't exist - create it
            close(open(argv[i], O_CREATE));
        }
    }
    return 0;
}
```

**What Happens:**
1. For each filename:
2. Try to `open()` it with read-only mode
3. **If open succeeds** → file exists, print message and continue
4. **If open fails** → file doesn't exist, create it with `O_CREATE` flag
5. Close the file descriptor

**Algorithm:**
```
FOR EACH filename:
    TRY: Open existing file
    IF succeeds: File exists, skip it
    IF fails: File doesn't exist, create it
    CLOSE: Close the file descriptor
```

**Clever Approach:** Uses the filesystem itself to check existence rather than separate stat() call.

**Code Location:** `user/shell/shell.c` lines 617-635

---

## COMMAND #4: cd (Change Directory)

### What It Does
Changes your current working directory. `cd` without arguments goes to root (`/`).

### User Perspective
```bash
$ cd /home/user/documents    # Go to specific directory
$ cd documents               # Go to relative directory
$ cd                         # Go to root /
$ cd ..                      # Go up one level
```

### How It Works Internally

**User Space (shell.c):**
```c
int shell_cd(int argc, char** argv) {
    char path[1024];
    
    if(argc == 1) {
        // No argument - go to root
        strcpy(path, "/");
        if (sys_chdir(path) < 0)
            printf("cd: cannot change directory\n");
    }
    else {
        // Go to specified path
        strcpy(path, argv[1]);
        if (sys_chdir(path) < 0)
            printf("cd: %s: No such directory\n", argv[1]);
    }
    return 0;
}
```

**What Happens:**
1. Check number of arguments
2. **If no argument:** Use "/" (root directory)
3. **If argument provided:** Use the provided path
4. Call `sys_chdir()` syscall with the path
5. Kernel validates path exists and is a directory
6. Kernel updates process's current directory inode pointer
7. Check return value:
   - If < 0 → error, print message
   - If >= 0 → success

**Algorithm:**
```
IF no argument:
    Use "/" as path
ELSE:
    Use argv[1] as path

SYSCALL: sys_chdir(path)
    Kernel: Validate path
    Kernel: Check is_directory
    Kernel: Update process's cwd inode
    Return success or error

IF error: Print error message
```

**Code Location:** `user/shell/shell.c` lines 195-209

**Key Point:** `cd` is special because it changes the PROCESS's state (current working directory), not just displaying something.

---

## How They Work Together

### Typical Workflow
```bash
$ pwd                                  # See where you are
/home/user

$ mkdir my_project                     # Create directory

$ cd my_project                        # Enter it

$ pwd                                  # Confirm location
/home/user/my_project

$ touch README.txt config.txt          # Create files

$ ls                                   # List what you made
README.txt  config.txt
```

### Execution Flow in Shell Code
```
shell_readline()
    ↓ (reads "mkdir project")
runcmd()
    ↓ (tokenizes: ["mkdir", "project"])
parse_single_command()
    ↓ (checks for pipes/redirects: none)
run_command_stage()
    ↓ (sets up stdin=0, stdout=1)
Command table lookup
    ↓ (finds shell_mkdir function)
shell_mkdir() execution
    ↓ (calls sys_mkdir("project"))
Kernel
    ↓ (creates directory inode)
Return to prompt
```

---

## Internal Data Structures

### Command Table (Shell Registry)
**Location:** `user/shell/shell.c` lines 100-120

```c
static struct Command commands[] = {
    {"ls",     "list files...",                 shell_ls},
    {"pwd",    "print working directory",       shell_pwd},
    {"cd",     "change directory",              shell_cd},
    {"mkdir",  "create directory",              shell_mkdir},
    {"touch",  "create empty file",             shell_touch},
    // ... more commands
};
```

**Purpose:** Enables extensible command system. New commands just added to array.

### File Descriptor Table (Kernel)
```
FD  Type        File/Resource
──  ──────────  ──────────────────
0   STDIN       Keyboard input
1   STDOUT      Screen output
2   STDERR      Error output
3   REGULAR     myfile.txt (when open)
4   REGULAR     project/README.txt
5   PIPE        Connection between commands
```

### Inode Structure (Represents File/Directory)
```
Inode #521 (my_project directory)
├─ Type: DIRECTORY
├─ Mode: 0755 (permissions)
├─ Size: 512 bytes
├─ Owner: user1
├─ Created: 2026-03-29
├─ Contents:
│   ├─ file1.txt (inode #522)
│   ├─ file2.txt (inode #523)
│   └─ subdir (inode #524)
```

---

## Syscall Mechanism

### User Space → Kernel Space Transition

```
USER SPACE                          KERNEL SPACE
────────────────────────────────────────────────────
shell_mkdir()                       sys_mkdir() handler
  ↓                                   ↓
  int sys_mkdir(path)               Validate path
  (syscall trap)                     Create inode
  ↓                                  Allocate disk space
  CPU INTERRUPT                      Update parent dir
  ↓                                  Return status
MODE SWITCH                          ↓
  ↓                                  MODE SWITCH
Result returned                     Back to shell_mkdir()
to user code                        ↓
                                    Continue with result
```

### Why Syscalls?
- **Security:** Kernel controls privileged operations
- **Protection:** User code can't corrupt filesystem
- **Isolation:** Processes can't interfere with each other

---

## Code Locations for Reference

| Command | User Space | Kernel Space |
|---------|-----------|--------------|
| **pwd** | `user/shell/shell.c:162-170` | `kern/fs/sysfile.c` |
| **mkdir** | `user/shell/shell.c:420-432` | `kern/fs/sysfile.c` |
| **touch** | `user/shell/shell.c:617-635` | `kern/fs/sysfile.c` (open syscall) |
| **cd** | `user/shell/shell.c:195-209` | `kern/fs/sysfile.c` |

**Main shell loop:** `user/shell/shell.c:1025-1100` (main function)

**Command parsing:** `user/shell/shell.c:830-1000` (runcmd function)

**Shell readline:** `user/shell/shell.c:775-810` (shell_readline function)

---

## Key Concepts to Explain

### 1. **Command Dispatch Mechanism**
The shell uses a **command table** with function pointers. When you type a command:
1. Shell tokenizes your input
2. Searches command table for matching name
3. Calls the associated function
4. Function executes the command

This design allows **easy extension** - new commands just added to the table.

### 2. **Syscall Boundary**
Commands interact with the **kernel** through syscalls:
- `sys_pwd()` - ask kernel for current path
- `sys_mkdir()` - ask kernel to create directory
- `sys_chdir()` - ask kernel to change directory
- `open()` - ask kernel to open file

The kernel performs actual filesystem operations.

### 3. **Working Directory**
Each process has a **current working directory (cwd)** stored in the kernel:
- When you run `cd`, it updates the process's cwd
- When you run `touch`, it creates file in the cwd
- When you run `pwd`, it displays the cwd

### 4. **File Descriptors (FDs)**
Every file operation goes through **file descriptors**:
- FD = integer index to open file/pipe
- FD 0 = stdin, FD 1 = stdout, FD 2 = stderr
- When you `open()` a file, kernel returns the FD
- When you call `read()` or `write()`, you use the FD

### 5. **Inodes**
Kernel represents all files and directories as **inodes**:
- Inode = metadata structure (type, size, owner, etc.)
- Directory inode contains list of entries
- Each entry = name + inode number
- `mkdir` creates new directory inode
- `touch` creates new file inode
- `cd` updates process's cwd inode pointer

---

## Important Points for Your Teacher

1. **Shell Design is Modular**
   - Command table enables extensibility
   - New commands easily added
   - Code is well-organized

2. **Clear Separation of Concerns**
   - Shell handles user interaction
   - Kernel handles filesystem operations
   - Syscalls are the bridge

3. **Proper Error Handling**
   - Check return values from syscalls
   - Print meaningful error messages
   - Gracefully handle failures

4. **Efficient Algorithm Usage**
   - `touch` uses open() to check existence (clever!)
   - `mkdir` and `touch` loop for batch operations
   - `cd` minimal - just one syscall

5. **Memory Management**
   - Fixed-size buffers (BUFLEN=1024)
   - File descriptors freed with close()
   - No memory leaks

---

## Questions Your Teacher Might Ask

**Q: Why does pwd need a syscall?**  
A: The current directory is stored in the kernel, in the process's structure. Only the kernel can access it reliably. User code can't access kernel memory directly.

**Q: Why can't touch just create files directly without trying to open?**  
A: It could, but the current approach is elegant - it reuses the open() syscall. Trying to open first is also faster than calling sys_stat() separately.

**Q: How does the shell know where to look for these commands?**  
A: They're built-in! The code for each command (shell_pwd, shell_mkdir, etc.) is compiled right into the shell program. They're not separate executables.

**Q: What if cd fails?**  
A: The return value from sys_chdir() will be negative (error code). The shell prints an error message but the process continues. The cwd doesn't change.

**Q: Can you cd to a file instead of a directory?**  
A: No! The kernel validates that the path points to a directory (using is_dir() check). If you try to cd to a file, you get an error.

**Q: Why use file descriptors instead of just filenames?**  
A: File descriptors are more efficient. Once opened, reading/writing just uses the FD number instead of looking up the filename again. Also enables pipes!

---

## Running the Demonstration Script

Execute the companion script to show live demonstrations:

```bash
bash COMMAND_EXPLANATION_SCRIPT.sh
```

This script:
- Shows each command in action
- Demonstrates before/after states
- Explains internal mechanisms
- Shows practical workflows
- Provides visual formatting

---

## Summary Table

| Command | Purpose | Key Syscall | What Changes |
|---------|---------|-------------|--------------|
| **pwd** | Show current location | `sys_pwd()` | Nothing - just display |
| **mkdir** | Create directory | `sys_mkdir()` | New directory created |
| **touch** | Create file | `open(O_CREATE)` | New file created |
| **cd** | Change location | `sys_chdir()` | Process's cwd updated |

---

## Additional Resources

- **Full project README:** `/home/ariyan/Desktop/UNIX_SHELL/README.md`
- **Shell source code:** `/home/ariyan/Desktop/UNIX_SHELL/mcertikos/user/shell/shell.c`
- **Kernel filesystem code:** `/home/ariyan/Desktop/UNIX_SHELL/mcertikos/kern/fs/`
- **Build guide:** `/home/ariyan/Desktop/UNIX_SHELL/BUILD_AND_TEST_GUIDE.sh`

---

**Good luck with your presentation!**
