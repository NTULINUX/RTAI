/*
 *   @ingroup hal
 *   @file
 *
 *   ARTI -- RTAI-compatible Adeos-based Real-Time Interface. Based on
 *   the original RTAI layer for x86.
 *
 *   Original RTAI/x86 layer implementation: \n
 *   Copyright &copy; 2000-2015 Paolo Mantegazza, \n
 *   Copyright &copy; 2000      Steve Papacharalambous, \n
 *   Copyright &copy; 2000      Stuart Hughes, \n
 *   and others.
 *
 *   RTAI/x86 rewrite over Adeos: \n
 *   Copyright &copy 2002 Philippe Gerum.
 *   Copyright &copy 2005 Paolo Mantegazza.
 *
 *   Unification of x86 architecture: \n
 *   Copyright &copy 2014-2016 Alec Ari.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, 
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

/**
 * @defgroup hal RTAI services functions.
 *
 * This module defines some functions that can be used by RTAI tasks, for
 * managing interrupts and communication services with Linux processes.
 *
 *@{*/


#undef CONFIG_TRACEPOINTS

#include <linux/module.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/console.h>
#include <asm/hw_irq.h>
#include <asm/irq.h>
#include <asm/desc.h>
#include <asm/io.h>
#include <asm/mmu_context.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/fixmap.h>
#include <asm/bitops.h>
#include <asm/mpspec.h>
#include <asm/io_apic.h>
#include <asm/apic.h>
#include <linux/sched/types.h>

#define __RTAI_HAL__
#define DEFINE_FPU_FPREGS_OWNER_CTX
#include <rtai.h>
#include <asm/rtai_hal.h>
#include <asm/rtai_lxrt.h>
#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <rtai_proc_fs.h>
#include <stdarg.h>

#ifdef CONFIG_IPIPE_LEGACY
#error "CONFIG_IPIPE_LEGACY MUST NOT BE ENABLED, RECONFIGURE LINUX AND REMAKE BOTH KERNEL AND RTAI."
#endif

#define RTAI_NR_IRQS  IPIPE_NR_IRQS

struct hal_domain_struct rtai_domain;

struct rtai_realtime_irq_s rtai_realtime_irq[RTAI_NR_IRQS];

static struct {
	unsigned long flags;
	int count;
} rtai_linux_irq[RTAI_NR_IRQS];

static struct {
	void (*k_handler)(void);
	long long (*u_handler)(unsigned long);
	unsigned long label;
} rtai_sysreq_table[RTAI_NR_SRQS];

static unsigned rtai_sysreq_virq;

static unsigned long rtai_sysreq_map = 1; /* srq 0 is reserved */

static unsigned long rtai_sysreq_pending;

static unsigned long rtai_sysreq_running;

static DEFINE_SPINLOCK(rtai_lsrq_lock);  // SPIN_LOCK_UNLOCKED

static volatile int rtai_sync_level;

static atomic_t rtai_sync_count = ATOMIC_INIT(1);

static RT_TRAP_HANDLER rtai_trap_handler;

struct rt_times rt_smp_times[RTAI_NR_CPUS];

struct rtai_switch_data rtai_linux_context[RTAI_NR_CPUS];

struct calibration_data rtai_tunables;

volatile unsigned long rtai_cpu_realtime;

struct global_lock rtai_cpu_lock[1];

unsigned long rtai_critical_enter (void (*synch)(void))
{
	unsigned long flags;

	flags = hal_critical_enter(synch);
	if (atomic_dec_and_test(&rtai_sync_count)) {
		rtai_sync_level = 0;
	} else if (synch != NULL) {
		printk(KERN_INFO "RTAI[hal]: warning: nested sync will fail.\n");
	}
	return flags;
}

void rtai_critical_exit (unsigned long flags)
{
	atomic_inc(&rtai_sync_count);
	hal_critical_exit(flags);
}

unsigned long IsolCpusMask = 0;
RTAI_MODULE_PARM(IsolCpusMask, ulong);

int rt_request_irq (unsigned irq, int (*handler)(unsigned irq, void *cookie), void *cookie, int retmode)
{
	int ret;
	 ret = ipipe_request_irq(&rtai_domain, irq, (void *)handler, cookie, NULL);
	if (!ret) {
		rtai_realtime_irq[irq].retmode = retmode ? 1 : 0;
		if (IsolCpusMask && irq < IPIPE_NR_XIRQS) {
			rtai_realtime_irq[irq].cpumask = rt_assign_irq_to_cpu(irq, IsolCpusMask);
		}
	}
	return ret;
}

int rt_release_irq (unsigned irq)
{
	ipipe_free_irq(&rtai_domain, irq);
	if (IsolCpusMask && irq < IPIPE_NR_XIRQS) {
		rt_assign_irq_to_cpu(irq, rtai_realtime_irq[irq].cpumask);
	}
	return 0;
}

int rt_set_irq_ack(unsigned irq, int (*irq_ack)(unsigned int, void *))
{
	if (irq >= RTAI_NR_IRQS) {
		return -EINVAL;
	}
	rtai_domain.irqs[irq].ackfn = irq_ack ? (void *)irq_ack : hal_root_domain->irqs[irq].ackfn;
	return 0;
}

void rt_set_irq_cookie (unsigned irq, void *cookie)
{
	if (irq < RTAI_NR_IRQS) {
		rtai_domain.irqs[irq].cookie = cookie;
	}
}

void rt_set_irq_retmode (unsigned irq, int retmode)
{
	if (irq < RTAI_NR_IRQS) {
		rtai_realtime_irq[irq].retmode = retmode ? 1 : 0;
	}
}


// A bunch of macros to support Linux developers moods in relation to 
// interrupt handling across various releases.
// Here we care about ProgrammableInterruptControllers (PIC) in particular.

// 1 - IRQs descriptor and chip
#define rtai_irq_desc(irq) (irq_to_desc(irq))[0]
#define rtai_irq_desc_chip(irq) (irq_to_desc(irq)->irq_data.chip)

// 2 - IRQs atomic protections
#define rtai_irq_desc_lock(irq, flags) raw_spin_lock_irqsave(&rtai_irq_desc(irq).lock, flags)
#define rtai_irq_desc_unlock(irq, flags) raw_spin_unlock_irqrestore(&rtai_irq_desc(irq).lock, flags)

// 3 - IRQs enabling/disabling naming and calling
#define rtai_irq_endis_fun(fun, irq) irq_##fun(&(rtai_irq_desc(irq).irq_data)) 

// 4 - IRQs affinity
#define rtai_irq_affinity(irq) (irq_to_desc(irq)->irq_common_data.affinity)

unsigned rt_startup_irq (unsigned irq)
{
	return rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(startup, irq);
}

void rt_shutdown_irq (unsigned irq)
{
	rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(shutdown, irq);
}

static inline void _rt_enable_irq (unsigned irq)
{
	if (rtai_irq_desc_chip(irq)->irq_enable) {
		rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(enable, irq);
	} else {
		rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(unmask, irq);
	}
}

void rt_enable_irq (unsigned irq)
{
	_rt_enable_irq(irq);
}

void rt_disable_irq (unsigned irq)
{
	if (rtai_irq_desc_chip(irq)->irq_disable) {
		rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(disable, irq);
	} else {
		rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(mask, irq);
	}
}

void rt_mask_and_ack_irq (unsigned irq)
{
	rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(mask_ack, irq);
}

void rt_mask_irq (unsigned irq)
{
	rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(mask, irq);
}

void rt_unmask_irq (unsigned irq)
{
	rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(unmask, irq);
}

void rt_ack_irq (unsigned irq)
{
	_rt_enable_irq(irq);
}

void rt_end_irq (unsigned irq)
{
	_rt_enable_irq(irq);
}

void rt_eoi_irq (unsigned irq)
{
        rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(eoi, irq);
}

int rt_request_linux_irq (unsigned irq, void *handler, char *name, void *dev_id)
{
	unsigned long flags;
	int retval;

	if (irq >= RTAI_NR_IRQS || !handler) {
		return -EINVAL;
	}

	rtai_save_flags_and_cli(flags);
	spin_lock(&rtai_irq_desc(irq).lock);
	if (rtai_linux_irq[irq].count++ == 0 && rtai_irq_desc(irq).action) {
		rtai_linux_irq[irq].flags = rtai_irq_desc(irq).action->flags;
		rtai_irq_desc(irq).action->flags |= IRQF_SHARED;
	}
	spin_unlock(&rtai_irq_desc(irq).lock);
	rtai_restore_flags(flags);

	retval = request_irq(irq, handler, IRQF_SHARED, name, dev_id);

	return retval;
}

int rt_free_linux_irq (unsigned irq, void *dev_id)
{
	unsigned long flags;

	if (irq >= RTAI_NR_IRQS || rtai_linux_irq[irq].count == 0) {
		return -EINVAL;
	}

	rtai_save_flags_and_cli(flags);
	free_irq(irq, dev_id);
	spin_lock(&rtai_irq_desc(irq).lock);
	if (--rtai_linux_irq[irq].count == 0 && rtai_irq_desc(irq).action) {
		rtai_irq_desc(irq).action->flags = rtai_linux_irq[irq].flags;
	}
	spin_unlock(&rtai_irq_desc(irq).lock);
	rtai_restore_flags(flags);

	return 0;
}

void rt_pend_linux_irq (unsigned irq)
{
	unsigned long flags;
	rtai_save_flags_and_cli(flags);
	hal_pend_uncond(irq, rtai_cpuid());
	rtai_restore_flags(flags);
}

RTAI_SYSCALL_MODE void usr_rt_pend_linux_irq (unsigned irq)
{
	unsigned long flags;
	rtai_save_flags_and_cli(flags);
	hal_pend_uncond(irq, rtai_cpuid());
	rtai_restore_flags(flags);
}

int rt_request_srq (unsigned label, void (*k_handler)(void), long long (*u_handler)(unsigned long))
{
	unsigned long flags;
	int srq;

	if (k_handler == NULL) {
		return -EINVAL;
	}

	rtai_save_flags_and_cli(flags);
	if (rtai_sysreq_map != ~0) {
		set_bit(srq = ffz(rtai_sysreq_map), &rtai_sysreq_map);
		rtai_sysreq_table[srq].k_handler = k_handler;
		rtai_sysreq_table[srq].u_handler = u_handler;
		rtai_sysreq_table[srq].label = label;
	} else {
		srq = -EBUSY;
	}
	rtai_restore_flags(flags);

	return srq;
}

int rt_free_srq (unsigned srq)
{
	return  (srq < 1 || srq >= RTAI_NR_SRQS || !test_and_clear_bit(srq, &rtai_sysreq_map)) ? -EINVAL : 0;
}

void rt_pend_linux_srq (unsigned srq)
{
	if (srq > 0 && srq < RTAI_NR_SRQS) {
		unsigned long flags;
		set_bit(srq, &rtai_sysreq_pending);
		rtai_save_flags_and_cli(flags);
		hal_pend_uncond(rtai_sysreq_virq, rtai_cpuid());
		rtai_restore_flags(flags);
	}
}

#include <linux/ipipe_tickdev.h>
void rt_linux_hrt_set_mode(enum clock_event_mode mode, struct clock_event_device *hrt_dev)
{
	if (mode == CLOCK_EVT_MODE_ONESHOT || mode == CLOCK_EVT_MODE_SHUTDOWN) {
		rt_smp_times[0].linux_tick = 0;
	} else if (mode == CLOCK_EVT_MODE_PERIODIC) {
		rt_smp_times[0].linux_tick = rtai_llimd((1000000000 + HZ/2)/HZ, rtai_tunables.clock_freq, 1000000000);
	}
}

void *rt_linux_hrt_next_shot;
EXPORT_SYMBOL(rt_linux_hrt_next_shot);

int _rt_linux_hrt_next_shot(unsigned long delay, struct clock_event_device *hrt_dev)
{
	rt_smp_times[0].linux_time = rt_smp_times[0].tick_time + rtai_llimd(delay, TIMER_FREQ, 1000000000);
	return 0;
}

int rt_request_timers(void *rtai_time_handler)
{
	int cpuid;

	if (!rt_linux_hrt_next_shot) {
		rt_linux_hrt_next_shot = _rt_linux_hrt_next_shot;
	}
	for (cpuid = 0; cpuid < num_active_cpus(); cpuid++) {
		struct rt_times *rtimes;
		int ret;
		ret = ipipe_timer_start(rtai_time_handler, rt_linux_hrt_set_mode, rt_linux_hrt_next_shot, cpuid);
		if (ret < 0 || ret == CLOCK_EVT_MODE_SHUTDOWN) {
			printk("THE TIMERS REQUEST FAILED RETURNING %d FOR CPUID %d, FREEING ALL TIMERS.\n", ret, cpuid);
			do {
				ipipe_timer_stop(cpuid);
			} while (--cpuid >= 0);
			return -1;
		}
		rtimes = &rt_smp_times[cpuid];
		if (ret == CLOCK_EVT_MODE_ONESHOT || ret == CLOCK_EVT_MODE_UNUSED) {
			rtimes->linux_tick = 0;
		} else {
			rt_smp_times[0].linux_tick = rtai_llimd((1000000000 + HZ/2)/HZ, rtai_tunables.clock_freq, 1000000000);
		}			
		rtimes->tick_time  = rtai_rdtsc();
                rtimes->intr_time  = rtimes->tick_time + rtimes->linux_tick;
                rtimes->linux_time = rtimes->tick_time + rtimes->linux_tick;
		rtimes->periodic_tick = rtimes->linux_tick;
	}
	return 0;
}
EXPORT_SYMBOL(rt_request_timers);

void rt_free_timers(void)
{
	int cpuid;
	for (cpuid = 0; cpuid < num_active_cpus(); cpuid++) {
		ipipe_timer_stop(cpuid);
	}
	rt_linux_hrt_next_shot = NULL;
}
EXPORT_SYMBOL(rt_free_timers);

#ifdef CONFIG_SMP

static unsigned long rtai_old_irq_affinity[IPIPE_NR_XIRQS];
static unsigned long rtai_orig_irq_affinity[IPIPE_NR_XIRQS];

static DEFINE_SPINLOCK(rtai_iset_lock);  // SPIN_LOCK_UNLOCKED

unsigned long rt_assign_irq_to_cpu (int irq, unsigned long cpumask)
{
	if (irq >= IPIPE_NR_XIRQS || &rtai_irq_desc(irq) == NULL || rtai_irq_desc_chip(irq) == NULL || rtai_irq_desc_chip(irq)->irq_set_affinity == NULL) {
		return 0;
	} else {
		unsigned long oldmask, flags;

		rtai_save_flags_and_cli(flags);
		spin_lock(&rtai_iset_lock);
		cpumask_copy((void *)&oldmask, rtai_irq_affinity(irq));
		hal_set_irq_affinity(irq, CPUMASK_T(cpumask)); 
		if (oldmask) {
			rtai_old_irq_affinity[irq] = oldmask;
		}
		spin_unlock(&rtai_iset_lock);
		rtai_restore_flags(flags);

		return oldmask;
	}
}

unsigned long rt_reset_irq_to_sym_mode (int irq)
{
	unsigned long oldmask, flags;

	if (irq >= IPIPE_NR_XIRQS) {
		return 0;
	} else {
		rtai_save_flags_and_cli(flags);
		spin_lock(&rtai_iset_lock);
		if (rtai_old_irq_affinity[irq] == 0) {
			spin_unlock(&rtai_iset_lock);
			rtai_restore_flags(flags);
			return -EINVAL;
		}
		cpumask_copy((void *)&oldmask, rtai_irq_affinity(irq));
		if (rtai_old_irq_affinity[irq]) {
	        	hal_set_irq_affinity(irq, CPUMASK_T(rtai_old_irq_affinity[irq]));
	        	rtai_old_irq_affinity[irq] = 0;
        	}
		spin_unlock(&rtai_iset_lock);
		rtai_restore_flags(flags);

		return oldmask;
	}
}

#else  /* !CONFIG_SMP */

unsigned long rt_assign_irq_to_cpu (int irq, unsigned long cpus_mask)
{
	return 0;
}

unsigned long rt_reset_irq_to_sym_mode (int irq)
{
	return 0;
}

#endif /* CONFIG_SMP */

RT_TRAP_HANDLER rt_set_trap_handler (RT_TRAP_HANDLER handler)
{
	return (RT_TRAP_HANDLER)xchg(&rtai_trap_handler, handler);
}

static void rtai_hirq_dispatcher(unsigned int irq)
{
	unsigned long cpuid;
	if (rtai_domain.irqs[irq].handler) {
		unsigned long sflags;
		sflags = rt_save_switch_to_real_time(cpuid = rtai_cpuid());
		rtai_domain.irqs[irq].handler(irq, rtai_domain.irqs[irq].cookie);
		rtai_cli();
		rt_restore_switch_to_linux(sflags, cpuid);
		if (test_bit(IPIPE_STALL_FLAG, ROOT_STATUS_ADR(cpuid))) {
			return;
		}
	}
	rtai_sti();
	hal_fast_flush_pipeline();
	return;
}

//#define HINT_DIAG_ECHO
//#define HINT_DIAG_TRAPS

#ifdef HINT_DIAG_ECHO
#define HINT_DIAG_MSG(x) x
#else
#define HINT_DIAG_MSG(x)
#endif

static int PrintFpuTrap = 0;
RTAI_MODULE_PARM(PrintFpuTrap, int);
static int PrintFpuInit = 0;
RTAI_MODULE_PARM(PrintFpuInit, int);

static int rtai_trap_fault (unsigned trap, struct pt_regs *regs)
{
#ifdef HINT_DIAG_TRAPS
	static unsigned long traps_in_hard_intr = 0;
        do {
                unsigned long flags;
                rtai_save_flags_and_cli(flags);
                if (!test_bit(RTAI_IFLAG, &flags)) {
                        if (!test_and_set_bit(trap, &traps_in_hard_intr)) {
                                HINT_DIAG_MSG(rt_printk("TRAP %d HAS INTERRUPT DISABLED (TRAPS PICTURE %lx).\n", trap, traps_in_hard_intr););
                        }
                }
        } while (0);
#endif
	static const int trap2sig[] = {
    		SIGFPE,         //  0 - Divide error
		SIGTRAP,        //  1 - Debug
		SIGSEGV,        //  2 - NMI (but we ignore these)
		SIGTRAP,        //  3 - Software breakpoint
		SIGSEGV,        //  4 - Overflow
		SIGSEGV,        //  5 - Bounds
		SIGILL,         //  6 - Invalid opcode
		SIGSEGV,        //  7 - Device not available
		SIGSEGV,        //  8 - Double fault
		SIGFPE,         //  9 - Coprocessor segment overrun
		SIGSEGV,        // 10 - Invalid TSS
		SIGBUS,         // 11 - Segment not present
		SIGBUS,         // 12 - Stack segment
		SIGSEGV,        // 13 - General protection fault
		SIGSEGV,        // 14 - Page fault
		0,              // 15 - Spurious interrupt
		SIGFPE,         // 16 - Coprocessor error
		SIGBUS,         // 17 - Alignment check
		SIGSEGV,        // 18 - Reserved
		SIGFPE,         // 19 - XMM fault
		0,0,0,0,0,0,0,0,0,0,0,0
	};
	if (!in_hrt_mode(rtai_cpuid())) {
		goto propagate;
	}
	if (trap == 7)	{
		struct task_struct *linux_task = current;
		rtai_cli();
		if (lnxtsk_uses_fpu(linux_task)) {
			restore_fpu(linux_task);
			if (PrintFpuTrap) {
				rt_printk("\nWARNING: FPU TRAP FROM HARD PID = %d\n", linux_task->pid);
			}
		} else {
			init_hard_fpu(linux_task);
			if (PrintFpuInit) {
				rt_printk("\nWARNING: FPU INITIALIZATION FROM HARD PID = %d\n", linux_task->pid);
			}
		}
		rtai_sti();
		return 1;
	}
	if (rtai_trap_handler && rtai_trap_handler(trap, trap2sig[trap], regs, NULL)) {
		return 1;
	}
propagate:
	return 0;
}

static void rtai_lsrq_dispatcher (unsigned virq)
{
	unsigned long pending, srq;

	spin_lock(&rtai_lsrq_lock);
	while ((pending = rtai_sysreq_pending & ~rtai_sysreq_running)) {
		set_bit(srq = ffnz(pending), &rtai_sysreq_running);
		clear_bit(srq, &rtai_sysreq_pending);
		spin_unlock(&rtai_lsrq_lock);
		if (test_bit(srq, &rtai_sysreq_map)) {
			rtai_sysreq_table[srq].k_handler();
		}
		clear_bit(srq, &rtai_sysreq_running);
		spin_lock(&rtai_lsrq_lock);
	}
	spin_unlock(&rtai_lsrq_lock);
}

long long rtai_usrq_dispatcher (unsigned long srq, unsigned long label)
{
	TRACE_RTAI_SRQ_ENTRY(srq);
	if (srq > 0 && srq < RTAI_NR_SRQS && test_bit(srq, &rtai_sysreq_map) && rtai_sysreq_table[srq].u_handler) {
		return rtai_sysreq_table[srq].u_handler(label);
	} else {
		for (srq = 1; srq < RTAI_NR_SRQS; srq++) {
			if (test_bit(srq, &rtai_sysreq_map) && rtai_sysreq_table[srq].label == label) {
				return (long long)srq;
			}
		}
	}
	TRACE_RTAI_SRQ_EXIT();
	return 0LL;
}
EXPORT_SYMBOL(rtai_usrq_dispatcher);

static int hal_intercept_syscall(struct pt_regs *regs)
{
	if (likely(regs->LINUX_SYSCALL_NR >= RTAI_SYSCALL_NR)) {
		unsigned long srq = regs->LINUX_SYSCALL_REG1;
		long long retval;
		retval = rtai_usrq_dispatcher(srq, regs->LINUX_SYSCALL_REG2);
#ifdef CONFIG_RTAI_USE_STACK_ARGS
		*((long long *)regs->LINUX_SYSCALL_REG3) = retval;
#else
		rt_put_user(retval, (long long *)regs->LINUX_SYSCALL_REG3);
#endif
		hal_test_and_fast_flush_pipeline();
	}
	return 0;
}

void rtai_uvec_handler(void);

#include <linux/clockchips.h>
#include <linux/ipipe_tickdev.h>

extern int (*rtai_syscall_hook)(struct pt_regs *);

void rtai_set_linux_task_priority (struct task_struct *task, int policy, int prio)
{
	struct sched_param param = { .sched_priority = prio };
	sched_setscheduler(task, policy, &param);
	if (task->rt_priority != prio || task->policy != policy) {
		printk("RTAI[hal]: sched_setscheduler(policy = %d, prio = %d) failed, (%s -- pid = %d)\n", policy, prio, task->comm, task->pid);
	}
}

struct proc_dir_entry *rtai_proc_root = NULL;

long long rtai_tsc_ofst[RTAI_NR_CPUS];
EXPORT_SYMBOL(rtai_tsc_ofst);
#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC)
void tsc_sync(void);
#endif

static int PROC_READ_FUN(rtai_read_proc)
{
	int i, none;
	PROC_PRINT_VARS;

	PROC_PRINT("\n** RTAI/x86:\n\n");
	PROC_PRINT("    CPU   Frequency: %lu (Hz)\n", rtai_tunables.clock_freq);
	PROC_PRINT("    TIMER Frequency: %lu (Hz)\n", TIMER_FREQ);
	PROC_PRINT("    TIMER Latency: %ld (ns)\n", (long)rtai_imuldiv(rtai_tunables.sched_latency, 1000000000, rtai_tunables.clock_freq));
	PROC_PRINT("    TIMER Setup: %ld (ns)\n", (long)rtai_imuldiv(rtai_tunables.setup_time_TIMER_CPUNIT, 1000000000, rtai_tunables.clock_freq));
    
	none = 1;
	PROC_PRINT("\n** Real-time IRQs used by RTAI: ");
    	for (i = 0; i < RTAI_NR_IRQS; i++) {
		if (rtai_domain.irqs[i].handler) {
			if (none) {
				PROC_PRINT("\n");
				none = 0;
			}
			PROC_PRINT("\n    #%d at %p", i, rtai_domain.irqs[i].handler);
		}
        }
	if (none) {
		PROC_PRINT("none");
	}
	PROC_PRINT("\n\n");

	PROC_PRINT("** RTAI extension traps: \n\n");

	none = 1;
	PROC_PRINT("** RTAI SYSREQs in use: ");
    	for (i = 0; i < RTAI_NR_SRQS; i++) {
		if (rtai_sysreq_table[i].k_handler || rtai_sysreq_table[i].u_handler) {
			PROC_PRINT("#%d ", i);
			none = 0;
		}
        }
	if (none) {
		PROC_PRINT("none");
	}
    	PROC_PRINT("\n\n");

#ifdef CONFIG_SMP
#ifdef CONFIG_RTAI_DIAG_TSC_SYNC
	PROC_PRINT("** RTAI TSC OFFSETs (nanosecs, ref. CPU %d): ", CONFIG_RTAI_MASTER_TSC_CPU);
    	for (i = 0; i < num_online_cpus(); i++) {
		long long tsc_ofst;
		tsc_ofst = rtai_tsc_ofst[i] >= 0 ?
			    rtai_llimd(rtai_tsc_ofst[i], 1000000000, rtai_tunables.clock_freq) :
			   -rtai_llimd(-rtai_tsc_ofst[i], 1000000000, rtai_tunables.clock_freq);
		PROC_PRINT("CPU#%d: %lld; ", i, tsc_ofst);
        }
    	PROC_PRINT("\n\n");
#endif
	PROC_PRINT("** MASK OF CPUs ISOLATED FOR RTAI: 0x%lx.", IsolCpusMask);
    	PROC_PRINT("\n\n");
#endif

	PROC_PRINT_DONE;
}

PROC_READ_OPEN_OPS(rtai_hal_proc_fops, rtai_read_proc);

static int rtai_proc_register (void)
{
	struct proc_dir_entry *ent;

	rtai_proc_root = CREATE_PROC_ENTRY("rtai", S_IFDIR, NULL, &rtai_hal_proc_fops);
	if (!rtai_proc_root) {
		printk(KERN_ERR "Unable to initialize /proc/rtai.\n");
		return -1;
        }
	ent = CREATE_PROC_ENTRY("hal", S_IFREG|S_IRUGO|S_IWUSR, rtai_proc_root, &rtai_hal_proc_fops);
	if (!ent) {
		printk(KERN_ERR "Unable to initialize /proc/rtai/hal.\n");
		return -1;
        }
	SET_PROC_READ_ENTRY(ent, rtai_read_proc);

	return 0;
}

static void rtai_proc_unregister (void)
{
	remove_proc_entry("hal", rtai_proc_root);
	remove_proc_entry("rtai", 0);
}

#define CPU_ISOLATED_MAP (cpu_isolated_map)
	extern cpumask_var_t cpu_isolated_map;

extern struct ipipe_domain ipipe_root;
extern void (*dispatch_irq_head)(unsigned int);
extern int (*rtai_trap_hook)(unsigned, struct pt_regs *);

int __rtai_hal_init (void)
{
	int i, ret = 0;
	struct hal_sysinfo_struct sysinfo;
	unsigned long CpuIsolatedMap;

	if (num_online_cpus() > RTAI_NR_CPUS) {
		printk("RTAI[hal]: RTAI CONFIGURED WITH LESS THAN NUM ONLINE CPUS.\n");
		ret = 1;
	}

	if (!boot_cpu_has(X86_FEATURE_APIC)) {
		printk("RTAI[hal]: ERROR, LOCAL APIC CONFIGURED BUT NOT AVAILABLE/ENABLED.\n");
		ret = 1;
	}

	if (!(rtai_sysreq_virq = ipipe_alloc_virq())) {
		printk(KERN_ERR "RTAI[hal]: NO VIRTUAL INTERRUPT AVAILABLE.\n");
		ret = 1;
	}

	if (ret) {
		return -1;
	}

	for (i = 0; i < RTAI_NR_IRQS; i++) {
		rtai_domain.irqs[i].ackfn = (void *)hal_root_domain->irqs[i].ackfn;
	}

	ipipe_request_irq(hal_root_domain, rtai_sysreq_virq, (void *)rtai_lsrq_dispatcher, NULL, NULL);
	dispatch_irq_head = rtai_hirq_dispatcher;

	ipipe_select_timers(cpu_active_mask);
	rtai_syscall_hook = hal_intercept_syscall;

	hal_get_sysinfo(&sysinfo);
	rtai_tunables.clock_freq      = sysinfo.sys_cpu_freq;
	rtai_tunables.timer_freq      = sysinfo.sys_hrtimer_freq;
	rtai_tunables.timer_irq       = sysinfo.sys_hrtimer_irq;
	rtai_tunables.linux_timer_irq = __ipipe_hrtimer_irq;

	rtai_proc_register();
	ipipe_register_head(&rtai_domain, "RTAI");
	rtai_trap_hook = rtai_trap_fault;

#ifdef CONFIG_SMP
	CpuIsolatedMap = 0;
	for (i = 0; i < RTAI_NR_CPUS; i++) {
		if (cpumask_test_cpu(i, CPU_ISOLATED_MAP)) {
			set_bit(i, &CpuIsolatedMap);
		}
	}
	if (IsolCpusMask && (IsolCpusMask != CpuIsolatedMap)) {
		printk("\nWARNING: IsolCpusMask (%lx) does not match cpu_isolated_map (%lx) set at boot time.\n", IsolCpusMask, CpuIsolatedMap);
	}
	if (!IsolCpusMask) {
		IsolCpusMask = CpuIsolatedMap;
	}
	if (IsolCpusMask) {
		for (i = 0; i < IPIPE_NR_XIRQS; i++) {
			rtai_orig_irq_affinity[i] = rt_assign_irq_to_cpu(i, ~IsolCpusMask);
		}
	}
#else
	IsolCpusMask = 0;
#endif

	printk(KERN_INFO "RTAI[hal]: mounted. ISOL_CPUS_MASK: %lx.\n", IsolCpusMask);

#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC)
	tsc_sync();
#endif

	printk("SYSINFO - # CPUs: %d, TIMER NAME: '%s', TIMER IRQ: %d, TIMER FREQ: %lu, CLOCK NAME: '%s', CLOCK FREQ: %lu, CPU FREQ: %llu, LINUX TIMER IRQ: %d.\n", sysinfo.sys_nr_cpus, ipipe_timer_name(), rtai_tunables.timer_irq, rtai_tunables.timer_freq, ipipe_clock_name(), rtai_tunables.clock_freq, sysinfo.sys_cpu_freq, __ipipe_hrtimer_irq); 

#ifdef CONFIG_RTAI_USE_STACK_ARGS
	printk("\nREMARK: RTAI WILL ACCESS USER SPACE ON STACKS ARGS ITS WAY.\n\n");
#else
	printk("\nREMARK: RTAI WILL NOT ACCESS USER SPACE ON STACKS ARGS ITS WAY.\n\n");
#endif
	return 0;
}

void __rtai_hal_exit (void)
{
	int i;
	rtai_proc_unregister();
	ipipe_unregister_head(&rtai_domain);
	dispatch_irq_head = NULL;
	rtai_trap_hook = NULL;
	ipipe_free_irq(hal_root_domain, rtai_sysreq_virq);
	ipipe_free_virq(rtai_sysreq_virq);

	ipipe_timers_release();
	rtai_syscall_hook = NULL;

	if (IsolCpusMask) {
		for (i = 0; i < IPIPE_NR_XIRQS; i++) {
			rt_reset_irq_to_sym_mode(i);
		}
	}

	printk(KERN_INFO "RTAI[hal]: unmounted.\n");
}

module_init(__rtai_hal_init);
module_exit(__rtai_hal_exit);

#define VSNPRINTF_BUF 256
asmlinkage int rt_printk(const char *fmt, ...)
{
	char buf[VSNPRINTF_BUF];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, VSNPRINTF_BUF, fmt, args);
	va_end(args);
	return printk("%s", buf);
}

asmlinkage int rt_sync_printk(const char *fmt, ...)
{
	char buf[VSNPRINTF_BUF];
	va_list args;

        va_start(args, fmt);
        vsnprintf(buf, VSNPRINTF_BUF, fmt, args);
        va_end(args);
	ipipe_prepare_panic();
	return printk("%s", buf);
}

extern struct calibration_data rtai_tunables;
#define CAL_LOOPS 200
int rtai_calibrate_hard_timer(void)
{
        unsigned long flags;
        RTIME t;
	int i, delay, dt;

	delay = rtai_tunables.clock_freq/50000;
        flags = rtai_critical_enter(NULL);
	rt_set_timer_delay(delay);
        t = rtai_rdtsc();
        for (i = 0; i < CAL_LOOPS; i++) {
		rt_set_timer_delay(delay);
        }
	dt = (int)(rtai_rdtsc() - t);
	rtai_critical_exit(flags);
	return rtai_imuldiv((dt + CAL_LOOPS/2)/CAL_LOOPS, 1000000000, rtai_tunables.clock_freq);
}

EXPORT_SYMBOL(rtai_calibrate_hard_timer);
EXPORT_SYMBOL(rtai_realtime_irq);
EXPORT_SYMBOL(rt_request_irq);
EXPORT_SYMBOL(rt_release_irq);
EXPORT_SYMBOL(rt_set_irq_cookie);
EXPORT_SYMBOL(rt_set_irq_retmode);
EXPORT_SYMBOL(rt_startup_irq);
EXPORT_SYMBOL(rt_shutdown_irq);
EXPORT_SYMBOL(rt_enable_irq);
EXPORT_SYMBOL(rt_disable_irq);
EXPORT_SYMBOL(rt_mask_and_ack_irq);
EXPORT_SYMBOL(rt_mask_irq);
EXPORT_SYMBOL(rt_unmask_irq);
EXPORT_SYMBOL(rt_ack_irq);
EXPORT_SYMBOL(rt_end_irq);
EXPORT_SYMBOL(rt_eoi_irq);
EXPORT_SYMBOL(rt_request_linux_irq);
EXPORT_SYMBOL(rt_free_linux_irq);
EXPORT_SYMBOL(rt_pend_linux_irq);
EXPORT_SYMBOL(usr_rt_pend_linux_irq);
EXPORT_SYMBOL(rt_request_srq);
EXPORT_SYMBOL(rt_free_srq);
EXPORT_SYMBOL(rt_pend_linux_srq);
EXPORT_SYMBOL(rt_assign_irq_to_cpu);
EXPORT_SYMBOL(rt_reset_irq_to_sym_mode);
EXPORT_SYMBOL(rt_set_trap_handler);
EXPORT_SYMBOL(rt_set_irq_ack);

EXPORT_SYMBOL(rtai_critical_enter);
EXPORT_SYMBOL(rtai_critical_exit);
EXPORT_SYMBOL(rtai_set_linux_task_priority);

EXPORT_SYMBOL(rtai_linux_context);
EXPORT_SYMBOL(rtai_domain);
EXPORT_SYMBOL(rtai_proc_root);
EXPORT_SYMBOL(rtai_tunables);
EXPORT_SYMBOL(rtai_cpu_lock);
EXPORT_SYMBOL(rtai_cpu_realtime);
EXPORT_SYMBOL(rt_smp_times);

EXPORT_SYMBOL(rt_printk);
EXPORT_SYMBOL(rt_sync_printk);

EXPORT_SYMBOL(IsolCpusMask);

#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC)

/*
 *	Hacked from arch/ia64/kernel/smpboot.c.
 *      Possible no_cpu_relax option from arch/x86_64/kernel/smpboot.c.
 */

//#define PRINT_DIAG_OUT_OF_SYNC_TSC

static volatile long long tsc_offset;

#define MASTER  (0)
#define SLAVE   (SMP_CACHE_BYTES/8)

#define NUM_ROUNDS      64      /* magic value */
#define NUM_ITERS       5       /* likewise */

static DEFINE_SPINLOCK(tsc_sync_lock);
#ifdef __i386__
static DEFINE_SPINLOCK(tsclock);
#define DECLARE_TSC_FLAGS unsigned long lflags
#define TSC_LOCK \
	do { spin_lock_irqsave(&tsclock, lflags); } while (0)
#define TSC_UNLOCK \
	do { spin_unlock_irqrestore(&tsclock, lflags); } while (0)
#else
#define DECLARE_TSC_FLAGS
#define TSC_LOCK
#define TSC_UNLOCK
#endif

static volatile unsigned long long go[SLAVE + 1];

#if 0
#define CPU_RELAX cpu_relax
#else
#define CPU_RELAX barrier
#endif

static void sync_master(void *arg)
{
	unsigned long flags, i;
	DECLARE_TSC_FLAGS;

	go[MASTER] = 0;

	local_irq_save(flags);
	for (i = 0; i < NUM_ROUNDS*NUM_ITERS; ++i) {
		while (!go[MASTER]) {
			CPU_RELAX();
		}
		go[MASTER] = 0;
		TSC_LOCK;
		go[SLAVE] = rtai_rdtsc();
		TSC_UNLOCK;
	}
	local_irq_restore(flags);
}

static inline long long get_delta(long long *rt, long long *master_time_stamp)
{
	unsigned long long best_t0 = 0, best_t1 = ~0ULL, best_tm = 0;
	unsigned long long tcenter = 0, t0, t1, tm;
	int i;
	DECLARE_TSC_FLAGS;

	for (i = 0; i < NUM_ITERS; ++i) {
		t0 = rtai_rdtsc() + tsc_offset;
		go[MASTER] = 1;
		TSC_LOCK;
		while (!(tm = go[SLAVE])) {
			TSC_UNLOCK;
			CPU_RELAX();
			TSC_LOCK;
		}
		TSC_UNLOCK;
		go[SLAVE] = 0;
		t1 = rtai_rdtsc() + tsc_offset;

		if ((t1 - t0) < (best_t1 - best_t0)) {
			best_t0 = t0;
			best_t1 = t1;
			best_tm = tm;
		}
	}

	*rt = best_t1 - best_t0;
	*master_time_stamp = best_tm - best_t0;

	/* average best_t0 and best_t1 without overflow: */
	tcenter = best_t0/2 + best_t1/2;
	if (best_t0 % 2 + best_t1 % 2 == 2) {
		++tcenter;
	}
	return (tcenter - best_tm);
}

void sync_tsc(unsigned int master)
{
	long long delta, adj, adjust_latency = 0;
	long long rt, master_time_stamp, bound;
	unsigned long flags;
	int i, done = 0;

#ifdef PRINT_DIAG_OUT_OF_SYNC_TSC
	struct {
		long long rt;		/* roundtrip time */
		long long master;	/* master's timestamp */
		long long diff;		/* difference between midpoint and master's timestamp */
		long long lat;		/* estimate of tsc adjustment latency */
	} t[NUM_ROUNDS];
#endif

	go[MASTER] = 1;

	if (smp_call_function_single(master, sync_master, NULL, 0) < 0) {
		printk(KERN_ERR "sync_tsc: failed to get attention of CPU %u!\n", master);
		return;
	}

	while (go[MASTER]) {
		CPU_RELAX();	/* wait for master to be ready */
	}

	spin_lock_irqsave(&tsc_sync_lock, flags);
	for (i = 0; i < NUM_ROUNDS; ++i) {
		delta = get_delta(&rt, &master_time_stamp);
		if (delta == 0) {
			done = 1;	/* let's lock on to this... */
			bound = rt;
		}

		if (!done) {
			if (i > 0) {
				adjust_latency += -delta;
				adj = -delta + adjust_latency/4;
			} else {
				adj = -delta;
			}

			tsc_offset += adj;
//			just for x86_64, true x86_32 should not allow writes into TSC
//			wrmsrl(MSR_IA32_TSC, rtai_rdtsc() + adj);  just for x86_64, x86_64
		}
#ifdef PRINT_DIAG_OUT_OF_SYNC_TSC
		t[i].rt = rt;
		t[i].master = master_time_stamp;
		t[i].diff = delta;
		t[i].lat = adjust_latency/4;
#endif
	}
	spin_unlock_irqrestore(&tsc_sync_lock, flags);

#ifdef PRINT_DIAG_OUT_OF_SYNC_TSC
	for (i = 0; i < NUM_ROUNDS; ++i)
		printk("rt=%5lld master=%5lld diff=%5lld adjlat=%5lld\n",
		       t[i].rt, t[i].master, t[i].diff, t[i].lat);
#endif

	printk(KERN_INFO "CPU %d: synchronized TSC with CPU %u (last diff %lld cycles, "
	       "maxerr %lld cycles), tsc_offset %lld (counts).\n", rtai_cpuid(), master, delta, rt, tsc_offset);
}
EXPORT_SYMBOL(sync_tsc);

static volatile int end;

static void kthread_fun(void *null)
{
        int i;
	 for (i = 0; i < num_online_cpus(); i++) {
		if (i != CONFIG_RTAI_MASTER_TSC_CPU) {
			tsc_offset = 0;
			set_cpus_allowed_ptr(current, cpumask_of(i));
			sync_tsc(CONFIG_RTAI_MASTER_TSC_CPU);
			rtai_tsc_ofst[i] = tsc_offset;
		}
	}
        end = 1;
}

#include <linux/kthread.h>

void tsc_sync(void)
{
        if (num_online_cpus() > 1) {
                kthread_run((void *)kthread_fun, NULL, "RTAI_TSC_SYNC");
                while(!end) {
                        msleep(100);
                }
        }
}

#endif /* defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC) */
