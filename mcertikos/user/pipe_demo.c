//
// Example program demonstrating UNIX Pipe and Redirection Usage
// Compile with mCertikOS user library
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <file.h>
#include <syscall.h>

// Example 1: Create a simple pipe
int example_basic_pipe() {
    printf("Example 1: Basic Pipe Creation\n");
    printf("==============================\n\n");
    
    int pfd[2];  // pfd[0] = read end, pfd[1] = write end
    
    if (pipe(pfd) == -1) {
        printf("Error: Failed to create pipe\n");
        return -1;
    }
    
    printf("Pipe created successfully!\n");
    printf("Read end:  fd = %d\n", pfd[0]);
    printf("Write end: fd = %d\n\n", pfd[1]);
    
    // Write data to pipe
    const char *msg = "Hello from pipe!";
    int n = write(pfd[1], msg, strlen(msg));
    printf("Wrote %d bytes to pipe\n", n);
    
    // Read data from pipe
    char buf[256];
    n = read(pfd[0], buf, sizeof(buf));
    printf("Read %d bytes from pipe: %s\n", n, buf);
    
    close(pfd[0]);
    close(pfd[1]);
    
    return 0;
}

// Example 2: File output redirection
int example_output_redirection() {
    printf("\nExample 2: Output Redirection\n");
    printf("==============================\n\n");
    
    // Open file for writing (create if doesn't exist)
    int fd = open("output.txt", O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("Error: Cannot open output.txt\n");
        return -1;
    }
    
    const char *content = "This is redirected output\nLine 2\nLine 3\n";
    int n = write(fd, content, strlen(content));
    printf("Wrote %d bytes to output.txt\n", n);
    
    close(fd);
    return 0;
}

// Example 3: File input redirection
int example_input_redirection() {
    printf("\nExample 3: Input Redirection\n");
    printf("=============================\n\n");
    
    // First create a test file
    int fd = open("input.txt", O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("Error: Cannot create input.txt\n");
        return -1;
    }
    
    const char *data = "This is test input data\nWith multiple lines\n";
    write(fd, data, strlen(data));
    close(fd);
    
    // Now read it back
    fd = open("input.txt", O_RDONLY);
    if (fd < 0) {
        printf("Error: Cannot open input.txt\n");
        return -1;
    }
    
    printf("Reading from input.txt:\n");
    printf("------------------------\n");
    
    char buf[256];
    int n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        fwrite(buf, 1, n, stdout);  // Print using fwrite
    }
    
    close(fd);
    return 0;
}

// Example 4: Combining pipes with file operations
int example_pipe_with_files() {
    printf("\nExample 4: Pipe with File Operations\n");
    printf("====================================\n\n");
    
    int pfd[2];
    if (pipe(pfd) == -1) {
        printf("Error: Cannot create pipe\n");
        return -1;
    }
    
    // Write some data to the pipe
    const char *data = "Data flowing through pipe";
    write(pfd[1], data, strlen(data));
    close(pfd[1]);  // Close write end
    
    // Read from pipe and write to file
    int outfd = open("pipe_output.txt", O_CREATE | O_RDWR);
    if (outfd < 0) {
        printf("Error: Cannot create pipe_output.txt\n");
        close(pfd[0]);
        return -1;
    }
    
    char buf[256];
    int n = read(pfd[0], buf, sizeof(buf));
    if (n > 0) {
        write(outfd, buf, n);
        printf("Transferred %d bytes from pipe to pipe_output.txt\n", n);
    }
    
    close(pfd[0]);
    close(outfd);
    return 0;
}

// Example 5: Demonstrate standard I/O redirection patterns
int example_stdio_operations() {
    printf("\nExample 5: Standard I/O Operations\n");
    printf("==================================\n\n");
    
    // Demonstrate using stdio functions
    FILE *f = fopen("stdio_output.txt", "w");
    if (f) {
        fprintf(f, "Using standard I/O with fprintf()\n");
        fprintf(f, "File descriptors: stdin=0, stdout=1, stderr=2\n");
        fclose(f);
        printf("Created stdio_output.txt using fprintf()\n");
    }
    
    return 0;
}

int main() {
    printf("====================================\n");
    printf("UNIX Shell: Pipe and Redirection\n");
    printf("Examples and Demonstrations\n");
    printf("====================================\n\n");
    
    // Run examples
    example_basic_pipe();
    example_output_redirection();
    example_input_redirection();
    example_pipe_with_files();
    example_stdio_operations();
    
    printf("\n====================================\n");
    printf("All examples completed!\n");
    printf("Check output files: output.txt, input.txt, pipe_output.txt\n");
    printf("====================================\n");
    
    return 0;
}

/*
 * Compilation:
 * 
 * This example shows how pipes and redirection work at the syscall level.
 * To use this:
 * 
 * 1. Compile as part of the user tools
 * 2. Run it from the shell: spawn <elf_id>
 * 
 * Or embed the functions in the shell for testing.
 * 
 * Key Points:
 * - pipe(pfd) creates a pipe and stores [read_fd, write_fd] in pfd array
 * - read(fd, buf, n) reads from any fd (file, pipe, or stdin)
 * - write(fd, buf, n) writes to any fd (file, pipe, or stdout)
 * - open("file", flags) opens files with redirection options
 * - Shell handles redirection: cmd < input > output
 * 
 * Advanced Usage:
 * - Multiple pipes for complex data flows
 * - Pipes between processes via spawn()
 * - Signal handling during pipe operations
 * - Non-blocking pipe operations
 */
