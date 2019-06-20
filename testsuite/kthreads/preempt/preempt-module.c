/*
 * Copyright (C) 2015 Paolo Mantegazza <mantegazza@aero.polimi.it>
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
#include <linux/delay.h>
#include <linux/init.h>
#include <asm/io.h>

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_nam2num.h>
#include <rtai_fifos.h>

MODULE_LICENSE("GPL");

#define FIFO     0
#define NAVRG    1000
#define FASTMUL  4
#define SLOWMUL  24
#define PERIOD   100000

static RT_TASK *thread;
static RT_TASK *Slow_Task;
static RT_TASK *Fast_Task;

static int period, slowjit, fastjit;
static RTIME expected;

static int endme;

static void Slow_Thread(long dummy)
{
	int jit;
	RTIME svt, t;

	Slow_Task = rt_thread_init(nam2num("SLWTHR"), 2, 1, SCHED_FIFO, 0x1);
	rt_task_make_periodic(Slow_Task, expected, SLOWMUL*period);

	svt = rt_get_cpu_time_ns() - SLOWMUL*PERIOD;
        while (!endme) {  
		jit = (int) ((t = rt_get_cpu_time_ns()) - svt - SLOWMUL*PERIOD);
		svt = t;
		if (jit) { jit = - jit; }
		if (jit > slowjit) { slowjit = jit; }
                rt_busy_sleep(SLOWMUL/2*PERIOD);
                rt_task_wait_period();                                        
        }
	rtai_cli();
	endme--;
	rtai_sti();
}                                        

static void Fast_Thread(long dummy) 
{                             
	int jit;
	RTIME svt, t;

	Fast_Task = rt_thread_init(nam2num("FSTTHR"), 1, 1, SCHED_FIFO, 0x1);
	rt_task_make_periodic(Fast_Task, expected, FASTMUL*period);

	svt = rt_get_cpu_time_ns() - FASTMUL*PERIOD;
        while (!endme) {
		jit = (int) ((t = rt_get_cpu_time_ns()) - svt - FASTMUL*PERIOD);
		svt = t;
		if (jit) { jit = - jit; }
		if (jit > fastjit) { fastjit = jit; }
                rt_busy_sleep(FASTMUL/2*PERIOD);
                rt_task_wait_period();                                        
        }                      
	rtai_cli();
	endme--;
	rtai_sti();
}

static void fun(long dumy)
{
	struct sample { long min, max, avrg, jitters[2]; } samp;
	int diff;
	int skip;
	int average;
	int min_diff;
	int max_diff;
	RTIME svt, t;

	thread = rt_thread_init(nam2num("CALIBR"), 0, 1, SCHED_FIFO, 0x2);
	rt_task_make_periodic(thread, expected, period);

	t = 0;
	min_diff = 1000000000;
	max_diff = -1000000000;
	while (!endme) {
		unsigned long flags;
		average = 0;

		svt = rt_get_cpu_time_ns();
		for (skip = 0; skip < NAVRG && !endme; skip++) {
			expected += period;
			rt_task_wait_period();

			rt_global_save_flags(&flags);
			diff = (int) count2nano(rt_get_time() - expected);
			if (diff < min_diff) {
				min_diff = diff;
			}
			if (diff > max_diff) {
				max_diff = diff;
			}
			average += diff;
		}
		samp.min = min_diff;
		samp.max = max_diff;
		samp.avrg = average/NAVRG;
		samp.jitters[0] = fastjit;
		samp.jitters[1] = slowjit;
		rtf_ovrwr_put(FIFO, &samp, sizeof(samp));
	}
	rtai_cli();
	endme--;
	rtai_sti();
}

static int __preempt_init(void)
{
	period = nano2count(PERIOD);
	rtf_create(FIFO, 1000);
	expected = rt_get_time() + nano2count(100000000);
	rt_thread_create(fun, 0, 0);
	rt_thread_create(Fast_Thread, 0, 0);
	rt_thread_create(Slow_Thread, 0, 0);

	return 0;
}

static void __preempt_exit(void)
{
	endme = 3;
	while(endme) msleep(10);
	rtf_destroy(FIFO);
	printk("END OF PREEMPT TEST\n\n");
}

module_init(__preempt_init);
module_exit(__preempt_exit);
