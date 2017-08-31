/**
 *   @ingroup hal
 *   @file
 *
 *   ARTI -- RTAI-compatible Adeos-based Real-Time Interface. Based on
 *   the original RTAI layer for x86.
 *
 *   Original RTAI/x86 layer implementation: \n
 *   Copyright &copy; 2000-2013 Paolo Mantegazza, \n
 *   Copyright &copy; 2000      Steve Papacharalambous, \n
 *   Copyright &copy; 2000      Stuart Hughes, \n
 *   and others.
 *
 *   RTAI/x86 rewrite over Adeos: \n
 *   Copyright &copy 2002 Philippe Gerum.
 *
 *   Porting to x86_64 architecture:
 *   Copyright &copy; 2005-2013 Paolo Mantegazza, \n
 *   Copyright &copy; 2005 Daniele Gasperini \n
 *
 *   Unification of x86 architecture:
 *   Copyright &copy; 2014-2016 Alec Ari \n
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @defgroup hal RTAI services functions.
 *
 * This module defines some functions that can be used by RTAI tasks, for
 * managing interrupts and communication services with Linux processes.
 *
 *@{*/


#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/console.h>
//#include <asm/system.h>
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
#define __RTAI_HAL__
#include <asm/rtai_hal.h>
#include <asm/rtai_lxrt.h>
#ifdef CONFIG_PROC_FS
#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <rtai_proc_fs.h>
#endif /* CONFIG_PROC_FS */
#include <stdarg.h>

MODULE_LICENSE("GPL");

static unsigned long rtai_cpufreq_arg = RTAI_CALIBRATED_CPU_FREQ;
RTAI_MODULE_PARM(rtai_cpufreq_arg, ulong);

#define RTAI_NR_IRQS  IPIPE_NR_IRQS

static unsigned long rtai_apicfreq_arg = RTAI_CALIBRATED_APIC_FREQ;
RTAI_MODULE_PARM(rtai_apicfreq_arg, ulong);

static int rtai_request_tickdev(void *);

static void rtai_release_tickdev(void);

static inline void rtai_setup_periodic_apic (unsigned count, unsigned vector)
{
if (!this_cpu_has(X86_FEATURE_TSC_DEADLINE_TIMER)) {
	apic_read(APIC_LVTT);
	apic_write(APIC_LVTT, APIC_LVT_TIMER_PERIODIC | vector);
	apic_read(APIC_TMICT);
	apic_write(APIC_TMICT, count);
}
}

static inline void rtai_setup_oneshot_apic (unsigned count, unsigned vector)
{
if (!this_cpu_has(X86_FEATURE_TSC_DEADLINE_TIMER)) {
	apic_read(APIC_LVTT);
	apic_write(APIC_LVTT, vector);
	apic_read(APIC_TMICT);
	apic_write(APIC_TMICT, count);
}
}

struct { volatile int locked, rqsted; } rt_scheduling[RTAI_NR_CPUS];

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

//static spinlock_t rtai_lsrq_lock = SPIN_LOCK_UNLOCKED;
static DEFINE_SPINLOCK(rtai_lsrq_lock);

static volatile int rtai_sync_level;

static atomic_t rtai_sync_count = ATOMIC_INIT(1);

static int rtai_last_8254_counter2;

static RTIME rtai_ts_8254;

static RT_TRAP_HANDLER rtai_trap_handler;

struct rt_times rt_times;

struct rt_times rt_smp_times[RTAI_NR_CPUS];

struct rtai_switch_data rtai_linux_context[RTAI_NR_CPUS];

struct calibration_data rtai_tunables;

volatile unsigned long rtai_cpu_realtime;

volatile unsigned long rtai_cpu_lock[2];

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
	 ret = ipipe_virtualize_irq(&rtai_domain, irq, (void *)handler, cookie, NULL, IPIPE_HANDLE_MASK | IPIPE_WIRED_MASK);
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
	int ret;
	ret = ipipe_virtualize_irq(&rtai_domain, irq, NULL, NULL, NULL, 0);
	if (!ret && IsolCpusMask && irq < IPIPE_NR_XIRQS) {
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

/**
 * start and initialize the PIC to accept interrupt request irq.
 *
 * The above function allow you to manipulate the PIC at hand, but you must
 * know what you are doing. Such a duty does not pertain to this manual and
 * you should refer to your PIC datasheet.
 *
 * Note that Linux has the same functions, but they must be used only for its
 * interrupts. Only the above ones can be safely used in real time handlers.
 *
 * It must also be remarked that when you install a real time interrupt handler,
 * RTAI already calls either rt_mask_and_ack_irq(), for level triggered
 * interrupts, or rt_ack_irq(), for edge triggered interrupts, before passing
 * control to you interrupt handler. hus generally you should just call
 * rt_unmask_irq() at due time, for level triggered interrupts, while nothing
 * should be done for edge triggered ones. Recall that in the latter case you
 * allow also any new interrupts on the same request as soon as you enable
 * interrupts at the CPU level.
 *
 * Often some of the above functions do equivalent things. Once more there is no
 * way of doing it right except by knowing the hardware you are manipulating.
 * Furthermore you must also remember that when you install a hard real time
 * handler the related interrupt is usually disabled, unless you are overtaking
 * one already owned by Linux which has been enabled by it.   Recall that if
 * have done it right, and interrupts do not show up, it is likely you have just
 * to rt_enable_irq() your irq.
 */
unsigned rt_startup_irq (unsigned irq)
{
	return rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(startup, irq);
}

/**
 * Shut down an IRQ source.
 *
 * No further interrupt request irq can be accepted.
 *
 * The above function allow you to manipulate the PIC at hand, but you must
 * know what you are doing. Such a duty does not pertain to this manual and
 * you should refer to your PIC datasheet.
 *
 * Note that Linux has the same functions, but they must be used only for its
 * interrupts. Only the above ones can be safely used in real time handlers.
 *
 * It must also be remarked that when you install a real time interrupt handler,
 * RTAI already calls either rt_mask_and_ack_irq(), for level triggered
 * interrupts, or rt_ack_irq(), for edge triggered interrupts, before passing
 * control to you interrupt handler. hus generally you should just call
 * rt_unmask_irq() at due time, for level triggered interrupts, while nothing
 * should be done for edge triggered ones. Recall that in the latter case you
 * allow also any new interrupts on the same request as soon as you enable
 * interrupts at the CPU level.
 *
 * Often some of the above functions do equivalent things. Once more there is no
 * way of doing it right except by knowing the hardware you are manipulating.
 * Furthermore you must also remember that when you install a hard real time
 * handler the related interrupt is usually disabled, unless you are overtaking
 * one already owned by Linux which has been enabled by it.   Recall that if
 * have done it right, and interrupts do not show up, it is likely you have just
 * to rt_enable_irq() your irq.
 */
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

/**
 * Enable an IRQ source.
 *
 * The above function allow you to manipulate the PIC at hand, but you must
 * know what you are doing. Such a duty does not pertain to this manual and
 * you should refer to your PIC datasheet.
 *
 * Note that Linux has the same functions, but they must be used only for its
 * interrupts. Only the above ones can be safely used in real time handlers.
 *
 * It must also be remarked that when you install a real time interrupt handler,
 * RTAI already calls either rt_mask_and_ack_irq(), for level triggered
 * interrupts, or rt_ack_irq(), for edge triggered interrupts, before passing
 * control to you interrupt handler. hus generally you should just call
 * rt_unmask_irq() at due time, for level triggered interrupts, while nothing
 * should be done for edge triggered ones. Recall that in the latter case you
 * allow also any new interrupts on the same request as soon as you enable
 * interrupts at the CPU level.
 *
 * Often some of the above functions do equivalent things. Once more there is no
 * way of doing it right except by knowing the hardware you are manipulating.
 * Furthermore you must also remember that when you install a hard real time
 * handler the related interrupt is usually disabled, unless you are overtaking
 * one already owned by Linux which has been enabled by it.   Recall that if
 * have done it right, and interrupts do not show up, it is likely you have just
 * to rt_enable_irq() your irq.
 */
void rt_enable_irq (unsigned irq)
{
	_rt_enable_irq(irq);
}

/**
 * Disable an IRQ source.
 *
 * The above function allow you to manipulate the PIC at hand, but you must
 * know what you are doing. Such a duty does not pertain to this manual and
 * you should refer to your PIC datasheet.
 *
 * Note that Linux has the same functions, but they must be used only for its
 * interrupts. Only the above ones can be safely used in real time handlers.
 *
 * It must also be remarked that when you install a real time interrupt handler,
 * RTAI already calls either rt_mask_and_ack_irq(), for level triggered
 * interrupts, or rt_ack_irq(), for edge triggered interrupts, before passing
 * control to you interrupt handler. hus generally you should just call
 * rt_unmask_irq() at due time, for level triggered interrupts, while nothing
 * should be done for edge triggered ones. Recall that in the latter case you
 * allow also any new interrupts on the same request as soon as you enable
 * interrupts at the CPU level.
 *
 * Often some of the above functions do equivalent things. Once more there is no
 * way of doing it right except by knowing the hardware you are manipulating.
 * Furthermore you must also remember that when you install a hard real time
 * handler the related interrupt is usually disabled, unless you are overtaking
 * one already owned by Linux which has been enabled by it.   Recall that if
 * have done it right, and interrupts do not show up, it is likely you have just
 * to rt_enable_irq() your irq.
 */
void rt_disable_irq (unsigned irq)
{
	if (rtai_irq_desc_chip(irq)->irq_disable) {
		rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(disable, irq);
	} else {
		rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(mask, irq);
	}
}

/**
 * Mask and acknowledge and IRQ source.
 *
 * No  * other interrupts can be accepted, once also the CPU will enable
 * interrupts, which ones depends on the PIC at hand and on how it is
 * programmed.
 *
 * The above function allow you to manipulate the PIC at hand, but you must
 * know what you are doing. Such a duty does not pertain to this manual and
 * you should refer to your PIC datasheet.
 *
 * Note that Linux has the same functions, but they must be used only for its
 * interrupts. Only the above ones can be safely used in real time handlers.
 *
 * It must also be remarked that when you install a real time interrupt handler,
 * RTAI already calls either rt_mask_and_ack_irq(), for level triggered
 * interrupts, or rt_ack_irq(), for edge triggered interrupts, before passing
 * control to you interrupt handler. hus generally you should just call
 * rt_unmask_irq() at due time, for level triggered interrupts, while nothing
 * should be done for edge triggered ones. Recall that in the latter case you
 * allow also any new interrupts on the same request as soon as you enable
 * interrupts at the CPU level.
 *
 * Often some of the above functions do equivalent things. Once more there is no
 * way of doing it right except by knowing the hardware you are manipulating.
 * Furthermore you must also remember that when you install a hard real time
 * handler the related interrupt is usually disabled, unless you are overtaking
 * one already owned by Linux which has been enabled by it.   Recall that if
 * have done it right, and interrupts do not show up, it is likely you have just
 * to rt_enable_irq() your irq.
 */
void rt_mask_and_ack_irq (unsigned irq)
{
	rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(mask_ack, irq);
}

/**
 * Unmask and IRQ source.
 *
 * The related request can then interrupt the CPU again, provided it has also
 * been acknowledged.
 *
 * The above function allow you to manipulate the PIC at hand, but you must
 * know what you are doing. Such a duty does not pertain to this manual and
 * you should refer to your PIC datasheet.
 *
 * Note that Linux has the same functions, but they must be used only for its
 * interrupts. Only the above ones can be safely used in real time handlers.
 *
 * It must also be remarked that when you install a real time interrupt handler,
 * RTAI already calls either rt_mask_and_ack_irq(), for level triggered
 * interrupts, or rt_ack_irq(), for edge triggered interrupts, before passing
 * control to you interrupt handler. hus generally you should just call
 * rt_unmask_irq() at due time, for level triggered interrupts, while nothing
 * should be done for edge triggered ones. Recall that in the latter case you
 * allow also any new interrupts on the same request as soon as you enable
 * interrupts at the CPU level.
 *
 * Often some of the above functions do equivalent things. Once more there is no
 * way of doing it right except by knowing the hardware you are manipulating.
 * Furthermore you must also remember that when you install a hard real time
 * handler the related interrupt is usually disabled, unless you are overtaking
 * one already owned by Linux which has been enabled by it.   Recall that if
 * have done it right, and interrupts do not show up, it is likely you have just
 * to rt_enable_irq() your irq.
 */
void rt_mask_irq (unsigned irq)
{
	rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(mask, irq);
}

void rt_unmask_irq (unsigned irq)
{
	rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(unmask, irq);
}

/**
 * Acknowledge an IRQ source.
 *
 * The related request can then interrupt the CPU again, provided it has not
 * been masked.
 *
 * The above function allow you to manipulate the PIC at hand, but you must
 * know what you are doing. Such a duty does not pertain to this manual and
 * you should refer to your PIC datasheet.
 *
 * Note that Linux has the same functions, but they must be used only for its
 * interrupts. Only the above ones can be safely used in real time handlers.
 *
 * It must also be remarked that when you install a real time interrupt handler,
 * RTAI already calls either rt_mask_and_ack_irq(), for level triggered
 * interrupts, or rt_ack_irq(), for edge triggered interrupts, before passing
 * control to you interrupt handler. hus generally you should just call
 * rt_unmask_irq() at due time, for level triggered interrupts, while nothing
 * should be done for edge triggered ones. Recall that in the latter case you
 * allow also any new interrupts on the same request as soon as you enable
 * interrupts at the CPU level.
 *
 * Often some of the above functions do equivalent things. Once more there is no
 * way of doing it right except by knowing the hardware you are manipulating.
 * Furthermore you must also remember that when you install a hard real time
 * handler the related interrupt is usually disabled, unless you are overtaking
 * one already owned by Linux which has been enabled by it.   Recall that if
 * have done it right, and interrupts do not show up, it is likely you have just
 * to rt_enable_irq() your irq.
 */
void rt_ack_irq (unsigned irq)
{
	_rt_enable_irq(irq);
}

void rt_end_irq (unsigned irq)
{
//      rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(unmask, irq);
	_rt_enable_irq(irq);
}

void rt_eoi_irq (unsigned irq)
{
	rtai_irq_desc_chip(irq)->rtai_irq_endis_fun(eoi, irq);
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

/**
 * Pend an IRQ to Linux.
 *
 * rt_pend_linux_irq appends a Linux interrupt irq for processing in Linux IRQ
 * mode, i.e. with hardware interrupts fully enabled.
 *
 * @note rt_pend_linux_irq does not perform any check on @a irq.
 */
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

/**
 * Uninstall a system request handler
 *
 * rt_free_srq uninstalls the specified system call @a srq, returned by
 * installing the related handler with a previous call to rt_request_srq().
 *
 * @retval EINVAL if @a srq is invalid.
 */
int rt_free_srq (unsigned srq)
{
	return  (srq < 1 || srq >= RTAI_NR_SRQS || !test_and_clear_bit(srq, &rtai_sysreq_map)) ? -EINVAL : 0;
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

irqreturn_t rtai_broadcast_to_local_timers (int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned long flags;

	rtai_hw_save_flags_and_cli(flags);
#ifdef CONFIG_SMP
	apic_wait_icr_idle();
//	apic_write_around(APIC_ICR,APIC_DM_FIXED|APIC_DEST_ALLINC|LOCAL_TIMER_VECTOR);
	apic_write_around(APIC_ICR,APIC_DM_FIXED|APIC_DEST_ALLBUT|LOCAL_TIMER_VECTOR);
#endif
	hal_pend_uncond(LOCAL_TIMER_IPI, rtai_cpuid());
	rtai_hw_restore_flags(flags);

	return RTAI_LINUX_IRQ_HANDLED;
}

static inline int REQUEST_LINUX_IRQ_BROADCAST_TO_APIC_TIMERS(void)
{
	return 0;
}

#define FREE_LINUX_IRQ_BROADCAST_TO_APIC_TIMERS();

#define RTAI_IRQ_ACK(irq) \
	do { \
		rtai_realtime_irq[irq].irq_ack(irq, &(rtai_irq_desc(irq))); \
	} while (0)

#ifdef CONFIG_SMP

static unsigned long rtai_old_irq_affinity[IPIPE_NR_XIRQS];
static unsigned long rtai_orig_irq_affinity[IPIPE_NR_XIRQS];

static DEFINE_SPINLOCK(rtai_iset_lock);

static long long rtai_timers_sync_time;

static struct apic_timer_setup_data rtai_timer_mode[RTAI_NR_CPUS];

static void rtai_critical_sync (void)
{
	struct apic_timer_setup_data *p;

	switch (rtai_sync_level) {
		case 1: {
			p = &rtai_timer_mode[rtai_cpuid()];
			while (rtai_rdtsc() < rtai_timers_sync_time);
			if (p->mode) {
				rtai_setup_periodic_apic(p->count, RTAI_APIC_TIMER_VECTOR);
			} else {
				rtai_setup_oneshot_apic(p->count, RTAI_APIC_TIMER_VECTOR);
			}
			break;
		}
		case 2: {
			rtai_setup_oneshot_apic(0, RTAI_APIC_TIMER_VECTOR);
			break;
		}
		case 3: {
			rtai_setup_periodic_apic(RTAI_APIC_ICOUNT, LOCAL_TIMER_VECTOR);
			break;
		}
	}
}

/**
 * Install a local APICs timer interrupt handler
 *
 * rt_request_apic_timers requests local APICs timers and defines the mode and
 * count to be used for each local APIC timer. Modes and counts can be chosen
 * arbitrarily for each local APIC timer.
 *
 * @param apic_timer_data is a pointer to a vector of structures
 * @code struct apic_timer_setup_data { int mode, count; }
 * @endcode sized with the number of CPUs available.
 *
 * Such a structure defines:
 * - mode: 0 for a oneshot timing, 1 for a periodic timing.
 * - count: is the period in nanoseconds you want to use on the corresponding
 * timer, not used for oneshot timers.  It is in nanoseconds to ease its
 * programming when different values are used by each timer, so that you do not
 * have to care converting it from the CPU on which you are calling this
 * function.
 *
 * The start of the timing should be reasonably synchronized.   You should call
 * this function with due care and only when you want to manage the related
 * interrupts in your own handler.   For using local APIC timers in pacing real
 * time tasks use the usual rt_start_timer(), which under the MUP scheduler sets
 * the same timer policy on all the local APIC timers, or start_rt_apic_timers()
 * that allows you to use @c struct @c apic_timer_setup_data directly.
 */
void rt_request_apic_timers (void (*handler)(void), struct apic_timer_setup_data *tmdata)
{
	volatile struct rt_times *rtimes;
	struct apic_timer_setup_data *p;
	unsigned long flags;
	int cpuid;

	TRACE_RTAI_TIMER(TRACE_RTAI_EV_TIMER_REQUEST_APIC,handler,0);

	flags = rtai_critical_enter(rtai_critical_sync);
	rtai_sync_level = 1;
	rtai_timers_sync_time = rtai_rdtsc() + rtai_imuldiv(LATCH, rtai_tunables.cpu_freq, RTAI_FREQ_8254);
	for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
		p = &rtai_timer_mode[cpuid];
		*p = tmdata[cpuid];
		rtimes = &rt_smp_times[cpuid];
		if (p->mode) {
			rtimes->linux_tick = RTAI_APIC_ICOUNT;
			rtimes->tick_time = rtai_llimd(rtai_timers_sync_time, RTAI_FREQ_APIC, rtai_tunables.cpu_freq);
			rtimes->periodic_tick = rtai_imuldiv(p->count, RTAI_FREQ_APIC, 1000000000);
			p->count = rtimes->periodic_tick;
		} else {
			rtimes->linux_tick = rtai_imuldiv(LATCH, rtai_tunables.cpu_freq, RTAI_FREQ_8254);
			rtimes->tick_time = rtai_timers_sync_time;
			rtimes->periodic_tick = rtimes->linux_tick;
			p->count = RTAI_APIC_ICOUNT;
		}
		rtimes->intr_time = rtimes->tick_time + rtimes->periodic_tick;
		rtimes->linux_time = rtimes->tick_time + rtimes->linux_tick;
	}

	p = &rtai_timer_mode[rtai_cpuid()];
	while (rtai_rdtsc() < rtai_timers_sync_time) ;

	if (p->mode) {
		rtai_setup_periodic_apic(p->count,RTAI_APIC_TIMER_VECTOR);
	} else {
		rtai_setup_oneshot_apic(p->count,RTAI_APIC_TIMER_VECTOR);
	}

	rt_release_irq(RTAI_APIC_TIMER_IPI);
	rt_request_irq(RTAI_APIC_TIMER_IPI, (rt_irq_handler_t)handler, NULL, 0);

	REQUEST_LINUX_IRQ_BROADCAST_TO_APIC_TIMERS();

	for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
		p = &tmdata[cpuid];
		if (p->mode) {
			p->count = rtai_imuldiv(p->count,RTAI_FREQ_APIC,1000000000);
		} else {
			p->count = rtai_imuldiv(p->count,rtai_tunables.cpu_freq,1000000000);
		}
	}

	rtai_request_tickdev(handler);
	rtai_critical_exit(flags);
}

/**
 * Uninstall a local APICs timer interrupt handler
 */
void rt_free_apic_timers(void)
{
	unsigned long flags;

	TRACE_RTAI_TIMER(TRACE_RTAI_EV_TIMER_APIC_FREE,0,0);

	FREE_LINUX_IRQ_BROADCAST_TO_APIC_TIMERS();
	flags = rtai_critical_enter(rtai_critical_sync);
	rtai_release_tickdev();
	rtai_sync_level = 3;
	rtai_setup_periodic_apic(RTAI_APIC_ICOUNT,LOCAL_TIMER_VECTOR);
	rt_release_irq(RTAI_APIC_TIMER_IPI);
	rtai_critical_exit(flags);
}

/**
 * Set IRQ->CPU assignment
 *
 * rt_assign_irq_to_cpu forces the assignment of the external interrupt @a irq
 * to the CPUs of an assigned mask.
 *
 * @the mask of the interrupts routing before its call.
 * @0 if @a irq is not a valid IRQ number or some internal data
 * inconsistency is found.
 *
 * @note This functions has effect only on multiprocessors systems.
 * @note With Linux 2.4.xx such a service has finally been made available
 * natively within the raw kernel. With such Linux releases
 * rt_reset_irq_to_sym_mode() resets the original Linux delivery mode, or
 * deliver affinity as they call it. So be warned that such a name is kept
 * mainly for compatibility reasons, as for such a kernel the reset operation
 * does not necessarily implies a symmetric external interrupt delivery.
 */
unsigned long rt_assign_irq_to_cpu (int irq, unsigned long cpumask)
{
	if (irq >= IPIPE_NR_XIRQS || &rtai_irq_desc(irq) == NULL || rtai_irq_desc_chip(irq) == NULL || rtai_irq_desc_chip(irq)->irq_set_affinity == NULL) {
		return 0;
	} else {
		unsigned long oldmask, flags;

		rtai_save_flags_and_cli(flags);
		spin_lock(&rtai_iset_lock);
		cpumask_copy((void *)&oldmask, irq_to_desc(irq)->irq_data.affinity);
		hal_set_irq_affinity(irq, CPUMASK_T(cpumask));
		if (oldmask) {
			rtai_old_irq_affinity[irq] = oldmask;
		}
		spin_unlock(&rtai_iset_lock);
		rtai_restore_flags(flags);

		return oldmask;
	}
}

/**
 * reset IRQ->CPU assignment
 *
 * rt_reset_irq_to_sym_mode resets the interrupt irq to the symmetric interrupts
 * management, whatever that means, existing before the very first use of RTAI
 * rt_assign_irq_to_cpu. This function applies to external interrupts only.
 *
 * @the mask of the interrupts routing before its call.
 * @0 if @a irq is not a valid IRQ number or some internal data
 * inconsistency is found.
 *
 * @note This function has effect only on multiprocessors systems.
 * @note With Linux 2.4.xx such a service has finally been made available
 * natively within the raw kernel. With such Linux releases
 * rt_reset_irq_to_sym_mode() resets the original Linux delivery mode, or
 * deliver affinity as they call it. So be warned that such a name is kept
 * mainly for compatibility reasons, as for such a kernel the reset operation
 * does not necessarily implies a symmetric external interrupt delivery.
 */
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
		cpumask_copy((void *)&oldmask, irq_to_desc(irq)->irq_data.affinity);
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

#define rtai_critical_sync NULL

void rt_request_apic_timers (void (*handler)(void), struct apic_timer_setup_data *tmdata)
{
	return;
}

void rt_free_apic_timers(void)
{
	rt_free_timer();
}

unsigned long rt_assign_irq_to_cpu (int irq, unsigned long cpus_mask)
{
	return 0;
}

unsigned long rt_reset_irq_to_sym_mode (int irq)
{
	return 0;
}

#endif /* CONFIG_SMP */

/**
 * Install a timer interrupt handler.
 *
 * rt_request_timer requests a timer of period tick ticks, and installs the
 * routine @a handler as a real time interrupt service routine for the timer.
 *
 * Set @a tick to 0 for oneshot mode (in oneshot mode it is not used).
 * If @a apic has a nonzero value the local APIC timer is used.   Otherwise
 * timing is based on the 8254.
 *
 */
static int unsigned long used_apic;

int rt_request_timer (void (*handler)(void), unsigned tick, int use_apic)
{
	unsigned long flags;
	int retval;

	TRACE_RTAI_TIMER(TRACE_RTAI_EV_TIMER_REQUEST,handler,tick);

	used_apic = use_apic;
	rtai_save_flags_and_cli(flags);
	rt_times.tick_time = rtai_rdtsc();
    	if (tick > 0) {
		rt_times.linux_tick = use_apic ? RTAI_APIC_ICOUNT : LATCH;
		rt_times.tick_time = ((RTIME)rt_times.linux_tick)*(jiffies + 1);
		rt_times.intr_time = rt_times.tick_time + tick;
		rt_times.linux_time = rt_times.tick_time + rt_times.linux_tick;
		rt_times.periodic_tick = tick;

		if (use_apic) {
			rt_release_irq(RTAI_APIC_TIMER_IPI);
			rt_request_irq(RTAI_APIC_TIMER_IPI, (rt_irq_handler_t)handler, NULL, 0);
			rtai_setup_periodic_apic(tick,RTAI_APIC_TIMER_VECTOR);
			retval = REQUEST_LINUX_IRQ_BROADCAST_TO_APIC_TIMERS();
		} else {
			outb(0x34, 0x43);
			outb(tick & 0xff, 0x40);
			outb(tick >> 8, 0x40);
			rt_release_irq(RTAI_TIMER_8254_IRQ);
 		    	retval = rt_request_irq(RTAI_TIMER_8254_IRQ, (rt_irq_handler_t)handler, NULL, 0);
/* The above rt_request_irq should not be made, it is done by the patch already, * see ipipe_timer_start; so if you install the timer handler in advance the
 * following rtai_request_tickdev will get an error and the related 8254 stuff
 * will not be initialized.
 * NOT TO BE MADE    	retval = rt_request_irq(RTAI_TIMER_8254_IRQ, (rt_irq_handler_t)handler, NULL, 0);
 * ... unless we change the patch, as we did. SO LET'S KEEP:
 */
		}
	} else {
		rt_times.linux_tick = rtai_imuldiv(LATCH,rtai_tunables.cpu_freq,RTAI_FREQ_8254);
		rt_times.intr_time = rt_times.tick_time + rt_times.linux_tick;
		rt_times.linux_time = rt_times.tick_time + rt_times.linux_tick;
		rt_times.periodic_tick = rt_times.linux_tick;

		if (use_apic) {
			rt_release_irq(RTAI_APIC_TIMER_IPI);
			rt_request_irq(RTAI_APIC_TIMER_IPI, (rt_irq_handler_t)handler, NULL, 0);
			rtai_setup_oneshot_apic(RTAI_APIC_ICOUNT,RTAI_APIC_TIMER_VECTOR);
    			retval = REQUEST_LINUX_IRQ_BROADCAST_TO_APIC_TIMERS();
		} else {
			outb(0x30, 0x43);
			outb(LATCH & 0xff, 0x40);
			outb(LATCH >> 8, 0x40);
			rt_release_irq(RTAI_TIMER_8254_IRQ);
 		    	retval = rt_request_irq(RTAI_TIMER_8254_IRQ, (rt_irq_handler_t)handler, NULL, 0);
/* The above rt_request_irq should not be made, it is done by the patch already, * see ipipe_timer_start; so if you install the timer handler in advance the
 * following rtai_request_tickdev will get an error and the related 8254 stuff
 * will not be initialized.
 * NOT TO BE MADE    	retval = rt_request_irq(RTAI_TIMER_8254_IRQ, (rt_irq_handler_t)handler, NULL, 0);
 * ... unless we change the patch, as we did. SO LET'S KEEP:
 */
		}
	}
	rtai_request_tickdev(handler);
	rtai_restore_flags(flags);
	return retval;
}

/**
 * Uninstall a timer interrupt handler.
 *
 * rt_free_timer uninstalls a timer previously set by rt_request_timer().
 */
void rt_free_timer (void)
{
	unsigned long flags;

	TRACE_RTAI_TIMER(TRACE_RTAI_EV_TIMER_FREE,0,0);

	rtai_save_flags_and_cli(flags);
	rtai_release_tickdev();
	if (used_apic) {
		FREE_LINUX_IRQ_BROADCAST_TO_APIC_TIMERS();
		rtai_setup_periodic_apic(RTAI_APIC_ICOUNT, LOCAL_TIMER_VECTOR);
		rt_release_irq(RTAI_APIC_TIMER_IPI);
		used_apic = 0;
	} else {
		outb(0x34, 0x43);
		outb(LATCH & 0xff, 0x40);
		outb(LATCH >> 8,0x40);
		if (!rt_release_irq(RTAI_TIMER_8254_IRQ)) {
		}
	}
	rtai_restore_flags(flags);
}

RT_TRAP_HANDLER rt_set_trap_handler (RT_TRAP_HANDLER handler)
{
	return (RT_TRAP_HANDLER)xchg(&rtai_trap_handler, handler);
}

RTIME rd_8254_ts (void)
{
	unsigned long flags;
	int inc, c2;
	RTIME t;

	rtai_hw_save_flags_and_cli(flags);
	outb(0xD8, 0x43);
	c2 = inb(0x42);
	inc = rtai_last_8254_counter2 - (c2 |= (inb(0x42) << 8));
	rtai_last_8254_counter2 = c2;
	t = (rtai_ts_8254 += (inc > 0 ? inc : inc + RTAI_COUNTER_2_LATCH));
	rtai_hw_restore_flags(flags);

	return t;
}

void rt_setup_8254_tsc (void)
{
	unsigned long flags;
	int c;

	flags = rtai_critical_enter(NULL);
	outb_p(0x00, 0x43);
	c = inb_p(0x40);
	c |= inb_p(0x40) << 8;
	outb_p(0xB4, 0x43);
	outb_p(RTAI_COUNTER_2_LATCH & 0xff, 0x42);
	outb_p(RTAI_COUNTER_2_LATCH >> 8, 0x42);
	rtai_ts_8254 = c + ((RTIME)LATCH)*jiffies;
	rtai_last_8254_counter2 = 0;
	outb_p((inb_p(0x61) & 0xFD) | 1, 0x61);
	rtai_critical_exit(flags);
}

#define HAL_LOCK_LINUX() \
do { sflags = rt_save_switch_to_real_time(cpuid = rtai_cpuid()); } while (0)
#define HAL_UNLOCK_LINUX() \
do { rtai_cli(); rt_restore_switch_to_linux(sflags, cpuid); } while (0)

#ifdef CONFIG_RTAI_SCHED_ISR_LOCK
void (*rtai_isr_sched)(int cpuid);
EXPORT_SYMBOL(rtai_isr_sched);
#define RTAI_SCHED_ISR_LOCK() \
	do { \
		if (!rt_scheduling[cpuid].locked++) { \
			rt_scheduling[cpuid].rqsted = 0; \
		} \
	} while (0)
#define RTAI_SCHED_ISR_UNLOCK() \
	do { \
		if (rt_scheduling[cpuid].locked && !(--rt_scheduling[cpuid].locked)) { \
			if (rt_scheduling[cpuid].rqsted > 0 && rtai_isr_sched) { \
				rtai_isr_sched(cpuid); \
			} \
		} \
	} while (0)
#else  /* !CONFIG_RTAI_SCHED_ISR_LOCK */
#define RTAI_SCHED_ISR_LOCK() \
	do {		       } while (0)
//      do { cpuid = rtai_cpuid(); } while (0)
#define RTAI_SCHED_ISR_UNLOCK() \
	do {		       } while (0)
#endif /* CONFIG_RTAI_SCHED_ISR_LOCK */

static int rtai_hirq_dispatcher (int irq)
{
	unsigned long cpuid;
	if (rtai_domain.irqs[irq].handler) {
		unsigned long sflags;
		HAL_LOCK_LINUX();
		RTAI_SCHED_ISR_LOCK();
		rtai_domain.irqs[irq].handler(irq, rtai_domain.irqs[irq].cookie);
		RTAI_SCHED_ISR_UNLOCK();
		HAL_UNLOCK_LINUX();
		if (rtai_realtime_irq[irq].retmode || test_bit(IPIPE_STALL_FLAG, ROOT_STATUS_ADR(cpuid))) {
			return 0;
		}
	}
	rtai_sti();
	hal_fast_flush_pipeline(cpuid);
	return 0;
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

static int rtai_trap_fault (unsigned event, void *evdata)
{
#ifdef HINT_DIAG_TRAPS
	static unsigned long traps_in_hard_intr = 0;
	do {
		unsigned long flags;
		rtai_save_flags_and_cli(flags);
		if (!test_bit(RTAI_IFLAG, &flags)) {
			if (!test_and_set_bit(event, &traps_in_hard_intr)) {
				HINT_DIAG_MSG(rt_printk("TRAP %d HAS INTERRUPT DISABLED (TRAPS PICTURE %lx).\n", event, traps_in_hard_intr););
			}
		}
	} while (0);
#endif

	static const int trap2sig[] = {
    		SIGFPE,	 //  0 - Divide error
		SIGTRAP,	//  1 - Debug
		SIGSEGV,	//  2 - NMI (but we ignore these)
		SIGTRAP,	//  3 - Software breakpoint
		SIGSEGV,	//  4 - Overflow
		SIGSEGV,	//  5 - Bounds
		SIGILL,	 //  6 - Invalid opcode
		SIGSEGV,	//  7 - Device not available
		SIGSEGV,	//  8 - Double fault
		SIGFPE,	 //  9 - Coprocessor segment overrun
		SIGSEGV,	// 10 - Invalid TSS
		SIGBUS,	 // 11 - Segment not present
		SIGBUS,	 // 12 - Stack segment
		SIGSEGV,	// 13 - General protection fault
		SIGSEGV,	// 14 - Page fault
		0,	      // 15 - Spurious interrupt
		SIGFPE,	 // 16 - Coprocessor error
		SIGBUS,	 // 17 - Alignment check
		SIGSEGV,	// 18 - Reserved
		SIGFPE,	 // 19 - XMM fault
		0,0,0,0,0,0,0,0,0,0,0,0
	};

	if (!in_hrt_mode(rtai_cpuid())) {
		goto propagate;
	}

	if (event == 7)	{ /* (FPU) Device not available. */
		struct task_struct *linux_task = current;
		rtai_hw_cli();
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
		rtai_hw_sti();
		goto endtrap;
	}
	if (rtai_trap_handler && rtai_trap_handler(event, trap2sig[event], (struct pt_regs *)evdata, NULL)) {
		goto endtrap;
	}
propagate:
	return 0;
endtrap:
	return 1;
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

#include <asm/rtai_usi.h>

static int intercept_syscall_prologue(unsigned long event, struct pt_regs *regs)
{
	if (likely(regs->LINUX_SYSCALL_NR >= RTAI_SYSCALL_NR)) {
		unsigned long srq = regs->LINUX_SYSCALL_REG1;
		IF_IS_A_USI_SRQ_CALL_IT(srq, regs->LINUX_SYSCALL_REG2, (long long *)regs->LINUX_SYSCALL_REG3, regs->LINUX_SYSCALL_FLAGS, 1);
		*((long long *)regs->LINUX_SYSCALL_REG3) = rtai_usrq_dispatcher(srq, regs->LINUX_SYSCALL_REG2);
		hal_test_and_fast_flush_pipeline(rtai_cpuid());
		return 1;
	}
	return 0;
}

void rtai_uvec_handler(void);

static unsigned long hal_request_apic_freq(void);

#include <linux/clockchips.h>
#include <linux/ipipe_tickdev.h>

static void rtai_install_archdep (void)
{
	ipipe_select_timers(cpu_present_mask);
	hal_catch_event(hal_root_domain, HAL_SYSCALL_PROLOGUE, (void *)intercept_syscall_prologue);

	if (rtai_cpufreq_arg == 0) {
		struct hal_sysinfo_struct sysinfo;
		hal_get_sysinfo(&sysinfo);

		rtai_cpufreq_arg = (unsigned long)sysinfo.sys_cpu_freq;
	}
	rtai_tunables.cpu_freq = rtai_cpufreq_arg;

	if (rtai_apicfreq_arg == 0) {
		rtai_apicfreq_arg = HZ*apic_read(APIC_TMICT);
		rtai_apicfreq_arg = hal_request_apic_freq();
	}
	rtai_tunables.apic_freq = rtai_apicfreq_arg;
}

static void rtai_uninstall_archdep(void)
{
	ipipe_timers_release();
	hal_catch_event(hal_root_domain, HAL_SYSCALL_PROLOGUE, NULL);
}

int rtai_calibrate_8254 (void)
{
	unsigned long flags;
	RTIME t, dt;
	int i;

	flags = rtai_critical_enter(NULL);
	outb(0x34,0x43);
	t = rtai_rdtsc();
	for (i = 0; i < 10000; i++) {
		outb(LATCH & 0xff,0x40);
		outb(LATCH >> 8,0x40);
	}
	dt = rtai_rdtsc() - t;
	rtai_critical_exit(flags);

	return rtai_imuldiv(dt, 100000, RTAI_CPU_FREQ);
}

extern void *sys_call_table[];

void rtai_set_linux_task_priority (struct task_struct *task, int policy, int prio)
{
	hal_set_linux_task_priority(task, policy, prio);
	if (task->rt_priority != prio || task->policy != policy) {
		printk("RTAI[hal]: sched_setscheduler(policy = %d, prio = %d) failed, (%s -- pid = %d)\n", policy, prio, task->comm, task->pid);
	}
}

#ifdef CONFIG_PROC_FS

struct proc_dir_entry *rtai_proc_root = NULL;

#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC)
extern void init_tsc_sync(void);
extern void cleanup_tsc_sync(void);
extern volatile long rtai_tsc_ofst[];
#endif

static int PROC_READ_FUN(rtai_read_proc)
{
	int i, none;
	PROC_PRINT_VARS;

	PROC_PRINT("\n** RTAI/x86:\n\n");
	PROC_PRINT("    CPU   Frequency: %lu (Hz)\n", rtai_tunables.cpu_freq);
	PROC_PRINT("    TIMER Frequency: %lu (Hz)\n", TIMER_FREQ);
	PROC_PRINT("    TIMER Latency: %ld (ns)\n", rtai_imuldiv(rtai_tunables.latency, 1000000000, rtai_tunables.cpu_freq));
	PROC_PRINT("    TIMER Setup: %ld (ns)\n", rtai_imuldiv(rtai_tunables.setup_time_TIMER_CPUNIT, 1000000000, rtai_tunables.cpu_freq));

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
//	PROC_PRINT("    SYSREQ=0x%x\n\n", RTAI_SYS_VECTOR);

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
	PROC_PRINT("** RTAI TSC OFFSETs (TSC units, 0 ref. CPU): ");
	for (i = 0; i < num_online_cpus(); i++) {
		PROC_PRINT("CPU#%d: %ld; ", i, rtai_tsc_ofst[i]);
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
	ent = CREATE_PROC_ENTRY("hal", S_IFREG|S_IRUGO|S_IWUSR, rtai_proc_root,
&rtai_hal_proc_fops);
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

#endif /* CONFIG_PROC_FS */

FIRST_LINE_OF_RTAI_DOMAIN_ENTRY
{
	{
		rt_printk(KERN_INFO "RTAI[hal]: compiled with %s.\n", CONFIG_RTAI_COMPILER);
	}
	for (;;) hal_suspend_domain();
}
LAST_LINE_OF_RTAI_DOMAIN_ENTRY

long rtai_catch_event (struct hal_domain_struct *from, unsigned long event, int (*handler)(unsigned long, void *))
{
	return (long)hal_catch_event(from, event, (void *)handler);
}

extern void *hal_irq_handler;

#undef ack_bad_irq
void ack_bad_irq(unsigned int irq)
{
	printk("unexpected IRQ trap at vector %02x\n", irq);
	if (cpu_has_apic) {
		__ack_APIC_irq();
	}
}

extern struct ipipe_domain ipipe_root;
void free_isolcpus_from_linux(void *);
extern unsigned long cpu_isolated_map;

int __rtai_hal_init (void)
{
	int trapnr, halinv = 0;
	struct hal_attr_struct attr;

	ipipe_catch_event(hal_root_domain, 0, 0);
	for (halinv = trapnr = 0; trapnr < HAL_NR_EVENTS; trapnr++) {
		if (hal_root_domain->legacy.handlers[trapnr] && hal_root_domain->legacy.handlers[trapnr] != hal_root_domain->legacy.handlers[0]) {
			halinv = 1;
			printk("EVENT %d INVALID %p.\n", trapnr, hal_root_domain->legacy.handlers[trapnr]);
		}
	}
	if (halinv) {
		printk(KERN_ERR "RTAI[hal]: HAL IMMEDIATE EVENT DISPATCHING BROKEN.\n");
	}

	if (num_online_cpus() > RTAI_NR_CPUS) {
		printk("RTAI[hal]: RTAI CONFIGURED WITH LESS THAN NUM ONLINE CPUS.\n");
		halinv = 1;
	}

	if (!(rtai_sysreq_virq = hal_alloc_irq())) {
		printk(KERN_ERR "RTAI[hal]: NO VIRTUAL INTERRUPT AVAILABLE.\n");
		halinv = 1;
	}

	if (halinv) {
		return -1;
	}

	for (trapnr = 0; trapnr < RTAI_NR_IRQS; trapnr++) {
		rtai_domain.irqs[trapnr].ackfn = (void *)hal_root_domain->irqs[trapnr].ackfn;
	}

	ipipe_virtualize_irq(hal_root_domain, rtai_sysreq_virq, (void *)rtai_lsrq_dispatcher, NULL, NULL, IPIPE_HANDLE_MASK);
	hal_irq_handler = rtai_hirq_dispatcher;

	rtai_install_archdep();

#ifdef CONFIG_PROC_FS
	rtai_proc_register();
#endif

	hal_init_attr(&attr);
	attr.name     = "RTAI";
	attr.domid    = RTAI_DOMAIN_ID;
	attr.entry    = (void *)rtai_domain_entry;
	attr.priority = IPIPE_HEAD_PRIORITY;
	hal_register_domain(&rtai_domain, &attr);
	for (trapnr = 0; trapnr < HAL_NR_FAULTS; trapnr++) {
		ipipe_catch_event(hal_root_domain, trapnr, (void *)rtai_trap_fault);
	}
	rtai_init_taskpri_irqs();

#ifdef CONFIG_SMP
	if (IsolCpusMask && (IsolCpusMask != cpu_isolated_map)) {
		printk("\nWARNING: IsolCpusMask (%lu) does not match cpu_isolated_map (%lu) set at boot time.\n", IsolCpusMask, cpu_isolated_map);
	}
	if (!IsolCpusMask) {
		IsolCpusMask = cpu_isolated_map;
	}
	if (IsolCpusMask) {
		for (trapnr = 0; trapnr < IPIPE_NR_XIRQS; trapnr++) {
			rtai_orig_irq_affinity[trapnr] = rt_assign_irq_to_cpu(trapnr, ~IsolCpusMask);
		}
	}
#else
	IsolCpusMask = 0;
#endif

	printk(KERN_INFO "RTAI[hal]: mounted (%s, IMMEDIATE (INTERNAL IRQs %s), ISOL_CPUS_MASK: %lx).\n", HAL_TYPE, CONFIG_RTAI_DONT_DISPATCH_CORE_IRQS ? "VECTORED" : "DISPATCHED", IsolCpusMask);

#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC)
	init_tsc_sync();
#endif

// (very) dirty development checks
{
struct hal_sysinfo_struct sysinfo;
hal_get_sysinfo(&sysinfo);
printk("SYSINFO: CPUs %d, LINUX APIC IRQ %d, TIM_FREQ %llu, CLK_FREQ %llu, CPU_FREQ %llu\n", sysinfo.sys_nr_cpus, sysinfo.sys_hrtimer_irq, sysinfo.sys_hrtimer_freq, sysinfo.sys_hrclock_freq, sysinfo.sys_cpu_freq);
printk("RTAI_APIC_TIMER_IPI: RTAI DEFINED %d, VECTOR %d; LINUX_APIC_TIMER_IPI: RTAI DEFINED %d, VECTOR %d\n", RTAI_APIC_TIMER_IPI, ipipe_apic_vector_irq(0xf1), LOCAL_TIMER_IPI, ipipe_apic_vector_irq(0xef));
printk("TIMER NAME: %s; VARIOUSLY FOUND APIC FREQs: %lu, %lu, %u\n", ipipe_timer_name(), hal_request_apic_freq(), hal_request_apic_freq(), apic_read(APIC_TMICT)*HZ);
}

	return 0;
}

void __rtai_hal_exit (void)
{
	int trapnr;
#ifdef CONFIG_PROC_FS
	rtai_proc_unregister();
#endif
	hal_irq_handler = NULL;
	hal_unregister_domain(&rtai_domain);
	for (trapnr = 0; trapnr < HAL_NR_FAULTS; trapnr++) {
		hal_catch_event(hal_root_domain, trapnr, NULL);
	}
	hal_virtualize_irq(hal_root_domain, rtai_sysreq_virq, NULL, NULL, 0);
	hal_free_irq(rtai_sysreq_virq);
	rtai_uninstall_archdep();

	if (IsolCpusMask) {
		for (trapnr = 0; trapnr < IPIPE_NR_XIRQS; trapnr++) {
			rt_reset_irq_to_sym_mode(trapnr);
		}
	}

#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC)
	cleanup_tsc_sync();
#endif

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
	hal_set_printk_sync(&rtai_domain);
	return printk("%s", buf);
}

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
EXPORT_SYMBOL(rt_request_apic_timers);
EXPORT_SYMBOL(rt_free_apic_timers);
EXPORT_SYMBOL(rt_request_timer);
EXPORT_SYMBOL(rt_free_timer);
EXPORT_SYMBOL(rt_set_trap_handler);
EXPORT_SYMBOL(rd_8254_ts);
EXPORT_SYMBOL(rt_setup_8254_tsc);
EXPORT_SYMBOL(rt_set_irq_ack);

EXPORT_SYMBOL(rtai_calibrate_8254);
EXPORT_SYMBOL(rtai_broadcast_to_local_timers);
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

EXPORT_SYMBOL(rt_scheduling);

EXPORT_SYMBOL(IsolCpusMask);

#if defined(CONFIG_GENERIC_CLOCKEVENTS) && CONFIG_RTAI_RTC_FREQ == 0

//#include <linux/clockchips.h>
//#include <linux/ipipe_tickdev.h>

void (*rt_linux_hrt_set_mode)(enum clock_event_mode, struct clock_event_device *);
int (*rt_linux_hrt_next_shot)(unsigned long, struct clock_event_device *);

#ifdef CONFIG_SMP
#define TEST_LINUX_TICK  RTAI_APIC_ICOUNT
#else
#define TEST_LINUX_TICK  (used_apic ? RTAI_APIC_ICOUNT : LATCH)
#endif

/*
 * _rt_linux_hrt_set_mode and _rt_linux_hrt_next_shot below should serve
 * RTAI examples only and assume that RTAI is in periodic mode always
 */

static void _rt_linux_hrt_set_mode(enum clock_event_mode mode, struct clock_event_device *hrt_dev)
{
	if (mode == CLOCK_EVT_MODE_ONESHOT || mode == CLOCK_EVT_MODE_SHUTDOWN) {
		rt_times.linux_tick = 0;
	} else if (mode == CLOCK_EVT_MODE_PERIODIC) {
		rt_times.linux_tick = rtai_llimd((1000000000 + HZ/2)/HZ, TIMER_FREQ, 1000000000);
	}
}

static int _rt_linux_hrt_next_shot(unsigned long delay, struct clock_event_device *hrt_dev)
{
	rt_times.linux_time = rt_times.tick_time + rtai_llimd(delay, TIMER_FREQ, 1000000000);
	return 0;
}

#ifdef __IPIPE_FEATURE_REQUEST_TICKDEV
#define  IPIPE_REQUEST_TICKDEV(a, b, c, d, e)  ipipe_request_tickdev(a, (void *)(b), (void *)(c), d, e)
#else
#define  IPIPE_REQUEST_TICKDEV(a, b, c, d, e)  ipipe_request_tickdev(a, b, c, d)
#endif

static int rtai_request_tickdev(void *handler)
{
	int mode, cpuid;
	for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
		if ((void *)rt_linux_hrt_set_mode != (void *)rt_linux_hrt_next_shot) {
			mode = ipipe_timer_start(handler, rt_linux_hrt_set_mode, rt_linux_hrt_next_shot, cpuid);
		} else {
			mode = ipipe_timer_start(handler, _rt_linux_hrt_set_mode, _rt_linux_hrt_next_shot, cpuid);
		}
		if (mode == CLOCK_EVT_MODE_UNUSED || mode == CLOCK_EVT_MODE_ONESHOT) {
			rt_times.linux_tick = 0;
		} else if (mode != CLOCK_EVT_MODE_PERIODIC) {
			return mode;
		}
	}
	return 0;
}

static void rtai_release_tickdev(void)
{
	int cpuid;
	for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
		ipipe_timer_stop(cpuid);
	}
}


static unsigned long hal_request_apic_freq(void)
{
		struct hal_sysinfo_struct sysinfo;
		hal_get_sysinfo(&sysinfo);
		return sysinfo.sys_hrtimer_freq;
}

#endif /* CONFIG_GENERIC_CLOCKEVENTS && CONFIG_RTAI_RTC_FREQ == 0 */

EXPORT_SYMBOL(rt_linux_hrt_set_mode);
EXPORT_SYMBOL(rt_linux_hrt_next_shot);
