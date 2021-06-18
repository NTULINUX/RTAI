/*
 * Copyright (C) 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _RTAI_USI_H
#define _RTAI_USI_H

#ifndef __KERNEL__

#include <asm/rtai_usi.h>
#include <asm/rtai_srq.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * It does not serve anymore, see IF_IS_A_USI_SRQ_CALL_IT in "asm/rtai_hal.h" 
 * and its use in rtai_hal.
 */
#if 0 
static inline int rt_startup_irq(unsigned int irq)
{
        return (int)rtai_srq(USI_SRQ_MASK | _STARTUP_IRQ, irq);
}
#endif
 
#define rt_shutdown_irq(irq) \
	do { rtai_srq(USI_SRQ_MASK | _SHUTDOWN_IRQ, irq); } while (0)
 
#define rt_enable_irq(irq) \
	do { rtai_srq(USI_SRQ_MASK | _ENABLE_IRQ, irq); } while (0)
 
#define rt_disable_irq(irq) \
	do { rtai_srq(USI_SRQ_MASK | _DISABLE_IRQ, irq); } while (0)
 
#define rt_mask_and_ack_irq(irq) \
	do { rtai_srq(USI_SRQ_MASK | _MASK_AND_ACK_IRQ, irq); } while (0)
 
#define rt_ack_irq(irq) \
	do { rtai_srq(USI_SRQ_MASK | _ACK_IRQ, irq); } while (0)
 
#define rt_unmask_irq(irq) \
	do { rtai_srq(USI_SRQ_MASK | _UNMASK_IRQ, irq); } while (0)
 
#define rtai_cli() \
	do { rtai_srq(USI_SRQ_MASK | _DISINT, 0); } while (0)

#define rtai_sti() \
	do { rtai_srq(USI_SRQ_MASK | _ENINT, 0); } while (0)

#define rtai_save_flags_and_cli(flags) \
	do { flags = (unsigned long)rtai_srq(USI_SRQ_MASK | _SAVE_FLAGS_CLI, 0); } while(0)
 
#define rtai_restore_flags(flags) \
	do { rtai_srq(USI_SRQ_MASK | _RESTORE_FLAGS, flags); } while (0)

#ifdef CONFIG_SMP

struct __usi_xchg_dummy { unsigned long a[100]; };
#define __usi_xg(x) ((struct __usi_xchg_dummy *)(x))

static inline unsigned long usi_atomic_cmpxchg(volatile void *ptr, unsigned long o, unsigned long n)
{
	unsigned long prev;
	__asm__ __volatile__	("lock; cmpxchgl %1, %2"
				: "=a" (prev)
				: "q"(n), "m" (*__usi_xg(ptr)), "0" (o)
				: "memory");
	return prev;
}

#define rt_spin_lock(lock) \
	do { while (usi_atomic_cmpxchg(lock, 0, 1)); } while (0)
 
#define rt_spin_unlock(lock) \
	do { *(volatile int *)lock = 0; } while (0)

#else
#define rt_spin_lock(lock);

#define rt_spin_unlock(lock);
#endif
 
#define rt_spin_lock_init(lock) \
	do { *(volatile int *)lock = 0; } while (0)

#define rt_spin_lock_irq(lock) \
	do { rtai_cli(); rt_spin_lock(lock); } while (0)
 
#define rt_spin_unlock_irq(lock) \
	do { rt_spin_unlock(lock); rtai_sti(); } while (0)
 
static inline unsigned long rt_spin_lock_irqsave(void *lock)
{
	unsigned long flags;
	rtai_save_flags_and_cli(flags);
	rt_spin_lock(lock);
	return flags;
}
 
#define rt_spin_unlock_irqrestore(flags, lock) \
	do { rt_spin_unlock(lock); rtai_restore_flags(flags); } while (0)
 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__KERNEL__ */

#endif /* !_RTAI_USI_H */
