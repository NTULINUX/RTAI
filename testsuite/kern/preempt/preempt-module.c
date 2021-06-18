/*
 * Copyright (C) 1999 Paolo Mantegazza <mantegazza@aero.polimi.it>
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
#include <asm/io.h>

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>

MODULE_LICENSE("GPL");

#define FIFO 0

#define NAVRG 1000

#define USE_FPU 0

#define FASTMUL  4

#define SLOWMUL  24

#if defined(CONFIG_UCLINUX) || defined(CONFIG_ARM) || defined(CONFIG_COLDFIRE)
#define TICK_TIME 1000000
#else
#define TICK_TIME 100000
#endif

static RT_TASK thread;
static RT_TASK Slow_Task;
static RT_TASK Fast_Task;

static int period, slowjit, fastjit;
static RTIME expected;

static int cpu_used[RTAI_NR_CPUS];

static void Slow_Thread(long dummy)
{
	int jit;
	RTIME svt, t;
	svt = rt_get_cpu_time_ns() - SLOWMUL*TICK_TIME;
        while (1) {  
		jit = (int) ((t = rt_get_cpu_time_ns()) - svt - SLOWMUL*TICK_TIME);
		svt = t;
		if (jit) { jit = - jit; }
		if (jit > slowjit) { slowjit = jit; }
		cpu_used[rtai_cpuid()]++;
                rt_busy_sleep(SLOWMUL/2*TICK_TIME);
                rt_task_wait_period();                                        
        }
}                                        

static void Fast_Thread(long dummy) 
{                             
	int jit;
	RTIME svt, t;
	svt = rt_get_cpu_time_ns() - FASTMUL*TICK_TIME;
        while (1) {
		jit = (int) ((t = rt_get_cpu_time_ns()) - svt - FASTMUL*TICK_TIME);
		svt = t;
		if (jit) { jit = - jit; }
		if (jit > fastjit) { fastjit = jit; }
		cpu_used[rtai_cpuid()]++;
                rt_busy_sleep(FASTMUL/2*TICK_TIME);
                rt_task_wait_period();                                        
        }                      
}

static void fun(long thread) {

	struct sample { long min, max, avrg, jitters[2]; } samp;
	int diff;
	int skip;
	int average;
	int min_diff;
	int max_diff;

	min_diff = 1000000000;
	max_diff = -1000000000;
	while (1) {
		unsigned long flags;
		average = 0;

		for (skip = 0; skip < NAVRG; skip++) {
			cpu_used[rtai_cpuid()]++;
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
}

static int __preempt_init(void)
{
	RTIME start;

	rtf_create(FIFO, 1000);
	rt_linux_use_fpu(USE_FPU);
	rt_task_init_cpuid(&thread, fun, 0, 5000, 0, USE_FPU, 0, 0);
	rt_task_init_cpuid(&Fast_Task, Fast_Thread, 0, 5000, 1, 0, 0, 0);
	rt_task_init_cpuid(&Slow_Task, Slow_Thread, 0, 5000, 2, 0, 0, 0);
	period = nano2count(TICK_TIME);
	expected = start = rt_get_time() + 100*period;
	rt_task_make_periodic(&thread, start, period);
	rt_task_make_periodic(&Fast_Task, start, FASTMUL*period);
	rt_task_make_periodic(&Slow_Task, start, SLOWMUL*period);

	return 0;
}

static void __preempt_exit(void)
{
	int cpuid;

	rt_task_delete(&thread);
	rt_task_delete(&Slow_Task);
	rt_task_delete(&Fast_Task);
	rtf_destroy(FIFO);
	printk("\n\nCPU USE SUMMARY\n");
	for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) {
		printk("# %d -> %d\n", cpuid, cpu_used[cpuid]);
	}
	printk("END OF CPU USE SUMMARY\n\n");
}

module_init(__preempt_init);
module_exit(__preempt_exit);
