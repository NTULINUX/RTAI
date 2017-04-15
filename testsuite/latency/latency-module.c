/*
 * Copyright (C) 2000 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *               2002  Robert Schwebel  <robert@schwebel.de>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/stringify.h>
#include <asm/io.h>

#include <asm/rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Latency measurement tool for RTAI");
MODULE_AUTHOR("Paolo Mantegazza <mantegazza@aero.polimi.it>, Robert Schwebel <robert@schwebel.de>");


/*
 *	command line parameters
 */

#if defined(CONFIG_UCLINUX) || defined(CONFIG_ARM) || defined(CONFIG_COLDFIRE)
#define DEFAULT_PERIOD 1000000
#else
#define DEFAULT_PERIOD 100000
#endif
int period = DEFAULT_PERIOD;
RTAI_MODULE_PARM(period, int);
MODULE_PARM_DESC(period, "period in ns (default: " __stringify(DEFAULT_PERIOD) ")");

static int loops;
int avrgtime = 1;
RTAI_MODULE_PARM(avrgtime, int);
MODULE_PARM_DESC(avrgtime, "Averages are calculated for <avrgtime (s)> runs (default: 1)");

int use_fpu = 0;
RTAI_MODULE_PARM(use_fpu, int);
MODULE_PARM_DESC(use_fpu, "do we want to use the FPU? (default: 0)");

int start_timer = 1;
RTAI_MODULE_PARM(start_timer, int);
MODULE_PARM_DESC(start_timer,
		 "declares if the timer should be started or not (default: 1)");

int timer_mode = 0;
RTAI_MODULE_PARM(timer_mode, int);
MODULE_PARM_DESC(timer_mode, "timer running mode: 0-oneshot, 1-periodic");

#define DEBUG_FIFO 3
#define TIMER_TO_CPU 3		// < 0  || > 1 to maintain a symmetric processed timer.
#define RUNNABLE_ON_CPUS 3	// 1: on cpu 0 only, 2: on cpu 1 only, 3: on any;
#define RUN_ON_CPUS (num_online_cpus() > 1 ? RUNNABLE_ON_CPUS : 1)

#define ECHOSPEED 1

/*
 *	Global Variables
 */

RT_TASK thread;
RTIME expected;
int period_counts;
struct sample {
	long long min;
	long long max;
	int index, ovrn;
} samp;
double dotres;

static int cpu_used[NR_RT_CPUS];

#define MAXDIM 10

#ifdef CONFIG_RTAI_FPU_SUPPORT
static double a[MAXDIM], b[MAXDIM];

static double dot(double *a, double *b, int n)
{
	int k; double s;
	for(k = n - 1, s = 0.0; k >= 0; k--) {
		s = s + a[k]*b[k];
	}
	return s;
}
#endif

/* 
 *	/proc/rtai/latency_calibrate entry
 */

#ifdef CONFIG_PROC_FS
static int PROC_READ_FUN(proc_read)
{
	PROC_PRINT_VARS;
	PROC_PRINT("\n## RTAI latency calibration tool ##\n");
	PROC_PRINT("# period = %i (ns) \n", period);
	PROC_PRINT("# avrgtime = %i (s)\n", avrgtime);
	PROC_PRINT("#%suse the FPU\n", use_fpu ? " " : " do not " );
	PROC_PRINT("#%sstart the timer\n", start_timer ? " " : " do not ");
	PROC_PRINT("# timer_mode is %s\n", timer_mode ? "periodic" : "oneshot");
	PROC_PRINT("\n");
	PROC_PRINT_DONE;
}
#endif


/*
 *	Periodic realtime thread 
 */
 
void
fun(long thread)
{

	int diff = 0;
	int i;
	int average;
	int min_diff = 0;
	int max_diff = 0;
	RTIME t, svt;

#ifdef CONFIG_RTAI_FPU_SUPPORT
	if (use_fpu) {
		for(i = 0; i < MAXDIM; i++) {
			a[i] = b[i] = 3.141592;
		}
	}
#endif

	svt = rt_get_cpu_time_ns();
	samp.ovrn = 0;
	while (1) {

		min_diff =  1000000000;
		max_diff = -1000000000;

		average = 0;

		for (i = 0; i < loops; i++) {
			cpu_used[hard_cpu_id()]++;
			expected += period_counts;

			if (!rt_task_wait_period()) {
				if (timer_mode) {
					diff = (int) ((t = rt_get_cpu_time_ns()) - svt - period);
					svt = t;
				} else {
					diff = (int) count2nano(rt_get_time() - expected);
				}
			} else {
				samp.ovrn++;
				diff = 0;
				if (timer_mode) {
					svt = rt_get_cpu_time_ns();
				}
			}

			if (diff < min_diff) { min_diff = diff; }
			if (diff > max_diff) { max_diff = diff; }
			average += diff;
#ifdef CONFIG_RTAI_FPU_SUPPORT
			if (use_fpu) {
				dotres = dot(a, b, MAXDIM);
			}
#endif
		}
		samp.min = min_diff;
		samp.max = max_diff;
		samp.index = average / loops;
		rtf_put(DEBUG_FIFO, &samp, sizeof (samp));
	}
	rt_printk("\nDOT PRODUCT RESULT = %lu\n", (unsigned long)dotres);
}


/*
 *	Initialisation. We have to select the scheduling mode and start 
 *      our periodical measurement task.  
 */

PROC_READ_OPEN_OPS(rtai_kern_lat_fops, proc_read);

static int
__latency_init(void)
{
	static struct proc_dir_entry *proc_rtai_kern_lat;

	/* XXX check option ranges here */

	/* register a proc entry */
#ifdef CONFIG_PROC_FS
	proc_rtai_kern_lat = CREATE_PROC_ENTRY("kern_latency_calibrate", 0, rtai_proc_root, &rtai_kern_lat_fops);
	SET_PROC_READ_ENTRY(proc_rtai_kern_lat, proc_read);
#endif

	rtf_create(DEBUG_FIFO, 16000);	/* create a fifo length: 16000 bytes */
	rt_linux_use_fpu(use_fpu);	/* declare if we use the FPU         */

	rt_task_init(			/* create our measuring task         */
			    &thread,	/* poiter to our RT_TASK             */
			    fun,	/* implementation of the task        */
			    0,		/* we could transfer data -> task    */
			    5000,	/* stack size                        */
			    0,		/* priority                          */
			    use_fpu,	/* do we use the FPU?                */
			    0		/* signal? XXX                       */
	);

	rt_set_runnable_on_cpus(	/* select on which CPUs the task is  */
		&thread,		/* allowed to run                    */
		RUN_ON_CPUS
	);

	/* Test if we have to start the timer                                */
	if (start_timer) {
		if (timer_mode) {
			rt_set_periodic_mode();
		} else {
			rt_set_oneshot_mode();
		}
		rt_assign_irq_to_cpu(TIMER_8254_IRQ, TIMER_TO_CPU);
		period_counts = start_rt_timer(nano2count(period));
	} else {
		period_counts = nano2count(period);
	}

	loops = ((1000000000*avrgtime)/period)/ECHOSPEED;

	/* Calculate the start time for the task. */
	/* We set this to "now plus 10 periods"   */
	expected = rt_get_time() + 10 * period_counts;
	rt_task_make_periodic(&thread, expected, period_counts);
	return 0;
}


/*
 *	Cleanup 
 */

static void
__latency_exit(void)
{
	int cpuid;

	/* If we started the timer we have to revert this now. */
	if (start_timer) {
		rt_reset_irq_to_sym_mode(TIMER_8254_IRQ);
		stop_rt_timer();
	}

	/* Now delete our task and remove the FIFO. */
	rt_task_delete(&thread);
	rtf_destroy(DEBUG_FIFO);

	/* Remove proc dir entry */
#ifdef CONFIG_PROC_FS
	remove_proc_entry("kern_latency_calibrate", rtai_proc_root);
#endif

	/* Output some statistics about CPU usage */
	printk("\n\nCPU USE SUMMARY\n");
	for (cpuid = 0; cpuid < NR_RT_CPUS; cpuid++) {
		printk("# %d -> %d\n", cpuid, cpu_used[cpuid]);
	}
	printk("END OF CPU USE SUMMARY\n\n");

}

module_init(__latency_init);
module_exit(__latency_exit);
