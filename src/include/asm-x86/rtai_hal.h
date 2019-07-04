/**
 *   @ingroup hal
 *   @file
 *
 *   Copyright: 2013-2015 Paolo Mantegazza.
 *   Copyright: 2019 Alec Ari.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or 
 *   (at your option) any later version.
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

#ifndef _RTAI_ASM_X86_HAL_H
#define _RTAI_ASM_X86_HAL_H

#define RTAI_KERN_BUSY_ALIGN_RET_DELAY  CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY
#define RTAI_USER_BUSY_ALIGN_RET_DELAY  CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY

#define RTAI_SYSCALL_MODE __attribute__((regparm(0)))
#define LOCKED_LINUX_IN_IRQ_HANDLER

#include <rtai_types.h>

#ifdef CONFIG_SMP
#define RTAI_NR_CPUS  CONFIG_RTAI_CPUS
#else /* !CONFIG_SMP */
#define RTAI_NR_CPUS  1
#endif /* CONFIG_SMP */

struct calibration_data {
	unsigned long clock_freq;
	unsigned long timer_freq;
	int linux_timer_irq;
	int timer_irq;
	int sched_latency;
	int kern_latency_busy_align_ret_delay;
	int user_latency_busy_align_ret_delay;
	int setup_time_TIMER_CPUNIT;
	int setup_time_TIMER_UNIT;
	int timers_tol[RTAI_NR_CPUS];
};

int rtai_calibrate_hard_timer(void);

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
#include <linux/version.h>
#include <rtai_hal_names.h>

struct rtai_realtime_irq_s {
        int retmode;
        unsigned long cpumask;
};

// da portare in config
#define RTAI_NR_TRAPS   IPIPE_NR_FAULTS //HAL_NR_FAULTS
#define RTAI_NR_SRQS    32

#define RTAI_TIMER_NAME        (ipipe_timer_name())
#define RTAI_LINUX_TIMER_IRQ   (rtai_tunables.linux_timer_irq)
#define RTAI_CLOCK_FREQ        (rtai_tunables.clock_freq)
#define RTAI_TIMER_FREQ        (rtai_tunables.timer_freq)

#define RTAI_RESCHED_IRQ       IPIPE_RESCHEDULE_IPI
#define RTAI_SCHED_LATENCY     CONFIG_RTAI_SCHED_LATENCY

#if 0
//#define RTAI_APIC_TIMER_VECTOR    RTAI_APIC_HIGH_VECTOR
//#define RTAI_APIC_TIMER_IPI       (rtai_tunables.timer_irq) RTAI_APIC_HIGH_IPI
//#define RTAI_SMP_NOTIFY_VECTOR    RTAI_APIC_LOW_VECTOR
//#define RTAI_SMP_NOTIFY_IPI         RTAI_APIC_LOW_IPI

//#define RTAI_TIMER_8254_IRQ       0
//#define RTAI_FREQ_8254            (rtai_tunables.timer_freq)
//#define RTAI_APIC_ICOUNT          ((RTAI_FREQ_APIC + HZ/2)/HZ)
//#define RTAI_COUNTER_2_LATCH      0xfffe
//#define RTAI_LATENCY_8254         CONFIG_RTAI_SCHED_8254_LATENCY
//#define RTAI_SETUP_TIME_8254      2011

//#define RTAI_CALIBRATED_APIC_FREQ 0
//#define RTAI_FREQ_APIC            (rtai_tunables.timer_freq)
//#define RTAI_LATENCY_APIC         CONFIG_RTAI_SCHED_LATENCY
//#define RTAI_SETUP_TIME_APIC      1000

//#define RTAI_CALIBRATED_CPU_FREQ   0
#endif

#define RTAI_TIME_LIMIT  0x7000000000000000LL

#define RTAI_IFLAG  9

#define rtai_cpuid()  ipipe_processor_id()

#if 0
#define rtai_tskext(tsk, i)   ((tsk)->ptd[i])  // old (legacy) way
#else
#define rtai_tskext(tsk, i)   ((&task_thread_info(tsk)->ipipe_data)->ptd[i])
#endif
#define rtai_tskext_t(tsk, i) ((RT_TASK *)(rtai_tskext((tsk), (i))))

#define rtai_cli()                  hard_local_irq_disable_notrace()
#define rtai_sti()                  hard_local_irq_enable_notrace()
#define rtai_save_flags_and_cli(x)  do { x = hard_local_irq_save_notrace(); } while(0)
#define rtai_restore_flags(x)       hard_local_irq_restore_notrace(x)
#define rtai_save_flags(x)          do { x = hard_local_save_flags(); } while(0)

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

struct global_lock { unsigned long mask; arch_spinlock_t lock; };

#ifdef CONFIG_SMP

#if 0
#define RTAI_SPIN_LOCK_TYPE(lock) ((raw_spinlock_t *)lock)
#define rt_spin_lock(lock)    do { barrier(); _raw_spin_lock(RTAI_SPIN_LOCK_TYPE(lock)); barrier(); } while (0)
#define rt_spin_unlock(lock)  do { barrier(); _raw_spin_unlock(RTAI_SPIN_LOCK_TYPE(lock)); barrier(); } while (0)
#else
#define RTAI_SPIN_LOCK_TYPE(lock) ((arch_spinlock_t *)lock)
#define rt_spin_lock(lock)    do { barrier(); arch_spin_lock(RTAI_SPIN_LOCK_TYPE(lock)); barrier(); } while (0)
#define rt_spin_unlock(lock)  do { barrier(); arch_spin_unlock(RTAI_SPIN_LOCK_TYPE(lock)); barrier(); } while (0)
#endif

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

extern struct global_lock rtai_cpu_lock[];

static inline void __rt_get_global_lock(void)
{
        barrier();
        if (!test_and_set_bit(rtai_cpuid(), &rtai_cpu_lock[0].mask)) {
                rt_spin_lock(&rtai_cpu_lock[0].lock);
        }
        barrier();
}

static inline void __rt_release_global_lock(void)
{
        barrier();
        if (test_and_clear_bit(rtai_cpuid(), &rtai_cpu_lock[0].mask)) {
                rt_spin_unlock(&rtai_cpu_lock[0].lock);
        }
        barrier();
}

static inline void rt_get_global_lock(void)
{
        barrier();
        rtai_cli();
	__rt_get_global_lock();
        barrier();
}

static inline void rt_release_global_lock(void)
{
        barrier();
        rtai_cli();
	__rt_release_global_lock();
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
        if (!test_and_set_bit(rtai_cpuid(), &rtai_cpu_lock[0].mask)) {
                rt_spin_lock(&rtai_cpu_lock[0].lock);
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

        *flags = test_bit(rtai_cpuid(), &rtai_cpu_lock[0].mask) ? hflags : hflags | 1;
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

#define apic_write_around apic_write

#ifdef CONFIG_SMP

//#define SCHED_VECTOR  RTAI_SMP_NOTIFY_VECTOR
//#define SCHED_IPI     RTAI_RESCHED_IRQ

#if 0 // *** Let's try the patch version ***
#define _send_sched_ipi(dest) \
do { \
        apic_wait_icr_idle(); \
	apic_write_around(APIC_ICR2, (int)SET_APIC_DEST_FIELD((unsigned int)dest)); \
        apic_write_around(APIC_ICR, APIC_DEST_LOGICAL | SCHED_VECTOR); \
} while (0)
#else
#define _send_sched_ipi(dest) \
        do { ipipe_send_ipi(RTAI_RESCHED_IRQ, *((cpumask_t *)&dest)); } while (0)
#endif

#else /* !CONFIG_SMP */

#define _send_sched_ipi(dest)

#endif

extern struct hal_domain_struct rtai_domain;

#define ROOT_STATUS_ADR(cpuid)  (&(__ipipe_root_status))
#define ROOT_STATUS_VAL(cpuid)  (*(&__ipipe_root_status))

#define hal_pend_domain_uncond(irq, domain, cpuid) \
        __ipipe_set_irq_pending(domain, irq)

#define hal_fast_flush_pipeline(cpuid) \
do { \
        if (__ipipe_ipending_p(ipipe_this_cpu_root_context())) { \
                rtai_cli(); \
                __ipipe_sync_stage(); \
        } \
} while (0)

#define hal_pend_uncond(irq, cpuid)  hal_pend_domain_uncond(irq, hal_root_domain, cpuid)

#define hal_test_and_fast_flush_pipeline(cpuid) \
do { \
        if (!test_bit(IPIPE_STALL_FLAG, ROOT_STATUS_ADR(cpuid))) { \
                hal_fast_flush_pipeline(cpuid); \
                rtai_sti(); \
        } \
} while (0)

extern struct rtai_switch_data {
        volatile unsigned long sflags;
        volatile unsigned long lflags;
} rtai_linux_context[RTAI_NR_CPUS];

#define _rt_switch_to_real_time(cpuid) \
do { \
        rtai_linux_context[cpuid].lflags = xchg(ROOT_STATUS_ADR(cpuid), (1 << IPIPE_STALL_FLAG)); \
        rtai_linux_context[cpuid].sflags = 1; \
        __ipipe_set_current_domain(&rtai_domain); \
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
                _rt_switch_to_real_time(cpuid); \
        } \
} while (0)

#define in_hrt_mode(cpuid)  (rtai_linux_context[cpuid].sflags)

/* Private interface -- Internal use only */

asmlinkage int rt_printk(const char *format, ...);
asmlinkage int rt_sync_printk(const char *format, ...);

typedef int (*rt_irq_handler_t)(unsigned irq, void *cookie);

struct apic_timer_setup_data { int mode; int count; };

extern struct rt_times rt_times;

extern struct rt_times rt_smp_times[RTAI_NR_CPUS];

extern struct calibration_data rtai_tunables;

irqreturn_t rtai_broadcast_to_local_timers(int irq, void *dev_id, 
					   struct pt_regs *regs);

unsigned long rtai_critical_enter(void (*synch)(void));

void rtai_critical_exit(unsigned long flags);

int rtai_calibrate_8254(void);

void rtai_set_linux_task_priority(struct task_struct *task, int policy, 
				  int prio);

#endif /* __KERNEL__ && !__cplusplus */

/* Public interface */

#ifdef __KERNEL__

#include <linux/kernel.h>

#define rtai_print_to_screen  rt_printk

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int rt_request_irq(unsigned irq,
                   int (*handler)(unsigned irq, void *cookie),
                   void *cookie,
                   int retmode);

int rt_release_irq(unsigned irq);

int ack_8259A_irq(unsigned int irq);

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

/*@}*/

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

unsigned long rt_assign_irq_to_cpu(int irq, unsigned long cpus_mask);

unsigned long rt_reset_irq_to_sym_mode(int irq);

void rt_request_timer_cpuid(void (*handler)(void),
                            unsigned tick,
                            int cpuid);

int rt_request_timers(void *handler);

void rt_free_timers(void);

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

#ifdef __i386__
#include "rtai_hal_32.h"
#else
#include "rtai_hal_64.h"
#endif

#include <linux/ipipe_tickdev.h>
static inline void rt_set_timer_delay(int delay)
{
	if (delay) ipipe_timer_set(delay); 
	return;
}

static inline void previous_rt_set_timer_delay(int delay)
{
	if (delay) {
		unsigned long flags;
		rtai_save_flags_and_cli(flags);
		if (this_cpu_has(X86_FEATURE_TSC_DEADLINE_TIMER)) {
			wrmsrl(MSR_IA32_TSC_DEADLINE, rtai_rdtsc() + delay);
		} else {
			delay = rtai_imuldiv(delay, rtai_tunables.timer_freq, rtai_tunables.clock_freq);
			apic_write(APIC_TMICT, delay);
		}
		rtai_restore_flags(flags);
	}
}

#endif /* __KERNEL__ */

/*@}*/

static inline RTIME rt_get_tscnt(void)
{
#ifdef __i386__
        unsigned long long t;
        __asm__ __volatile__ ("rdtsc" : "=A" (t));
       return t;
#else
        union { unsigned int __ad[2]; RTIME t; } t;
        __asm__ __volatile__ ("rdtsc" : "=a" (t.__ad[0]), "=d" (t.__ad[1]));
        return t.t;
#endif
}

#endif /* !_RTAI_ASM_X86_HAL_H */

