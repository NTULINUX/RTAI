/**
 *   @ingroup hal
 *   @file
 *
 *   ARTI -- RTAI-compatible Adeos-based Real-Time Interface. Based on
 *   the original RTAI layer for x86.
 *
 *   Original RTAI/x86 layer implementation: \n
 *   Copyright &copy; 2013 Paolo Mantegazza, \n
 *   Copyright &copy; 2000 Steve Papacharalambous, \n
 *   Copyright &copy; 2000 Stuart Hughes, \n
 *   and others.
 *
 *   RTAI/x86 rewrite over Adeos: \n
 *   Copyright &copy 2002 Philippe Gerum.
 *
 *   Porting to x86_64 architecture:
 *   Copyright &copy; 2005-2013 Paolo Mantegazza, \n
 *   Copyright &copy; 2005      Daniele Gasperini \n
 *
 *   Clean-up:
 *   Copyright &copy; 2017      Alec Ari \n
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
 * @addtogroup hal
 *@{*/


#ifndef _RTAI_ASM_X86_64_HAL_H
#define _RTAI_ASM_X86_64_HAL_H

#define RTAI_SYSCALL_MODE //__attribute__((regparm(0)))

#define LOCKED_LINUX_IN_IRQ_HANDLER
#define UNWRAPPED_CATCH_EVENT

#include <rtai_hal_names.h>
#include <asm/rtai_vectors.h>
#include <rtai_types.h>

static __inline__ unsigned long ffnz (unsigned long word) {
    /* Derived from bitops.h's ffs() */
    __asm__("bsfq %1, %0"
	    : "=r" (word)
	    : "r"  (word));
    return word;
}

static inline unsigned long long rtai_ulldiv(unsigned long long ull, unsigned long uld, unsigned long *r)
{
	if (r) {
		*r = ull%uld;
	}
	return ull/uld;
}

static inline long rtai_imuldiv (long i, long mult, long div) {

    /* Returns (int)i = (int)i*(int)(mult)/(int)div. */

    int dummy;

    __asm__ __volatile__ ( \
	"mulq %%rdx\t\n" \
	"divq %%rcx\t\n" \
	: "=a" (i), "=d" (dummy)
       	: "a" (i), "d" (mult), "c" (div));

    return i;
}

static inline long long rtai_llimd(long long ll, long mult, long div) {
	return rtai_imuldiv(ll, mult, div);
}

/*
 *  u64div32c.c is a helper function provided, 2003-03-03, by:
 *  Copyright (C) 2003 Nils Hagge <hagge@rts.uni-hannover.de>
 */

static inline unsigned long long rtai_u64div32c(unsigned long long a,
						unsigned long b,
						int *r) {

	union { unsigned long long ull; unsigned long ul[2]; } u;
	u.ull = a;
	__asm__ __volatile(
	"\n	movq    %%rax,%%rbx"
	"\n	movq    %%rdx,%%rax"
	"\n	xorq    %%rdx,%%rdx"
	"\n	divq    %%rcx"
	"\n	xchgq   %%rax,%%rbx"
	"\n	divq    %%rcx"
	"\n	movq    %%rdx,%%rcx"
	"\n	movq    %%rbx,%%rdx"
	: "=a" (u.ul[0]), "=d" (u.ul[1])
	: "a"  (u.ul[0]), "d"  (u.ul[1]), "c" (b)
	: "%rbx" );

	return a;
}

#if defined(__KERNEL__) && !defined(__cplusplus)
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/desc.h>
#include <asm/io.h>
#include <asm/rtai_atomic.h>
#include <asm/rtai_fpu.h>
#include <asm/fixmap.h>
#include <asm/apic.h>
#include <rtai_trace.h>

#ifndef IPIPE_IRQ_DOALL
#define IPIPE_IRQ_DOALL
#endif

#if defined(__IPIPE_2LEVEL_IRQMAP) || defined(__IPIPE_3LEVEL_IRQMAP)
#define irqpend_himask     irqpend_himap
#define irqpend_lomask     irqpend_lomap
#define irqheld_mask       irqheld_map
#define IPIPE_IRQMASK_ANY  IPIPE_IRQ_DOALL
#define IPIPE_IRQ_IMASK    (BITS_PER_LONG - 1)
#define IPIPE_IRQ_ISHIFT   5
#endif

struct rtai_realtime_irq_s {
	int retmode;
	unsigned long cpumask;
};

/*
 * Linux has this information in io_apic.c, but it does not export it;
 * on the other hand it should be fairly stable this way and so we try
 * to avoid putting something else in our patch.
 */

#ifndef FIRST_DEVICE_VECTOR
#define FIRST_DEVICE_VECTOR (FIRST_EXTERNAL_VECTOR + VECTOR_OFFSET_START)
#endif
static inline int ext_irq_vector(int irq)
{
	if (irq != 2) {
		return (FIRST_DEVICE_VECTOR + 8*(irq < 2 ? irq : irq - 1));
	}
	return -EINVAL;
}

#define RTAI_DOMAIN_ID  0x9ac15d93  // nam2num("rtai_d")
#define RTAI_NR_TRAPS   HAL_NR_FAULTS
#define RTAI_NR_SRQS    32

#define RTAI_APIC_TIMER_VECTOR    RTAI_APIC_HIGH_VECTOR
#define RTAI_APIC_TIMER_IPI       RTAI_APIC_HIGH_IPI
#define RTAI_SMP_NOTIFY_VECTOR    RTAI_APIC_LOW_VECTOR
#define RTAI_SMP_NOTIFY_IPI       RTAI_APIC_LOW_IPI

#define RTAI_TIMER_8254_IRQ       0
#define RTAI_FREQ_8254	    1193180
#define RTAI_APIC_ICOUNT	  ((RTAI_FREQ_APIC + HZ/2)/HZ)
#define RTAI_COUNTER_2_LATCH      0xfffe
#define RTAI_LATENCY_8254	 CONFIG_RTAI_SCHED_8254_LATENCY
#define RTAI_SETUP_TIME_8254      2011

#define RTAI_CALIBRATED_APIC_FREQ 0
#define RTAI_FREQ_APIC	    (rtai_tunables.apic_freq)
#define RTAI_LATENCY_APIC	 CONFIG_RTAI_SCHED_APIC_LATENCY
#define RTAI_SETUP_TIME_APIC      1000

#define RTAI_TIME_LIMIT	    0x7000000000000000LL

#define RTAI_IFLAG  9

#define rtai_cpuid()      hal_processor_id()
#define rtai_tskext(idx)  hal_tskext[idx]

/* Use these to grant atomic protection when accessing the hardware */
#define rtai_hw_cli()		  hal_hw_cli()
#define rtai_hw_sti()		  hal_hw_sti()
#define rtai_hw_save_flags_and_cli(x)  hal_hw_local_irq_save(x)
#define rtai_hw_restore_flags(x)       hal_hw_local_irq_restore(x)
#define rtai_hw_save_flags(x)	  hal_hw_local_irq_flags(x)

/* Use these to grant atomic protection in hard real time code */
#define rtai_cli()		  hal_hw_cli()
#define rtai_sti()		  hal_hw_sti()
#define rtai_save_flags_and_cli(x)  hal_hw_local_irq_save(x)
#define rtai_restore_flags(x)       hal_hw_local_irq_restore(x)
#define rtai_save_flags(x)	  hal_hw_local_irq_flags(x)

#define ROOT_STATUS_ADR(cpuid)  (&(__ipipe_root_status))
#define ROOT_STATUS_VAL(cpuid)  (*(&__ipipe_root_status))


#if defined(__IPIPE_2LEVEL_IRQMAP) || defined(__IPIPE_3LEVEL_IRQMAP)
#define hal_pend_domain_uncond(irq, domain, cpuid) \
	__ipipe_set_irq_pending(domain, irq)

#define hal_fast_flush_pipeline(cpuid) \
do { \
	if (__ipipe_ipending_p(ipipe_this_cpu_root_context())) { \
		rtai_cli(); \
		__ipipe_sync_stage(); \
	} \
} while (0)
#else
#define hal_pend_domain_uncond(irq, domain, cpuid) \
do { \
	if (likely(!test_bit(IPIPE_LOCK_FLAG, &(domain)->irqs[irq].control))) { \
		__set_bit((irq) & IPIPE_IRQ_IMASK, &ipipe_cpudom_var(domain, irqpend_lomask)[(irq) >> IPIPE_IRQ_ISHIFT]); \
		__set_bit((irq) >> IPIPE_IRQ_ISHIFT, &ipipe_cpudom_var(domain, irqpend_himask)); \
	} else { \
		__set_bit((irq) & IPIPE_IRQ_IMASK, &ipipe_cpudom_var(domain, irqheld_mask)[(irq) >> IPIPE_IRQ_ISHIFT]); \
	} \
	ipipe_cpudom_var(domain, irqall)[irq]++; \
} while (0)

#define hal_fast_flush_pipeline(cpuid) \
do { \
	if (ipipe_cpudom_var(hal_root_domain, irqpend_himask) != 0) { \
		rtai_cli(); \
		hal_sync_stage(IPIPE_IRQMASK_ANY); \
	} \
} while (0)

#endif

#define hal_pend_uncond(irq, cpuid)  hal_pend_domain_uncond(irq, hal_root_domain, cpuid)

extern volatile unsigned long *ipipe_root_status[];

#define hal_test_and_fast_flush_pipeline(cpuid) \
do { \
	if (!test_bit(IPIPE_STALL_FLAG, ROOT_STATUS_ADR(cpuid))) { \
		hal_fast_flush_pipeline(cpuid); \
		rtai_sti(); \
	} \
} while (0)

#ifdef CONFIG_PREEMPT
#define rtai_save_and_lock_preempt_count() \
	do { int *prcntp, prcnt; prcnt = xchg(prcntp = &preempt_count(), 1);
#define rtai_restore_preempt_count() \
	     *prcntp = prcnt; } while (0)
#else
#define rtai_save_and_lock_preempt_count();
#define rtai_restore_preempt_count();
#endif

typedef int (*rt_irq_handler_t)(unsigned irq, void *cookie);

#define RTAI_CALIBRATED_CPU_FREQ   0
#define RTAI_CPU_FREQ	      (rtai_tunables.cpu_freq)

static inline unsigned long long rtai_rdtsc (void)
{
     unsigned int __a,__d;
     asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
     return ((unsigned long)__a) | (((unsigned long)__d)<<32);
}

struct apic_timer_setup_data {
    int mode;
    int count;
};

extern struct rt_times rt_times;

extern struct rt_times rt_smp_times[RTAI_NR_CPUS];

extern struct calibration_data rtai_tunables;

extern volatile unsigned long rtai_cpu_lock[];

#define apic_write_around apic_write

//#define RTAI_TASKPRI 0xf0  // simplest usage without changing Linux code base
#if defined(CONFIG_X86_LOCAL_APIC) && defined(RTAI_TASKPRI)
#define SET_TASKPRI(cpuid) \
	if (!rtai_linux_context[cpuid].set_taskpri) { \
		apic_write_around(APIC_TASKPRI, ((apic_read(APIC_TASKPRI) & ~APIC_TPRI_MASK) | RTAI_TASKPRI)); \
		rtai_linux_context[cpuid].set_taskpri = 1; \
	}
#define CLR_TASKPRI(cpuid) \
	if (rtai_linux_context[cpuid].set_taskpri) { \
		apic_write_around(APIC_TASKPRI, (apic_read(APIC_TASKPRI) & ~APIC_TPRI_MASK)); \
		rtai_linux_context[cpuid].set_taskpri = 0; \
	}
#else
#define SET_TASKPRI(cpuid)
#define CLR_TASKPRI(cpuid)
#endif

extern struct rtai_switch_data {
	volatile unsigned long sflags;
	volatile unsigned long lflags;
#if defined(CONFIG_X86_LOCAL_APIC) && defined(RTAI_TASKPRI)
	volatile unsigned long set_taskpri;
#endif
} rtai_linux_context[RTAI_NR_CPUS];

irqreturn_t rtai_broadcast_to_local_timers(int irq,
					   void *dev_id,
					   struct pt_regs *regs);

static inline unsigned long rtai_save_flags_irqbit(void)
{
	unsigned long flags;
	rtai_save_flags(flags);
	return flags & (1 << RTAI_IFLAG);
}

static inline unsigned long rtai_save_flags_irqbit_and_cli(void)
{
	unsigned long flags;
	rtai_save_flags_and_cli(flags);
	return flags & (1 << RTAI_IFLAG);
}

#ifdef CONFIG_SMP

#define SCHED_VECTOR  RTAI_SMP_NOTIFY_VECTOR
#define SCHED_IPI     RTAI_SMP_NOTIFY_IPI

#define _send_sched_ipi(dest) \
do { \
	apic_wait_icr_idle(); \
	apic_write_around(APIC_ICR2, (int)SET_APIC_DEST_FIELD((unsigned int)dest)); \
	apic_write_around(APIC_ICR, APIC_DEST_LOGICAL | SCHED_VECTOR); \
} while (0)

#define RTAI_SPIN_LOCK_TYPE(lock) ((raw_spinlock_t *)lock)
#define rt_spin_lock(lock)    do { barrier(); _raw_spin_lock(RTAI_SPIN_LOCK_TYPE(lock)); barrier(); } while (0)
#define rt_spin_unlock(lock)  do { barrier(); _raw_spin_unlock(RTAI_SPIN_LOCK_TYPE(lock)); barrier(); } while (0)

static inline void rt_spin_lock_hw_irq(spinlock_t *lock)
{
	rtai_hw_cli();
	rt_spin_lock(lock);
}

static inline void rt_spin_unlock_hw_irq(spinlock_t *lock)
{
	rt_spin_unlock(lock);
	rtai_hw_sti();
}

static inline unsigned long rt_spin_lock_hw_irqsave(spinlock_t *lock)
{
	unsigned long flags;
	rtai_hw_save_flags_and_cli(flags);
	rt_spin_lock(lock);
	return flags;
}

static inline void rt_spin_unlock_hw_irqrestore(unsigned long flags, spinlock_t *lock)
{
	rt_spin_unlock(lock);
	rtai_hw_restore_flags(flags);
}

static inline void rt_spin_lock_irq(spinlock_t *lock) {

    rtai_cli();
    rt_spin_lock(lock);
}

static inline void rt_spin_unlock_irq(spinlock_t *lock) {

    rt_spin_unlock(lock);
    rtai_sti();
}

static inline unsigned long rt_spin_lock_irqsave(spinlock_t *lock) {

    unsigned long flags;
    rtai_save_flags_and_cli(flags);
    rt_spin_lock(lock);
    return flags;
}

static inline void rt_spin_unlock_irqrestore(unsigned long flags, spinlock_t *lock)
{
	rt_spin_unlock(lock);
	rtai_restore_flags(flags);
}

#if RTAI_NR_CPUS > 2

// taken from Linux, see the related code there for an explanation

static inline void rtai_spin_glock(volatile unsigned long *lock)
{
 short inc = 0x0100;
 __asm__ __volatile__ (
 LOCK_PREFIX "xaddw %w0, %1\n"
 "1:\t"
 "cmpb %h0, %b0\n\t"
 "je 2f\n\t"
 "rep; nop\n\t"
 "movb %1, %b0\n\t"
 "jmp 1b\n"
 "2:"
 :"+Q" (inc), "+m" (lock[1])
 :
 :"memory", "cc");
}

static inline void rtai_spin_gunlock(volatile unsigned long *lock)
{
 __asm__ __volatile__(
 LOCK_PREFIX "incb %0"
 :"+m" (lock[1])
 :
 :"memory", "cc");
}

#else

static inline void rtai_spin_glock(volatile unsigned long *lock)
{
	while (test_and_set_bit(31, lock)) {
		cpu_relax();
	}
	barrier();
}

static inline void rtai_spin_gunlock(volatile unsigned long *lock)
{
	test_and_clear_bit(31, lock);
	cpu_relax();
}

#endif

static inline void rt_get_global_lock(void)
{
	barrier();
	rtai_cli();
	if (!test_and_set_bit(hal_processor_id(), &rtai_cpu_lock[0])) {
		rtai_spin_glock(&rtai_cpu_lock[0]);
	}
	barrier();
}

static inline void rt_release_global_lock(void)
{
	barrier();
	rtai_cli();
	if (test_and_clear_bit(hal_processor_id(), &rtai_cpu_lock[0])) {
		rtai_spin_gunlock(&rtai_cpu_lock[0]);
	}
	barrier();
}

/**
 * Disable interrupts across all CPUs
 *
 * rt_global_cli hard disables interrupts (cli) on the requesting CPU and
 * acquires the global spinlock to the calling CPU so that any other CPU
 * synchronized by this method is blocked. Nested calls to rt_global_cli within
 * the owner CPU will not cause a deadlock on the global spinlock, as it would
 * happen for a normal spinlock.
 *
 * rt_global_sti hard enables interrupts (sti) on the calling CPU and releases
 * the global lock.
 */
static inline void rt_global_cli(void)
{
    rt_get_global_lock();
}

/**
 * Enable interrupts across all CPUs
 *
 * rt_global_sti hard enables interrupts (sti) on the calling CPU and releases
 * the global lock.
 */
static inline void rt_global_sti(void)
{
    rt_release_global_lock();
    rtai_sti();
}

/**
 * Save CPU flags
 *
 * rt_global_save_flags_and_cli combines rt_global_save_flags() and
 * rt_global_cli().
 */
static inline int rt_global_save_flags_and_cli(void)
{
	unsigned long flags;

	barrier();
	flags = rtai_save_flags_irqbit_and_cli();
	if (!test_and_set_bit(hal_processor_id(), &rtai_cpu_lock[0])) {
		rtai_spin_glock(&rtai_cpu_lock[0]);
		barrier();
		return flags | 1;
	}
	barrier();
	return flags;
}

/**
 * Save CPU flags
 *
 * rt_global_save_flags saves the CPU interrupt flag (IF) bit 9 of @a flags and
 * ORs the global lock flag in the first 8 bits of flags. From that you can
 * rightly infer that RTAI does not support more than 8 CPUs.
 */
static inline void rt_global_save_flags(unsigned long *flags)
{
	unsigned long hflags = rtai_save_flags_irqbit_and_cli();

	*flags = test_bit(hal_processor_id(), &rtai_cpu_lock[0]) ? hflags : hflags | 1;
	if (hflags) {
		rtai_sti();
	}
}

/**
 * Restore CPU flags
 *
 * rt_global_restore_flags restores the CPU hard interrupt flag (IF)
 * and the state of the global inter-CPU lock, according to the state
 * given by flags.
 */
static inline void rt_global_restore_flags(unsigned long flags)
{
	barrier();
	if (test_and_clear_bit(0, &flags)) {
		rt_release_global_lock();
	} else {
		rt_get_global_lock();
	}
	if (flags) {
		rtai_sti();
	}
	barrier();
}

#else /* !CONFIG_SMP */

#define _send_sched_ipi(dest)

#define rt_spin_lock(lock)
#define rt_spin_unlock(lock)

#define rt_spin_lock_irq(lock)    do { rtai_cli(); } while (0)
#define rt_spin_unlock_irq(lock)  do { rtai_sti(); } while (0)

static inline unsigned long rt_spin_lock_irqsave(spinlock_t *lock)
{
	unsigned long flags;
	rtai_save_flags_and_cli(flags);
	return flags;
}
#define rt_spin_unlock_irqrestore(flags, lock)  do { rtai_restore_flags(flags); } while (0)

#define rt_get_global_lock()      do { rtai_cli(); } while (0)
#define rt_release_global_lock()

#define rt_global_cli()  do { rtai_cli(); } while (0)
#define rt_global_sti()  do { rtai_sti(); } while (0)

static inline unsigned long rt_global_save_flags_and_cli(void)
{
	unsigned long flags;
	rtai_save_flags_and_cli(flags);
	return flags;
}
#define rt_global_restore_flags(flags)  do { rtai_restore_flags(flags); } while (0)

#define rt_global_save_flags(flags)     do { rtai_save_flags(*flags); } while (0)

#endif

asmlinkage int rt_printk(const char *format, ...);
asmlinkage int rt_sync_printk(const char *format, ...);

extern struct hal_domain_struct rtai_domain;

#define _rt_switch_to_real_time(cpuid) \
do { \
	rtai_linux_context[cpuid].lflags = xchg(ROOT_STATUS_ADR(cpuid), (1 << IPIPE_STALL_FLAG)); \
	rtai_linux_context[cpuid].sflags = 1; \
	__ipipe_set_current_domain(&rtai_domain); /*hal_current_domain(cpuid) = &rtai_domain;*/ \
} while (0)

#define rt_switch_to_linux(cpuid) \
do { \
	if (rtai_linux_context[cpuid].sflags) { \
		__ipipe_set_current_domain(hal_root_domain); /*hal_current_domain(cpuid) = hal_root_domain; */\
		ROOT_STATUS_VAL(cpuid) = rtai_linux_context[cpuid].lflags; \
		rtai_linux_context[cpuid].sflags = 0; \
	} \
} while (0)

#define rt_switch_to_real_time(cpuid) \
do { \
	if (!rtai_linux_context[cpuid].sflags) { \
		_rt_switch_to_real_time(cpuid); \
	} \
} while (0)

#define rtai_get_intr_handler(v) \
	((((unsigned long)idt_table[v].offset_high) << 32) | (((unsigned long)idt_table[v].offset_middle) << 16) | ((unsigned long)idt_table[v].offset_low))
#define ack_bad_irq hal_ack_system_irq // linux does not export ack_bad_irq

#define rtai_init_taskpri_irqs() \
do { \
	int v; \
	for (v = SPURIOUS_APIC_VECTOR + 1; v < 256; v++) { \
		hal_virtualize_irq(hal_root_domain, v - FIRST_EXTERNAL_VECTOR, (void (*)(unsigned))rtai_get_intr_handler(v), (void *)ack_bad_irq, IPIPE_HANDLE_MASK); \
	} \
} while (0)

static inline int rt_save_switch_to_real_time(int cpuid)
{
	if (!rtai_linux_context[cpuid].sflags) {
		_rt_switch_to_real_time(cpuid);
		return 0;
	}
	return 1;
}

#define rt_restore_switch_to_linux(sflags, cpuid) \
do { \
	if (!sflags) { \
		rt_switch_to_linux(cpuid); \
	} else if (!rtai_linux_context[cpuid].sflags) { \
		SET_TASKPRI(cpuid); \
		_rt_switch_to_real_time(cpuid); \
	} \
} while (0)

#define in_hrt_mode(cpuid)  (rtai_linux_context[cpuid].sflags)

static inline unsigned long save_and_set_taskpri(unsigned long taskpri)
{
	unsigned long saved_taskpri = apic_read(APIC_TASKPRI);
	apic_write(APIC_TASKPRI, taskpri);
	return saved_taskpri;
}

#define restore_taskpri(taskpri) \
	do { apic_write_around(APIC_TASKPRI, taskpri); } while (0)

static inline void rt_set_timer_delay(int delay)
{
	if (delay) {
		unsigned long flags;
		rtai_hw_save_flags_and_cli(flags);
		if (this_cpu_has(X86_FEATURE_TSC_DEADLINE_TIMER)) {
			wrmsrl(MSR_IA32_TSC_DEADLINE, rtai_rdtsc() + delay);
		} else {
			delay = rtai_imuldiv(delay, rtai_tunables.apic_freq, rtai_tunables.cpu_freq);
			apic_write_around(APIC_TMICT, delay);
		}
		rtai_hw_restore_flags(flags);
	}
}

    /* Private interface -- Internal use only */

unsigned long rtai_critical_enter(void (*synch)(void));

void rtai_critical_exit(unsigned long flags);

int rtai_calibrate_8254(void);

void rtai_set_linux_task_priority(struct task_struct *task,
				  int policy,
				  int prio);

long rtai_catch_event (struct hal_domain_struct *domain, unsigned long event, int (*handler)(unsigned long, void *));

#endif /* __KERNEL__ && !__cplusplus */

    /* Public interface */

#ifdef __KERNEL__

#include <linux/kernel.h>

#define rtai_print_to_screen  rt_printk

void *ll2a(long long ll, char *s);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int rt_request_irq(unsigned irq,
		   int (*handler)(unsigned irq, void *cookie),
		   void *cookie,
		   int retmode);

int rt_release_irq(unsigned irq);

int rt_set_irq_ack(unsigned int irq, int (*irq_ack)(unsigned int, void *));

static inline int rt_request_irq_wack(unsigned irq, int (*handler)(unsigned irq, void *cookie), void *cookie, int retmode, int (*irq_ack)(unsigned int, void *))
{
	int retval;
	if ((retval = rt_request_irq(irq, handler, cookie, retmode)) < 0) {
		return retval;
	}
	return rt_set_irq_ack(irq, irq_ack);
}

void rt_set_irq_cookie(unsigned irq, void *cookie);

void rt_set_irq_retmode(unsigned irq, int fastret);

/**
 * @name Programmable Interrupt Controllers (PIC) management functions.
 *
 *@{*/
unsigned rt_startup_irq(unsigned irq);

void rt_shutdown_irq(unsigned irq);

void rt_enable_irq(unsigned irq);

void rt_disable_irq(unsigned irq);

void rt_mask_and_ack_irq(unsigned irq);

void rt_unmask_irq(unsigned irq);

void rt_ack_irq(unsigned irq);

int rt_request_linux_irq(unsigned irq,
			 void *handler,
			 char *name,
			 void *dev_id);

int rt_free_linux_irq(unsigned irq,
		      void *dev_id);

void rt_pend_linux_irq(unsigned irq);

RTAI_SYSCALL_MODE void usr_rt_pend_linux_irq(unsigned irq);

void rt_pend_linux_srq(unsigned srq);

int rt_request_srq(unsigned label,
		   void (*k_handler)(void),
		   long long (*u_handler)(unsigned long));

int rt_free_srq(unsigned srq);

unsigned long rt_assign_irq_to_cpu (int irq, unsigned long cpumask);

unsigned long rt_reset_irq_to_sym_mode(int irq);

void rt_request_timer_cpuid(void (*handler)(void),
			    unsigned tick,
			    int cpuid);

void rt_request_apic_timers(void (*handler)(void),
			    struct apic_timer_setup_data *tmdata);

void rt_free_apic_timers(void);

int rt_request_timer(void (*handler)(void), unsigned tick, int use_apic);

void rt_free_timer(void);

RT_TRAP_HANDLER rt_set_trap_handler(RT_TRAP_HANDLER handler);

void rt_release_rtc(void);

void rt_request_rtc(long rtc_freq, void *handler);

#define rt_mount()

#define rt_umount()

RTIME rd_8254_ts(void);

void rt_setup_8254_tsc(void);

void (*rt_set_ihook(void (*hookfn)(int)))(int);

/* Deprecated calls. */

static inline int rt_request_global_irq(unsigned irq, void (*handler)(void))
{
	return rt_request_irq(irq, (int (*)(unsigned,void *))handler, 0, 0);
}

static inline int rt_request_global_irq_ext(unsigned irq, void (*handler)(void), unsigned long cookie)
{
	return rt_request_irq(irq, (int (*)(unsigned,void *))handler, (void *)cookie, 1);
}

static inline void rt_set_global_irq_ext(unsigned irq, int ext, unsigned long cookie)
{
	rt_set_irq_cookie(irq, (void *)cookie);
}

static inline int rt_free_global_irq(unsigned irq)
{
	return rt_release_irq(irq);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#include <asm/rtai_oldnames.h>

#define RTAI_DEFAULT_TICK    100000
#ifdef CONFIG_RTAI_TRACE
#define RTAI_DEFAULT_STACKSZ 8192
#else /* !CONFIG_RTAI_TRACE */
#define RTAI_DEFAULT_STACKSZ 1024
#endif /* CONFIG_RTAI_TRACE */

#endif /* !_RTAI_ASM_X86_64_HAL_H */

#ifndef _RTAI_HAL_XN_H
#define _RTAI_HAL_XN_H

#define __range_ok(addr, size) (__range_not_ok(addr,size) == 0)

#define NON_RTAI_SCHEDULE(cpuid)  do { schedule(); } while (0)

#endif /* !_RTAI_HAL_XN_H */

