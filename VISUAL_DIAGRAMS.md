# Visual Diagrams - Command Flow Explanations

## DIAGRAM 1: Shell Command Execution Flow

```
┌─────────────────────────────────────────────────────────────┐
│                  USER TYPES COMMAND                         │
│                                                             │
│                    > mkdir project                          │
└─────────────────────────────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│              SHELL_READLINE() - Input Reading              │
│                                                             │
│  • Display ">" prompt                                      │
│  • Read character by character                             │
│  • Echo each character to screen                           │
│  • Handle backspace editing                                │
│  • Stop at Enter key                                       │
│                                                             │
│  Result: buf = "mkdir\0project\0"                          │
└─────────────────────────────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│              RUNCMD() - Tokenization Phase                 │
│                                                             │
│  Input buffer: "mkdir project"                             │
│                                                             │
│  Replace whitespace with nulls:                            │
│  "mkdir\0project\0"                                        │
│                                                             │
│  Create token pointers:                                    │
│  tokens[0] → "mkdir"                                       │
│  tokens[1] → "project"                                     │
│  tokens[2] → NULL                                          │
│  argc = 2                                                  │
└─────────────────────────────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│        PARSE_SINGLE_COMMAND() - Parse Redirections         │
│                                                             │
│  Check for pipe (|): None                                 │
│  Check for input (<): None                                │
│  Check for output (>): None                               │
│                                                             │
│  Result:                                                   │
│  stages[0].argv = ["mkdir", "project"]                    │
│  stages[0].infile = NULL                                  │
│  stages[0].outfile = NULL                                 │
│  stage_count = 1                                          │
└─────────────────────────────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│       RUN_COMMAND_STAGE() - Setup I/O Channels            │
│                                                             │
│  Input FD:  0 (stdin)                                     │
│  Output FD: 1 (stdout)                                    │
│                                                             │
│  No file redirections → use default FDs                   │
└─────────────────────────────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│          Command Table Lookup - Find Function              │
│                                                             │
│  Loop through commands[]:                                 │
│                                                             │
│  i=0: strcmp("mkdir", "ls") → NO                          │
│  i=1: strcmp("mkdir", "pwd") → NO                         │
│  i=2: strcmp("mkdir", "cd") → NO                          │
│  i=3: strcmp("mkdir", "mkdir") → YES! ✓                  │
│                                                             │
│  func = commands[3].func = shell_mkdir                    │
└─────────────────────────────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│               SHELL_MKDIR() - Execute                     │
│                                                             │
│  argc = 2, argv = ["mkdir", "project"]                    │
│                                                             │
│  for (int i = 1; i < argc; i++) {                         │
│    int ret = sys_mkdir(argv[i]);                          │
│    if (ret != 0)                                          │
│      printf("cannot create...");                          │
│  }                                                         │
│                                                             │
│  • i=1: sys_mkdir("project")                              │
│  • Loop ends                                              │
└─────────────────────────────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│           SYSCALL BOUNDARY - Kernel Mode                   │
│                                                             │
│  sys_mkdir("project")                                     │
│      ↓                                                      │
│  CPU INTERRUPT (mode switch to kernel mode)               │
│      ↓                                                      │
│  Kernel Handler:                                          │
│    • Validate path "project"                              │
│    • Check permissions on parent directory                │
│    • Allocate new inode                                   │
│    • Add entry to parent directory                        │
│    • Return 0 (success)                                   │
│      ↓                                                      │
│  MODE SWITCH (back to user mode)                          │
│      ↓                                                      │
│  Result returned to shell_mkdir()                         │
└─────────────────────────────────────────────────────────────┘
                         │
                         ↓
┌─────────────────────────────────────────────────────────────┐
│              Return to Shell Main Loop                      │
│                                                             │
│  Directory "project" created successfully!                 │
│                                                             │
│  Main loop in main():                                      │
│    while(1) {                                              │
│      shell_readline(buf);  ← Loop back here               │
│      runcmd(buf);                                         │
│    }                                                       │
│                                                             │
│  Show prompt ">                                            │
│  Ready for next command                                   │
└─────────────────────────────────────────────────────────────┘
```

---

## DIAGRAM 2: pwd Command - Step by Step

```
USER INPUT
    ↓
    > pwd
    ↓
    ┌─────────────────────────────────────────┐
    │ SHELL_READLINE()                        │
    │ Reads: "pwd"                            │
    │ Stores: buf = "pwd\0"                   │
    └─────────────────────────────────────────┘
    ↓
    ┌─────────────────────────────────────────┐
    │ RUNCMD()                                │
    │ Tokenize: tokens[0]="pwd", argc=1      │
    │ No pipes, no redirects                 │
    │ stages[0] = {argv:["pwd"], infile:NULL}│
    └─────────────────────────────────────────┘
    ↓
    ┌─────────────────────────────────────────┐
    │ COMMAND LOOKUP                          │
    │ Found: shell_pwd function               │
    │ Call: shell_pwd(1, ["pwd"])            │
    └─────────────────────────────────────────┘
    ↓
    ┌─────────────────────────────────────────┐
    │ SHELL_PWD() EXECUTION                   │
    │                                         │
    │ int shell_pwd(int argc, char** argv) {  │
    │   if (argc != 1) {                      │
    │     // Error handling                   │
    │   }                                     │
    │   sys_pwd(shell_buf);                   │
    │   printf("%s\n", shell_buf);            │
    │ }                                       │
    │                                         │
    │ 1. Check argc == 1 ✓                    │
    │ 2. Call sys_pwd() to get path          │
    │ 3. Print result to stdout              │
    └─────────────────────────────────────────┘
    ↓
    ┌─────────────────────────────────────────┐
    │ SYSCALL: sys_pwd()                      │
    │                                         │
    │ KERNEL OPERATION:                       │
    │ • Get current process's cwd inode      │
    │ • Convert inode to path string         │
    │ • Copy to user-provided buffer         │
    │ • Return 0 (success)                   │
    └─────────────────────────────────────────┘
    ↓
OUTPUT
    /home/user/documents
```

---

## DIAGRAM 3: mkdir Command - Multiple Directories

```
USER INPUT
    > mkdir project docs tests
    ↓
TOKENIZATION:
    tokens = ["mkdir", "project", "docs", "tests"]
    argc = 4
    ↓
COMMAND LOOKUP:
    Found: shell_mkdir
    ↓
SHELL_MKDIR() EXECUTION:
    
    argc=4, argv=["mkdir", "project", "docs", "tests"]
    
    Validation:
        if (argc < 2) error → PASS (argc=4)
    
    Loop iteration:
        ┌──────────────────────────────────┐
        │ i=1: sys_mkdir("project")        │
        │ Kernel:                          │
        │   Create inode #521              │
        │   Add "project" entry            │
        │ Return: 0 (success)              │
        │ Print: (nothing, success)        │
        └──────────────────────────────────┘
        ↓
        ┌──────────────────────────────────┐
        │ i=2: sys_mkdir("docs")           │
        │ Kernel:                          │
        │   Create inode #522              │
        │   Add "docs" entry               │
        │ Return: 0 (success)              │
        │ Print: (nothing, success)        │
        └──────────────────────────────────┘
        ↓
        ┌──────────────────────────────────┐
        │ i=3: sys_mkdir("tests")          │
        │ Kernel:                          │
        │   Create inode #523              │
        │   Add "tests" entry              │
        │ Return: 0 (success)              │
        │ Print: (nothing, success)        │
        └──────────────────────────────────┘
        ↓
        i < argc? (4 < 4) → NO, exit loop

RESULT:
    Three directories created:
    project/ (inode #521)
    docs/    (inode #522)
    tests/   (inode #523)
```

---

## DIAGRAM 4: touch Command - File Creation Logic

```
USER INPUT
    > touch file1.txt file2.txt
    ↓
TOKENIZATION:
    tokens = ["touch", "file1.txt", "file2.txt"]
    argc = 3
    ↓
COMMAND LOOKUP:
    Found: shell_touch
    ↓
SHELL_TOUCH() EXECUTION:

    argc=3, argv=["touch", "file1.txt", "file2.txt"]
    
    Validation:
        if (argc == 1) error → PASS (argc=3)
    
    Loop iteration:
        
        ┌──────────────────────────────────────────┐
        │ i=1: Processing "file1.txt"              │
        │                                          │
        │ fd = open("file1.txt", O_RDONLY)        │
        │                                          │
        │ ┌─ File already exists?                 │
        │ │                                       │
        │ │ Open succeeds? fd >= 0? YES           │
        │ │                                       │
        │ │ ├─ File exists, skip it              │
        │ │ ├─ close(fd)                         │
        │ │ └─ continue                          │
        │ │                                       │
        │ └─ File doesn't exist?                 │
        │                                        │
        │   Open fails? fd < 0?                 │
        │                                       │
        │   ├─ File doesn't exist, create it    │
        │   ├─ open(argv[1], O_CREATE)          │
        │   ├─ Kernel creates inode #701        │
        │   └─ close(fd)                        │
        │                                       │
        │ Result: file1.txt created            │
        └──────────────────────────────────────────┘
        ↓
        ┌──────────────────────────────────────────┐
        │ i=2: Processing "file2.txt"              │
        │                                          │
        │ (Same process as above)                  │
        │                                          │
        │ Result: file2.txt created              │
        └──────────────────────────────────────────┘
        ↓
        i < argc? (3 < 3) → NO, exit loop

RESULT:
    Two files created:
    file1.txt (inode #700, size=0)
    file2.txt (inode #701, size=0)
```

---

## DIAGRAM 5: cd Command - Directory Change

```
USER INPUT
    > cd documents
    ↓
TOKENIZATION:
    tokens = ["cd", "documents"]
    argc = 2
    ↓
COMMAND LOOKUP:
    Found: shell_cd
    ↓
SHELL_CD() EXECUTION:

    int shell_cd(int argc, char** argv) {
        char path[1024];
        
        if(argc == 1) {              ← NOT THIS PATH
            strcpy(path, "/");
            if (sys_chdir(path) < 0)
                printf("error\n");
        }
        else {                        ← THIS PATH (argc=2)
            strcpy(path, argv[1]);   ← strcpy(path, "documents")
            if (sys_chdir(path) < 0)
                printf("cd: documents: No such directory\n");
        }
    }
    
    Steps:
    1. argc=2, so take else branch
    2. path = "documents"
    3. Call sys_chdir("documents")
    
    ↓
    
SYSCALL: sys_chdir("documents")
    
    Kernel Operation:
    ├─ Look up "documents" in current directory
    ├─ Find its inode (#321)
    ├─ Validate it's a DIRECTORY (not a file)
    ├─ Update process's cwd inode to #321
    ├─ Return 0 (success)
    
    ↓
    
CHECK RETURN VALUE:
    ret = 0 (success)
    if (ret < 0) → FALSE
    No error message printed
    
    ↓

PROCESS STATE CHANGE:
    
    BEFORE:
    ┌─────────────────────┐
    │ Process Structure   │
    │ ├─ PID: 42         │
    │ ├─ cwd_inode: 100  │ ← Current inode
    │ └─ ...             │
    └─────────────────────┘
    
    AFTER:
    ┌─────────────────────┐
    │ Process Structure   │
    │ ├─ PID: 42         │
    │ ├─ cwd_inode: 321  │ ← Updated to documents
    │ └─ ...             │
    └─────────────────────┘
    
    Inode #321 represents /home/user/documents

RESULT:
    Current working directory is now: documents/
    Next mkdir/touch commands operate here
```

---

## DIAGRAM 6: Complete Workflow - All 4 Commands Together

```
┌─────────────────────────────────────────────────┐
│ STEP 1: See Current Location                    │
├─────────────────────────────────────────────────┤
│ > pwd                                           │
│ /home/user                                      │
│                                                 │
│ Kernel: Returns current cwd inode path         │
└─────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────┐
│ STEP 2: Create Project Directory                │
├─────────────────────────────────────────────────┤
│ > mkdir my_project                              │
│ (no output = success)                           │
│                                                 │
│ Kernel: Creates new directory inode            │
│ Directory structure updated                     │
└─────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────┐
│ STEP 3: Enter the Directory                     │
├─────────────────────────────────────────────────┤
│ > cd my_project                                 │
│ (no output = success)                           │
│                                                 │
│ Kernel: Updates process cwd_inode               │
│ Process now operates in my_project/             │
└─────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────┐
│ STEP 4: Verify Location                         │
├─────────────────────────────────────────────────┤
│ > pwd                                           │
│ /home/user/my_project                           │
│                                                 │
│ Kernel: Returns new cwd path                    │
└─────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────┐
│ STEP 5: Create Files                            │
├─────────────────────────────────────────────────┤
│ > touch README.txt config.txt                   │
│ (no output = success)                           │
│                                                 │
│ Kernel: Creates two file inodes in              │
│ my_project directory                            │
└─────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────┐
│ FINAL STATE                                     │
├─────────────────────────────────────────────────┤
│ Current directory: /home/user/my_project/       │
│                                                 │
│ Directory contents:                             │
│ ├─ README.txt (inode #700)                      │
│ └─ config.txt (inode #701)                      │
│                                                 │
│ File tree:                                      │
│ /home/user/                                     │
│ └─ my_project/    (inode #300)                  │
│    ├─ README.txt  (inode #700)                  │
│    └─ config.txt  (inode #701)                  │
└─────────────────────────────────────────────────┘
```

---

## DIAGRAM 7: Memory Layout - Tokenization Example

```
INPUT: "mkdir my_project docs"

STEP 1: Raw String in shell_buf
┌────────────────────────────────────────────────────┐
│m│i│k│d│i│r│ │m│y│_│p│r│o│j│e│c│t│ │d│o│c│s│\0│
└────────────────────────────────────────────────────┘
 0 1 2 3 4 5 6 7 8 9...                       index

STEP 2: After Tokenization (replace spaces with nulls)
┌────────────────────────────────────────────────────┐
│m│i│k│d│i│r│\0│m│y│_│p│r│o│j│e│c│t│\0│d│o│c│s│\0│
└────────────────────────────────────────────────────┘
          ↑               ↑               ↑
       NULL            NULL            NULL

STEP 3: Token Pointers
┌─────────────────────┐
│ tokens[0] ──────────┼──→ "mkdir\0..."
│ tokens[1] ──────────┼──→ "my_project\0..."
│ tokens[2] ──────────┼──→ "docs\0..."
│ tokens[3] = NULL    │
│ argc = 3            │
└─────────────────────┘

STEP 4: Passing to Function
shell_mkdir(3, tokens)

Inside function:
    argc = 3
    argv[0] = "mkdir"
    argv[1] = "my_project"
    argv[2] = "docs"
    argv[3] = NULL

    for (int i = 1; i < argc; i++) {
        sys_mkdir(argv[i]);  // argv[1], argv[2]
    }
```

---

## DIAGRAM 8: File Descriptor vs Inode Relationship

```
FILE DESCRIPTOR TABLE (Kernel - Per Process)
┌─────────────────────────────────────────┐
│ FD │ Inode # │ File/Resource            │
├─────────────────────────────────────────┤
│ 0  │ --      │ STDIN (keyboard)         │
│ 1  │ --      │ STDOUT (screen)          │
│ 2  │ --      │ STDERR (console)         │
│ 3  │ #521    │ "documents" (directory)  │
│ 4  │ #522    │ "file1.txt"              │
│ 5  │ #523    │ "config.txt"             │
└─────────────────────────────────────────┘

When you run: open("file1.txt", O_RDONLY)
    ↓
Kernel:
    1. Looks up "file1.txt" in filesystem
    2. Finds inode #522
    3. Creates file descriptor entry
    4. Returns FD = 4
    ↓
Now you can:
    read(4, buf, 100)   ← Read from file1.txt
    write(4, data, 50)  ← Write to file1.txt (if permission)
    close(4)            ← Close the file

INODE STRUCTURE (Kernel Filesystem)
┌─────────────────────────────────┐
│ Inode #521                      │
├─────────────────────────────────┤
│ Type: DIRECTORY                 │
│ Mode: 0755                      │
│ Size: 4096 bytes                │
│ Owner: user1                    │
│ Created: 2026-03-29             │
│ Modified: 2026-03-29            │
│                                 │
│ Contents:                       │
│ ├─ name: "file1.txt"            │
│ │  inode: #522                  │
│ │                               │
│ ├─ name: "config.txt"           │
│ │  inode: #523                  │
│ │                               │
│ └─ name: ".." (parent)          │
│    inode: #100                  │
└─────────────────────────────────┘
```

---

## DIAGRAM 9: Syscall Mechanism - Mode Transition

```
USER SPACE                          SYSTEM CALL                         KERNEL SPACE
────────────────────────────────────────────────────────────────────────────────────

int fd = open("file.txt", O_RDONLY);
         │
         ├─ This is a syscall
         │
         ↓

         USER MODE                                                     KERNEL MODE
         ≡≡≡≡≡════════════════════════════════════════════════════════≡≡≡≡≡
         
         CPU Executes:                          CPU TRAP/INTERRUPT
           • save registers                        │
           • switch to kernel mode        ←────────┤
           • jump to kernel handler               │
                                                  ↓
                                        Kernel Handler: sys_open()
                                        ├─ Validate filename
                                        ├─ Check permissions
                                        ├─ Find file's inode
                                        ├─ Create FD table entry
                                        ├─ Return FD number
                                        
                                        CPU EXECUTES:
                                          • restore registers
                                          • switch to user mode
                                          • return to user code

         Return value: fd = 4

         USER MODE continues with fd
         
         read(fd, buf, 100);
         write(fd, data, 50);
         close(fd);

KEY INSIGHT:
- Only the kernel can access filesystem structures
- Syscalls provide "controlled gateway" between user and kernel
- User code cannot directly modify inodes or access disk
```

---

## DIAGRAM 10: Command Table - How Lookup Works

```
COMMAND TABLE (Global Array)
─────────────────────────────────────

struct Command commands[] = {
    [0] {"ls",     "list...",       shell_ls},
    [1] {"pwd",    "print...",      shell_pwd},
    [2] {"cd",     "change...",     shell_cd},
    [3] {"mkdir",  "make...",       shell_mkdir},
    [4] {"touch",  "create...",     shell_touch},
    [5] {"cat",    "print...",      shell_cat},
    ...
};

#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))


LOOKUP PROCESS WHEN USER TYPES: "mkdir project"
────────────────────────────────────────────────

Parsed command name: "mkdir"

Loop through commands table:
    for (i = 0; i < NCOMMANDS; i++) {
        if (strcmp(cmd->argv[0], commands[i].name) == 0) {
            commands[i].func(argc, argv);
            break;
        }
    }

Iteration by iteration:
    
    i=0: strcmp("mkdir", "ls") 
         ├─ "mkdir" vs "ls"
         ├─ NOT EQUAL
         └─ Continue loop
    
    i=1: strcmp("mkdir", "pwd")
         ├─ "mkdir" vs "pwd"
         ├─ NOT EQUAL
         └─ Continue loop
    
    i=2: strcmp("mkdir", "cd")
         ├─ "mkdir" vs "cd"
         ├─ NOT EQUAL
         └─ Continue loop
    
    i=3: strcmp("mkdir", "mkdir")
         ├─ "mkdir" vs "mkdir"
         ├─ EQUAL! ✓
         ├─ commands[3].func(argc, argv)
         │  ↓
         │  Call: shell_mkdir(2, ["mkdir", "project"])
         │
         └─ break;

MATCHED! Called shell_mkdir()


ADVANTAGE OF THIS DESIGN:
─────────────────────────

✓ Adding new commands is EASY
  Just add one line to commands[] array
  
✓ No need for if-else chains
  Dynamic lookup via table
  
✓ Extensible
  Commands can be added at compile time
  
✓ Maintainable
  All commands in one place
  Easy to see what's available
```

---

## Quick Reference

| Command | System Call | What It Does | Files Affected |
|---------|------------|-------------|-----------------|
| **pwd** | sys_pwd() | Gets current directory path | None (read-only) |
| **mkdir** | sys_mkdir() | Creates new directory inode | Parent directory updated |
| **touch** | open(O_CREATE) | Creates new file inode | Directory updated |
| **cd** | sys_chdir() | Updates process's cwd | Process's kernel struct |

