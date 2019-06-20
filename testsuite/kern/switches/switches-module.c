/*
 * Copyright (C) 2000 Paolo Mantegazza <mantegazza@aero.polimi.it>
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
#include <rtai_sem.h>
#include <rtai_msg.h>

MODULE_DESCRIPTION("Measures task switching times");
MODULE_AUTHOR("Paolo Mantegazza <mantegazza@aero.polimi.it>");
MODULE_LICENSE("GPL");

/*
 * Command line parameters
 */
int ntasks = 10;
RTAI_MODULE_PARM(ntasks, int);
MODULE_PARM_DESC(ntasks, "Number of tasks to switch (default: 30)");

int loops = 2000;
RTAI_MODULE_PARM(loops, int);
MODULE_PARM_DESC(loops, "Number of switches per task (default: 2000)");

#ifdef CONFIG_RTAI_FPU_SUPPORT
int use_fpu = 1;
RTAI_MODULE_PARM(use_fpu, int);
MODULE_PARM_DESC(use_fpu, "Use full FPU support (default: 1)");
#else
int use_fpu = 0;
#endif

int stack_size = 4096;
RTAI_MODULE_PARM(stack_size, int);
MODULE_PARM_DESC(stack_size, "Task stack size in bytes (default: 2000)");

//#define DISTRIBUTE /* Define this to have tasks distributed among CPUs */

#define SEM_TYPE (CNT_SEM | FIFO_Q)

static RT_TASK *thread, task;

static int cpu_used[RTAI_NR_CPUS];

static SEM sem;

static volatile int change;

static void pend_task (long t)
{
	unsigned long msg;
	while(1) {
		switch (change) {
			case 0:
				rt_task_suspend(thread + t);
				break;
			case 1:
				rt_sem_wait(&sem);
				break;
			case 2:
				rt_return(rt_receive(NULL, &msg), 0);
				break;
		}
		cpu_used[rtai_cpuid()]++;
	}
}

static void sched_task(long t) {
	int i, k;
	unsigned long msg;

	change = 0;
	t =rtai_rdtsc();
	for (i = 0; i < loops; i++) {
		for (k = 0; k < ntasks; k++) {
			rt_task_resume(thread + k);
		}
	}
	t = rtai_rdtsc() - t;
	rt_printk("\n\nFOR %d TASKS: ", ntasks);
	rt_printk("TIME %d (ms), SUSP/RES SWITCHES %d, ", (int)rtai_llimd(t, 1000, RTAI_CLOCK_FREQ), 2*ntasks*loops);
	rt_printk("SWITCH TIME%s %d (ns)\n", use_fpu ? " (INCLUDING FULL FP SUPPORT)":"",
	       (int)rtai_llimd(rtai_llimd(t, 1000000000, RTAI_CLOCK_FREQ), 1, 2*ntasks*loops));

	change = 1;
	for (k = 0; k < ntasks; k++) {
		rt_task_resume(thread + k);
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
	rt_printk("SWITCH TIME%s %d (ns)\n", use_fpu ? " (INCLUDING FULL FP SUPPORT)":"",
	       (int)rtai_llimd(rtai_llimd(t, 1000000000, RTAI_CLOCK_FREQ), 1, 2*ntasks*loops));

	change = 2;
	for (k = 0; k < ntasks; k++) {
		rt_sem_signal(&sem);
	}
	t = rtai_rdtsc();
	for (i = 0; i < loops; i++) {
		for (k = 0; k < ntasks; k++) {
			rt_rpc(thread + k, 0, &msg);
		}
	}
	t = rtai_rdtsc() - t;
	rt_printk("\nFOR %d TASKS: ", ntasks);
	rt_printk("TIME %d (ms), RPC/RCV-RET SWITCHES %d, ", (int)rtai_llimd(t, 1000, RTAI_CLOCK_FREQ), 2*ntasks*loops);
	rt_printk("SWITCH TIME%s %d (ns)\n\n", use_fpu ? " (INCLUDING FULL FP SUPPORT)":"",
	       (int)rtai_llimd(rtai_llimd(t, 1000000000, RTAI_CLOCK_FREQ), 1, 2*ntasks*loops));
}

static int __switches_init(void)
{
	int i;
	int e;

	printk("\nWait for it ...\n");
	rt_typed_sem_init(&sem, 1, SEM_TYPE);
	rt_linux_use_fpu(1);
        thread = (RT_TASK *)kmalloc(ntasks*sizeof(RT_TASK), GFP_KERNEL);
	for (i = 0; i < ntasks; i++) {
#ifdef DISTRIBUTE
		e = rt_task_init_cpuid(thread + i, pend_task, i, stack_size, 0, use_fpu, 0,  i%2);
#else
		e = rt_task_init_cpuid(thread + i, pend_task, i, stack_size, 0, use_fpu, 0,  rtai_cpuid());
#endif
		if (e < 0) {
		task_init_has_failed:
		    rt_printk("switches: failed to initialize task %d, error=%d\n", i, e);
		    while (--i >= 0)
			rt_task_delete(thread + i);
		    return -1;
		}
	}
	e = rt_task_init_cpuid(&task, sched_task, i, stack_size, 1, use_fpu, 0, rtai_cpuid());
	if (e < 0)
	    goto task_init_has_failed;
	rt_task_resume(&task);

	return 0;
}


static void __switches_exit(void)
{
	int i;
	for (i = 0; i < ntasks; i++) {
		rt_task_delete(thread + i);
	}
	rt_task_delete(&task);
        kfree(thread);
	printk("\nCPU USE SUMMARY\n");
	for (i = 0; i < RTAI_NR_CPUS; i++) {
		printk("# %d -> %d\n", i, cpu_used[i]);
	}
	printk("END OF CPU USE SUMMARY\n\n");
	rt_sem_delete(&sem);
}

module_init(__switches_init);
module_exit(__switches_exit);
