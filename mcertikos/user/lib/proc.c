#include <proc.h>
#include <syscall.h>
#include <types.h>

pid_t
spawn(uintptr_t exec, unsigned int quota)
{
	return sys_spawn(exec, quota);
}

pid_t
spawn_io(uintptr_t exec, unsigned int quota, int infd, int outfd)
{
	return sys_spawn_io(exec, quota, infd, outfd);
}

void
yield(void)
{
	sys_yield();
}

// Removed: sys_produce and sys_consume deprecated syscalls no longer available
// void
// produce(void)
// {
//	sys_produce();
// }

// void
// consume(void)
// {
//	sys_consume();
// }
