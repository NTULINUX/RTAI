/**
 *   @ingroup hal
 *   @file
 *
 *   Copyright: 2013 Paolo Mantegazza.
 *   Copyright: 2019 Alec Ari.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., either version 2 of the License, 
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _RTAI_ASM_X86_SCHED_H
#define _RTAI_ASM_X86_SCHED_H

#define rt_exchange_tasks(oldtask, newtask) \
	__asm__ __volatile__( \
	"pushq %%rdi\n\t" \
	"pushq %%rsi\n\t" \
	"pushq %%rdx\n\t" \
	"pushq %%rcx\n\t" \
	"pushq %%rax\n\t" \
	"pushq %%r8\n\t" \
	"pushq %%r9\n\t" \
	"pushq %%r10\n\t" \
	"pushq %%r11\n\t" \
	"pushq %%rbx\n\t" \
	"pushq %%rbp\n\t" \
	"pushq %%r12\n\t" \
	"pushq %%r13\n\t" \
	"pushq %%r14\n\t" \
	"pushq %%r15\n\t" \
\
	"pushq $1f\n\t" \
\
	"movq (%%rcx), %%rbx\n\t" \
	"movq %%rsp, (%%rbx)\n\t" \
	"movq (%%rdx), %%rsp\n\t" \
	"movq %%rdx, (%%rcx)\n\t" \
	"ret\n" \
\
"1:	 popq %%r15\n\t" \
	"popq %%r14\n\t" \
	"popq %%r13\n\t" \
	"popq %%r12\n\t" \
	"popq %%rbp\n\t" \
	"popq %%rbx\n\t" \
	"popq %%r11\n\t" \
	"popq %%r10\n\t" \
	"popq %%r9\n\t" \
	"popq %%r8\n\t" \
	"popq %%rax\n\t" \
	"popq %%rcx\n\t" \
	"popq %%rdx\n\t" \
	"popq %%rsi\n\t" \
	"popq %%rdi\n\t" \
\
	: \
	: "c" (&oldtask), "d" (newtask) \
	);

#define init_arch_stack() \
do { \
	*--(task->stack) = data;			\
	*--(task->stack) = (unsigned long) rt_thread;	\
	*--(task->stack) = 0;				\
	*--(task->stack) = (unsigned long) rt_startup;	\
} while(0)

#define DEFINE_LINUX_CR0      static unsigned long linux_cr0;

#define DEFINE_LINUX_SMP_CR0  static unsigned long linux_smp_cr0[NR_RT_CPUS];

#define init_task_fpenv(task)  do { init_fpenv((task)->fpu_reg); } while(0)

#define RT_SET_RTAI_TRAP_HANDLER(x)  rt_set_rtai_trap_handler(x)

#define DO_TIMER_PROPER_OP()

static inline void *get_stack_pointer(void)
{
	void *sp;
	asm volatile ("movq %%rsp, %0" : "=r" (sp));
	return sp;
}

#endif /* !_RTAI_ASM_X86_SCHED_H */
