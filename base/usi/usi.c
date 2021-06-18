/*
 * Copyright (C) 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#if 0

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_malloc.h>
#include <rtai_lxrt.h>
#include <rtai_tasklets.h>
#include <rtai_usi.h>
#include <rtai_sem.h>

#define MODULE_NAME "RTAI_USI"

static void *rt_spin_lock_init(void)
{
	spinlock_t *lock;
 
	if ((lock = rt_malloc(sizeof(spinlock_t)))) {
		spin_lock_init(lock);
		return lock;
	}
	return 0;
}

static inline int rt_spin_lock_delete(void *lock)
{
	rt_free(lock);
	return 0;
}
                                                                               
static void usi_spin_lock(spinlock_t *lock)          
{
	rt_spin_lock(lock);
}

static void usi_spin_unlock(spinlock_t *lock)          
{
	rt_spin_unlock(lock);
}

static void usi_spin_lock_irq(spinlock_t *lock)          
{
	rt_spin_lock_irq(lock);
}

static void usi_spin_unlock_irq(spinlock_t *lock)
{
	rt_spin_unlock_irq(lock);
}

static unsigned long usi_spin_lock_irqsave(spinlock_t *lock)          
{
	return rt_spin_lock_irqsave(lock);
}

static void usi_spin_unlock_irqrestore(unsigned long flags, spinlock_t *lock)
{
	rt_spin_unlock_irqrestore(flags, lock);
}

static void usi_global_cli(void)
{
	rt_global_cli();
}

static void usi_global_sti(void)
{
	rt_global_sti();
}

static unsigned long usi_global_save_flags_and_cli(void)
{
	return rt_global_save_flags_and_cli();
}

static unsigned long usi_global_save_flags(void)
{
	unsigned long flags;
	rt_global_save_flags(&flags);
	return flags;
}

static void usi_global_restore_flags(unsigned long flags)
{
	rt_global_restore_flags(flags);
}

static void usi_cli(void)
{
	rtai_cli();
}

static void usi_sti(void)
{
	rtai_sti();
}

static unsigned long usi_save_flags_and_cli(void)
{
	unsigned long flags;
	rtai_save_flags_and_cli(flags);
	return flags;
}

static unsigned long usi_save_flags(void)
{
	unsigned long flags;
	rtai_save_flags(flags);
	return flags;
}

static void usi_restore_flags(unsigned long flags)
{
	rtai_restore_flags(flags);
}

static struct rt_fun_entry rtai_usi_fun[] = {
	[_STARTUP_IRQ]		= { 0, rt_startup_irq },
	[_SHUTDOWN_IRQ]		= { 0, rt_shutdown_irq },
	[_ENABLE_IRQ]		= { 0, rt_enable_irq },
	[_DISABLE_IRQ]		= { 0, rt_disable_irq },
	[_MASK_AND_ACK_IRQ]	= { 0, rt_mask_and_ack_irq },
	[_ACK_IRQ]		= { 0, rt_ack_irq },
	[_UNMASK_IRQ ]		= { 0, rt_unmask_irq },
	[_INIT_SPIN_LOCK]	= { 0, rt_spin_lock_init },
	[_SPIN_LOCK]		= { 0, usi_spin_lock },
	[_SPIN_UNLOCK]		= { 0, usi_spin_unlock },
	[_SPIN_LOCK_IRQ]	= { 0, usi_spin_lock_irq },
	[_SPIN_UNLOCK_IRQ]	= { 0, usi_spin_unlock_irq },
	[_SPIN_LOCK_IRQSV]	= { 0, usi_spin_lock_irqsave },
	[_SPIN_UNLOCK_IRQRST]	= { 0, usi_spin_unlock_irqrestore },
	[_GLB_CLI]		= { 0, usi_global_cli },
	[_GLB_STI]		= { 0, usi_global_sti},
	[_GLB_SVFLAGS_CLI]	= { 0, usi_global_save_flags_and_cli },
	[_GLB_SVFLAGS]		= { 0, usi_global_save_flags },
	[_GLB_RSTFLAGS]		= { 0, usi_global_restore_flags },
	[_CLI]			= { 0, usi_cli },
	[_STI]			= { 0, usi_sti},
	[_SVFLAGS_CLI]		= { 0, usi_save_flags_and_cli },
	[_SVFLAGS]		= { 0, usi_save_flags },
	[_RSTFLAGS]		= { 0, usi_restore_flags }
};

static int register_lxrt_usi_support(void)
{
	if(set_rt_fun_ext_index(rtai_usi_fun, FUN_USI_LXRT_INDX)) {
		printk("LXRT EXTENSION SLOT FOR USI (%d) ALREADY USED\n", FUN_USI_LXRT_INDX);
		return -EACCES;
	}
	return 0;
}

static void unregister_lxrt_usi_support(void)
{
	reset_rt_fun_ext_index(rtai_usi_fun, FUN_USI_LXRT_INDX);
}

int __rtai_usi_init(void)
{
	if (!register_lxrt_usi_support()) {
		printk(KERN_INFO "RTAI[usi]: loaded.\n");
		return 0;
	}
	return -EACCES;
}

void __rtai_usi_exit(void)
{
	unregister_lxrt_usi_support();
	printk(KERN_INFO "RTAI[usi]: unloaded.\n");
}

#endif

int __rtai_usi_init(void)
{
	printk(KERN_INFO "RTAI[usi]: loaded.\n");
	return 0;
}

void __rtai_usi_exit(void)
{
	printk(KERN_INFO "RTAI[usi]: unloaded.\n");
}

#ifndef CONFIG_RTAI_USI_BUILTIN
module_init(__rtai_usi_init);
module_exit(__rtai_usi_exit);
#endif /* !CONFIG_RTAI_USI_BUILTIN */
