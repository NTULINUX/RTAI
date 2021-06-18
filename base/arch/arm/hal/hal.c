/*
 * ARM RTAI over Adeos -- based on ARTI for x86 and RTHAL for ARM.
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
 * RTAI/ARM over Adeos : 
 *   Copyright (C) 2007 Adeneo
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
#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/stddef.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <asm/mach/irq.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#	include <asm/proc/ptrace.h>
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
#	include <asm/ptrace.h>
#	include <asm/uaccess.h>
#	include <asm/unistd.h>
#	include <stdarg.h>
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */
#define __RTAI_HAL__
#include <asm/rtai_hal.h>
#include <asm/rtai_lxrt.h>
#include <asm/rtai_usi.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <rtai_proc_fs.h>
#endif /* CONFIG_PROC_FS */
#include <rtai_version.h>

#include <asm/rtai_usi.h>
#include <asm/unistd.h>

MODULE_LICENSE("GPL");

static unsigned long rtai_cpufreq_arg = RTAI_CALIBRATED_CPU_FREQ;
RTAI_MODULE_PARM(rtai_cpufreq_arg, ulong);

static unsigned long IsolCpusMask = 0;
RTAI_MODULE_PARM(IsolCpusMask, ulong);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,31) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) || LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#define rtai_irq_desc(irq) (irq_desc[irq].handler)
#else
#define rtai_irq_desc(irq) (irq_desc[irq].chip)
#endif

#define BEGIN_PIC()
#define END_PIC()
#undef hal_lock_irq
#undef hal_unlock_irq
#define hal_lock_irq(x, y, z)
#define hal_unlock_irq(x, y)

#else

extern struct hw_interrupt_type hal_std_irq_dtype[];
#define rtai_irq_desc(irq) (&hal_std_irq_dtype[irq])

#define BEGIN_PIC() \
do { \
        unsigned long flags, pflags, cpuid; \
	rtai_save_flags_and_cli(flags); \
	cpuid = rtai_cpuid(); \
	pflags = xchg(ipipe_root_status[cpuid], 1 << IPIPE_STALL_FLAG); \
	rtai_save_and_lock_preempt_count()

#define END_PIC() \
	rtai_restore_preempt_count(); \
	*ipipe_root_status[cpuid] = pflags; \
	rtai_restore_flags(flags); \
} while (0)

#endif /* (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,31) && LINUX_VERSION_ ... */

/* global */

struct rtai_realtime_irq_s rtai_realtime_irq[RTAI_NR_IRQS];
struct hal_domain_struct rtai_domain;
struct rt_times		rt_times;
struct rt_times		rt_smp_times[RTAI_NR_CPUS] = { { 0 } };
struct rtai_switch_data rtai_linux_context[RTAI_NR_CPUS];
volatile unsigned long *ipipe_root_status[RTAI_NR_CPUS];
struct calibration_data rtai_tunables;
volatile unsigned long	rtai_cpu_realtime;
volatile unsigned long	rtai_cpu_lock;
long long		(*rtai_lxrt_invoke_entry)(unsigned long, void *); /* hook for lxrt calls */
struct { volatile int locked, rqsted; } rt_scheduling[RTAI_NR_CPUS];
#ifdef CONFIG_PROC_FS
struct proc_dir_entry	*rtai_proc_root = NULL;
#endif

#ifdef CONFIG_RTAI_SCHED_ISR_LOCK
static void (*rtai_isr_hook)(int cpuid);
#endif 

#define CHECK_KERCTX()

/* local */

static struct {
    unsigned long flags;
    int count;
}			rtai_linux_irq[NR_IRQS];
static struct {
    void (*k_handler)(void);
    long long (*u_handler)(unsigned long);
    unsigned long label;
}			rtai_sysreq_table[RTAI_NR_SRQS];
static unsigned		rtai_sysreq_virq;
static unsigned long	rtai_sysreq_map = 3; /* srqs #[0-1] are reserved */
static unsigned long	rtai_sysreq_pending;
static unsigned long	rtai_sysreq_running;
static spinlock_t	rtai_lsrq_lock = SPIN_LOCK_UNLOCKED;
static volatile int	rtai_sync_level;
static atomic_t		rtai_sync_count = ATOMIC_INIT(1);
static RT_TRAP_HANDLER	rtai_trap_handler;
volatile unsigned long	hal_pended;

unsigned long
rtai_critical_enter(void (*synch)(void))
{
	unsigned long flags = hal_critical_enter(synch);

    if (atomic_dec_and_test(&rtai_sync_count))
	rtai_sync_level = 0;
    else if (synch != NULL)
	printk(KERN_INFO "RTAI[hal]: warning: nested sync will fail.\n");

    return flags;
}

void
rtai_critical_exit(unsigned long flags)
{
    atomic_inc(&rtai_sync_count);
	hal_critical_exit(flags);
}

int
rt_request_irq(unsigned irq, int (*handler)(unsigned irq, void *cookie), void *cookie, int retmode)
{
    unsigned long flags;

	if (handler == NULL || irq >= RTAI_NR_IRQS)
	return -EINVAL;

    if (rtai_realtime_irq[irq].handler != NULL)
	return -EBUSY;

    flags = rtai_critical_enter(NULL);
	rtai_realtime_irq[irq].handler = (void *)handler;
	rtai_realtime_irq[irq].irq_ack = hal_root_domain->irqs[irq].acknowledge;
    rtai_realtime_irq[irq].cookie = cookie;
    rtai_critical_exit(flags);
    return 0;
}

int
rt_release_irq(unsigned irq)
{
    unsigned long flags;
	if (irq >= RTAI_NR_IRQS || !rtai_realtime_irq[irq].handler) {
	return -EINVAL;
	}
    flags = rtai_critical_enter(NULL);
    rtai_realtime_irq[irq].handler = NULL;
	rtai_realtime_irq[irq].irq_ack = hal_root_domain->irqs[irq].acknowledge;
    rtai_critical_exit(flags);
	return 0;
}

int rt_set_irq_ack(unsigned irq, int (*irq_ack)(unsigned int))
{
	if (irq >= RTAI_NR_IRQS) {
		return -EINVAL;
	}
	rtai_realtime_irq[irq].irq_ack = irq_ack ? irq_ack : hal_root_domain->irqs[irq].acknowledge;
    return 0;
}

void
rt_set_irq_cookie(unsigned irq, void *cookie)
{
    if (irq < NR_IRQS)
	rtai_realtime_irq[irq].cookie = cookie;
}

/*
 * Upgrade this function for SMP systems
 */

void rt_request_apic_timers (void (*handler)(void), struct apic_timer_setup_data *tmdata) { return; }
void rt_free_apic_timers(void) { rt_free_timer(); }
int rt_assign_irq_to_cpu (int irq, unsigned long cpus_mask) { return 0; }
int rt_reset_irq_to_sym_mode (int irq) { return 0; }

void rt_request_rtc(long rtc_freq, void *handler)
{
	rt_printk("*** RTC NOT IMPLEMENTED YET ON THIS ARCH ***\n");
}

void rt_release_rtc(void)
{
	rt_printk("*** RTC NOT IMPLEMENTED YET ON THIS ARCH ***\n");
}

/*
 * The function below allow you to manipulate the PIC at hand, but you must
 * know what you are doing. Such a duty does not pertain to this manual and
 * you should refer to your PIC datasheet.
 *
 * Note that Linux has the same functions, but they must be used only for its
 * interrupts. Only the ones below can be safely used in real time handlers.
 *
 * It must also be remarked that when you install a real time interrupt handler,
 * RTAI already calls either rt_mask_and_ack_irq(), for level triggered
 * interrupts, or rt_ack_irq(), for edge triggered interrupts, before passing
 * control to your interrupt handler. Thus, generally you should just call
 * rt_unmask_irq() at due time, for level triggered interrupts, while nothing
 * should be done for edge triggered ones. Recall that in the latter case you
 * allow also any new interrupts on the same request as soon as you enable
 * interrupts at the CPU level.
 * 
 * Often some of the functions below do equivalent things. Once more there is no
 * way of doing it right except by knowing the hardware you are manipulating.
 * Furthermore you must also remember that when you install a hard real time
 * handler the related interrupt is usually disabled, unless you are overtaking
 * one already owned by Linux which has been enabled by it.   Recall that if
 * have done it right, and interrupts do not show up, it is likely you have just
 * to rt_enable_irq() your irq.
 *
 * Note: Adeos already does all the magic that allows to call the
 * interrupt controller routines safely.
 */

unsigned
rt_startup_irq(unsigned irq)
{
    	int retval;

    BEGIN_PIC();
	hal_unlock_irq(hal_root_domain, irq);
	rtai_irq_desc(irq)->unmask(irq);
	retval = rtai_irq_desc(irq)->startup(irq);
    END_PIC();
        return retval;
}

void
rt_shutdown_irq(unsigned irq)
{
    BEGIN_PIC();
	rtai_irq_desc(irq)->shutdown(irq);
	hal_clear_irq(hal_root_domain, irq);
    END_PIC();
}

void
rt_enable_irq(unsigned irq)
{
    BEGIN_PIC();
	hal_unlock_irq(hal_root_domain, irq);
	rtai_irq_desc(irq)->enable(irq);
    END_PIC();
}

void
rt_disable_irq(unsigned irq)
{
    BEGIN_PIC();
	rtai_irq_desc(irq)->disable(irq);
	hal_lock_irq(hal_root_domain, cpuid, irq);
    END_PIC();
}

void
rt_mask_and_ack_irq(unsigned irq)
{
    BEGIN_PIC();
	rtai_irq_desc(irq)->mask(irq);
    END_PIC();
}

static inline void _rt_end_irq (unsigned irq)
{
    BEGIN_PIC();
	if (
	    !(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS))) {
		hal_unlock_irq(hal_root_domain, irq);
	}
	rtai_irq_desc(irq)->end(irq);
    END_PIC();
}

void
rt_unmask_irq(unsigned irq)
{
    BEGIN_PIC();
	rtai_irq_desc(irq)->unmask(irq);
    END_PIC();
}

void
rt_ack_irq(unsigned irq)
{
    BEGIN_PIC();
	hal_unlock_irq(hal_root_domain, irq);
	rtai_irq_desc(irq)->enable(irq);
    END_PIC();
}

void rt_end_irq (unsigned irq)
{
	_rt_end_irq(irq);
}

/**
 * Install shared Linux interrupt handler.
 *
 * rt_request_linux_irq installs function @a handler as a standard Linux
 * interrupt service routine for IRQ level @a irq forcing Linux to share the IRQ
 * with other interrupt handlers, even if it does not want. The handler is
 * appended to any already existing Linux handler for the same irq and is run by
 * Linux irq as any of its handler. In this way a real time application can
 * monitor Linux interrupts handling at its will. The handler appears in
 * /proc/interrupts.
 *
 * @param handler pointer on the interrupt service routine to be installed.
 *
 * @param name is a name for /proc/interrupts.
 *
 * @param dev_id is to pass to the interrupt handler, in the same way as the
 * standard Linux irq request call.
 *
 * The interrupt service routine can be uninstalled with rt_free_linux_irq().
 *
 * @retval 0 on success.
 * @retval EINVAL if @a irq is not a valid IRQ number or handler is @c NULL.
 * @retval EBUSY if there is already a handler of interrupt @a irq.
 */

int
rt_request_linux_irq (unsigned irq, void *handler, char *name, void *dev_id)
{
    unsigned long flags;

	if (irq >= RTAI_NR_IRQS || !handler) {
	return -EINVAL;
	}

    rtai_save_flags_and_cli(flags);
	spin_lock(&irq_desc[irq].lock);
    if (rtai_linux_irq[irq].count++ == 0 && irq_desc[irq].action) {
	rtai_linux_irq[irq].flags = irq_desc[irq].action->flags;
	irq_desc[irq].action->flags |= SA_SHIRQ;
    }
	spin_unlock(&irq_desc[irq].lock);
    rtai_restore_flags(flags);

    request_irq(irq, handler, SA_SHIRQ, name, dev_id);

    return 0;
}

/**
 * Uninstall shared Linux interrupt handler.
 *
 * @param dev_id is to pass to the interrupt handler, in the same way as the
 * standard Linux irq request call.
 *
 * @param irq is the IRQ level of the interrupt handler to be freed.
 *
 * @retval 0 on success.
 * @retval EINVAL if @a irq is not a valid IRQ number.
 */
int
rt_free_linux_irq(unsigned irq, void *dev_id)
{
    unsigned long flags;

	if (irq >= RTAI_NR_IRQS || rtai_linux_irq[irq].count == 0) {
	return -EINVAL;
	}

    rtai_save_flags_and_cli(flags);
    free_irq(irq, dev_id);

	spin_lock(&irq_desc[irq].lock);
	if (--rtai_linux_irq[irq].count == 0 && irq_desc[irq].action) {
	irq_desc[irq].action->flags = rtai_linux_irq[irq].flags;
	}
	spin_unlock(&irq_desc[irq].lock);

    rtai_restore_flags(flags);

    return 0;
}

/**
 * Pend an IRQ to Linux.
 *
 * rt_pend_linux_irq appends a Linux interrupt irq for processing in Linux IRQ
 * mode, i.e. with hardware interrupts fully enabled.
 *
 * @note rt_pend_linux_irq does not perform any check on @a irq.
 */
void
rt_pend_linux_irq(unsigned irq)
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

/**
 * Install a system request handler
 *
 * rt_request_srq installs a two way RTAI system request (srq) by assigning
 * @a u_handler, a function to be used when a user calls srq from user space,
 * and @a k_handler, the function to be called in kernel space following its
 * activation by a call to rt_pend_linux_srq(). @a k_handler is in practice
 * used to request a service from the kernel. In fact Linux system requests
 * cannot be used safely from RTAI so you can setup a handler that receives real
 * time requests and safely executes them when Linux is running.
 *
 * @param u_handler can be used to effectively enter kernel space without the
 * overhead and clumsiness of standard Unix/Linux protocols.   This is very
 * flexible service that allows you to personalize your use of  RTAI.
 *
 * @return the number of the assigned system request on success.
 * @retval EINVAL if @a k_handler is @c NULL.
 * @retval EBUSY if no free srq slot is available.
 */
int
rt_request_srq(unsigned label, void (*k_handler)(void), long long (*u_handler)(unsigned long))
{
    unsigned long flags;
    int srq;

    if (k_handler == NULL)
	return -EINVAL;

    rtai_save_flags_and_cli(flags);

    if (rtai_sysreq_map != ~0) {
	srq = ffz(rtai_sysreq_map);
	set_bit(srq, &rtai_sysreq_map);
	rtai_sysreq_table[srq].k_handler = k_handler;
	rtai_sysreq_table[srq].u_handler = u_handler;
	rtai_sysreq_table[srq].label = label;
    } else
	srq = -EBUSY;

    rtai_restore_flags(flags);

    return srq;
}

/**
 * Uninstall a system request handler
 *
 * rt_free_srq uninstalls the specified system call @a srq, returned by
 * installing the related handler with a previous call to rt_request_srq().
 *
 * @retval EINVAL if @a srq is invalid.
 */
int
rt_free_srq(unsigned srq)
{
    return (srq < 2 || srq >= RTAI_NR_SRQS || !test_and_clear_bit(srq, &rtai_sysreq_map))
	? -EINVAL
	: 0;
}

/**
 * Append a Linux IRQ.
 *
 * rt_pend_linux_srq appends a system call request srq to be used as a service
 * request to the Linux kernel.
 *
 * @param srq is the value returned by rt_request_srq.
 *
 * @note rt_pend_linux_srq does not perform any check on irq.
 */
void
rt_pend_linux_srq(unsigned srq)
{
    if (srq > 0 && srq < RTAI_NR_SRQS) {
		unsigned long flags;
	set_bit(srq, &rtai_sysreq_pending);
		rtai_save_flags_and_cli(flags);
		hal_pend_uncond(rtai_sysreq_virq, rtai_cpuid());
		rtai_restore_flags(flags);
    }
}

#ifdef CONFIG_RTAI_SCHED_ISR_LOCK
static void (*rtai_isr_hook)(int cpuid);
#define RTAI_SCHED_ISR_LOCK() \
    do { \
		if (!rt_scheduling[cpuid = rtai_cpuid()].locked++) { \
            rt_scheduling[cpuid].rqsted = 0; \
	} \
    } while (0)
#define RTAI_SCHED_ISR_UNLOCK() \
    do { \
	if (rt_scheduling[cpuid].locked && !(--rt_scheduling[cpuid].locked)) { \
	    if (rt_scheduling[cpuid].rqsted > 0 && rtai_isr_hook) { \
             	rtai_isr_hook(cpuid); \
	    } \
	} \
    } while (0)
#else  /* !CONFIG_RTAI_SCHED_ISR_LOCK */
#define RTAI_SCHED_ISR_LOCK() \
	do { cpuid = rtai_cpuid(); } while (0)
#define RTAI_SCHED_ISR_UNLOCK() \
	do {                       } while (0)
#endif /* CONFIG_RTAI_SCHED_ISR_LOCK */

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,31) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) || LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9)
#define HAL_TICK_REGS hal_tick_regs[cpuid]
#else
#define HAL_TICK_REGS hal_tick_regs
#endif /* (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,31) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) || LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9) */

#ifdef LOCKED_LINUX_IN_IRQ_HANDLER
#define HAL_LOCK_LINUX()  do { sflags = rt_save_switch_to_real_time(cpuid); } while (0)
#define HAL_UNLOCK_LINUX()  do { rtai_cli(); rt_restore_switch_to_linux(sflags, cpuid); } while (0)
#else
#define HAL_LOCK_LINUX()  do { sflags = xchg(ipipe_root_status[cpuid], (1 << IPIPE_STALL_FLAG)); } while (0)
#define HAL_UNLOCK_LINUX()  do { rtai_cli(); *ipipe_root_status[cpuid] = sflags; } while (0)
#endif /* LOCKED_LINUX_IN_IRQ_HANDLER */

/* this can be a prototype for a handler pending something for Linux */
int rtai_timer_handler(struct pt_regs *regs)
{
        unsigned long cpuid=rtai_cpuid();
	unsigned long sflags;

        RTAI_SCHED_ISR_LOCK();
        HAL_LOCK_LINUX();
	rtai_realtime_irq[RTAI_TIMER_IRQ].irq_ack(RTAI_TIMER_IRQ);
	((void (*)(void))rtai_realtime_irq[RTAI_TIMER_IRQ].handler)();
        HAL_UNLOCK_LINUX();
        RTAI_SCHED_ISR_UNLOCK();

        if (test_and_clear_bit(cpuid, &hal_pended) && !test_bit(IPIPE_STALL_FLAG, ipipe_root_status[cpuid])) {
                rtai_sti();
                hal_fast_flush_pipeline(cpuid);
                return 1;
        }
        return 0;
}

/* rt_request_timer(): install a timer interrupt handler and set hardware-timer
 * to requested period. This is arch-specific (stopping/reprogramming/...
 * timer). Hence, the function is contained in mach-ARCH/ARCH-timer.c
 */

/* rt_free_timer(): uninstall a timer handler previously set by
 * rt_request_timer() and reset hardware-timer to Linux HZ-tick.
 * This is arch-specific (stopping/reprogramming/... timer).
 * Hence, the function is contained in mach-ARCH/ARCH-timer.c
 */

/*
 * rtai_hirq_dispatcher
 */

static void rtai_hirq_dispatcher(int irq, struct pt_regs *regs)
{
	unsigned long cpuid;
    if (rtai_realtime_irq[irq].handler) {
                unsigned long sflags;

	RTAI_SCHED_ISR_LOCK();
                HAL_LOCK_LINUX();
                rtai_realtime_irq[irq].irq_ack(irq); mb();
	rtai_realtime_irq[irq].handler(irq, rtai_realtime_irq[irq].cookie);
                HAL_UNLOCK_LINUX();
	RTAI_SCHED_ISR_UNLOCK();
                if (rtai_realtime_irq[irq].retmode || !test_and_clear_bit(cpuid, &hal_pended) || test_bit(IPIPE_STALL_FLAG, ipipe_root_status[cpuid])) {
                        return;
    }
        } else {
                unsigned long lflags;

                lflags = xchg(ipipe_root_status[cpuid = rtai_cpuid()], (1 << IPIPE_STALL_FLAG));
                rtai_realtime_irq[irq].irq_ack(irq); mb();
                hal_pend_uncond(irq, cpuid);
                *ipipe_root_status[cpuid] = lflags;
                if (test_bit(IPIPE_STALL_FLAG, &lflags)) {
                        return;
	}
    }
        rtai_sti();
        hal_fast_flush_pipeline(cpuid);
        return;
}

RT_TRAP_HANDLER
rt_set_trap_handler(RT_TRAP_HANDLER handler)
{
    return (RT_TRAP_HANDLER)xchg(&rtai_trap_handler, handler);
}

static int rtai_trap_fault (unsigned event, void *evdata)
{
	static const int trap2sig[] = {
		SIGSEGV,	// 0 - Data or instruction access exception
		SIGBUS,		// 1 - Alignment exception
		SIGFPE,		// 2 - Altivec unavailable
		SIGFPE,		// 3 - Program check exception
		SIGFPE,		// 4 - Machine check exception
		SIGFPE,		// 5 - Unknown exception
		SIGTRAP,	// 6 - Instruction breakpoint
		SIGFPE,		// 7 - Run mode exception
		SIGTRAP,	// 8 - Single-step exception
		SIGSEGV,	// 9 - Non-recoverable exception
		SIGILL,		// 10 - Software emulation
		SIGTRAP,	// 11 - Debug exception
		SIGSEGV,	// 12 - SPE exception
		SIGFPE,		// 13 - Altivec assist exception
		0,		// 14
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0
	}; 

	TRACE_RTAI_TRAP_ENTRY(evdata->event, 0);

	if (!in_hrt_mode(rtai_cpuid())) {
	goto propagate;
	}

    /* We don't treat SIGILL as "FPU usage" as there is no FPU support in RTAI for ARM.
     * *FIXME* The whole FPU kernel emulation issue has to be sorted out (is it
     * reentrentant, do we need to save the emulated registers, can it be used
     * in kernel space, etc.). */

	// if a trap handler is set call it
	if (rtai_trap_handler && rtai_trap_handler(event, trap2sig[event], (struct pt_regs *)evdata, NULL)) {
	goto endtrap;
	}

propagate:
	return 0;

endtrap:
    TRACE_RTAI_TRAP_EXIT();
	return 1;
}

/*
 * rtai_lsrq_dispatcher
 */

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


/*
 * rtai_usrq_dispatcher
 */

static inline long long rtai_usrq_dispatcher (unsigned long srq, unsigned long label)
{
	TRACE_RTAI_SRQ_ENTRY(srq);

	if (srq > 1 && srq < RTAI_NR_SRQS && test_bit(srq, &rtai_sysreq_map) && rtai_sysreq_table[srq].u_handler) {
		return rtai_sysreq_table[srq].u_handler(label);
	}
	else {
		for (srq = 2; srq < RTAI_NR_SRQS; srq++) {
			if (test_bit(srq, &rtai_sysreq_map) && rtai_sysreq_table[srq].label == label) {
				return (long long)srq;
			}
		}
	}

    TRACE_RTAI_SRQ_EXIT();

	return 0LL;
}

long long (*rtai_lxrt_dispatcher)(unsigned long, unsigned long, void *);

static int (*sched_intercept_syscall_prologue)(struct pt_regs *);

static int intercept_syscall_prologue(unsigned long event, struct pt_regs *regs){
        if (likely(regs->ARM_r0 >= RTAI_SYSCALL_NR)) {
                unsigned long srq  = regs->ARM_r1;
		unsigned long arg  = regs->ARM_r2;

                IF_IS_A_USI_SRQ_CALL_IT(srq, arg, (long long *)regs->ARM_r5, regs->msr, 1);
                *((long long *)regs->ARM_r3) = srq > RTAI_NR_SRQS ?  rtai_lxrt_dispatcher(srq, arg, regs) : rtai_usrq_dispatcher(srq, arg);
                if (!in_hrt_mode(srq = rtai_cpuid())) {
                        hal_test_and_fast_flush_pipeline(srq);
                        return 0;
    }
                return 1;
        }
        return likely(sched_intercept_syscall_prologue != NULL) ? sched_intercept_syscall_prologue(regs) : 0;
}

int rtai_syscall_dispatcher (struct pt_regs *regs)
{
	unsigned long srq = regs->ARM_r0;
	unsigned long arg = regs->ARM_r1;

	IF_IS_A_USI_SRQ_CALL_IT(srq, regs->ARM_r2, (long long *)regs->ARM_r3, regs->msr, 1);

	*((long long*)&regs->ARM_r0) = srq > RTAI_NR_SRQS ?  rtai_lxrt_dispatcher(srq, arg, regs) : rtai_usrq_dispatcher(srq, arg);
        if (!in_hrt_mode(srq = rtai_cpuid())) {
                hal_test_and_fast_flush_pipeline(srq);
                return 1;
        }
        return 0;
}

static void rtai_domain_entry(int iflag)
{
	if (iflag) {
		rt_printk(KERN_INFO "RTAI[hal]: <%s> mounted over %s %s.\n", PACKAGE_VERSION, HAL_TYPE, HAL_VERSION_STRING);
		rt_printk(KERN_INFO "RTAI[hal]: compiled with %s.\n", CONFIG_RTAI_COMPILER);
	}
	for (;;) hal_suspend_domain();
}

long rtai_catch_event (struct hal_domain_struct *from, unsigned long event, int (*handler)(unsigned long, void *))
{
        if (event == HAL_SYSCALL_PROLOGUE) {
                sched_intercept_syscall_prologue = (void *)handler;
                return 0;
        }
	return (long)hal_catch_event(from, event, (void *)handler);
}

static void *saved_hal_irq_handler;
extern void *hal_irq_handler;

#ifdef CONFIG_PROC_FS

static int
rtai_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    PROC_PRINT_VARS;
    int i, none;

    PROC_PRINT("\n** RTAI/ARM %s over Adeos %s\n\n", RTAI_RELEASE, HAL_VERSION_STRING);
    PROC_PRINT("    TSC frequency: %d Hz\n", RTAI_TSC_FREQ);
    PROC_PRINT("    Timer frequency: %d Hz\n", RTAI_TIMER_FREQ);
    PROC_PRINT("    Timer latency: %d ns, %d TSC ticks\n", RTAI_TIMER_LATENCY,
	rtai_imuldiv(RTAI_TIMER_LATENCY, RTAI_TSC_FREQ, 1000000000));
    PROC_PRINT("    Timer setup: %d ns\n", RTAI_TIMER_SETUP_TIME);
    PROC_PRINT("    Timer setup: %d TSC ticks, %d IRQ-timer ticks\n",
	rtai_imuldiv(RTAI_TIMER_SETUP_TIME, RTAI_TSC_FREQ, 1000000000),
	rtai_imuldiv(RTAI_TIMER_SETUP_TIME, RTAI_TIMER_FREQ, 1000000000));

    none = 1;

    PROC_PRINT("\n** Real-time IRQs used by RTAI: ");

    for (i = 0; i < NR_IRQS; i++) {
	if (rtai_realtime_irq[i].handler) {
	    if (none) {
		PROC_PRINT("\n");
		none = 0;
	    }
	    PROC_PRINT("\n    #%d at %p", i, rtai_realtime_irq[i].handler);
	}
    }

    if (none)
	PROC_PRINT("none");

    PROC_PRINT("\n\n");

    PROC_PRINT("** RTAI extension traps: \n\n");
    PROC_PRINT("    SYSREQ=0x%x\n", RTAI_SYS_VECTOR);
#if 0
    PROC_PRINT("       SHM=0x%x\n", RTAI_SHM_VECTOR);
#endif
    PROC_PRINT("\n");

    none = 1;
    PROC_PRINT("** RTAI SYSREQs in use: ");

    for (i = 0; i < RTAI_NR_SRQS; i++) {
	if (rtai_sysreq_table[i].k_handler || rtai_sysreq_table[i].u_handler) {
	    PROC_PRINT("#%d ", i);
	    none = 0;
	}
    }

    if (none)
	PROC_PRINT("none");

    PROC_PRINT("\n\n");

    PROC_PRINT_DONE;
}

static int
rtai_proc_register(void)
{
    struct proc_dir_entry *ent;

    rtai_proc_root = create_proc_entry("rtai", S_IFDIR, 0);

    if (!rtai_proc_root) {
	printk(KERN_ERR "RTAI[hal]: Unable to initialize /proc/rtai.\n");
	return -1;
    }

    rtai_proc_root->owner = THIS_MODULE;

    ent = create_proc_entry("rtai", S_IFREG|S_IRUGO|S_IWUSR, rtai_proc_root);

    if (!ent) {
	printk(KERN_ERR "RTAI[hal]: Unable to initialize /proc/rtai/rtai.\n");
	return -1;
    }

    ent->read_proc = rtai_read_proc;

    return 0;
}

static void
rtai_proc_unregister(void)
{
    remove_proc_entry("rtai", rtai_proc_root);
    remove_proc_entry("rtai", 0);
}

#endif /* CONFIG_PROC_FS */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
static inline void *hal_set_irq_handler(void *hirq_dispatcher)
{
	if (saved_hal_irq_handler != hirq_dispatcher) {
		saved_hal_irq_handler = hal_irq_handler;
		hal_irq_handler = hirq_dispatcher;
		return saved_hal_irq_handler;
    }
	hal_irq_handler = hirq_dispatcher;
	return rtai_hirq_dispatcher;
}
#endif

void (*rt_set_ihook (void (*hookfn)(int)))(int)
{
#ifdef CONFIG_RTAI_SCHED_ISR_LOCK
	return (void (*)(int))xchg(&rtai_isr_hook, hookfn); /* This is atomic */
#else  /* !CONFIG_RTAI_SCHED_ISR_LOCK */
	return NULL;
#endif /* CONFIG_RTAI_SCHED_ISR_LOCK */
}

void rtai_set_linux_task_priority (struct task_struct *task, int policy, int prio)
{
	hal_set_linux_task_priority(task, policy, prio);
	if (task->rt_priority != prio || task->policy != policy) {
		printk("RTAI[hal]: sched_setscheduler(policy = %d, prio = %d) failed, (%s -- pid = %d)\n", policy, prio, task->comm, task->pid);
	}
}

/*
 * rtai_install_archdep
 */

static void rtai_install_archdep (void)
{
	struct hal_sysinfo_struct sysinfo;

#if !defined(USE_LINUX_SYSCALL) && !defined(CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL)	
	/* empty till a direct RTAI syscall way is decided */
#endif

	hal_catch_event(hal_root_domain, HAL_SYSCALL_PROLOGUE, (void *)intercept_syscall_prologue);

	hal_get_sysinfo(&sysinfo);

	rtai_cpufreq_arg = (unsigned long)sysinfo.cpufreq;

	rtai_tunables.cpu_freq = rtai_cpufreq_arg;
}

/*
 * rtai_uninstall_archdep
 */

static void rtai_uninstall_archdep (void)
{
/* something to be added when a direct RTAI syscall way is decided */
	hal_catch_event(hal_root_domain, HAL_SYSCALL_PROLOGUE, NULL);
	rtai_archdep_exit();
}

#define RT_PRINTK_SRQ  1

static void *saved_hal_irq_handler;
extern void *hal_irq_handler;

int
__rtai_hal_init(void)
{
	int trapnr, halinv;
	struct hal_attr_struct attr;

	// check event handler registration, check for any already installed
	for (halinv = trapnr = 0; trapnr < HAL_NR_EVENTS; trapnr++) {
		if (hal_root_domain->hal_event_handler_fun(trapnr)) {
			halinv = 1;
			printk("EVENT %d INVALID\n", trapnr);
		}
	}
	if (halinv) {
		printk(KERN_ERR "RTAI[hal]: HAL IMMEDIATE EVENT DISPATCHING BROKEN\n");
		return -1;
	}

	// request a virtual interrupt for RTAI sysrqs
	if (!(rtai_sysreq_virq = hal_alloc_irq())) {
	printk(KERN_ERR "RTAI[hal]: no virtual interrupt available.\n");
		return -1;
    }

	// copy HAL proper pointers locally for a more effective use
	for (trapnr = 0; trapnr < RTAI_NR_IRQS; trapnr++) {
		rtai_realtime_irq[trapnr].irq_ack = hal_root_domain->irqs[trapnr].acknowledge;
	}
	for (trapnr = 0; trapnr < RTAI_NR_CPUS; trapnr++) {
		ipipe_root_status[trapnr] = &hal_root_domain->cpudata[trapnr].status;
    }

	// assign the RTAI sysrqs handler
	hal_virtualize_irq(hal_root_domain, rtai_sysreq_virq, &rtai_lsrq_dispatcher, NULL, IPIPE_HANDLE_MASK);
	saved_hal_irq_handler = hal_irq_handler;
	hal_irq_handler = rtai_hirq_dispatcher;

	// architecture dependent RTAI installation
	rtai_install_archdep();

#ifdef CONFIG_PROC_FS
    rtai_proc_register();
#endif /* CONFIG_PROC_FS */

	// register RTAI domain
	hal_init_attr(&attr);
    attr.name = "RTAI";
    attr.domid = RTAI_DOMAIN_ID;
	attr.entry    = (void *)rtai_domain_entry;
	attr.priority = get_domain_pointer(1)->priority + 100;
	hal_register_domain(&rtai_domain, &attr);

	// register trap handler for all FAULTS in the root domain
	for (trapnr = 0; trapnr < HAL_NR_FAULTS; trapnr++) {
		hal_catch_event(hal_root_domain, trapnr, (void *)rtai_trap_fault);
	}

	// log RTAI mounted
	printk(KERN_INFO "RTAI[hal]: mounted (%s, IMMEDIATE (INTERNAL IRQs %s).\n", HAL_TYPE, CONFIG_RTAI_DONT_DISPATCH_CORE_IRQS ? "VECTORED" : "DISPATCHED");

	// log PIPELINE layers
	printk("PIPELINE layers:\n");
	for (trapnr = 1; ; trapnr++) {
		struct hal_domain_struct *next_domain;
		next_domain = get_domain_pointer(trapnr);
		if ((unsigned long)next_domain < 10) break;
		printk("%p %x %s %d\n", next_domain, next_domain->domid, next_domain->name, next_domain->priority);
	}

#ifdef CONFIG_RTAI_DIAG_TSC_SYNC
	init_tsc_sync();
#endif

    return 0;
}

void
__rtai_hal_exit(void)
{
    int trapnr;
#ifdef CONFIG_PROC_FS
    rtai_proc_unregister();
#endif

	hal_irq_handler = saved_hal_irq_handler;
	hal_unregister_domain(&rtai_domain);
	for (trapnr = 0; trapnr < HAL_NR_FAULTS; trapnr++) {
		hal_catch_event(hal_root_domain, trapnr, NULL);
	}
	hal_virtualize_irq(hal_root_domain, rtai_sysreq_virq, NULL, NULL, 0);
	hal_free_irq(rtai_sysreq_virq);

	rtai_uninstall_archdep();

#ifdef CONFIG_RTAI_DIAG_TSC_SYNC
	cleanup_tsc_sync();
#endif

    printk(KERN_INFO "RTAI[hal]: unmounted.\n");
}

module_init(__rtai_hal_init);
module_exit(__rtai_hal_exit);

/*
 * rt_printk
 */

asmlinkage int rt_printk(const char *fmt, ...)
{
    va_list args;
	int r;

    va_start(args, fmt);
		r = vprintk(fmt, args);
    va_end(args);

	return r;
}

/*
 * rt_sync_printk
 */

asmlinkage int rt_sync_printk(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
		hal_set_printk_sync(&rtai_domain);
			r = vprintk(fmt, args);
		hal_set_printk_async(&rtai_domain);
	va_end(args);

	return r;
}

EXPORT_SYMBOL(rtai_realtime_irq);
EXPORT_SYMBOL(rt_request_irq);
EXPORT_SYMBOL(rt_release_irq);
EXPORT_SYMBOL(rt_set_irq_cookie);
EXPORT_SYMBOL(rt_startup_irq);
EXPORT_SYMBOL(rt_shutdown_irq);
EXPORT_SYMBOL(rt_enable_irq);
EXPORT_SYMBOL(rt_disable_irq);
EXPORT_SYMBOL(rt_mask_and_ack_irq);
EXPORT_SYMBOL(rt_unmask_irq);
EXPORT_SYMBOL(rt_ack_irq);
EXPORT_SYMBOL(rt_request_linux_irq);
EXPORT_SYMBOL(rt_free_linux_irq);
EXPORT_SYMBOL(rt_pend_linux_irq);
EXPORT_SYMBOL(usr_rt_pend_linux_irq);
EXPORT_SYMBOL(rt_set_irq_ack);

EXPORT_SYMBOL(rt_request_srq);
EXPORT_SYMBOL(rt_free_srq);
EXPORT_SYMBOL(rt_pend_linux_srq);
EXPORT_SYMBOL(rt_assign_irq_to_cpu);

EXPORT_SYMBOL(rt_reset_irq_to_sym_mode);
EXPORT_SYMBOL(rt_request_timer);
EXPORT_SYMBOL(rt_free_timer);
EXPORT_SYMBOL(rt_request_rtc);
EXPORT_SYMBOL(rt_release_rtc);

extern int rtai_calibrate_TC (void);
EXPORT_SYMBOL(rtai_calibrate_TC);

EXPORT_SYMBOL(rt_set_trap_handler);

EXPORT_SYMBOL(rt_set_ihook);

EXPORT_SYMBOL(rtai_critical_enter);
EXPORT_SYMBOL(rtai_critical_exit);
EXPORT_SYMBOL(rtai_set_linux_task_priority);
EXPORT_SYMBOL(rtai_linux_context);
EXPORT_SYMBOL(rtai_domain);
EXPORT_SYMBOL(rtai_proc_root);
EXPORT_SYMBOL(rtai_tunables);
EXPORT_SYMBOL(rtai_cpu_lock);
EXPORT_SYMBOL(rtai_cpu_realtime);
EXPORT_SYMBOL(rt_times);
EXPORT_SYMBOL(rt_smp_times);

EXPORT_SYMBOL(rt_printk);
EXPORT_SYMBOL(rt_sync_printk);

EXPORT_SYMBOL(rtai_catch_event);

EXPORT_SYMBOL(rtai_lxrt_dispatcher);
EXPORT_SYMBOL(rt_scheduling);
EXPORT_SYMBOL(hal_pended);
EXPORT_SYMBOL(ipipe_root_status);
