#ifndef _KERN_FS_PIPE_H_
#define _KERN_FS_PIPE_H_

#include <kern/lib/types.h>
#include <kern/lib/spinlock.h>

#define PIPESIZE 512

struct pipe {
  spinlock_t lock;
  char data[PIPESIZE];
  uint32_t nread;     // number of bytes read
  uint32_t nwrite;    // number of bytes written
  int readopen;       // read fd is still open
  int writeopen;      // write fd is still open
};

// Allocate a pipe
struct pipe* pipe_alloc(void);

// Read from a pipe
int pipe_read(struct pipe *p, char *addr, int n);

// Write to a pipe
int pipe_write(struct pipe *p, char *addr, int n);

// Close a pipe for reading
void pipe_close_read(struct pipe *p);

// Close a pipe for writing
void pipe_close_write(struct pipe *p);

#endif /* !_KERN_FS_PIPE_H_ */
