#!/bin/bash

################################################################################
#                                                                              #
#  SHELL COMMANDS EXPLANATION SCRIPT                                          #
#  A demonstration script to explain how basic shell commands work            #
#  Commands covered: pwd, mkdir, touch, cd                                    #
#                                                                              #
#  Author: [Your Name]                                                        #
#  Date: March 2026                                                           #
#                                                                              #
################################################################################

# Colors for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to print section headers
print_header() {
    echo ""
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo ""
}

# Function to print subheaders
print_subheader() {
    echo -e "${CYAN}→ $1${NC}"
    echo ""
}

# Function to print command being executed
print_command() {
    echo -e "${YELLOW}COMMAND:${NC} ${GREEN}$1${NC}"
    echo ""
}

# Function to print explanation
print_explanation() {
    echo -e "${YELLOW}EXPLANATION:${NC}"
    echo -e "  $1"
    echo ""
}

# Function to show output
print_output() {
    echo -e "${YELLOW}OUTPUT:${NC}"
    echo -e "  $1"
    echo ""
}

# Create a demo directory
DEMO_DIR="/tmp/shell_demo_$$"
mkdir -p "$DEMO_DIR"

clear

echo -e "${CYAN}"
cat << "EOF"
╔═══════════════════════════════════════════════════════════════════════════╗
║                   SHELL COMMANDS EXPLANATION GUIDE                        ║
║                                                                           ║
║  This script demonstrates and explains:                                  ║
║    • pwd   - Print Working Directory                                     ║
║    • mkdir - Make Directory                                              ║
║    • touch - Create Empty File                                           ║
║    • cd    - Change Directory                                            ║
║                                                                           ║
║  For: CSE4501 Operating Systems Course                                   ║
║  Project: UNIX Shell with Pipes and Redirection                          ║
╚═══════════════════════════════════════════════════════════════════════════╝
EOF
echo -e "${NC}"

sleep 2

################################################################################
#                         COMMAND #1: pwd                                     #
################################################################################

print_header "COMMAND #1: pwd (Print Working Directory)"

print_subheader "What does it do?"
echo "pwd displays the full path of the current directory you are in."
echo "It shows you WHERE you are in the file system."
sleep 1

print_subheader "How it works internally"
cat << 'EOF'
In the kernel (kern/fs/sysfile.c):
  1. pwd calls the syscall: sys_pwd(path)
  2. The kernel retrieves the current process's working directory
  3. Returns the full path as a string
  
In the shell (user/shell/shell.c):
  1. shell_pwd() function is called
  2. Allocates a buffer to store the path
  3. Calls sys_pwd() syscall to get the path from kernel
  4. Prints the path to stdout using printf()
EOF
sleep 1

print_subheader "Live Demo"
print_command "pwd"
CURRENT_DIR=$(pwd)
eval "pwd"
print_explanation "This shows your current location in the file system tree."

sleep 2

################################################################################
#                         COMMAND #2: mkdir                                   #
################################################################################

print_header "COMMAND #2: mkdir (Make Directory)"

print_subheader "What does it do?"
echo "mkdir creates a new directory (folder) at the location you specify."
echo "You can create one or multiple directories in a single command."
sleep 1

print_subheader "How it works internally"
cat << 'EOF'
In the kernel (kern/fs/sysfile.c):
  1. mkdir calls the syscall: sys_mkdir(dirname)
  2. Kernel validates the path doesn't already exist
  3. Kernel creates a new directory inode
  4. Adds entry to parent directory
  5. Returns success/failure status
  
In the shell (user/shell/shell.c):
  int shell_mkdir(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
      int ret = sys_mkdir(argv[i]);  // Call syscall for each directory
      if (ret != 0) {
        printf("Error creating %s\n", argv[i]);
      }
    }
  }

The function:
  1. Loops through each argument
  2. Calls sys_mkdir() for each directory name
  3. Checks return value for errors
  4. Prints error message if creation fails
EOF
sleep 1

print_subheader "Live Demo"
print_command "mkdir test_directory"
mkdir -p "$DEMO_DIR/test_directory"
echo "✓ Directory created successfully"
echo ""

print_command "ls -la $DEMO_DIR/"
ls -la "$DEMO_DIR/"
echo ""
print_explanation "You can see 'test_directory' now exists in the demo folder."

print_subheader "Creating Multiple Directories"
print_command "mkdir dir1 dir2 dir3"
mkdir -p "$DEMO_DIR/dir1" "$DEMO_DIR/dir2" "$DEMO_DIR/dir3"
echo "✓ All three directories created"
echo ""

print_command "ls -la $DEMO_DIR/"
ls -la "$DEMO_DIR/"
echo ""
print_explanation "All three directories (dir1, dir2, dir3) are now created in one command."

sleep 2

################################################################################
#                         COMMAND #3: touch                                   #
################################################################################

print_header "COMMAND #3: touch (Create Empty File)"

print_subheader "What does it do?"
echo "touch creates a new empty file with the specified name."
echo "If the file already exists, it updates its timestamp."
echo "It's useful for creating placeholder files without content."
sleep 1

print_subheader "How it works internally"
cat << 'EOF'
In the kernel (kern/fs/sysfile.c):
  1. touch tries to open the file with O_RDONLY
  2. If file exists → just update timestamp (in real OS)
  3. If file doesn't exist → create it with O_CREATE flag
  
In the shell (user/shell/shell.c):
  int shell_touch(int argc, char** argv) {
    for (i = 1; i < argc; i++) {
      int fd = open(argv[i], O_RDONLY);  // Try to open
      if(fd >= 0){
        // File already exists
        printf("%s file exist\n", argv[i]);
        close(fd);
        continue;
      } else {
        // File doesn't exist, create it
        close(open(argv[i], O_CREATE));
      }
    }
  }

The algorithm:
  1. For each filename argument:
  2. Try to open the file (O_RDONLY = open for reading)
  3. If open succeeds → file exists, skip it
  4. If open fails → file doesn't exist, so create it (O_CREATE)
  5. Close the file descriptor
EOF
sleep 1

print_subheader "Live Demo"
print_command "touch myfile.txt"
touch "$DEMO_DIR/myfile.txt"
echo "✓ File created"
echo ""

print_command "ls -la $DEMO_DIR/"
ls -la "$DEMO_DIR/"
echo ""
print_explanation "You can see 'myfile.txt' is now listed as a file (size 0 bytes)."

print_subheader "Creating Multiple Files"
print_command "touch file1.txt file2.txt file3.txt"
touch "$DEMO_DIR/file1.txt" "$DEMO_DIR/file2.txt" "$DEMO_DIR/file3.txt"
echo "✓ All three files created"
echo ""

print_command "ls -la $DEMO_DIR/"
ls -la "$DEMO_DIR/"
echo ""
print_explanation "Multiple files created with correct timestamps."

print_subheader "Touching an Existing File"
print_command "touch file1.txt  (again)"
touch "$DEMO_DIR/file1.txt"
echo "✓ File exists, timestamp updated (in real OS)"
echo ""
print_explanation "When touching an existing file, it would update the modification time."

sleep 2

################################################################################
#                         COMMAND #4: cd                                      #
################################################################################

print_header "COMMAND #4: cd (Change Directory)"

print_subheader "What does it do?"
echo "cd changes your current working directory."
echo "Without arguments, it takes you to the home/root directory."
echo "With a path, it moves you to that directory."
sleep 1

print_subheader "How it works internally"
cat << 'EOF'
In the kernel (kern/fs/sysfile.c):
  1. cd calls the syscall: sys_chdir(path)
  2. Kernel validates the path exists and is a directory
  3. Kernel updates process's current working directory pointer
  4. Returns success/failure status
  
In the shell (user/shell/shell.c):
  int shell_cd(int argc, char** argv) {
    char path[1024];
    
    if(argc == 1){
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
  }

The algorithm:
  1. Check number of arguments
  2. If no argument → use "/" (root)
  3. If with argument → use the provided path
  4. Call sys_chdir() syscall to change directory
  5. Check return value for errors
  6. Print error if directory doesn't exist
EOF
sleep 1

print_subheader "Live Demo - Before & After"

print_command "pwd  (before cd)"
BEFORE_DIR=$(pwd)
echo "Current directory: $BEFORE_DIR"
echo ""

print_command "cd /tmp"
cd /tmp 2>/dev/null || true
echo "✓ Changed to /tmp"
echo ""

print_command "pwd  (after cd)"
echo "Current directory: $(pwd)"
echo ""
print_explanation "Notice how the directory changed from $BEFORE_DIR to /tmp"

print_subheader "Directory Navigation Sequence"

print_command "pwd"
echo "Currently in: $(pwd)"
echo ""

print_command "cd .  (stay in current directory)"
echo "Still in: $(pwd)"
echo ""
print_explanation "Using '.' refers to the current directory, so 'cd .' doesn't change location."

# Return to original directory
cd "$BEFORE_DIR" 2>/dev/null || true

sleep 2

################################################################################
#                    HOW THEY WORK TOGETHER - FLOW DIAGRAM                    #
################################################################################

print_header "HOW THESE COMMANDS WORK TOGETHER"

cat << 'EOF'
TYPICAL WORKFLOW:

Step 1: See where you are
  $ pwd
  → Shows: /home/user

Step 2: Create a new project directory
  $ mkdir my_project
  → Creates folder: /home/user/my_project

Step 3: Move into that directory
  $ cd my_project
  → Current location is now: /home/user/my_project

Step 4: Confirm you're in the right place
  $ pwd
  → Shows: /home/user/my_project

Step 5: Create files in this directory
  $ touch README.txt config.txt data.txt
  → Creates three empty files

Step 6: See what you created
  $ ls
  → Lists: README.txt, config.txt, data.txt

EXECUTION FLOW IN SHELL CODE:
  
  shell_readline()          -- Read "mkdir my_project" from user
       ↓
  runcmd()                  -- Parse and tokenize the command
       ↓
  parse_single_command()    -- Check for pipes/redirections
       ↓
  run_command_stage()       -- Set up input/output
       ↓
  Find "mkdir" in command table
       ↓
  Call shell_mkdir(argc, argv)
       ↓
  Loop through arguments and call sys_mkdir() for each
       ↓
  Kernel creates directory inode
       ↓
  Return to shell prompt
EOF

sleep 3

################################################################################
#                         INTERNAL DATA STRUCTURES                            #
################################################################################

print_header "INTERNAL DATA STRUCTURES"

cat << 'EOF'
FILE DESCRIPTOR TABLE (in kernel):
┌─────────────────────────────────────┐
│ FD  │  Type     │  File/Pipe        │
├─────────────────────────────────────┤
│  0  │  STDIN    │  Keyboard         │
│  1  │  STDOUT   │  Screen           │
│  2  │  STDERR   │  Error output     │
│  3  │  Regular  │  myfile.txt       │
│  4  │  Regular  │  config.txt       │
│  5  │  PIPE     │  cat | grep       │
└─────────────────────────────────────┘

When you: $ cat myfile.txt
  1. Shell opens myfile.txt → gets fd=3
  2. Calls shell_cat(argc, argv, infd=0, outfd=1)
  3. Reads from fd=3 (the file)
  4. Writes to fd=1 (stdout/screen)
  5. Closes fd=3

INODE STRUCTURE (for files/directories):
┌─────────────────────────────────────┐
│ Inode #521 (my_project directory)   │
├─────────────────────────────────────┤
│ Type: DIRECTORY                     │
│ Mode: 0755 (rwxr-xr-x)              │
│ Size: 512 bytes                     │
│ Owner: user1                        │
│ Created: 2026-03-29                 │
│ Contents:                           │
│   - file1.txt (inode #522)          │
│   - file2.txt (inode #523)          │
│   - subdir (inode #524)             │
└─────────────────────────────────────┘

When you: $ cd my_project
  1. Kernel gets "my_project" from current directory
  2. Finds its inode #521
  3. Checks it's a DIRECTORY (not a file)
  4. Updates process's "current directory inode" to #521
  5. Now pwd shows path of inode #521
EOF

sleep 3

################################################################################
#                         DETAILED CODE WALKTHROUGH                           #
################################################################################

print_header "DETAILED CODE WALKTHROUGH"

print_subheader "Command Table (Command Registry)"
cat << 'EOF'
Location: user/shell/shell.c (lines 9-24)

static struct Command commands[] = {
    {"pwd", "print working directory", shell_pwd},
    {"cd", "change directory", shell_cd},
    {"mkdir", "create directory", shell_mkdir},
    {"touch", "create empty file", shell_touch},
    ...
};

WHEN YOU TYPE A COMMAND:
  1. Shell parses "pwd" from input
  2. Loops through commands[] array
  3. Compares "pwd" with each command name using strcmp()
  4. When match found → calls the function pointer
  5. shell_pwd() executes

STRUCTURE:
  - commands[i].name      = string name of command
  - commands[i].desc      = help text
  - commands[i].func      = function pointer to implementation
EOF

sleep 2

print_subheader "Command Lookup & Execution"
cat << 'EOF'
Location: user/shell/shell.c (lines 938-955)

void run_command_stage(struct ParsedCommand *cmd, ...) {
    for (i = 0; i < NCOMMANDS; i++) {
        if (strcmp(cmd->argv[0], commands[i].name) == 0) {
            // Found matching command
            commands[i].func(cmd->argc, cmd->argv);
            break;
        }
    }
    if (i == NCOMMANDS) {
        printf("Unknown command '%s'\n", cmd->argv[0]);
    }
}

EXECUTION STEPS:
  1. cmd->argv[0] = "pwd" (the command name)
  2. Loop i=0: strcmp("pwd", "pwd") == 0? YES!
  3. Call: commands[0].func(1, ["pwd", NULL])
     Which is: shell_pwd(1, ["pwd", NULL])
  4. Break out of loop
  5. Execute next stage or return
EOF

sleep 2

print_subheader "Memory Layout - Command Line Parsing"
cat << 'EOF'
When you type: "mkdir my_project my_second_dir"

MEMORY IN shell_buf (before parsing):
[m][i][k][d][i][r][ ][m][y]...[d][i][r][\0]

STEP 1 - TOKENIZATION:
Replace spaces with null bytes:
[m][i][k][d][i][r][\0][m][y]...[d][i][r][\0]

STEP 2 - CREATE POINTERS:
tokens[0] → points to first 'm' in "mkdir"
tokens[1] → points to first 'm' in "my_project"
tokens[2] → points to first 'm' in "my_second_dir"
tokens[3] → NULL (terminator)
argc = 3

STEP 3 - CALL FUNCTION:
shell_mkdir(3, tokens)
  └─ argc=3, argv=["mkdir", "my_project", "my_second_dir", NULL]

STEP 4 - LOOP AND EXECUTE:
for (int i = 1; i < argc; i++) {
    sys_mkdir(argv[i]);  // argv[1]="my_project", argv[2]="my_second_dir"
}
EOF

sleep 3

################################################################################
#                         SYSCALL INTERFACE                                   #
################################################################################

print_header "SYSCALL INTERFACE - KERNEL BOUNDARY"

cat << 'EOF'
USER SPACE vs KERNEL SPACE:

USER SPACE (shell.c):                KERNEL SPACE (kern/fs/):
├─ shell_pwd()                       ├─ sys_pwd() syscall handler
│  └─ calls sys_pwd()                │  └─ Retrieves actual path
│                                    │
├─ shell_cd()                        ├─ sys_chdir() syscall handler
│  └─ calls sys_chdir()              │  └─ Changes directory
│                                    │
├─ shell_mkdir()                     ├─ sys_mkdir() syscall handler
│  └─ calls sys_mkdir()              │  └─ Creates directory inode
│                                    │
└─ shell_touch()                     └─ open() syscall handler
   └─ calls open(O_CREATE)              └─ Creates file inode

SYSCALL MECHANISM:
  1. User code calls: int fd = open("file.txt", O_CREATE);
  2. Triggers CPU interrupt (system call trap)
  3. Switches from USER MODE to KERNEL MODE
  4. Kernel handler executes privileged operations
  5. Kernel returns result to user code
  6. Switches back to USER MODE
  7. User code continues with result (fd)

LOCATION: kern/lib/syscall.h and kern/trap/TSyscall/TSyscall.c
EOF

sleep 3

################################################################################
#                         PRACTICAL EXAMPLE                                   #
################################################################################

print_header "PRACTICAL EXAMPLE - STEP BY STEP"

echo "Let's trace what happens when you type: mkdir project && cd project && touch README.txt"
echo ""

print_subheader "STEP 1: USER INPUT"
cat << 'EOF'
You type at shell prompt:
  > mkdir project

Characters read one by one:
  'm' → echo 'm' → buf[0]='m'
  'i' → echo 'i' → buf[1]='i'
  'k' → echo 'k' → buf[2]='k'
  'd' → echo 'd' → buf[3]='d'
  'i' → echo 'i' → buf[4]='i'
  'r' → echo 'r' → buf[5]='r'
  ' ' → echo ' ' → buf[6]=' '
  'p' → echo 'p' → buf[7]='p'
  ...
  Enter key → null-terminate string
  
Result: buf="mkdir project\0"
EOF

print_subheader "STEP 2: TOKENIZATION"
cat << 'EOF'
runcmd() function:
  Replace spaces with null: "mkdir\0project\0"
  
  tokens[0] → "mkdir"
  tokens[1] → "project"
  tokens[2] → NULL
  argc = 2
EOF

print_subheader "STEP 3: COMMAND LOOKUP"
cat << 'EOF'
Loop through commands[]:
  i=0: strcmp("mkdir", "ls") ≠ 0
  i=1: strcmp("mkdir", "pwd") ≠ 0
  i=2: strcmp("mkdir", "cd") ≠ 0
  i=3: strcmp("mkdir", "mkdir") = 0 ✓ FOUND!
  
Call: commands[3].func(2, ["mkdir", "project", NULL])
Which is: shell_mkdir(2, ["mkdir", "project", NULL])
EOF

print_subheader "STEP 4: EXECUTION - shell_mkdir()"
cat << 'EOF'
int shell_mkdir(int argc, char** argv) {
    if (argc < 2) {
        printf("mkdir: missing operand\n");
        return 0;
    }
    
    // argc=2, argv=["mkdir", "project", NULL]
    for (int i = 1; i < argc; i++) {  // i=1
        int ret = sys_mkdir(argv[i]);  // sys_mkdir("project")
        if (ret != 0) {
            printf("mkdir: cannot create directory '%s'\n", argv[i]);
        }
    }
    return 0;
}

Execution:
  i=1: sys_mkdir(argv[1]) = sys_mkdir("project")
  Loop ends (i=2, which is not < argc=2)
  Return 0
EOF

print_subheader "STEP 5: KERNEL SYSCALL - sys_mkdir('project')"
cat << 'EOF'
Location: kern/fs/sysfile.c

Kernel does:
  1. Validate path "project"
  2. Check current directory permissions
  3. Create new inode structure
  4. Allocate space on disk
  5. Add entry to parent directory
  6. Update timestamps
  7. Return 0 (success) or -1 (error)

Result: New directory "project" exists!
EOF

print_subheader "STEP 6: READY FOR NEXT COMMAND"
cat << 'EOF'
Shell returns to main loop in main():
  while(1) {
    shell_readline(buf);  ← Loop back here
    runcmd(buf);
  }

Shows prompt ">
Ready for next command
EOF

sleep 3

################################################################################
#                         SUMMARY AND KEY POINTS                              #
################################################################################

print_header "SUMMARY - KEY TAKEAWAYS"

cat << 'EOF'
╔════════════════════════════════════════════════════════════════════╗
║                         pwd COMMAND                               ║
╠════════════════════════════════════════════════════════════════════╣
║ Purpose:  Display current working directory path                   ║
║ Code:     Calls sys_pwd() syscall                                 ║
║ Result:   Prints path to stdout                                   ║
║ Example:  $ pwd  →  /home/user/desktop                            ║
╚════════════════════════════════════════════════════════════════════╝

╔════════════════════════════════════════════════════════════════════╗
║                        mkdir COMMAND                              ║
╠════════════════════════════════════════════════════════════════════╣
║ Purpose:  Create new directories                                   ║
║ Code:     Loops through args, calls sys_mkdir() for each          ║
║ Result:   New directory inodes created                            ║
║ Example:  $ mkdir dir1 dir2  →  Creates both dir1 and dir2       ║
╚════════════════════════════════════════════════════════════════════╝

╔════════════════════════════════════════════════════════════════════╗
║                        touch COMMAND                              ║
╠════════════════════════════════════════════════════════════════════╣
║ Purpose:  Create empty files                                       ║
║ Code:     Tries to open, creates with O_CREATE if missing         ║
║ Result:   New empty file inodes created                           ║
║ Example:  $ touch file1.txt  →  Creates empty file               ║
╚════════════════════════════════════════════════════════════════════╝

╔════════════════════════════════════════════════════════════════════╗
║                         cd COMMAND                                ║
╠════════════════════════════════════════════════════════════════════╣
║ Purpose:  Change current working directory                         ║
║ Code:     Calls sys_chdir() syscall with path                    ║
║ Result:   Process's current directory inode updated               ║
║ Example:  $ cd project  →  Now in project directory              ║
╚════════════════════════════════════════════════════════════════════╝

UNIFIED FLOW FOR ALL COMMANDS:
  INPUT → PARSE → TOKENIZE → LOOKUP → EXECUTE → STATUS → PROMPT

KEY CONCEPTS:
  ✓ File Descriptors (FDs) connect processes to files/pipes
  ✓ Inodes represent files and directories in kernel
  ✓ Syscalls are the bridge between user and kernel space
  ✓ Command table enables extensible command system
  ✓ Tokenization splits input into arguments
  ✓ Function pointers allow dynamic command dispatch
EOF

sleep 4

################################################################################
#                         CLEANUP AND CLOSING                                 #
################################################################################

print_header "CLEANUP"

print_subheader "Removing demo directory"
rm -rf "$DEMO_DIR"
echo "✓ Demo directory cleaned up"
echo ""

print_header "PRESENTATION COMPLETE"

cat << 'EOF'
╔═══════════════════════════════════════════════════════════════════╗
║                                                                   ║
║  KEY POINTS TO TELL YOUR TEACHER:                                ║
║                                                                   ║
║  1. SHELL ARCHITECTURE                                           ║
║     - Uses command table for extensible design                   ║
║     - Parses input into tokens and stages                        ║
║     - Executes via function pointers                             ║
║                                                                   ║
║  2. SYSCALL MECHANISM                                            ║
║     - User code calls sys_mkdir(), sys_chdir(), etc.             ║
║     - These trigger system calls (mode switch to kernel)         ║
║     - Kernel performs privileged operations                      ║
║     - Returns result to user code                                ║
║                                                                   ║
║  3. FILE SYSTEM OPERATIONS                                       ║
║     - mkdir creates directory inodes                             ║
║     - touch creates file inodes                                  ║
║     - cd changes process's current directory inode               ║
║     - pwd retrieves and displays current directory name          ║
║                                                                   ║
║  4. DATA STRUCTURES                                              ║
║     - File Descriptor Table (kernel)                             ║
║     - Inode structures (files/directories)                       ║
║     - Command table (shell)                                      ║
║     - Argument vectors (argc, argv)                              ║
║                                                                   ║
║  5. FLOW                                                         ║
║     User Input → Parse → Tokenize → Lookup → Execute            ║
║                                                                   ║
╚═══════════════════════════════════════════════════════════════════╝

For more details, see:
  - Source code: /home/ariyan/Desktop/UNIX_SHELL/mcertikos/user/shell/shell.c
  - Kernel code: /home/ariyan/Desktop/UNIX_SHELL/mcertikos/kern/fs/sysfile.c
  - README: /home/ariyan/Desktop/UNIX_SHELL/README.md

Questions? Check the inline code comments!
EOF

echo ""
echo -e "${GREEN}Presentation script completed!${NC}"
echo ""
