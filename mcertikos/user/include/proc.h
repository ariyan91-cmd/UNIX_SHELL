#ifndef _USER_PROC_H_
#define _USER_PROC_H_

#include <types.h>

pid_t spawn(uintptr_t exe, unsigned int quota);
pid_t spawn_io(uintptr_t exe, unsigned int quota, int infd, int outfd);
void  yield(void);

#endif /* !_USER_PROC_H_ */
