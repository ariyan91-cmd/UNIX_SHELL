# DETAILED WALKTHROUGH - Study Guide for Teacher Presentation

## Table of Contents
1. [pwd - Print Working Directory](#pwd)
2. [mkdir - Make Directory](#mkdir)
3. [touch - Create Empty File](#touch)
4. [cd - Change Directory](#cd)
5. [Complete Workflow](#workflow)
6. [Practice Questions](#questions)

---

## pwd - Print Working Directory

### What It Does
`pwd` displays the full path of your current location in the file system.

### User Perspective
```bash
$ pwd
/home/ariyan/Desktop/UNIX_SHELL
```

### Internal Implementation

#### Step-by-Step Code Execution

**File:** `user/shell/shell.c:162-170`

```c
int shell_pwd(int argc, char ** argv)
{
    if (argc != 1) {
        printf("pwd: too many arguments\n");
        return 0;
    }
    sys_pwd(shell_buf);
    printf("%s\n",shell_buf);
    return 0;
}
```

#### Execution Trace

1. **Input Validation**
   - Check `argc == 1` (pwd takes NO arguments)
   - If user typed `pwd something` → argc would be 2
   - Print error and return

2. **Syscall**
   - Call `sys_pwd(shell_buf)`
   - `shell_buf` is allocated buffer of size BUFLEN (1024 bytes)
   - Kernel fills this buffer with current directory path

3. **Kernel Operation**
   - Kernel looks up current process's **cwd inode**
   - cwd = "current working directory"
   - cwd inode is stored in process structure in kernel
   - Kernel converts inode number to path string
   - Copies path string to user-provided buffer

4. **Output**
   - Use printf() to print the buffer
   - Output goes to stdout (fd=1)

#### Key Concept: What is cwd?

Every process has a "current working directory" (cwd) stored in kernel:

```
Process Structure in Kernel:
┌─────────────────────────┐
│ PID: 42                 │
│ cwd_inode: #100         │ ← This is pwd's result!
│ uid: 1000               │
│ gid: 1000               │
│ ... other fields ...    │
└─────────────────────────┘

Inode #100 represents: /home/ariyan/Desktop/UNIX_SHELL
```

When `pwd` calls `sys_pwd()`, kernel:
1. Looks at current process's `cwd_inode` field
2. Finds inode #100
3. Converts it to path string
4. Returns the path

#### Why Does pwd Need a Syscall?

- **Security:** Current directory is confidential kernel data
- **Reliability:** Only kernel can guarantee accuracy
- **Isolation:** User code cannot access kernel memory

If allowed user space to read cwd directly:
- User code could corrupt the data
- Processes could see each other's private data
- Security vulnerability!

#### Memory Layout

```
Before calling pwd:
┌──────────────────────────────┐
│ shell_buf (uninitialized)    │
│ size: 1024 bytes             │
│ [garbage data still here]    │
└──────────────────────────────┘

sys_pwd(shell_buf) execution:
┌──────────────────────────────┐
│ Kernel looks at cwd_inode    │
│ Converts to path:            │
│ "/home/ariyan/Desktop/..."   │
│ Copies to shell_buf          │
└──────────────────────────────┘

After calling pwd:
┌──────────────────────────────┐
│ shell_buf (now contains path)│
│ "/home/ariyan/Desktop/UNIX_S │
│ HELL\0"                      │
└──────────────────────────────┘

printf() displays it
```

---

## mkdir - Make Directory

### What It Does
Creates one or more new directories (folders) at specified paths.

### User Perspective
```bash
$ mkdir project
$ mkdir docs tests configs     # Multiple directories
$ mkdir a b c                  # Or like this
```

### Internal Implementation

#### Step-by-Step Code Execution

**File:** `user/shell/shell.c:420-432`

```c
int shell_mkdir(int argc, char** argv) {
    if (argc < 2) {
        printf("mkdir: missing operand\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        int ret = sys_mkdir(argv[i]);
        if (ret != 0) {
            printf("mkdir: cannot create directory '%s'\n", argv[i]);
        }
    }

    return 0;
}
```

#### Detailed Execution

**Scenario:** User types: `mkdir project docs`

1. **Input Parsing**
   - Input: `"mkdir project docs"`
   - Tokenized: `["mkdir", "project", "docs"]`
   - `argc = 3`, `argv = ["mkdir", "project", "docs", NULL]`

2. **Validation**
   ```c
   if (argc < 2) {  // argc=3, so not < 2
       printf("mkdir: missing operand\n");
       return 0;
   }
   ```
   - Check passes ✓

3. **Loop - First Iteration (i=1)**
   ```c
   for (int i = 1; i < argc; i++)  // i=1, argc=3
   // Loop runs? 1 < 3? YES
   ```
   - `argv[1] = "project"`
   - Call: `ret = sys_mkdir("project")`
   
   **Kernel Operation:**
   ```
   sys_mkdir("project") {
       1. Validate path doesn't exist
       2. Validate parent directory valid
       3. Create new directory inode
       4. Allocate inode number (e.g., #521)
       5. Add entry to parent directory:
          "project\0" → inode #521
       6. Return 0 (success)
   }
   ```
   
   - `ret = 0` (success)
   - Check: `if (ret != 0)` → FALSE, don't print error

4. **Loop - Second Iteration (i=2)**
   ```c
   for (int i = 1; i < argc; i++)  // i=2, argc=3
   // Loop runs? 2 < 3? YES
   ```
   - `argv[2] = "docs"`
   - Call: `ret = sys_mkdir("docs")`
   
   **Kernel Operation:**
   - Similar to above
   - Create new inode #522 for "docs"
   - Add entry to parent directory
   - Return 0 (success)

5. **Loop - Check Exit Condition**
   ```c
   for (int i = 1; i < argc; i++)  // i=3, argc=3
   // Loop runs? 3 < 3? NO
   // Exit loop
   ```

6. **Return**
   ```c
   return 0;  // Success
   ```

#### Algorithm Pseudocode

```
FUNCTION shell_mkdir(argc, argv):
    IF argc < 2:
        Print error "missing operand"
        Return
    
    FOR i = 1 TO argc-1:
        dirpath = argv[i]
        result = SYSCALL sys_mkdir(dirpath)
        
        IF result != 0:  // 0 = success, negative = error
            Print error message
    
    Return 0
```

#### What's Inside an Inode?

When kernel creates inode #521 for "project":

```c
struct Inode {
    int type;              // DIR (2) vs REG (1)
    int nlinks;            // Reference count
    int size;              // Number of entries (for dir)
    int owner;             // User ID
    int permissions;       // Mode: 0755
    struct TimeStamp {
        int created;       // Creation time
        int modified;      // Last modification
        int accessed;      // Last access
    } time;
    struct DiskBlock {
        int blocks[12];    // Block numbers on disk
    } disk;
};
```

#### Directory Entry Structure

When "project" is added to parent directory:

```
Parent Directory (inode #100):
┌─────────────────────────────────────┐
│ Entry #0:                           │
│   name: "."       → inode: #100     │
│                                     │
│ Entry #1:                           │
│   name: ".."      → inode: #0       │
│                                     │
│ Entry #2:                           │
│   name: "project" → inode: #521     │
│                                     │
│ Entry #3:                           │
│   name: "docs"    → inode: #522     │
└─────────────────────────────────────┘
```

#### Creating Multiple Directories

The loop enables **batch operation**:

```
Command: mkdir a b c d e

Loop iteration 1: sys_mkdir("a")  → inode #521
Loop iteration 2: sys_mkdir("b")  → inode #522
Loop iteration 3: sys_mkdir("c")  → inode #523
Loop iteration 4: sys_mkdir("d")  → inode #524
Loop iteration 5: sys_mkdir("e")  → inode #525

Result: Five directories created with one command
```

#### Error Handling

What if one directory creation fails?

```
Command: mkdir a existing_dir c
         (existing_dir already exists)

Loop iteration 1: sys_mkdir("a")  → 0 (success)
Loop iteration 2: sys_mkdir("existing_dir") → -1 (error)
                  if (ret != 0):  TRUE
                  Print: "mkdir: cannot create directory 'existing_dir'"
Loop iteration 3: sys_mkdir("c")  → 0 (success)

Result: Three attempts, one fails, others succeed
```

---

## touch - Create Empty File

### What It Does
Creates a new empty file (size 0 bytes). If file already exists, it updates timestamp.

### User Perspective
```bash
$ touch README.txt
$ touch file1.txt file2.txt file3.txt
$ touch data.txt                # Creates empty file
```

### Internal Implementation

#### Step-by-Step Code Execution

**File:** `user/shell/shell.c:617-635`

```c
int shell_touch(int argc, char** argv) {
    if (argc == 1) {
        printf("touch failed. No Path. \n");
        return 0;
    }
    int i;
    for (i = 1; i < argc; i++) {
        int fd;
        fd = open(argv[i], O_RDONLY);
        if(fd >= 0) {
            printf("%s file exist\n", argv[i]);
            close(fd);
            continue;
        } else {
            close(open(argv[i], O_CREATE));
        }
    }
    return 0;
}
```

#### Detailed Execution

**Scenario:** User types: `touch README.txt`

1. **Input Parsing**
   - Input: `"touch README.txt"`
   - Tokenized: `["touch", "README.txt"]`
   - `argc = 2`, `argv = ["touch", "README.txt", NULL]`

2. **Validation**
   ```c
   if (argc == 1) {  // argc=2, so not == 1
       printf("touch failed. No Path. \n");
       return 0;
   }
   ```
   - Check passes ✓

3. **Loop - First Iteration (i=1)**
   ```c
   for (i = 1; i < argc; i++)  // i=1, argc=2
   // Loop runs? 1 < 2? YES
   ```
   
   - `argv[1] = "README.txt"`
   
   **Step 3a: Try to Open Existing File**
   ```c
   fd = open(argv[i], O_RDONLY);
   // fd = open("README.txt", O_RDONLY)
   ```
   
   - **If file EXISTS:**
     ```c
     if(fd >= 0) {  // File opened successfully
         printf("%s file exist\n", argv[i]);
         close(fd);
         continue;  // Skip to next iteration
     }
     ```
     - File already exists → skip it
     
   - **If file DOESN'T EXIST:**
     ```c
     else {
         close(open(argv[i], O_CREATE));
     }
     ```
     
     Let me break this down (confusing line!):
     
     ```c
     open(argv[i], O_CREATE)
     // open("README.txt", O_CREATE)
     // This opens (creates) the file and returns fd
     // Returns fd >= 0 if successful
     
     close(...)
     // Closes the file descriptor immediately
     // We don't need to keep it open
     ```
     
     **Kernel Operation:**
     ```
     open("README.txt", O_CREATE) {
         1. Kernel checks if file exists
         2. File doesn't exist → CREATE IT
         3. Create new file inode (e.g., #700)
         4. File size = 0 bytes (empty)
         5. Add entry to current directory:
            "README.txt\0" → inode #700
         6. Return file descriptor (e.g., 4)
     }
     ```
     
     Then:
     ```c
     close(4);  // Close the file descriptor
     // File is now created but closed
     // We don't keep it open
     ```

4. **Loop - Check Exit Condition**
   ```c
   for (i = 1; i < argc; i++)  // i=2, argc=2
   // Loop runs? 2 < 2? NO
   // Exit loop
   ```

5. **Return**
   ```c
   return 0;  // Success
   ```

#### The Clever Algorithm

**Pattern:** Check existence by attempting to open

```
FOR EACH filename:
    TRY: fd = open(filename, O_RDONLY)
    
    IF succeeds (fd >= 0):
        File already exists
        close(fd)
        skip to next file
    
    ELSE (fd < 0):
        File doesn't exist
        create it: open(filename, O_CREATE)
        close the returned fd
```

This is **clever** because:
1. No need for separate `stat()` syscall
2. Reuses `open()` mechanism
3. Efficient - one or two syscalls per file

#### File Descriptor Flags

```c
O_RDONLY  = 0x000     // Open for reading only
O_WRONLY  = 0x001     // Open for writing only
O_RDWR    = 0x002     // Open for reading AND writing
O_CREATE  = 0x200     // Create if doesn't exist
O_TRUNC   = 0x400     // Truncate to 0 bytes

// Example usage:
open("file.txt", O_RDONLY)           // Just read
open("file.txt", O_CREATE|O_RDWR)    // Create, read, write
open("file.txt", O_CREATE|O_TRUNC)   // Create or clear
```

#### Creating Multiple Files

```
Command: touch file1.txt file2.txt file3.txt

Iteration 1:
  Try open("file1.txt", O_RDONLY)
  Fails (doesn't exist)
  open("file1.txt", O_CREATE) → fd=4
  close(4)
  Result: file1.txt created ✓

Iteration 2:
  Try open("file2.txt", O_RDONLY)
  Fails (doesn't exist)
  open("file2.txt", O_CREATE) → fd=4
  close(4)
  Result: file2.txt created ✓

Iteration 3:
  Try open("file3.txt", O_RDONLY)
  Fails (doesn't exist)
  open("file3.txt", O_CREATE) → fd=4
  close(4)
  Result: file3.txt created ✓
```

#### Error Scenarios

**Scenario 1: File Already Exists**
```
Command: touch existing.txt

open("existing.txt", O_RDONLY) → fd=4 (succeeds!)
if(fd >= 0):  // TRUE
    printf("existing.txt file exist\n")
    close(4)
    continue

Result: Nothing created, message printed
```

**Scenario 2: Permission Denied**
```
Command: touch /root/file.txt  (where /root is not accessible)

open("/root/file.txt", O_RDONLY) → fd=-1 (fails)
else:
    open("/root/file.txt", O_CREATE) → STILL FAILS
    close(-1)  // Closing -1 has no effect

Result: File not created, no error message
```

---

## cd - Change Directory

### What It Does
Changes the current working directory of the process.

### User Perspective
```bash
$ cd /home/user/documents    # Absolute path
$ cd documents               # Relative path
$ cd                         # Go to root /
$ cd ..                      # Go to parent directory
```

### Internal Implementation

#### Step-by-Step Code Execution

**File:** `user/shell/shell.c:195-209`

```c
int shell_cd(int argc, char** argv) {
    char path[1024];
    
    if(argc == 1) {
        strcpy(path, "/");
        if (sys_chdir(path) < 0)
            printf("cd: cannot change directory\n");
    }
    else {
        strcpy(path, argv[1]);
        if (sys_chdir(path) < 0)
            printf("cd: %s: No such directory\n", argv[1]);
    }
    return 0;
}
```

#### Detailed Execution

**Scenario 1:** User types: `cd documents`

1. **Input Parsing**
   - Input: `"cd documents"`
   - Tokenized: `["cd", "documents"]`
   - `argc = 2`, `argv = ["cd", "documents", NULL]`

2. **Argument Check**
   ```c
   if(argc == 1) {  // argc=2, so not == 1
       strcpy(path, "/");
       if (sys_chdir(path) < 0)
           printf("cd: cannot change directory\n");
   }
   else {  // ← THIS PATH
       strcpy(path, argv[1]);
       // strcpy(path, "documents")
       // path = "documents\0"
       
       if (sys_chdir(path) < 0)
           printf("cd: %s: No such directory\n", argv[1]);
   }
   ```

3. **Copy Path**
   ```c
   strcpy(path, argv[1]);
   // Copies "documents" to path variable
   ```

4. **Syscall to Kernel**
   ```c
   ret = sys_chdir(path);
   // ret = sys_chdir("documents")
   ```
   
   **Kernel Operation:**
   ```
   sys_chdir("documents") {
       1. Parse path "documents"
       2. Look up "documents" in current directory
       3. Find its inode (e.g., #250)
       4. VALIDATE: Is inode #250 a directory?
          - Check: inode->type == DIR
          - If file: return -1 (error)
          - If directory: continue
       5. Update process's cwd (current working directory)
          - process->cwd_inode = 250
       6. Return 0 (success)
   }
   ```

5. **Check Return Value**
   ```c
   if (sys_chdir(path) < 0)  // ret < 0?
       // 0 < 0? NO
       printf("cd: %s: No such directory\n", argv[1]);
   ```
   
   - Return value is 0 (success)
   - Don't print error message ✓

6. **Return**
   ```c
   return 0;  // cd completed
   ```

#### Process State Change

This is the KEY difference between pwd/mkdir/touch vs cd:

**Before cd:**
```
Process Structure (in kernel):
┌──────────────────────┐
│ PID: 42              │
│ cwd_inode: #100      │ ← Currently in /home/user
│ uid: 1000            │
│ ...                  │
└──────────────────────┘
```

**During cd documents:**
```
Process Structure (in kernel):
┌──────────────────────┐
│ PID: 42              │
│ cwd_inode: #100      │ ← Kernel looks this up
│ uid: 1000            │
│ ...                  │
└──────────────────────┘

Find inode #100 contents:
  - "documents" entry → inode #250
```

**After cd documents:**
```
Process Structure (in kernel):
┌──────────────────────┐
│ PID: 42              │
│ cwd_inode: #250      │ ← Updated!
│ uid: 1000            │
│ ...                  │
└──────────────────────┘
```

All future commands now execute in inode #250 context!

#### Scenario 2: User types: `cd` (no arguments)

```c
if(argc == 1) {  // argc=1, so YES
    strcpy(path, "/");
    // path = "/" (root directory)
    
    if (sys_chdir(path) < 0)
        printf("cd: cannot change directory\n");
}
```

- Change to root directory `/`
- Very simple!

#### Error Handling

**Scenario: cd to non-existent directory**

```
Command: cd nonexistent
pwd before: /home/user
cwd_inode before: #100

Execution:
  argc=2 (cd + nonexistent)
  strcpy(path, "nonexistent")
  sys_chdir("nonexistent")
  
  Kernel:
    Look for "nonexistent" in current directory (#100)
    NOT FOUND
    Return -1 (error)
  
  ret = -1
  if (sys_chdir(path) < 0)  // -1 < 0? YES
      printf("cd: nonexistent: No such directory\n")

Result:
  Error message printed
  cwd_inode NOT changed (still #100)
  pwd still shows /home/user
```

**Scenario: cd to a FILE instead of directory**

```
Command: cd myfile.txt

Execution:
  strcpy(path, "myfile.txt")
  sys_chdir("myfile.txt")
  
  Kernel:
    Find inode #701 (myfile.txt)
    Check: is |
 it a directory?
    inode->type == DIR? NO (it's REG for regular file)
    Return -1 (error, not a directory)
  
  ret = -1
  Print error message

Result:
  Error message: "cd: myfile.txt: No such directory"
  cwd NOT changed
```

---

## Complete Workflow

### Example: Create a Project Structure

```bash
$ pwd
/home/user

$ mkdir my_project
(creates inode #300)

$ cd my_project
(updates process cwd_inode to #300)

$ pwd
/home/user/my_project

$ touch README.txt
(creates file inode #701)

$ touch config.txt
(creates file inode #702)

$ mkdir src
(creates directory inode #701)

$ cd src
(updates process cwd_inode to #701)

$ touch main.c
(creates file inode #710)

$ cd ..
(goes back, cwd_inode = #300)

$ pwd
/home/user/my_project
```

### Resulting File Tree

```
Inode #1000 (/home/user):
├─ my_project → inode #300

Inode #300 (/home/user/my_project):
├─ README.txt → inode #701
├─ config.txt → inode #702
└─ src → inode #701

Inode #701 (/home/user/my_project/src):
└─ main.c → inode #710
```

### Process CWD Evolution

```
Start:        cwd_inode = #1000
After "mkdir my_project": cwd_inode = #1000 (unchanged)
After "cd my_project":    cwd_inode = #300
After "touch README.txt": cwd_inode = #300 (unchanged)
After "mkdir src":        cwd_inode = #300 (unchanged)
After "cd src":           cwd_inode = #701
After "touch main.c":     cwd_inode = #701 (unchanged)
After "cd ..":            cwd_inode = #300
```

---

## Practice Questions

### Question 1: pwd Behavior
**Q:** If you type `pwd` twice in a row, what do you expect?
```
$ pwd
/home/user/documents
$ pwd
/home/user/documents
```
**A:** Same output both times. pwd just reads process state, doesn't modify anything.

### Question 2: mkdir Side Effects
**Q:** What happens when you run `mkdir` five times with different directories?

**A:** Five new inodes created, parent directory updated five times. Process state (cwd) unchanged.

### Question 3: touch on Existing File
**Q:** What happens if you `touch` a file that already exists?

```
$ touch file.txt
(creates file)
$ touch file.txt
(file already exists, what happens?)
```

**A:** According to the code:
```c
fd = open("file.txt", O_RDONLY);
if(fd >= 0) {  // File already exists!
    printf("file.txt file exist\n");
    close(fd);
    continue;  // Skip it
}
```
- Prints message: "file.txt file exist"
- File unchanged
- No error

### Question 4: cd to Non-existent Path
**Q:** What happens if you try `cd` to a path that doesn't exist?

**A:** 
- sys_chdir() returns -1
- Error message printed: "cd: path: No such directory"
- Process cwd NOT changed
- Still in previous directory

### Question 5: Process State vs Filesystem Changes
**Q:** Which commands change process state and which change filesystem?

| Command | Process State Changed | Filesystem Changed |
|---------|---------------------|--------------------|
| pwd | NO | NO |
| mkdir | NO | YES |
| touch | NO | YES |
| cd | YES | NO |

### Question 6: Syscall Necessity
**Q:** Why does pwd need a syscall but not mkdir?

**A:** pwd READS kernel data (cwd inode). Only kernel can provide reliable answer.
mkdir MODIFIES filesystem, which requires kernel's unique access to storage.

Both need syscalls but for different reasons!

### Question 7: Code Tracing - mkdir
**Q:** Trace execution of: `mkdir a b`

```
Input: "mkdir a b"
Tokens: ["mkdir", "a", "b"]
argc=3, argv=["mkdir", "a", "b"]

shell_mkdir(3, argv):
  if (argc < 2): 3 < 2? NO
  for (i=1; i<3; i++):
    i=1: sys_mkdir("a") → inode #100
    i=2: sys_mkdir("b") → inode #101
    i=3: 3 < 3? NO, exit loop
  return 0

Result: Two directories created
```

### Question 8: Code Tracing - touch
**Q:** Trace execution of: `touch file.txt`

```
Input: "touch file.txt"
Tokens: ["touch", "file.txt"]
argc=2, argv=["touch", "file.txt"]

shell_touch(2, argv):
  if (argc == 1): 2 == 1? NO
  for (i=1; i<2; i++):
    i=1:
      fd = open("file.txt", O_RDONLY)
      File doesn't exist, so fd < 0
      else:
        fd = open("file.txt", O_CREATE) → fd >= 0
        Kernel creates inode #200
        close(fd)
    i=2: 2 < 2? NO, exit loop
  return 0

Result: Empty file created
```

### Question 9: Data Structure Knowledge
**Q:** What's stored in an inode?

**A:** Type, size, owner, permissions, timestamps, disk block locations, and (for directories) list of entries.

### Question 10: File Descriptor Understanding
**Q:** When you run `touch README.txt`, how many file descriptors are used?

**A:** Temporarily 1:
1. `open("README.txt", O_CREATE)` → returns fd=4
2. `close(4)` → closes it immediately
3. File remains created but not open

---

## Summary Comparison

| Aspect | pwd | mkdir | touch | cd |
|--------|-----|-------|-------|-----|
| **Purpose** | Display location | Create directory | Create file | Change location |
| **Syscall** | sys_pwd() | sys_mkdir() | open(O_CREATE) | sys_chdir() |
| **Process State Changed** | NO | NO | NO | YES |
| **Filesystem Changed** | NO | YES | YES | NO |
| **Arguments Required** | 0 | 1+ | 1+ | 0-1 |
| **Supports Batch** | N/A | YES | YES | NO |
| **Loop in Code** | NO | YES | YES | NO |
| **Error Message** | minimal | on error | on error | on error |

---

**You now have complete understanding! Go present with confidence! 🚀**
