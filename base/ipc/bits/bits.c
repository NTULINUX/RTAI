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
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include <rtai_schedcore.h>
#include <rtai_sched.h>
#include <rtai_lxrt.h>
#include <rtai_bits.h>

MODULE_LICENSE("GPL");

#define MASK0(x) ((unsigned long *)&(x))[0]
#define MASK1(x) ((unsigned long *)&(x))[1]

static int all_set(BITS *bits, unsigned long mask)
{
	return (bits->mask & mask) == mask;
}

static int any_set(BITS *bits, unsigned long mask)
{
	return (bits->mask & mask);
}

static int all_clr(BITS *bits, unsigned long mask)
{
	return (~bits->mask & mask) == mask;
}

static int any_clr(BITS *bits, unsigned long mask)
{
	return (~bits->mask & mask);
}

static int all_set_and_any_set(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK1(masks)) && (bits->mask & MASK0(masks)) == MASK0(masks);
}

static int all_set_and_all_clr(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK0(masks)) == MASK0(masks) && (~bits->mask & MASK1(masks)) == MASK1(masks);
}

static int all_set_and_any_clr(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK0(masks)) == MASK0(masks) && (~bits->mask & MASK1(masks));
}

static int any_set_and_all_clr(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK0(masks)) && (~bits->mask & MASK1(masks)) == MASK1(masks);
}

static int any_set_and_any_clr(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK0(masks)) && (~bits->mask & MASK1(masks));
}

static int all_clr_and_any_clr(BITS *bits, unsigned long masks)
{
	return (~bits->mask & MASK1(masks)) && (~bits->mask & MASK0(masks)) == MASK0(masks);
}

static int all_set_or_any_set(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK1(masks)) || (bits->mask & MASK0(masks)) == MASK0(masks);
}

static int all_set_or_all_clr(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK0(masks)) == MASK0(masks) || (~bits->mask & MASK1(masks)) == MASK1(masks);
}

static int all_set_or_any_clr(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK0(masks)) == MASK0(masks) || (~bits->mask & MASK1(masks));
}

static int any_set_or_all_clr(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK0(masks)) || (~bits->mask & MASK1(masks)) == MASK1(masks);
}

static int any_set_or_any_clr(BITS *bits, unsigned long masks)
{
	return (bits->mask & MASK0(masks)) || (~bits->mask & MASK1(masks));
}

static int all_clr_or_any_clr(BITS *bits, unsigned long masks)
{
	return (~bits->mask & MASK1(masks)) || (~bits->mask & MASK0(masks)) == MASK0(masks);
}

static void set_bits_mask(BITS *bits, unsigned long mask)
{
	bits->mask |= mask;
}

static void clr_bits_mask(BITS *bits, unsigned long mask)
{
	bits->mask &= ~mask;
}

static void set_clr_bits_mask(BITS *bits, unsigned long masks)
{
	bits->mask =  (bits->mask | MASK0(masks)) & ~MASK1(masks);
}

static void nop_fun(BITS *bits, unsigned long mask)
{
}

static int (*test_fun[])(BITS *, unsigned long) = {
	all_set, any_set,             all_clr,             any_clr, 
	         all_set_and_any_set, all_set_and_all_clr, all_set_and_any_clr,
	                              any_set_and_all_clr, any_set_and_any_clr,
	                                                   all_clr_and_any_clr,
	         all_set_or_any_set,  all_set_or_all_clr,  all_set_or_any_clr,
	                              any_set_or_all_clr,  any_set_or_any_clr,
	                                                   all_clr_or_any_clr
};

static void (*exec_fun[])(BITS *, unsigned long) = {
	set_bits_mask, clr_bits_mask,
	          set_clr_bits_mask,
	nop_fun
};

#define CHECK_BITS_MAGIC(bits) \
	do { if (bits->magic != RT_BITS_MAGIC) return RTE_OBJINV; } while (0)

void rt_bits_init(BITS *bits, unsigned long mask)
{
	bits->magic      = RT_BITS_MAGIC;
	bits->queue.prev = &(bits->queue);
	bits->queue.next = &(bits->queue);
	bits->queue.task = 0;
	bits->mask       = mask;
}

int rt_bits_delete(BITS *bits)
{
	unsigned long flags, schedmap;
	RT_TASK *task;
	QUEUE *q;

	CHECK_BITS_MAGIC(bits);

	schedmap = 0;
	q = &bits->queue;
	flags = rt_global_save_flags_and_cli();
	bits->magic = 0;
	while ((q = q->next) != &bits->queue && (task = q->task)) {
		rem_timed_task(task);
		if (task->state != RT_SCHED_READY && (task->state &= ~(RT_SCHED_SEMAPHORE | RT_SCHED_DELAYED)) == RT_SCHED_READY) {
			task->blocked_on = RTP_OBJREM;
			enq_ready_task(task);	
#ifdef CONFIG_SMP
			set_bit(task->runnable_on_cpus & 0x1F, &schedmap);
#endif
		}
	}
	RT_SCHEDULE_MAP(schedmap);
	rt_global_restore_flags(flags);
	return 0;
}

#define TEST_BUF(x, y)  do { (x)->retval = (unsigned long)(y); } while (0)
#define TEST_FUN(x)     ((long *)((unsigned long)(x)->retval))[0]
#define TEST_MASK(x)    ((unsigned long *)((unsigned long)(x)->retval))[1]

RTAI_SYSCALL_MODE unsigned long rt_get_bits(BITS *bits)
{
	return bits->mask;
}

RTAI_SYSCALL_MODE unsigned long rt_bits_reset(BITS *bits, unsigned long mask)
{
	unsigned long flags, schedmap, oldmask;
	RT_TASK *task;
	QUEUE *q;

	CHECK_BITS_MAGIC(bits);

	schedmap = 0;
	q = &bits->queue;
	flags = rt_global_save_flags_and_cli();
	oldmask = bits->mask;
	bits->mask = mask;
	while ((q = q->next) != &bits->queue) {
		dequeue_blocked(task = q->task);
		rem_timed_task(task);
		if (task->state != RT_SCHED_READY && (task->state &= ~(RT_SCHED_SEMAPHORE | RT_SCHED_DELAYED)) == RT_SCHED_READY) {
			enq_ready_task(task);
#ifdef CONFIG_SMP
			set_bit(task->runnable_on_cpus & 0x1F, &schedmap);
#endif
		}
	}
	bits->queue.prev = bits->queue.next = &bits->queue;
	RT_SCHEDULE_MAP(schedmap);
	rt_global_restore_flags(flags);
	return oldmask;
}

RTAI_SYSCALL_MODE unsigned long rt_bits_signal(BITS *bits, int setfun, unsigned long masks)
{
	unsigned long flags, schedmap;
	RT_TASK *task;
	QUEUE *q;

	CHECK_BITS_MAGIC(bits);

	schedmap = 0;
	q = &bits->queue;
	flags = rt_global_save_flags_and_cli();
	exec_fun[setfun](bits, masks);
	masks = bits->mask;
	while ((q = q->next) != &bits->queue) {
		task = q->task;
		if (test_fun[TEST_FUN(task)](bits, TEST_MASK(task))) {
			dequeue_blocked(task);
			rem_timed_task(task);
			if (task->state != RT_SCHED_READY && (task->state &= ~(RT_SCHED_SEMAPHORE | RT_SCHED_DELAYED)) == RT_SCHED_READY) {
				enq_ready_task(task);
#ifdef CONFIG_SMP
				set_bit(task->runnable_on_cpus & 0x1F, &schedmap);
#endif
			}
		}
	}
	RT_SCHEDULE_MAP(schedmap);
	rt_global_restore_flags(flags);
	return masks;
}

RTAI_SYSCALL_MODE int _rt_bits_wait(BITS *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, unsigned long *resulting_mask, int space)
{
	RT_TASK *rt_current;
	unsigned long flags, mask = 0;
	int retval;

	CHECK_BITS_MAGIC(bits);

	flags = rt_global_save_flags_and_cli();
	if (!test_fun[testfun](bits, testmasks)) {
		void *retpnt;
		long bits_test[2];	
		rt_current = RT_CURRENT;
		TEST_BUF(rt_current, bits_test);
		TEST_FUN(rt_current)  = testfun;
		TEST_MASK(rt_current) = testmasks;
		rt_current->state |= RT_SCHED_SEMAPHORE;
		rem_ready_current(rt_current);
		enqueue_blocked(rt_current, &bits->queue, 1);
		rt_schedule();
		if (unlikely((retpnt = rt_current->blocked_on) != NULL)) {
			if (likely(retpnt != RTP_OBJREM)) {
				dequeue_blocked(rt_current);
				retval = RTE_UNBLKD;
			} else {
				rt_current->prio_passed_to = NULL;
				retval = RTE_OBJREM;
			}
			goto retmask;
		}
	}
	retval = 0;
	mask = bits->mask;
	exec_fun[exitfun](bits, exitmasks);
retmask:
	rt_global_restore_flags(flags);
	if (resulting_mask) {
		if (space) {
			*resulting_mask = mask;
		} else {
			rt_copy_to_user(resulting_mask, &mask, sizeof(mask));
		}
	}
	return retval;
}

RTAI_SYSCALL_MODE int _rt_bits_wait_if(BITS *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, unsigned long *resulting_mask, int space)
{
	unsigned long flags, mask;
	int retval;

	CHECK_BITS_MAGIC(bits);

	flags = rt_global_save_flags_and_cli();
	mask = bits->mask;
	if (test_fun[testfun](bits, testmasks)) {
		exec_fun[exitfun](bits, exitmasks);
		retval = 1;
	} else {
		retval = 0;
	}
	rt_global_restore_flags(flags);
	if (resulting_mask) {
		if (space) {
			*resulting_mask = mask;
		} else {
			rt_copy_to_user(resulting_mask, &mask, sizeof(mask));
		}
	}
	return retval;
}

RTAI_SYSCALL_MODE int _rt_bits_wait_until(BITS *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, RTIME time, unsigned long *resulting_mask, int space)
{
	RT_TASK *rt_current;
	unsigned long flags, mask = 0;
	int retval;

	CHECK_BITS_MAGIC(bits);

	flags = rt_global_save_flags_and_cli();
	if (!test_fun[testfun](bits, testmasks)) {
		void *retpnt;
		long bits_test[2];
		rt_current = RT_CURRENT;
		TEST_BUF(rt_current, bits_test);
		TEST_FUN(rt_current)  = testfun;
		TEST_MASK(rt_current) = testmasks;
		rt_current->blocked_on = &bits->queue;
		if ((rt_current->resume_time = time) > get_time()) {
			rt_current->state |= (RT_SCHED_SEMAPHORE | RT_SCHED_DELAYED);
			rem_ready_current(rt_current);
			enqueue_blocked(rt_current, &bits->queue, 1);
			enq_timed_task(rt_current);
			rt_schedule();
		} else {
			rt_current->queue.prev = rt_current->queue.next = &rt_current->queue;
		}
		if (unlikely((retpnt = rt_current->blocked_on) != NULL)) {
			if (likely(retpnt != RTP_OBJREM)) {
				dequeue_blocked(rt_current);
				retval = likely(retpnt > RTP_HIGERR) ? RTE_TIMOUT : RTE_UNBLKD;
			} else {
				rt_current->prio_passed_to = NULL;
				retval = RTE_OBJREM;
			}
			goto retmask;
		}
	}
	retval = 0;
	mask = bits->mask;
	exec_fun[exitfun](bits, exitmasks);
retmask:
	rt_global_restore_flags(flags);
	if (resulting_mask) {
		if (space) {
			*resulting_mask = mask;
		} else {
			rt_copy_to_user(resulting_mask, &mask, sizeof(mask));
		}
	}
	return retval;
}

RTAI_SYSCALL_MODE int _rt_bits_wait_timed(BITS *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, RTIME delay, unsigned long *resulting_mask, int space)
{
	return _rt_bits_wait_until(bits, testfun, testmasks, exitfun, exitmasks, get_time() + delay, resulting_mask, space);
}

/* +++++++++++++++++++++++++++++ NAMED BITS +++++++++++++++++++++++++++++++++ */

#include <rtai_registry.h>

RTAI_SYSCALL_MODE BITS *rt_named_bits_init(const char *bits_name, unsigned long mask)
{
	BITS *bits;
	unsigned long name;

	if ((bits = rt_get_adr(name = nam2num(bits_name)))) {
		return bits;
	}
	if ((bits = rt_malloc(sizeof(SEM)))) {
		rt_bits_init(bits, mask);
		if (rt_register(name, bits, IS_BIT, 0)) {
			return bits;
		}
		rt_bits_delete(bits);
	}
	rt_free(bits);
	return NULL;
}

RTAI_SYSCALL_MODE int rt_named_bits_delete(BITS *bits)
{
	if (!rt_bits_delete(bits)) {
		rt_free(bits);
	}
	return rt_drg_on_adr(bits);
}

RTAI_SYSCALL_MODE void *rt_bits_init_u(unsigned long name, unsigned long mask)
{
	BITS *bits;
	if (rt_get_adr(name)) {
		return NULL;
	}
	if ((bits = rt_malloc(sizeof(BITS)))) {
		rt_bits_init(bits, mask);
		if (rt_register(name, bits, IS_BIT, current)) {
			return bits;
		} else {
			rt_free(bits);
		}
	}
	return NULL;
}

RTAI_SYSCALL_MODE int rt_bits_delete_u(BITS *bits)
{
	if (rt_bits_delete(bits)) {
		return -EFAULT;
	}
	rt_free(bits);
	return rt_drg_on_adr(bits);
}

/* ++++++++++++++++++++++++++++ BITS ENTRIES ++++++++++++++++++++++++++++++++ */

struct rt_native_fun_entry rt_bits_entries[] = {
	{ { 0, rt_bits_init_u },          	BITS_INIT },
	{ { 0, rt_bits_delete_u },        	BITS_DELETE },
	{ { 0, rt_named_bits_init },    	NAMED_BITS_INIT },
	{ { 0, rt_named_bits_delete },  	NAMED_BITS_DELETE },
	{ { 1, rt_get_bits },           	BITS_GET },
	{ { 1, rt_bits_reset },         	BITS_RESET },
	{ { 1, rt_bits_signal },        	BITS_SIGNAL },
	{ { 1, _rt_bits_wait },          	BITS_WAIT },
	{ { 1, _rt_bits_wait_if },       	BITS_WAIT_IF },
	{ { 1, _rt_bits_wait_until },    	BITS_WAIT_UNTIL },
	{ { 1, _rt_bits_wait_timed },		BITS_WAIT_TIMED },
	{ { 0, 0 },                            	000 }
};

extern int set_rt_fun_entries(struct rt_native_fun_entry *entry);
extern void reset_rt_fun_entries(struct rt_native_fun_entry *entry);

int __rtai_bits_init(void)
{
	return set_rt_fun_entries(rt_bits_entries);
}

void __rtai_bits_exit(void)
{
	reset_rt_fun_entries(rt_bits_entries);
}

#ifndef CONFIG_RTAI_BITS_BUILTIN
module_init(__rtai_bits_init);
module_exit(__rtai_bits_exit);
#endif /* !CONFIG_RTAI_BITS_BUILTIN */

#ifdef CONFIG_KBUILD
EXPORT_SYMBOL(rt_bits_init);
EXPORT_SYMBOL(rt_bits_delete);
EXPORT_SYMBOL(rt_get_bits);
EXPORT_SYMBOL(rt_bits_reset);
EXPORT_SYMBOL(rt_bits_signal);
EXPORT_SYMBOL(_rt_bits_wait);
EXPORT_SYMBOL(_rt_bits_wait_if);
EXPORT_SYMBOL(_rt_bits_wait_until);
EXPORT_SYMBOL(_rt_bits_wait_timed);
EXPORT_SYMBOL(rt_named_bits_init);
EXPORT_SYMBOL(rt_named_bits_delete);
EXPORT_SYMBOL(rt_bits_init_u);
EXPORT_SYMBOL(rt_bits_delete_u);
#endif /* CONFIG_KBUILD */
