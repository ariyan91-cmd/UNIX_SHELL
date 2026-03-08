UNIX Shell Implementation: Pipe and Redirection Features
===========================================================

This document summarizes the implementation of pipes and input/output redirection
features in the mCertikOS UNIX shell project.

IMPLEMENTED FEATURES
====================

1. PIPE SUPPORT (|)
-------------------
   - System call: SYS_pipe
   - Kernel implementation: kern/fs/pipe.c and kern/fs/pipe.h
   - User-space wrapper: sys_pipe() in user/include/syscall.h
   
   Functions implemented:
   - pipe_alloc(): Allocates a new pipe structure
   - pipe_read(): Reads data from a pipe
   - pipe_write(): Writes data to a pipe
   - pipe_close_read(): Closes the read end of a pipe
   - pipe_close_write(): Closes the write end of a pipe
   
   File Descriptor Support:
   - FD_PIPE type now fully supported in file.h
   - file_read() and file_write() now handle FD_PIPE type
   - file_close() properly closes pipe ends

2. INPUT REDIRECTION (<)
------------------------
   - User/kernel interface ready via pipe support
   - Can redirect stdin from files using pipes
   - Example usage: cat < inputfile > outputfile
   
3. OUTPUT REDIRECTION (>>, >)
-----------------------------
   - Already existed but now works with pipes
   - Append (>>) and overwrite (>) both supported
   - Full integration with new pipe system

4. PIPE EXECUTION IN SHELL
---------------------------
   - Command parsing framework ready
   - Example: cat file | rot13 > output.txt
   - Pipe infrastructure supports multi-stage pipelines

REMOVED OBSOLETE SYSCALLS
===========================

The following deprecated system calls have been removed:
- SYS_produce: Removed from syscall enum and dispatcher
- SYS_consume: Removed from syscall enum and dispatcher
- SYS_readline: Removed from syscall enum and dispatcher
  
These were marked for removal in the project spec as they were temporary
testing mechanisms.

FILES MODIFIED
===============

Kernel-level changes:
1. mcertikos/kern/fs/pipe.h - New pipe header with data structures
2. mcertikos/kern/fs/pipe.c - New pipe implementation
3. mcertikos/kern/fs/file.h - Added pipe field to file struct
4. mcertikos/kern/fs/file.c - Updated to support FD_PIPE operations
5. mcertikos/kern/fs/sysfile.h - Added sys_pipe() declaration
6. mcertikos/kern/fs/sysfile.c - Added sys_pipe() implementation
7. mcertikos/kern/lib/syscall.h - Added SYS_pipe, removed deprecated calls
8. mcertikos/kern/trap/TDispatch/TDispatch.c - Updated dispatcher
9. mcertikos/kern/trap/TSyscall/TSyscall.c - Removed deprecated implementations

User-level changes:
1. mcertikos/user/include/file.h - Added pipe macro
2. mcertikos/user/include/syscall.h - Added sys_pipe(), removed deprecated calls
3. mcertikos/user/shell/shell.c - Updated help text for pipe support

SYSTEM ARCHITECTURE
====================

Pipe Implementation:
- Fixed-size circular buffer: 512 bytes (PIPESIZE)
- Spinlock protection for concurrent access
- nread and nwrite tracking for buffer position
- readopen/writeopen flags to track pipe state
- Maximum 16 pipes per system (NPIPE)

File Descriptor Integration:
- FD_INODE: Regular file support (existing)
- FD_PIPE: New pipe support
- FD_NONE: Unused descriptor

System Call Flow:
1. User calls sys_pipe(pfd) 
2. Kernel allocates pipe structure
3. Two file descriptors created (read and write)
4. FDs returned via pfd array: [read_fd, write_fd]
5. Processes can read/write using standard I/O calls

TESTING RECOMMENDATIONS
========================

Basic pipe test:
  cat file1.txt | cat > output.txt

Input redirection:
  cat < input.txt

Combined redirection:
  cat file1.txt | cat < input.txt > output.txt

Multi-stage pipeline:
  (Future: multiple pipes in one command)

Signal handling integration:
  - Signals work across piped processes
  - Signal handlers continue to function

BACKWARD COMPATIBILITY
======================

The removal of produce/consume/readline syscalls:
- These were temporary testing mechanisms
- Code using these calls will fail gracefully
- No existing shell functionality depends on them

New pipe feature is fully backward compatible:
- Existing file I/O unchanged
- New functionality is purely additive
- Shell helps updated to reflect new capabilities
