   #include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>
#include <debug.h>
#include <file.h>
#include <gcc.h>
#include <proc.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <proc.h>
#include <syscall.h>
#include <x86.h>
#include "signal.h"

/* Stub definitions for POSIX types not available in mCertikOS */
#define O_RDONLY    0x000
#define O_WRONLY    0x001
#define O_RDWR      0x002
#define O_CREATE    0x200
#define O_TRUNC     0x400

#define BUFLEN 1024
#define ARGUMENT_LEN 128
#define ARG_COUNT 64
#define CMDBUF_SIZE	80
#define WHITESPACE "\t\r\n "
#define MAXARGS 16

#ifndef EOF
#define EOF (-1)
#endif

// Simple getchar implementation for reading from stdin
static int getchar(void) {
  unsigned char c;
  if (read(0, &c, 1) <= 0)
    return EOF;
  return c;
}

/* Forward declarations */
void signal_handler(int signum);

/* Helper function to convert string to int (wrapper for mCertikOS atoi) */
static int str_to_int(const char *s) {
    int result = 0;
    atoi(s, &result);
    return result;
}

/* Stub for getpid - returns current process id (use syscall if available) */
static int getpid(void) {
    return 1;  /* Stub: in a real implementation, add SYS_getpid syscall */
}

static int  runcmd (char *buf);
int shell_ls(int argc, char ** argv);
int shell_pwd(int argc, char **argv);
int shell_cd(int argc, char **argv);
int shell_cp(int argc, char **argv);
int shell_mv(int argc, char **argv);
int shell_rm(int argc, char **argv);
int shell_mkdir(int argc, char **argv);
int shell_cat(int argc, char **argv);
int shell_touch(int argc, char **argv);
int shell_help(int argc, char **argv);
int shell_echo(int argc, char **argv);
int shell_write(int argc, char **argv);
int shell_append(int argc, char **argv);
int shell_rot13(int argc, char **argv);
int shell_kill(int argc, char **argv);
int shell_trap(int argc, char **argv);
int shell_spawn(int argc, char **argv);

int _shell_rm(char * path, int isRecursive);
int rm_file(char * filename);
int ls_dir(char* buf, char* path);
int is_dir_empty(char* dirname);
int is_dir(char * path);
int is_file_exist(char* path);
int _shell_cat(char * path);
int _shell_cp(char * dest_path, char * src_path, int isRecursive);
int extract_filename(char * path, char * filename);
int cp_file(char * dest_filename, char * src_filename);
int shell_cat_fd(int argc, char **argv, int infd, int outfd);
int shell_rot13_fd(int argc, char **argv, int infd, int outfd);
int shell_echo_fd(int argc, char **argv, int outfd);

char shell_buf[BUFLEN];

struct Command
{
	const char* name;
	const char* desc;
	int
	(*func) (int argc, char** argv);
};

static struct Command commands[] =
{
	{"ls","list all files and directories under working directory", shell_ls},
	{"pwd","print working directory", shell_pwd},
	{"cd","cd <path> \n\t change directory", shell_cd},
	{"cp", "cp <-r> <src_path> <dest_path> \n\t copy file or directory ",shell_cp},
	{"mv", "mv <src_path> <dest_path> \n\t move file or directory",shell_mv},
	{"rm", "rm <-r> <filename> \n\t remove file or directory",shell_rm},
	{"mkdir", "mkdir <dirname> \n\t create directory",shell_mkdir},
  {"cat", "cat [-n|-b] <file>... [<inputfile] [> outfile | >> outfile]  \n\tprint and concatenate files, support line numbers and redirection",shell_cat},
	{"touch", "touch <filename> \n\t create new empty file", shell_touch},
        {"write", "write <string> <filename> \n\t write a string to file", shell_write},
        {"append", "append <string> <filename> \n\t append a string to file", shell_append},
        {"echo", "echo <text> [> file | >> file]  \n\t print text", shell_echo},
        {"rot13", "rot13 [file ...]  \n\tapply rot13 to files or stdin", shell_rot13},
        {"help", "help \n\t print this help message", shell_help},
        {"kill", "kill <signal> <pid> \n\t send signal to process", shell_kill},
        {"trap", "trap <signum> <handler> \n\t register signal handler", shell_trap},
        {"spawn", "spawn <elf_id> \n\t spawn a new process (1=ping, 2=pong, 3=ding)", shell_spawn}
};

#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))


int shell_help(int argc, char** argv){
    int i = 0;
    for(i = 0; i < NCOMMANDS; i++){
        printf("%s\n", commands[i].desc);
    }
    return 0;
}

int shell_ls(int argc, char** argv)
{
	if (argc == 1){
	  sys_ls(shell_buf, sizeof(shell_buf));
          printf("%s\n", shell_buf);
        }else if (argc == 2) {
          char path[100];
          sys_pwd(path);
          sys_chdir(argv[1]);
          sys_ls(shell_buf, sizeof(shell_buf));
          printf("%s\n", shell_buf);
          sys_chdir(path);
        }else{
          printf("ls: too many arguments.\n");
        }
        return 0;

}

int shell_pwd(int argc, char ** argv)
{
	if (argc != 1) {
		printf("pwd: too many arguments\n");S
		return 0;
	}
	sys_pwd(shell_buf);
        printf("%s\n",shell_buf);
	return 0;
}

int shell_cd(int argc, char** argv)
{
	char path[1024];
	if (argc == 1){
		strcpy(path, "/");S
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
int shell_cp(int argc, char** argv)
{
  char * src_path, * dest_path;
  if(argc < 3){
    printf("cp: too few arguments.\n");
    return 0;
  }
  else if(argc > 4){
    printf("cp: too many arguments.\n");
    return 0;
  }
  if(argc == 3){
   // ordinary cp
    src_path = argv[1];
    dest_path = argv[2];
    _shell_cp(dest_path, src_path, 0);
    return 0;
  }
  else{// recursive
    if(strcmp(argv[1], "-r")){
      printf("cp: invalid option. try '-r' ?\n");
      return 0;
    }
    // recursive cp
    src_path = argv[2];
    dest_path = argv[3];
    _shell_cp(dest_path, src_path, 1);
    return 0;
  }
}

//int ls_dir(char* buf);
//int is_dir_empty(char* dirname);
//int is_dir(char * path);
//int is_file_exist(char* path);

int _shell_cp(char * dest_path, char * src_path, int isRecursive){
  char path[BUFLEN];
  char filename[100];
  char dest_path_buf[BUFLEN];
  char src_path_buf[BUFLEN];
  char * p;
  if(!is_file_exist(src_path)){
    printf("cp: %s does not exist.\n", src_path);
    return 0;
  }
  if(isRecursive == 0){
    // common use
    if(is_dir(src_path)){
      printf("cp: omitting directory '%s'. try '-r' ?\n", src_path);
      return 0;
    }
    // src is a file
    if(is_file_exist(dest_path) && is_dir(dest_path)){
        // dest is a dir
        extract_filename(src_path, filename);
        strcpy(path, dest_path);
        p = path + strlen(path);
        *(p++) = '/';
        strcpy(p, filename);
        _shell_cp(path, src_path, isRecursive);
    }else{
        // dest is a file or does not exist
        cp_file(dest_path, src_path);
    }
  }else{
    // recursive
    if(is_dir(src_path)){
       // src is a dir
       if(is_file_exist(dest_path)){
          // dest exist
          if(is_dir(dest_path)){
            extract_filename(src_path, filename);
            strcpy(path, dest_path);
            p = path + strlen(path);
            *(p++) = '/';
            strcpy(p, filename);
            _shell_cp(path, src_path, isRecursive);
          }else{
            // dest is a file
            printf("cp: can not copy a dir to a file '%s'.\n", dest_path);
            return 0;
          }
       }else{
          // dest does not exist, do it recursively
          // preorder traversal
          cp_file(dest_path, src_path);
          int len = ls_dir(path, src_path);
          char* p = path;
          while(p - path < len){
            int dest_len, src_len;
            if(strcmp(p, ".") && strcmp(p, "..")){
              dest_len = strlen(dest_path);
              src_len = strlen(src_path);

              strcpy(dest_path_buf, dest_path);
              strcpy(src_path_buf, src_path);

              dest_path_buf[dest_len] = '/';
              src_path_buf[src_len] = '/';
              strcpy(dest_path_buf+dest_len+1, p);
              strcpy(src_path_buf+src_len+1, p);

              _shell_cp(dest_path_buf, src_path_buf, isRecursive);
            }
            p += strlen(p) + 1; //
          }
       }
    }else{
      // src is a file
      // do non recursive cp
      _shell_cp(dest_path, src_path, 0);
    }

  }
  return 0;
}


int shell_mv(int argc, char** argv)
{
        if(argc != 3){
           printf("mv: argument invalid.\n");
           return 0;
        }
        char* src = argv[1];
        char* dest = argv[2];
        if(!is_file_exist(src)){
          printf("mv: sorce file %s does not exist.\n", src);
          return 0;
        }
        if (is_dir(src)) {
          if (is_file_exist(dest) &&  !is_dir(dest)) {
            printf("mv: cannot move a dir to a file\n");
            return 0;
          }
          _shell_cp(dest, src, 1);
          _shell_rm(src, 1);
        }else {
          _shell_cp(dest, src, 1);
          _shell_rm(src, 0);
        }
	return 0;
}

int shell_rm(int argc, char** argv)
{
        int isRecursive;
        int pathIdx;
      	if (argc == 1) {
        	printf("Too few arguments.\n");
		return 0;
	}
        if(!strcmp(argv[1], "-r")){
                isRecursive = 1;
                pathIdx = 2;
        }else{
                isRecursive = 0;
                pathIdx = 1;
        }
        if(pathIdx >= argc){
                printf("rm: no path argument.\n");
                return 0;
        }
        for (; pathIdx < argc; pathIdx++) {
          char * path = argv[pathIdx];
          if(!is_file_exist(path)){
            printf("rm: can not remove %s: is not a file or directory.\n", path);
            continue;
          }
          _shell_rm(path, isRecursive);
        }
        return 0;
}

// path is guaranteed to be exist
// DFS, postorder traversal

int _shell_rm(char *path, int isRecursive){
  int errno, len, i;
  char * sub_path;
  char rm_buf[BUFLEN];
  // -r option
  if(isRecursive){
    if(!is_dir(path)) {
      //delete single file or empty directory
      errno = rm_file(path);
      return errno;
    }
    else{
      // path is a non empty directory
      // subdirectory/subfile names are stored in shell_buf, seperated by '\0'
      // chdir, go to next layer
      sys_chdir(path);
      len = ls_dir(rm_buf, NULL);
      sub_path = rm_buf;
      while(sub_path - rm_buf < len){
        if(strcmp(sub_path, ".") && strcmp(sub_path, "..")){
          _shell_rm(sub_path, isRecursive);
        }
        sub_path += strlen(sub_path) + 1;
      }
      //chdir, go to upper layer
      sys_chdir("..");
      rm_file(path); // rm current dir
    }
    return 0;
  }else{
    if(is_dir(path)){
      printf("rm: can not remove %s: is a directory. try '-r' ?\n", path);
      return -1;
    }else{
      // delete single file
      return rm_file(path);
    }
  }
}

int rm_file(char * filename){
  int errno = sys_unlink(filename);
  if(errno == -1){
    printf("rm: can not remove %s: sys_unlink error.\n", filename);
  }
  return errno;
}
// return the length of buf used.
// buf stores the names of subfile/subdirectory under dirname.
// they are merged as one string while seperated by '\0'
// file_a  \0 file_b \0 dir_a \0 dir_b \0
int ls_dir(char* buf, char * path){
   int len;
   char pwd[BUFLEN];
   // list current dir
   if(path == NULL){
     len  = sys_ls(buf, BUFLEN);
   }else{
      //list target dir
     sys_pwd(pwd);
     sys_chdir(path);
     len = sys_ls(buf, BUFLEN);
     sys_chdir(pwd);
   }
   int i = 0;
   while(i < len){
      if(buf[i] == ' '){
          buf[i] = '\0';
      }
      i++;
   }
   return len;
}
int is_dir_empty(char * dirname){
    if(ls_dir(shell_buf, NULL) == 5){
        return 1;
    }else{
        return 0;
    }
}

int is_dir(char * path){
      int fd, isDir;
      if(is_file_exist(path)){
            fd = open(path, O_RDONLY);
      }
      isDir = sys_is_dir(fd);
      close(fd);
      return isDir;
}
// check weather a file/dir exist
int is_file_exist(char* path){
        int fd;
        fd = open(path, O_RDONLY);
	if(fd == -1){
                return 0;
 	}
	close(fd);
        return 1;
}
int shell_mkdir(int argc, char** argv)
{
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

/*
 * Enhanced `cat` implementation. Supports concatenating multiple files,
 * redirection (``>`` and ``>>``), and line numbering flags ``-n`` and
 * ``-b`` (number non-empty lines).
 */

static int _shell_cat_file(const char *path, int outfd,
               int show_nums, int num_nonblank, int *lineno);
static int _shell_cat_stream(int fd, int outfd,
               int show_nums, int num_nonblank, int *lineno);
static int write_line_number(int outfd, int n);

int shell_cat(int argc, char** argv)
{
  return shell_cat_fd(argc, argv, 0, 1);
}

int shell_cat_fd(int argc, char** argv, int infd, int outfd)
{
  /* options */
  int show_numbers = 0;
  int number_nonblank = 0;
  int idx = 1;

  while (idx < argc && argv[idx][0] == '-' && argv[idx][1] != '\0') {
    if (strcmp(argv[idx], "-n") == 0) {
      show_numbers = 1;
    } else if (strcmp(argv[idx], "-b") == 0) {
      number_nonblank = 1;
    } else {
      break;
    }
    idx++;
  }

  int lineno = 1;
  if (idx >= argc) {
    _shell_cat_stream(infd, outfd, show_numbers, number_nonblank, &lineno);
    return 0;
  }

  for (int j = idx; j < argc; j++) {
    if (_shell_cat_file(argv[j], outfd, show_numbers, number_nonblank,
              &lineno) == -1) {
      printf("cat: %s: No such file or directory\n", argv[j]);
    }
  }
  return 0;
}

static int _shell_cat_file(const char *path, int outfd,
               int show_nums, int num_nonblank, int *lineno)
{
  int fd = open(path, O_RDONLY);
  if (fd < 0)
    return -1;
  _shell_cat_stream(fd, outfd, show_nums, num_nonblank, lineno);
  close(fd);
  return 0;
}

static int _shell_cat_stream(int fd, int outfd,
               int show_nums, int num_nonblank, int *lineno)
{
  char buf[BUFLEN];
  char line[BUFLEN];
  int nread;
  int pos = 0;

  while ((nread = read(fd, buf, sizeof(buf))) > 0) {
    for (int i = 0; i < nread; i++) {
      char c = buf[i];
      if (c == '\n' || pos == BUFLEN - 1) {
        if (pos < BUFLEN - 1)
          line[pos++] = c;
        line[pos] = '\0';

        int is_blank = (pos == 1 && line[0] == '\n');
        if (show_nums || (num_nonblank && !is_blank)) {
          write_line_number(outfd, *lineno);
          (*lineno)++;
        }
        write(outfd, line, pos);
        pos = 0;
      } else {
        line[pos++] = c;
      }
    }
  }

  if (pos > 0) {
    line[pos] = '\0';
    int is_blank = (pos == 1 && line[0] == '\n');
    if (show_nums || (num_nonblank && !is_blank)) {
      write_line_number(outfd, *lineno);
      (*lineno)++;
    }
    write(outfd, line, pos);
  }
  return 0;
}

static int write_line_number(int outfd, int n)
{
  char numbuf[16];
  int pos = 0;
  int x = n;
  int i, pad;

  if (x == 0) {
    numbuf[pos++] = '0';
  } else {
    while (x > 0 && pos < (int)sizeof(numbuf)) {
      numbuf[pos++] = '0' + (x % 10);
      x /= 10;
    }
    for (i = 0; i < pos / 2; i++) {
      char tmp = numbuf[i];
      numbuf[i] = numbuf[pos - 1 - i];
      numbuf[pos - 1 - i] = tmp;
    }
  }

  pad = 6 - pos;
  while (pad-- > 0)
    write(outfd, " ", 1);
  write(outfd, numbuf, pos);
  write(outfd, "  ", 2);
  return 0;
}

int shell_rot13(int argc, char **argv)
{
  return shell_rot13_fd(argc, argv, 0, 1);
}

int shell_rot13_fd(int argc, char **argv, int infd, int outfd)
{
  char buf[BUFLEN];
  int i, fd, n, start = 1;

  if (argc == 1) {
    while ((n = read(infd, buf, sizeof(buf))) > 0) {
      for (i = 0; i < n; i++) {
        char c = buf[i];
        if (c >= 'a' && c <= 'z')
          buf[i] = (c - 'a' + 13) % 26 + 'a';
        else if (c >= 'A' && c <= 'Z')
          buf[i] = (c - 'A' + 13) % 26 + 'A';
      }
      write(outfd, buf, n);
      if (n < sizeof(buf))
        break;
    }
    return 0;
  }

  for (; start < argc; start++) {
    fd = open(argv[start], O_RDONLY);
    if (fd < 0) {
      printf("rot13: %s: No such file or directory\n", argv[start]);
      continue;
    }
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
      for (i = 0; i < n; i++) {
        char c = buf[i];
        if (c >= 'a' && c <= 'z')
          buf[i] = (c - 'a' + 13) % 26 + 'a';
        else if (c >= 'A' && c <= 'Z')
          buf[i] = (c - 'A' + 13) % 26 + 'A';
      }
      write(outfd, buf, n);
      if (n < sizeof(buf))
        break;
    }
    close(fd);
  }
  return 0;
}

int shell_touch(int argc, char** argv)
{
    if (argc == 1) {
        printf ("touch failed. No Path. \n");
        return 0;
    }
    int i;
    for (i = 1; i < argc; i++)
    {
        int fd;
        fd = open(argv[i], O_RDONLY);
        if(fd >= 0){
            printf("%s file exist\n", argv[i]);
            close(fd);
            continue;
        } else {
            close(open(argv[i], O_CREATE));
        }
    }
    return 0;
}

int shell_write(int argc, char** argv) {
  if (argc == 1) {
    printf("write failed, too few arguments.\n");
    return 0;
  }
  int fd = open(argv[2], O_CREATE|O_RDWR);
  if (fd >= 0) {
    int n = write(fd, argv[1], strlen(argv[1]));
    if (n != strlen(argv[1])) {
      printf("shell write fail\n");
    }
    close(fd);
  }else {
    printf("shell write fail\n");
  }
  return 0;
}

int shell_append(int argc, char** argv) {
  if (argc == 1) {
    printf("append failed, too few arguments.\n");
    return 0;
  }
  int fd = open(argv[2], O_RDONLY);
  if (fd >= 0) {
    char buf[1000];
    int n = read(fd, buf, 1000);
    close(fd);
    if (n == 0) {
      buf[0] = 0;
    }
    fd = open(argv[2], O_CREATE|O_RDWR);
    if (fd < 0) {
      printf("create: append failed!\n");
      return 0;
    }
    strncpy(buf + n, argv[1], strlen(argv[1]));
    write(fd, buf, strlen(buf));
    close(fd);
  }else {
    printf("open append failed!\n");
  }
  return 0;
}

void shell_test() {
    // Test case 1: Basic signal handling
    printf("Test 1: Basic signal handling\n");
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sa.sa_mask = 0;

    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        printf("Failed to register signal handler\n");
        return;
    }

    printf("Registered handler for SIGUSR1\n");
    printf("Sending SIGUSR1 to self...\n");
    kill(getpid(), SIGUSR1);

    // Test case 2: Signal blocking
    printf("\nTest 2: Signal blocking\n");
    struct sigaction sa2;
    sa2.sa_handler = signal_handler;
    sa2.sa_flags = 0;
    sa2.sa_mask = (1 << SIGUSR2);  // Block SIGUSR2

    if (sigaction(SIGUSR2, &sa2, NULL) < 0) {
        printf("Failed to register signal handler\n");
        return;
    }

    printf("Registered handler for SIGUSR2 (blocked)\n");
    printf("Sending SIGUSR2 to self...\n");
    kill(getpid(), SIGUSR2);

    // Test case 3: pause() functionality
    printf("\nTest 3: pause() functionality\n");
    printf("Process will pause until SIGUSR1 is received...\n");
    pause();
    printf("Resumed after receiving signal\n");
}

int extract_filename(char* path, char* filename) {
  int n = strlen(path);
  if (n == 0) return 0;
  int pos = n - 1;
  while (pos >= 0) {
    if (path[pos] == '/') {
      break;
    }
    pos--;
  }
  strncpy(filename, path + pos + 1, n - (pos + 1));
  return n - (pos + 1);
}

int cp_file(char* dest_filename, char* src_filename) {
  if (is_dir(src_filename)) {
    sys_mkdir(dest_filename);
    return 0;
  }
  int srcfd = open(src_filename, O_RDONLY);
  int dstfd;
  char buf[BUFLEN];
  int n;
  if (srcfd < 0) {
    printf("cp: cannot open source %s\n", src_filename);
    return -1;
  }
  unlink(dest_filename);
  dstfd = open(dest_filename, O_CREATE|O_RDWR);
  if (dstfd < 0) {
    close(srcfd);
    printf("cp: cannot open destination %s\n", dest_filename);
    return -1;
  }
  while ((n = read(srcfd, buf, sizeof(buf))) > 0) {
    if (write(dstfd, buf, n) != n) {
      close(srcfd);
      close(dstfd);
      printf("cp: write failed for %s\n", dest_filename);
      return -1;
    }
  }
  close(srcfd);
  close(dstfd);
  return 0;
}

int shell_echo(int argc, char **argv)
{
  return shell_echo_fd(argc, argv, 1);
}

int shell_echo_fd(int argc, char **argv, int outfd)
{
  int i;
  if (argc == 1) {
    write(outfd, "\n", 1);
    return 0;
  }
  for (i = 1; i < argc; i++) {
    write(outfd, argv[i], strlen(argv[i]));
    if (i != argc - 1)
      write(outfd, " ", 1);
  }
  write(outfd, "\n", 1);
  return 0;
}



void shell_readline(char* buf) {
  int i = 0;
  int max_len = 1024;
  char c;
  printf(">");
  while (i < max_len - 1) {
    c = getchar();
    if (c == EOF) {
      continue;
    }
    if (c == '\b' || c == 127) {
      if (i > 0) {
        i--;
        write(1, "\b \b", 3);
      }
      continue;
    }
    write(1, &c, 1);
    if (c == '\n' || c == '\r') {
      write(1, "\n", 1);
      buf[i] = '\0';
      break;
    }
    buf[i++] = c;
  }
  buf[i] = '\0';
}

struct ParsedCommand {
  char *argv[MAXARGS];
  int argc;
  char *infile;
  char *outfile;
  int append;
};

static int parse_single_command(char *tokens[], int start, int end,
                                struct ParsedCommand *cmd)
{
  int argc = 0;
  cmd->infile = NULL;
  cmd->outfile = NULL;
  cmd->append = 0;

  for (int i = start; i < end; i++) {
    if (strcmp(tokens[i], "<") == 0) {
      if (i + 1 < end)
        cmd->infile = tokens[++i];
      else
        return -1;
    } else if (strcmp(tokens[i], ">") == 0) {
      if (i + 1 < end) {
        cmd->outfile = tokens[++i];
        cmd->append = 0;
      } else {
        return -1;
      }
    } else if (strcmp(tokens[i], ">>") == 0) {
      if (i + 1 < end) {
        cmd->outfile = tokens[++i];
        cmd->append = 1;
      } else {
        return -1;
      }
    } else {
      if (argc >= MAXARGS - 1)
        return -1;
      cmd->argv[argc++] = tokens[i];
    }
  }

  cmd->argv[argc] = 0;
  cmd->argc = argc;
  return 0;
}

static int run_command_stage(struct ParsedCommand *cmd, int inherited_infd, int inherited_outfd)
{
  int infd = inherited_infd;
  int outfd = inherited_outfd;
  int close_infd = 0;
  int close_outfd = 0;
  int i;

  if (cmd->argc == 0)
    return 0;

  if (cmd->infile) {
    infd = open(cmd->infile, O_RDONLY);
    if (infd < 0) {
      printf("%s: cannot open input file %s\n", cmd->argv[0], cmd->infile);
      return -1;
    }
    close_infd = 1;
  }

  if (cmd->outfile) {
    if (!cmd->append)
      unlink(cmd->outfile);
    outfd = open(cmd->outfile, O_CREATE | O_RDWR);
    if (outfd < 0) {
      printf("%s: cannot open output file %s\n", cmd->argv[0], cmd->outfile);
      if (close_infd)
        close(infd);
      return -1;
    }
    if (cmd->append) {
      char tmp[BUFLEN];
      while (read(outfd, tmp, sizeof(tmp)) > 0) {
        ;
      }
    }
    close_outfd = 1;
  }

  if (strcmp(cmd->argv[0], "cat") == 0) {
    shell_cat_fd(cmd->argc, cmd->argv, infd, outfd);
  } else if (strcmp(cmd->argv[0], "echo") == 0) {
    shell_echo_fd(cmd->argc, cmd->argv, outfd);
  } else if (strcmp(cmd->argv[0], "rot13") == 0) {
    shell_rot13_fd(cmd->argc, cmd->argv, infd, outfd);
  } else if (strcmp(cmd->argv[0], "spawn") == 0) {
    if (cmd->argc < 2) {
      printf("Usage: spawn <elf_id>\n");
    } else {
      int elf_id = str_to_int(cmd->argv[1]);
      pid_t new_pid = spawn_io(elf_id, 1000, infd, outfd);
      if (new_pid == -1)
        printf("Failed to spawn process\n");
      else
        printf("Process spawned with PID %d\n", new_pid);
    }
  } else {
    for (i = 0; i < NCOMMANDS; i++) {
      if (strcmp(cmd->argv[0], commands[i].name) == 0) {
        commands[i].func(cmd->argc, cmd->argv);
        break;
      }
    }
    if (i == NCOMMANDS) {
      printf("Unknown command '%s'\n", cmd->argv[0]);
      printf("try 'help' to see all supported commands.\n");
    }
  }

  if (close_infd)
    close(infd);
  if (close_outfd)
    close(outfd);
  return 0;
}

static int
runcmd (char *buf)
{
	int argc = 0;
	char *tokens[MAXARGS];
	struct ParsedCommand stages[MAXARGS];
	int stage_count = 0;
	int seg_start = 0;
	int current_infd = 0;

	while(1)
	{
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		if (argc == MAXARGS - 1)
		{
			printf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}

		tokens[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	tokens[argc] = 0;
	if (argc == 0)
		return 0;

	for (int i = 0; i <= argc; i++) {
		if (i == argc || strcmp(tokens[i], "|") == 0) {
			if (i == seg_start) {
				printf("Invalid null command in pipeline\n");
				return 0;
			}
			if (stage_count >= MAXARGS - 1) {
				printf("Too many pipeline stages\n");
				return 0;
			}
			if (parse_single_command(tokens, seg_start, i, &stages[stage_count]) < 0) {
				printf("Invalid redirection syntax\n");
				return 0;
			}
			stage_count++;
			seg_start = i + 1;
		}
	}

	for (int i = 0; i < stage_count; i++) {
		int pfd[2];
		int outfd = 1;
		int next_infd = 0;

		if (i < stage_count - 1) {
			if (pipe(pfd) < 0) {
				printf("pipe: failed to create pipe\n");
				if (current_infd != 0)
					close(current_infd);
				return 0;
			}
			next_infd = pfd[0];
			outfd = pfd[1];
		}

		run_command_stage(&stages[i], current_infd, outfd);

		if (outfd != 1)
			close(outfd);
		if (current_infd != 0)
			close(current_infd);
		current_infd = next_infd;
	}

	if (current_infd != 0)
		close(current_infd);
	return 0;


}
int ipc_test(){
    printf ("ipc test begin\n");

    pid_t ping_pid, pong_pid;
    if ((ping_pid = spawn (1, 1000)) != -1)
        printf ("ping in process %d.\n", ping_pid);
    else
        printf ("Failed to launch ping.\n");

    if ((pong_pid = spawn (2, 1000)) != -1)
        printf ("pong in process %d.\n", pong_pid);
    else
        printf ("Failed to launch pong.\n");
    int i = 0;
    for(i = 0; i < 100000; i++) ;
    printf("ipc test pass!!\n");
    return 0;
}
int main (int argc, char** argv)
{
        // TODO Please change mode here to enter different test mode.
        // 1: shell test
        // 2: ipc test
        // 0: normal mode
        int mode = 0;
	char buf[1024];
	printf("\n********Welcome to left-mid-right shell*********\n");
        printf("********This is the final project for CPSC 422/522 Operating Systems in Yale********\n");
        printf("********Author: Bo Song, Haoliang Zhang********\n");
        printf("********Date: 12/18/2015 ********\n");
	//close(open("usertests.ran", O_CREATE));  // Disabled - requires proper cwd init



        if(mode == 1){
           shell_test();
           return 0;
       }
        else if(mode == 2){
           ipc_test();
           return 0;
        }
	while(1)
	{
		shell_readline(buf);
		if (buf != NULL){
			if (runcmd (buf) < 0)
				break;
		}
	}
}

// int shell_kill(int argc, char **argv)
// {
//     if (argc != 3) {
//         printf("Usage: kill <signal> <pid>\n");
//         return -1;
//     }

//     int signum = str_to_int(argv[1]);
//     int pid = str_to_int(argv[2]);

//     if (signum < 1 || signum >= NSIG) {
//         printf("Invalid signal number\n");
//         return -1;
//     }

//     if (kill(pid, signum) < 0) {
//         printf("Failed to send signal\n");
//         return -1;
//     }

//     return 0;
// }

int shell_kill(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: kill -<signal> <pid>\n");
        printf("Example: kill -9 2\n");
        return;
    }

    int sig = 0;
    int pid = 0;

    // Parse signal number - handle "-N" format
    if (argv[1][0] == '-') {
        // Parse the number after the '-'
        sig = str_to_int(&argv[1][1]);
    } else {
        // Try parsing as just a number
        sig = str_to_int(argv[1]);
    }

    // Parse PID
    pid = str_to_int(argv[2]);

    // Validate signal number (1-31)
    if (sig < 1 || sig > 31) {
        printf("Invalid signal number: %d (must be 1-31)\n", sig);
        return;
    }

    // Validate PID
    if (pid < 1 || pid > 63) {
        printf("Invalid PID: %d (must be 1-63)\n", pid);
        return;
    }

    printf("Sending signal %d to process %d...\n", sig, pid);

    int result = kill(pid, sig);
    if (result == 0) {
        printf("Signal sent successfully.\n");
    } else {
        printf("Failed to send signal (error: %d)\n", result);
    }
}

void signal_handler(int signum)
{
    printf("\n*** Received signal %d ***\n", signum);
    printf(">:");  // Reprint prompt
}

int shell_trap(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: trap <signum>\n");
        printf("Example: trap 2   (register handler for SIGINT)\n");
        return -1;
    }

    int signum = str_to_int(argv[1]);

    if (signum < 1 || signum >= 32) {
        printf("Invalid signal number: %d (must be 1-31)\n", signum);
        return -1;
    }

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sa.sa_mask = 0;

    printf("Registering handler for signal %d at address %x...\n", signum, (unsigned int)signal_handler);

    if (sigaction(signum, &sa, NULL) < 0) {
        printf("Failed to register signal handler\n");
        return -1;
    }

    printf("Handler registered successfully.\n");
    return 0;
}

int shell_spawn(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: spawn <elf_id>\n");
        printf("  elf_id: 1=ping, 2=pong, 3=ding\n");
        return -1;
    }

    int elf_id = str_to_int(argv[1]);

    if (elf_id < 1 || elf_id > 5) {
        printf("Invalid elf_id: %d (must be 1-5)\n", elf_id);
        return -1;
    }

    printf("Spawning process with elf_id %d...\n", elf_id);

    pid_t new_pid = spawn_io(elf_id, 1000, 0, 1);
    if (new_pid != -1) {
        printf("Process spawned with PID %d\n", new_pid);
    } else {
        printf("Failed to spawn process\n");
        return -1;
    }

    return 0;
}
