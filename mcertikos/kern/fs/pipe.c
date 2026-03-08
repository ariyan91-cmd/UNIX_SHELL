//
// Pipe implementation
//

#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/spinlock.h>
#include <kern/lib/string.h>
#include "pipe.h"

#define NPIPE 16

struct {
  spinlock_t lock;
  struct pipe pipe[NPIPE];
} ptable;

void
pipe_init(void)
{
  spinlock_init(&ptable.lock);
}

/**
 * Allocate a pipe and return its pointer.
 */
struct pipe*
pipe_alloc(void)
{
  struct pipe *p;

  spinlock_acquire(&ptable.lock);
  for(p = ptable.pipe; p < ptable.pipe + NPIPE; p++){
    if(p->readopen == 0 && p->writeopen == 0){
      p->nread = 0;
      p->nwrite = 0;
      p->readopen = 1;
      p->writeopen = 1;
      spinlock_init(&p->lock);
      spinlock_release(&ptable.lock);
      return p;
    }
  }
  spinlock_release(&ptable.lock);
  return 0;
}

/**
 * Read from pipe p.
 */
int
pipe_read(struct pipe *p, char *addr, int n)
{
  int i;

  spinlock_acquire(&p->lock);
  while(p->nread == p->nwrite && p->writeopen){
    // pipe is empty and write end is still open
    // in real OS, would sleep here
    spinlock_release(&p->lock);
    return 0; // Return 0 bytes for now (non-blocking)
  }
  for(i = 0; i < n; i++){
    if(p->nread == p->nwrite)
      break;
    addr[i] = p->data[p->nread++ % PIPESIZE];
  }
  spinlock_release(&p->lock);
  return i;
}

/**
 * Write to pipe p.
 */
int
pipe_write(struct pipe *p, char *addr, int n)
{
  int i;

  spinlock_acquire(&p->lock);
  for(i = 0; i < n; i++){
    if(p->nwrite - p->nread >= PIPESIZE){
      // pipe is full
      spinlock_release(&p->lock);
      return i; // Return bytes written so far
    }
    p->data[p->nwrite++ % PIPESIZE] = addr[i];
  }
  spinlock_release(&p->lock);
  return n;
}

/**
 * Close read end of pipe.
 */
void
pipe_close_read(struct pipe *p)
{
  spinlock_acquire(&p->lock);
  p->readopen = 0;
  spinlock_release(&p->lock);
}

/**
 * Close write end of pipe.
 */
void
pipe_close_write(struct pipe *p)
{
  spinlock_acquire(&p->lock);
  p->writeopen = 0;
  spinlock_release(&p->lock);
}
