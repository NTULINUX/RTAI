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
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <rtai_sem.h>
#include <rtai_msg.h>

MODULE_DESCRIPTION("Measures task switching times");
MODULE_AUTHOR("Paolo Mantegazza <mantegazza@aero.polimi.it>");
MODULE_LICENSE("GPL");

//#define DISTRIBUTE
#define SEM_TYPE (CNT_SEM | FIFO_Q)
#define NTASKS 10

static int ntasks = NTASKS;
static int loops  = 2000;

static RT_TASK *thread[NTASKS], *task;

static SEM sem;

static volatile int change;

static void pend_task(long t)
{
	unsigned long msg, tmsk;
	tmsk = 1 << t%2;
	t = t/2;
	thread[t] = rt_thread_init(0xcacca + t, 0, 1, SCHED_FIFO, tmsk);
	while(change < 3) {
		switch (change) {
			case 0:
				rt_task_suspend(thread[t]);
				break;
			case 1:
				rt_sem_wait(&sem);
				break;
			case 2:
				rt_return(rt_receive(NULL, &msg), 0);
				break;
		}
	}
	rt_make_soft_real_time(thread[t]);
	thread[t] = NULL;
	return;
}

static void sched_task(long id) 
{
	int i, k;
	unsigned long msg;
	RTIME t;
	do {
		for (i = k = 0; i < ntasks; i++) {
			if (thread[i]) k++;
		}
		msleep(10);
	} while (k != ntasks);

	task = rt_thread_init(0xcacca + id, 1, 1, SCHED_FIFO, rtai_cpuid() ? 2 : 1);

	t = rtai_rdtsc();
	for (i = 0; i < loops; i++) {
		for (k = 0; k < ntasks; k++) {
			rt_task_resume(thread[k]);
		}
	}
	t = rtai_rdtsc() - t;
	
	rt_printk("\n\nFOR %d TASKS: ", ntasks);
	rt_printk("TIME %d (ms), SUSP/RES SWITCHES %d, ", (int)rtai_llimd(t, 1000, RTAI_CLOCK_FREQ), 2*ntasks*loops);
	rt_printk("SWITCH TIME %d (ns)\n", (int)rtai_llimd(rtai_llimd(t, 1000000000, RTAI_CLOCK_FREQ), 1, 2*ntasks*loops));

	change = 1;
	for (k = 0; k < ntasks; k++) {
		rt_task_resume(thread[k]);
	}
	t = rtai_rdtsc();
	for (i = 0; i < loops; i++) {
		for (k = 0; k < ntasks; k++) {
			rt_sem_signal(&sem);
		}
	}
	t = rtai_rdtsc() - t;

	rt_printk("\nFOR %d TASKS: ", ntasks);
	rt_printk("TIME %d (ms), SEM SIG/WAIT SWITCHES %d, ", (int)rtai_llimd(t, 1000, RTAI_CLOCK_FREQ), 2*ntasks*loops);
	rt_printk("SWITCH TIME %d (ns)\n", (int)rtai_llimd(rtai_llimd(t, 1000000000, RTAI_CLOCK_FREQ), 1, 2*ntasks*loops));

	change = 2;
	for (k = 0; k < ntasks; k++) {
		rt_sem_signal(&sem);
	}
	t = rtai_rdtsc();
	for (i = 0; i < loops; i++) {
		for (k = 0; k < ntasks; k++) {
			rt_rpc(thread[k], 0, &msg);
		}
	}
	t = rtai_rdtsc() - t;

	rt_printk("\nFOR %d TASKS: ", ntasks);
	rt_printk("TIME %d (ms), RPC/RCV-RET SWITCHES %d, ", (int)rtai_llimd(t, 1000, RTAI_CLOCK_FREQ), 2*ntasks*loops);
	rt_printk("SWITCH TIME %d (ns)\n\n", (int)rtai_llimd(rtai_llimd(t, 1000000000, RTAI_CLOCK_FREQ), 1, 2*ntasks*loops));

	change = 3;
	for (k = 0; k < ntasks; k++) {
		rt_rpc(thread[k], 0, &msg);
	}
	rt_make_soft_real_time(task);
	task = NULL;
	return;
}

static int __switches_init(void)
{
	int i;

	rt_typed_sem_init(&sem, 1, SEM_TYPE);
	printk("\nWait for it ...\n");
	for (i = 0; i < ntasks; i++) thread[i] = NULL; 

	for (i = 0; i < ntasks; i++) {
#ifdef DISTRIBUTE
		rt_thread_create(pend_task, (void *)(long)(2*i + i%2), 0);
#else
		rt_thread_create(pend_task, (void *)(long)(2*i + rtai_cpuid()), 0);
#endif
	}
	rt_thread_create(sched_task, (void *)(long)(ntasks + 1), 0);

	return 0;
}


static void __switches_exit(void)
{
	int i, k;
	do {
		for (i = k = 0; i < ntasks; i++) {
			if (!thread[i]) k++;
		}
		msleep(10);
	} while (k != ntasks && task);

	rt_sem_delete(&sem);
}

module_init(__switches_init);
module_exit(__switches_exit);
