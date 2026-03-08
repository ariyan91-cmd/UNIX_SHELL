#include <proc.h>
#include <syscall.h>
#include <types.h>

pid_t
spawn(uintptr_t exec, unsigned int quota)
{
	return sys_spawn(exec, quota);
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

