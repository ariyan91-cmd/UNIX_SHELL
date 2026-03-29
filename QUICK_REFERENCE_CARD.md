# QUICK REFERENCE CARD - Shell Commands

## Color-Coded Quick Guide

### pwd - Print Working Directory
```
What it does:   Display current directory path
Syscall:        sys_pwd()
Code location:  user/shell/shell.c:162-170
Key steps:
  1. Call sys_pwd(buffer)
  2. Kernel retrieves process's cwd inode
  3. Convert inode to path string
  4. Return path to shell
  5. Print to stdout

Example:
  > pwd
  /home/user/documents
```

---

### mkdir - Make Directory
```
What it does:   Create one or more directories
Syscall:        sys_mkdir()
Code location:  user/shell/shell.c:420-432
Key steps:
  1. Validate argc >= 2
  2. For each argument:
     a. Call sys_mkdir(dirname)
     b. Kernel creates directory inode
     c. Add entry to parent directory
     d. If error, print message
  3. Return to shell

Example:
  > mkdir project docs tests
  (Creates 3 directories)
  
Internally:
  ├─ sys_mkdir("project")  → creates inode #521
  ├─ sys_mkdir("docs")     → creates inode #522
  └─ sys_mkdir("tests")    → creates inode #523
```

---

### touch - Create Empty File
```
What it does:   Create empty file (or update timestamp)
Syscall:        open() with O_CREATE flag
Code location:  user/shell/shell.c:617-635
Key steps:
  1. Validate argc >= 2
  2. For each filename:
     a. Try: fd = open(filename, O_RDONLY)
     b. If succeeds: file exists, skip it
     c. If fails: file doesn't exist
     d. Create: open(filename, O_CREATE)
     e. Kernel creates file inode
     f. close(fd)
  3. Return to shell

Example:
  > touch README.txt config.txt
  (Creates 2 empty files)
  
File descriptor flow:
  open() → kernel creates inode → returns fd → close()
```

---

### cd - Change Directory
```
What it does:   Change current working directory
Syscall:        sys_chdir()
Code location:  user/shell/shell.c:195-209
Key steps:
  1. Check argc:
     a. If argc==1: path = "/" (root)
     b. If argc>1: path = argv[1]
  2. Call sys_chdir(path)
     a. Kernel finds directory inode
     b. Validates it's a directory (not file)
     c. Updates process's cwd_inode
     d. Return 0 (success) or -1 (error)
  3. Check return value:
     a. If negative: print error
     b. If positive: continue

Example:
  > cd documents
  > pwd
  /home/user/documents
  
State change:
  Process's cwd_inode changed in kernel
  All future file operations use new cwd
```

---

## Execution Flow - Simple Diagram

```
TYPE COMMAND
    ↓
shell_readline()      ← Character-by-character input
    ↓
runcmd()              ← Tokenize into ["cmd", "arg1", "arg2"]
    ↓
parse_single_command()← Parse redirects (<, >, >>)
    ↓
Command table lookup  ← Find matching function
    ↓
Call function         ← shell_pwd() or shell_mkdir() etc.
    ↓
Function calls        ← sys_pwd(), sys_mkdir(), sys_chdir()
    ↓
Kernel syscall       ← Performs privileged operation
    ↓
Return to shell      ← Back to main loop
```

---

## Key Points to Remember

### 1. Command Table Pattern
```c
struct Command {
    const char* name;      // "pwd", "mkdir", etc.
    const char* desc;      // Help text
    int (*func)(...)       // Function pointer
};

// Enables dynamic command lookup without huge if-else
```

### 2. Syscall Mechanism
- **User space**: Shell calls sys_*()
- **Kernel space**: Kernel performs actual work
- **Security**: Only kernel can modify filesystem
- **Protection**: Prevents user code from corrupting data

### 3. Important Data Structures
- **File Descriptor**: Integer index to open file
  - FD 0 = stdin, FD 1 = stdout, FD 2 = stderr
  - FD 3+ = opened files
  
- **Inode**: Kernel's representation of file/directory
  - Stores type, permissions, owner, size, etc.
  - Contains directory entries for folders
  
- **Process CWD**: Current working directory (inode number)
  - Stored in kernel's process structure
  - Updated by cd command

### 4. Process State vs File System
```
pwd:   Reads process state (cwd) from kernel    [no changes]
mkdir: Changes filesystem (creates inode)       [filesystem changed]
touch: Changes filesystem (creates inode)       [filesystem changed]
cd:    Changes process state (cwd inode)        [process changed]
```

---

## Code Patterns Used

### Pattern 1: Loop Through Arguments
```c
for (int i = 1; i < argc; i++) {
    sys_mkdir(argv[i]);  // mkdir, touch do this
}
```

### Pattern 2: Check File Existence
```c
int fd = open(filename, O_RDONLY);
if (fd >= 0) {
    // File exists
    close(fd);
} else {
    // File doesn't exist
}
```

### Pattern 3: Handle Syscall Return Value
```c
int ret = sys_chdir(path);
if (ret < 0) {
    printf("Error: cannot change directory\n");
}
```

### Pattern 4: Command Dispatch
```c
for (int i = 0; i < NCOMMANDS; i++) {
    if (strcmp(cmd_name, commands[i].name) == 0) {
        commands[i].func(argc, argv);
        break;
    }
}
```

---

## Common Questions & Answers

**Q: Why does pwd need a syscall?**  
A: Current directory is stored in kernel memory (process structure). User code cannot access kernel memory directly.

**Q: Can I create a file when opening with O_RDONLY?**  
A: No! O_RDONLY is read-only. To create, use O_CREATE flag.

**Q: What happens if cd fails?**  
A: sys_chdir() returns -1 (error code). Shell prints error but continues. Current directory doesn't change.

**Q: Why does mkdir support multiple directories?**  
A: The loop allows batch creation. User convenience - one command, multiple results.

**Q: Where does the shell code execute?**  
A: User space (unprivileged). Syscalls switch to kernel space briefly for privileged operations.

**Q: Can files occupy just 0 bytes disk space?**  
A: Yes! Empty files created by touch have size 0 bytes.

**Q: What's the difference between directory and file inode?**  
A: File inode stores data blocks. Directory inode stores list of entries (name + inode pairs).

**Q: How does shell find commands?**  
A: Uses command table - array of Command structures. Searches table by name using strcmp().

---

## Comparison Table

| Aspect | pwd | mkdir | touch | cd |
|--------|-----|-------|-------|-----|
| **Reads kernel state** | ✓ | ✗ | ✗ | ✗ |
| **Modifies filesystem** | ✗ | ✓ | ✓ | ✗ |
| **Changes process state** | ✗ | ✗ | ✗ | ✓ |
| **Takes arguments** | ✗ | ✓ | ✓ | ✓ |
| **Supports multiple args** | N/A | ✓ | ✓ | ✗ |
| **Batch operation** | N/A | ✓ | ✓ | ✗ |
| **Uses loop internally** | ✗ | ✓ | ✓ | ✗ |
| **Error messages** | minimal | yes | yes | yes |

---

## File Locations

```
Project Root: /home/ariyan/Desktop/UNIX_SHELL/

Shell Source Code:
  └─ mcertikos/user/shell/shell.c
     ├─ main()              lines 1025-1100
     ├─ shell_readline()    lines 775-810
     ├─ runcmd()            lines 830-1000
     ├─ shell_pwd()         lines 162-170
     ├─ shell_mkdir()       lines 420-432
     ├─ shell_touch()       lines 617-635
     └─ shell_cd()          lines 195-209

Kernel Source Code:
  └─ mcertikos/kern/fs/sysfile.c
     ├─ sys_pwd()
     ├─ sys_mkdir()
     ├─ sys_chdir()
     └─ open() (for touch)

Documentation:
  ├─ README.md
  ├─ TEACHER_PRESENTATION_GUIDE.md
  ├─ VISUAL_DIAGRAMS.md
  └─ This file
```

---

## Presentation Tips

1. **Start with pwd**
   - Simplest command
   - Shows syscall basics
   - No state changes

2. **Then mkdir**
   - Shows filesystem modification
   - Demonstrate multiple arguments
   - Show inode creation

3. **Then touch**
   - Similar to mkdir
   - Shows file creation
   - Clever check pattern

4. **Finally cd**
   - Most interesting
   - Changes process state
   - Explain cwd inode

5. **Wrap up**
   - Show all 4 working together
   - Demonstrate complete workflow
   - Answer questions

---

## Running the Demo Script

```bash
# Make the script executable
chmod +x /home/ariyan/Desktop/UNIX_SHELL/COMMAND_EXPLANATION_SCRIPT.sh

# Run it
bash /home/ariyan/Desktop/UNIX_SHELL/COMMAND_EXPLANATION_SCRIPT.sh
```

The script will:
- ✓ Demonstrate each command
- ✓ Show before/after states
- ✓ Explain internal mechanisms
- ✓ Provide visual formatting
- ✓ Clean up demo directories

---

## Study Checklist

Before presenting to your teacher, make sure you understand:

- [ ] What pwd does and how it works
- [ ] What mkdir does and how loops through arguments
- [ ] What touch does and how it checks file existence
- [ ] What cd does and how it changes process state
- [ ] The difference between user space and kernel space
- [ ] What syscalls are and why they're needed
- [ ] How the command table works
- [ ] How tokenization works
- [ ] What a file descriptor is
- [ ] What an inode is
- [ ] The complete execution flow from input to output
- [ ] Why each command needs what it needs

---

## Good Luck! 🎯

You now have:
1. ✓ Detailed explanation script (COMMAND_EXPLANATION_SCRIPT.sh)
2. ✓ Teacher presentation guide (TEACHER_PRESENTATION_GUIDE.md)
3. ✓ Visual diagrams (VISUAL_DIAGRAMS.md)
4. ✓ This quick reference card

Present with confidence! You know this material well! 💪

