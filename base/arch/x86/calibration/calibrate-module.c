/*
 * Copyright (C) 2003  Paolo Mantegazza <mantegazza@aero.polimi.it>,
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

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include "calibrate.h"

MODULE_LICENSE("GPL");

#if 0
#define COUNT               0xFFFFFFFFU

static struct params_t params = { 0, SETUP_TIME_8254, LATENCY_8254, 0, LATENCY_APIC, SETUP_TIME_APIC, CALIBRATED_APIC_FREQ, 0, CALIBRATED_CPU_FREQ, CLOCK_TICK_RATE, LATCH };

static int reset_count, count;
static struct times_t times;

static void calibrate(void)
{
	static RTIME cpu_tbase;
	static unsigned long apic_tbase;

	times.cpu_time = rd_CPU_ts();
#ifdef CONFIG_X86_LOCAL_APIC
	if (params.mp) {
		times.apic_time = apic_read(APIC_TMCCT);
	}
#endif /* CONFIG_X86_LOCAL_APIC */
	if (times.intrs < 0) {
		cpu_tbase  = times.cpu_time;
		apic_tbase = times.apic_time;
	}
	times.intrs++;
	if (++count == reset_count) {
		count = 0;
		times.cpu_time -= cpu_tbase;
		times.apic_time = apic_tbase - times.apic_time;
		rtf_put(0, &times, sizeof(times));
	}
	rt_pend_linux_irq(TIMER_8254_IRQ);
#ifdef CONFIG_X86_LOCAL_APIC
	if (params.mp) {
	        unsigned temp = (apic_read(APIC_ICR) & (~0xCDFFF)) | (APIC_DM_FIXED | APIC_DEST_ALLINC | LOCAL_TIMER_VECTOR);
		apic_write(APIC_ICR, temp);
	}
#endif /* CONFIG_X86_LOCAL_APIC */
}

static void just_ret(void)
{
        return;
}

static RT_TASK rtask;
static int period;
static RTIME expected;

static void spv(long loops)
{
	int skip, average = 0;
	for (skip = 0; skip < loops; skip++) {
		expected += period;
		rt_task_wait_period();
		average += (int)count2nano(rt_get_time() - expected);
	}
	rtf_put(0, &average, sizeof(average));
	rt_task_suspend(0);
}

static RTIME t0;
static int bus_period, bus_threshold, use_parport, loops, maxj, bit;

static int rt_timer_tick_ext(int irq, unsigned long data)
{
	RTIME t;
	int jit;

	if (loops++ < INILOOPS) {
		t0 = rdtsc();
	} else {
		t = rdtsc();
		if (use_parport) {
			outb(bit = 1 - bit, PARPORT); 
		}
		if ((jit = abs((int)(t - t0) - bus_period)) > maxj) {
			maxj = jit;
			if (maxj > bus_threshold) {
				int msg;
				msg = imuldiv(maxj, 1000000000, CPU_FREQ);
				rtf_put(0, &msg, sizeof(msg));
			}
		}
		t0 = t;
	}
	rt_times.tick_time = rt_times.intr_time;
	rt_times.intr_time = rt_times.tick_time + rt_times.periodic_tick;
	rt_set_timer_delay(0);
	if (rt_times.tick_time >= rt_times.linux_time) {
		rt_times.linux_time += rt_times.linux_tick;
		hard_sti();
		rt_pend_linux_irq(TIMER_8254_IRQ);
		return 0;
	} 
	hard_sti();
	return 1;
}

static long long user_srq(unsigned long whatever)
{
	extern int calibrate_8254(void);
	unsigned long args[MAXARGS];
	int ret;

	ret = copy_from_user(args, (unsigned long *)whatever, MAXARGS*sizeof(unsigned long));
	switch (args[0]) {
		case CAL_8254: {
			return calibrate_8254();
			break;
		}

		case KTHREADS:
		case KLATENCY: {
			rt_set_oneshot_mode();
			period = start_rt_timer(nano2count(args[1]));
			if (args[0] == KLATENCY) {
				rt_task_init_cpuid(&rtask, spv, args[2], STACKSIZE, 0, 0, 0, hard_cpu_id());
			} else {
//				rt_kthread_init_cpuid(&rtask, spv, args[2], STACKSIZE, 0, 0, 0, hard_cpu_id());	
			}
			expected = rt_get_time() + 100*period;
			rt_task_make_periodic(&rtask, expected, period);
			break;
		}

		case END_KLATENCY: {
			stop_rt_timer();
			rt_task_delete(&rtask);
			break;
		}

		case FREQ_CAL: {
			times.intrs = -1;
			reset_count = args[1]*HZ;
			count = 0;
			rt_assign_irq_to_cpu(TIMER_8254_IRQ, 1 << hard_cpu_id());
			rt_request_timer(just_ret, COUNT, 1);
			rt_request_global_irq(TIMER_8254_IRQ, calibrate);
			break;
		}

		case END_FREQ_CAL: {
		        rt_free_timer();
		        rt_reset_irq_to_sym_mode(TIMER_8254_IRQ);
		        rt_free_global_irq(TIMER_8254_IRQ);
			break;
		}

		case BUS_CHECK: {
			loops = maxj = 0;
			bus_period = imuldiv(args[1], CPU_FREQ, 1000000000);
			bus_threshold = imuldiv(args[2], CPU_FREQ, 1000000000);
			use_parport = args[3];
			rt_assign_irq_to_cpu(TIMER_8254_IRQ, 1 << hard_cpu_id());
			rt_request_timer((void *)rt_timer_tick_ext, imuldiv(args[1], FREQ_8254, 1000000000), 0);
			rt_set_global_irq_ext(TIMER_8254_IRQ, 1, 0);
			break;
		}

		case END_BUS_CHECK: {
		        rt_free_timer();
		        rt_reset_irq_to_sym_mode(TIMER_8254_IRQ);
			break;
		}
		case GET_PARAMS: {
			rtf_put(0, &params, sizeof(params));
			break;
		}
	} 
	return 0;
}

static int srq;

int init_module(void)
{
#ifdef CONFIG_X86_LOCAL_APIC
	params.mp        = 1;
#endif /* CONFIG_X86_LOCAL_APIC */
	params.freq_apic = RTAI_FREQ_APIC;
	params.cpu_freq  = RTAI_CPU_FREQ;
	rtf_create(0, FIFOBUFSIZE);
	if ((srq = rt_request_srq(CALSRQ, (void *)user_srq, user_srq)) < 0) {
                printk("No sysrq available for the calibration.\n");
                return srq;
	}
	return 0;
}

void cleanup_module(void)
{
	rt_free_srq(srq);
	rtf_destroy(0);
	return;
}
#endif
