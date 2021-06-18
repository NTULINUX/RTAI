/*
 * Architecture specific scheduler support.
 *
 * Original RTAI/x86 layer implementation:
 *   Copyright (c) 2000 Paolo Mantegazza (mantegazza@aero.polimi.it)
 *   Copyright (c) 2000 Steve Papacharalambous (stevep@zentropix.com)
 *   Copyright (c) 2000 Stuart Hughes
 *   and others.
 *
 * RTAI/x86 rewrite over Adeos:
 *   Copyright (c) 2002 Philippe Gerum (rpm@xenomai.org)
 *
 * Original RTAI/ARM RTHAL implementation:
 *   Copyright (c) 2000 Pierre Cloutier (pcloutier@poseidoncontrols.com)
 *   Copyright (c) 2001 Alex Züpke, SYSGO RTS GmbH (azu@sysgo.de)
 *   Copyright (c) 2002 Guennadi Liakhovetski DSA GmbH (gl@dsa-ac.de)
 *   Copyright (c) 2002 Steve Papacharalambous (stevep@zentropix.com)
 *   Copyright (c) 2002 Wolfgang Müller (wolfgang.mueller@dsa-ac.de)
 *   Copyright (c) 2003 Bernard Haible, Marconi Communications
 *   Copyright (c) 2003 Thomas Gleixner (tglx@linutronix.de)
 *   Copyright (c) 2003 Philippe Gerum (rpm@xenomai.org)
 *
 * RTAI/ARM over Adeos rewrite:
 *   Copyright (c) 2004-2005 Michael Neuhauser, Firmix Software GmbH (mike@firmix.at)
 *
 * RTAI/ARM over Adeos rewrite for PXA255_2.6.7:
 *   Copyright (c) 2005 Stefano Gafforelli (stefano.gafforelli@tiscali.it)
 *   Copyright (c) 2005 Luca Pizzi (lucapizzi@hotmail.com)
 *
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _RTAI_ASM_ARM_RTAI_SCHED_H
#define _RTAI_ASM_ARM_RTAI_SCHED_H

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#	include <asm/proc/ptrace.h>
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
#	include <asm/ptrace.h>
#	define	I_BIT PSR_I_BIT
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */

#include <rtai_schedcore.h>

/*
 * Registers according to the ARM procedure call standard:
 *   Reg	Description
 *   r0-r3	argument/scratch registers
 *   r4-r9	variable register
 *   r10=sl	stack limit/variable register
 *   r11=fp	frame pointer/variable register
 *   r12=ip	intra-procedure-call scratch register
 *   r13=sp	stack pointer (auto preserved)
 *   r14=lr	link register
 *   r15=pc	program counter (auto preserved)
 *
 * Take special care to restore r0/r1 for startup of new task (->
 * init_arch_stack, rt_startup)!
 *
 * Stack layout:
 *
 *	Offset	Content
 *
 *	12	r14=lr (i.e. resume point for task)
 *	11	r11=fp
 *	10	r10=sl
 *	 9	r9
 *	 8	r8
 *	 7	r7
 *	 6	r6
 *	 5	r5
 *	 4	r4
 *	 3	cpsr
 *	 2	domain_access_control
 *	 1	r1 (startup: data)
 *	 0	r0 (startup: rt_thread)
 */

#define rt_exchange_tasks(old_task, new_task) \
    asm volatile( \
	"mrc	p15, 0, r2, c3, c0\n\t"		/* get current domain_access_control */ \
	"adr	lr, 1f\n\t"			/* get address of resume point */ \
	"mrs	r3, cpsr\n\t"			/* get current cpsr */ \
	"ldr	ip, [%[oldp]]\n\t"		/* get pointer to old task */ \
	"stmfd	sp!, {r0 - fp, lr}\n\t"		/* push registers on stack */ \
	"str	%[new], [%[oldp]]\n\t"		/* *oldp = new */ \
	"str	sp, [ip]\n\t"			/* save current stack-pointer to old task */ \
	"ldr	sp, [%[new]]\n\t"		/* get stack-pointer of new task */ \
	"ldmfd	sp!, {r0-r3}\n\t"		/* pop new tasks' r0, r1, cpsr & d.a.c from stack */ \
	"mcr	p15, 0, r2, c3, c0\n\t"		/* restore previous domain_access_control */ \
	"msr	cpsr_c, %[psr]\n\t"		/* disable hw-irqs */ \
	"msr	spsr, r3\n\t"			/* set spsr to previous cpsr */ \
	"ldmfd	sp!, {r4 - fp, pc}^\n\t"	/* pop registers, pc = resume point, cpsr = spsr */ \
	"1:"					/* resume point (except for startup) */ \
	: /* output  */ /* none */ \
	: /* input   */	[oldp] "r" (&old_task), \
			[new]  "r" (new_task), \
			[psr]  "i" (SVC_MODE|I_BIT) \
	: /* clobber */ "lr", "ip", "r2", "r3", "memory" \
    )

extern inline unsigned long
current_domain_access_control(void)
{
    unsigned long domain_access_control;
    asm("mrc p15, 0, %0, c3, c0" : "=r" (domain_access_control));
    return domain_access_control;
}

/*
 * Set up stack for new proper RTAI task (i.e. kernel task without
 * kernel-thread). This is called in rt_task_init_cpuid() where the following
 * variables are used (should be arguments to macro!):
 *	rt_thread	pointer to tasks function
 *	data		integer to pass to rt_thread
 *
 * The stack needs to be set-up so that a rt_exchange_tasks() on the newly
 * created task will call rt_startup(rt_thread = r0, data = r1).
 *
 * See rt_exchange_tasks() above for the stack layout.
 *
 */
#define	init_arch_stack()						\
    do {								\
	task->stack -= 13; /* make room on stack */			\
	task->stack[12] = (int)rt_startup; /* entry point */		\
	task->stack[ 3] = SVC_MODE;	   /* cpsr */			\
	task->stack[ 2] = (int)current_domain_access_control();		\
	task->stack[ 1] = (int)data;	   /* arg 2 of rt_startup() */	\
	task->stack[ 0] = (int)rt_thread;  /* arg 1 "  " */		\
    } while (0)

#define DEFINE_LINUX_CR0
#define DEFINE_LINUX_SMP_CR0

#ifdef CONFIG_RTAI_FP_SUPPORT
#define init_fp_env() \
do { \
      memset(&task->fpu_reg, 0, sizeof(task->fpu_reg)); \
}while(0)
#else
#define init_fp_env() 
#endif

#define init_task_fpenv(task)  do { init_fpenv((task)->fpu_reg); } while(0)

extern inline void *
get_stack_pointer(void)
{
    void *sp;
    asm("mov %0, sp" : "=r" (sp));
    return sp;
}

/* acknowledge timer interrupt in scheduler's timer-handler (using the
 * arch-specific rtai_timer_irq_ack()) also allows to bail out of timer irq
 * handler (because of spurious interrupt or whatever) */
#ifndef CONFIG_ARCH_AT91
#define DO_TIMER_PROPER_OP()		\
do {					\
	rtai_timer_irq_ack();		\
} while (0)
#else
/* since we are using extern_timer_isr in __ipipe_grab_irq
 * we need to update tsc manually in periodic mode*/
#define DO_TIMER_PROPER_OP()		\
do {					\
	if(rt_periodic) {		\
		rtai_at91_update_tsc();	\
	}				\
} while (0)
#endif

#endif /* _RTAI_ASM_ARM_RTAI_SCHED_H */
