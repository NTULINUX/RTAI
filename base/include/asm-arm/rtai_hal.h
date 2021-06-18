/*
 * ARTI -- RTAI-compatible Adeos-based Real-Time Interface.
 *	   Based on ARTI for x86 and RTHAL for ARM.
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
#ifndef _RTAI_ASM_ARM_HAL_H
#define _RTAI_ASM_ARM_HAL_H

#include <linux/version.h>
#include <linux/autoconf.h>

#define RTAI_SYSCALL_MODE

#define LOCKED_LINUX_IN_IRQ_HANDLER
#define DOMAIN_TO_STALL  (fusion_domain)

#include <rtai_hal_names.h>
#include <asm/rtai_vectors.h>
#include <rtai_types.h>	
#include <asm/div64.h>

#define RTAI_NR_CPUS	1
#define RTAI_NR_IRQS  IPIPE_NR_XIRQS

#ifdef CONFIG_ARCH_AT91
#ifndef __LINUX_ARM_ARCH__
#define __LINUX_ARM_ARCH__ 5
#endif
#else 
#ifndef __LINUX_ARM_ARCH__
#warning "by default, __LINUX_ARM_ARCH__ is setted to 4 in order to ensure compatibility with all other arm arch."
#define __LINUX_ARM_ARCH__ 4
#endif
#endif

#ifndef _RTAI_FUSION_H

#ifndef __KERNEL__

/*
 * Utility function for interrupt handler.
 */
extern inline unsigned long
ffnz(unsigned long word)
{
    int count;

    /* CLZ is available only on ARMv5 */
    asm( "clz %0, %1" : "=r" (count) : "r" (word) );
    return 31-count;
}

#else /* __KERNEL__ */

extern inline unsigned long
ffnz(unsigned long word)
{
    return ffs(word) - 1;
}
#endif /* !__KERNEL__ */

#endif /* !_RTAI_FUSION_H */

#ifdef __KERNEL__
#include <asm/system.h>
#else
#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"
#endif /* __KERNEL__ */

#ifdef __BIG_ENDIAN
#define endianstruct struct { unsigned _h; unsigned _l; } _s
#else /* __LITTLE_ENDIAN */
#define endianstruct struct { unsigned _l; unsigned _h; } _s
#endif

#ifndef __rtai_u64tou32
#define __rtai_u64tou32(ull, h, l) ({          \
    union { unsigned long long _ull;            \
    endianstruct;                               \
    } _u;                                       \
    _u._ull = (ull);                            \
    (h) = _u._s._h;                             \
    (l) = _u._s._l;                             \
})
#endif /* !__rtai_u64tou32 */

#ifndef __rtai_u64fromu32
#define __rtai_u64fromu32(h, l) ({             \
    union { unsigned long long _ull;            \
    endianstruct;                               \
    } _u;                                       \
    _u._s._h = (h);                             \
    _u._s._l = (l);                             \
    _u._ull;                                    \
})
#endif /* !__rtai_u64fromu32 */

#ifndef rtai_ullmul
extern inline unsigned long long
__rtai_generic_ullmul(const unsigned m0, 
                       const unsigned m1)
{
    return (unsigned long long) m0 * m1;
}
#define rtai_ullmul(m0,m1) __rtai_generic_ullmul((m0),(m1))
#endif /* !rtai_ullmul */

#ifndef rtai_ulldiv
static inline unsigned long long 
__rtai_generic_ulldiv (unsigned long long ull,
                        const unsigned uld,
                        unsigned long *const rp)
{
    const unsigned r = do_div(ull, uld);

    if (rp)
        *rp = r;

    return ull;
}
#define rtai_ulldiv(ull,uld,rp) __rtai_generic_ulldiv((ull),(uld),(rp))
#endif /* !rtai_ulldiv */

#ifndef rtai_uldivrem
#define rtai_uldivrem(ull,ul,rp) ((unsigned) rtai_ulldiv((ull),(ul),(rp)))
#endif /* !rtai_uldivrem */

#ifndef rtai_imuldiv
extern inline int
__rtai_generic_imuldiv (int i,
                         int mult,
                         int div)
{
    /* Returns (int)i = (unsigned long long)i*(unsigned)(mult)/(unsigned)div. */
    const unsigned long long ull = rtai_ullmul(i, mult);
    return rtai_uldivrem(ull, div, NULL);
}
#define rtai_imuldiv(i,m,d) __rtai_generic_imuldiv((i),(m),(d))
#endif /* !rtai_imuldiv */

#ifndef rtai_llimd
/* Division of an unsigned 96 bits ((h << 32) + l) by an unsigned 32 bits. 
   Building block for llimd. Without const qualifiers, gcc reload registers
   after each call to uldivrem. */
static inline unsigned long long
__rtai_generic_div96by32 (const unsigned long long h,
                           const unsigned l,
                           const unsigned d,
                           unsigned long *const rp)
{
    unsigned long rh;
    const unsigned qh = rtai_uldivrem(h, d, &rh);
    const unsigned long long t = __rtai_u64fromu32(rh, l);
    const unsigned ql = rtai_uldivrem(t, d, rp);

    return __rtai_u64fromu32(qh, ql);
}

extern inline unsigned long long 
__rtai_generic_ullimd (const unsigned long long op,
                        const unsigned m,
                        const unsigned d)
{
    unsigned oph, opl, tlh, tll;
    unsigned long long th, tl;

    __rtai_u64tou32(op, oph, opl);
    tl = rtai_ullmul(opl, m);
    __rtai_u64tou32(tl, tlh, tll);
    th = rtai_ullmul(oph, m);
    th += tlh;

    return __rtai_generic_div96by32(th, tll, d, NULL);
}

extern inline  long long 
__rtai_generic_llimd (long long op,
                       unsigned m,
                       unsigned d)
{

    if(op < 0LL)
        return -__rtai_generic_ullimd(-op, m, d);
    return __rtai_generic_ullimd(op, m, d);
}
#define rtai_llimd(ll,m,d) __rtai_generic_llimd((ll),(m),(d))
#endif /* !rtai_llimd */

#if defined(__KERNEL__) && !defined(__cplusplus)
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/rtai_atomic.h>
#include <asm/rtai_fpu.h>

#include <rtai_trace.h>

struct rtai_realtime_irq_s {
        int (*handler)(unsigned irq, void *cookie);
        void *cookie;
        int retmode;
        int cpumask;
        int (*irq_ack)(unsigned int);
};

#define RTAI_DOMAIN_ID  0x9ac15d93  // nam2num("rtai_d")
#define RTAI_NR_TRAPS   HAL_NR_FAULTS
#define RTAI_NR_SRQS			32

#define RTAI_TIME_LIMIT            0x7000000000000000LL

#define RTAI_IFLAG  9

#define rtai_cpuid()      hal_processor_id()
#define rtai_tskext(idx)  hal_tskext[idx]

/* Use these to grant atomic protection when accessing the hardware */
#define rtai_hw_cli()                  hal_hw_cli()
#define rtai_hw_sti()                  hal_hw_sti()
#define rtai_hw_save_flags_and_cli(x)  hal_hw_local_irq_save(x)
#define rtai_hw_restore_flags(x)       hal_hw_local_irq_restore(x)
#define rtai_hw_save_flags(x)          hal_hw_local_irq_flags(x)

/* Use these to grant atomic protection in hard real time code */
#define rtai_cli()                  hal_hw_cli()
#define rtai_sti()                  hal_hw_sti()
#define rtai_save_flags_and_cli(x)  hal_hw_local_irq_save(x)
#define rtai_restore_flags(x)       hal_hw_local_irq_restore(x)
#define rtai_save_flags(x)          hal_hw_local_irq_flags(x)

extern volatile unsigned long hal_pended;

static inline struct hal_domain_struct *get_domain_pointer(int n)
{
	struct list_head *p = hal_pipeline.next;
	struct hal_domain_struct *d;
	unsigned long i = 0;
	while (p != &hal_pipeline) {
		d = list_entry(p, struct hal_domain_struct, p_link);
		if (++i == n) {
			return d;
		}
		p = d->p_link.next;
	}
	return (struct hal_domain_struct *)i;
}

#define hal_pend_domain_uncond(irq, domain, cpuid) \
do { \
        hal_irq_hits_pp(irq, domain, cpuid); \
        __set_bit((irq) & IPIPE_IRQ_IMASK, &domain->cpudata[cpuid].irq_pending_lo[(irq) >> IPIPE_IRQ_ISHIFT]); \
        __set_bit((irq) >> IPIPE_IRQ_ISHIFT, &domain->cpudata[cpuid].irq_pending_hi); \
        test_and_set_bit(cpuid, &hal_pended); /* cautious, cautious */ \
} while (0)

#define hal_pend_uncond(irq, cpuid)  hal_pend_domain_uncond(irq, hal_root_domain, cpuid)

#define hal_fast_flush_pipeline(cpuid) \
do { \
        if (hal_root_domain->cpudata[cpuid].irq_pending_hi != 0) { \
                rtai_cli(); \
                hal_sync_stage(IPIPE_IRQMASK_ANY); \
        } \
} while (0)

extern volatile unsigned long *ipipe_root_status[];

#define hal_test_and_fast_flush_pipeline(cpuid) \
do { \
       	if (!test_bit(IPIPE_STALL_FLAG, ipipe_root_status[cpuid])) { \
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

#define RTAI_CPU_FREQ              (rtai_tunables.cpu_freq)

struct calibration_data {

    unsigned long cpu_freq;
    unsigned long apic_freq;
    int latency;
    int setup_time_TIMER_CPUNIT;
    int setup_time_TIMER_UNIT;
    int timers_tol[RTAI_NR_CPUS];
};

struct apic_timer_setup_data {

    int mode;
    int count;
};

extern struct rt_times			rt_times;
extern struct rt_times			rt_smp_times[RTAI_NR_CPUS];
extern struct calibration_data rtai_tunables;
extern volatile unsigned long rtai_cpu_lock;

#define SET_TASKPRI(cpuid)
#define CLR_TASKPRI(cpuid)

extern struct rtai_switch_data {
	volatile unsigned long sflags;
	volatile unsigned long lflags;
} rtai_linux_context[RTAI_NR_CPUS];

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

asmlinkage int rt_printk(const char *format, ...);
asmlinkage int rt_printk_sync(const char *format, ...);

extern struct hal_domain_struct rtai_domain;
extern struct hal_domain_struct *fusion_domain;

#define _rt_switch_to_real_time(cpuid) \
do { \
	rtai_linux_context[cpuid].lflags = xchg(ipipe_root_status[cpuid], (1 << IPIPE_STALL_FLAG)); \
	rtai_linux_context[cpuid].sflags = 1; \
	hal_current_domain(cpuid) = &rtai_domain; \
} while (0)

#define rt_switch_to_linux(cpuid) \
do { \
	if (rtai_linux_context[cpuid].sflags) { \
		hal_current_domain(cpuid) = hal_root_domain; \
		*ipipe_root_status[cpuid] = rtai_linux_context[cpuid].lflags; \
		rtai_linux_context[cpuid].sflags = 0; \
		CLR_TASKPRI(cpuid); \
	} \
} while (0)

#define rt_switch_to_real_time(cpuid) \
do { \
	if (!rtai_linux_context[cpuid].sflags) { \
		_rt_switch_to_real_time(cpuid); \
	} \
} while (0)

#define rtai_get_intr_handler(v) \
	((idt_table[v].b & 0xFFFF0000) | (idt_table[v].a & 0x0000FFFF))
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
	SET_TASKPRI(cpuid);
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

    /* Private interface -- Internal use only */

unsigned long rtai_critical_enter(void (*synch)(void));

void rtai_critical_exit(unsigned long flags);

int rtai_calibrate_8254(void);

void rtai_set_linux_task_priority(struct task_struct *task,
				  int policy,
				  int prio);

long rtai_catch_event (struct hal_domain_struct *ipd, unsigned long event, int (*handler)(unsigned long, void *));

#endif /* __KERNEL__ && !__cplusplus */

    /* Public interface */

#ifdef __KERNEL__

#include <linux/kernel.h>

#define rtai_print_to_screen  		rt_printk

void *ll2a(long long ll, char *s);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int rt_request_irq(unsigned irq,
		   int (*handler)(unsigned irq, void *cookie),
		   void *cookie,
		   int retmode);

int rt_release_irq(unsigned irq);

int rt_set_irq_ack(unsigned int irq, int (*irq_ack)(unsigned int));

static inline int rt_request_irq_wack(unsigned irq, int (*handler)(unsigned irq, void *cookie), void *cookie, int retmode, int (*irq_ack)(unsigned int))
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
 **/
unsigned rt_startup_irq(unsigned irq);

void rt_shutdown_irq(unsigned irq);

void rt_enable_irq(unsigned irq);

void rt_disable_irq(unsigned irq);

void rt_mask_and_ack_irq(unsigned irq);

void rt_unmask_irq(unsigned irq);

void rt_ack_irq(unsigned irq);

void rt_do_irq(unsigned irq);

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

int rt_assign_irq_to_cpu(int irq,
			 unsigned long cpus_mask);

int rt_reset_irq_to_sym_mode(int irq);

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

/* include rtai_timer.h late (it might need something from above)
 * it has to provide:
 * - rtai_timer_irq_ack()
 * - rtai_rdtsc()
 * - void rt_set_timer_delay(int delay)
 */
#include <asm-arm/arch/rtai_timer.h>

#endif /* __KERNEL__ */

#include <asm/rtai_oldnames.h>

#define RTAI_DEFAULT_TICK    100000
#ifdef CONFIG_RTAI_TRACE
#define RTAI_DEFAULT_STACKSZ 8192
#else /* !CONFIG_RTAI_TRACE */
#define RTAI_DEFAULT_STACKSZ 1024
#endif /* CONFIG_RTAI_TRACE */

/*@}*/

#ifndef _RTAI_HAL_XN_H
#define _RTAI_HAL_XN_H

#define SET_FUSION_TIMER_RUNNING()

#define CLEAR_FUSION_TIMER_RUNNING()

#define IS_FUSION_TIMER_RUNNING()  (0)

#define NON_RTAI_SCHEDULE(cpuid)  do { schedule(); } while (0)

#endif /* _RTAI_HAL_XN_H */

#endif /* !_RTAI_ASM_ARM_HAL_H */
