/*
 * Copyright (C) 2003-2013 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */

#include <stdio.h>
#include <getopt.h>

#include <rtai_lxrt.h>

struct option options[] = {
	{ "help",       0, 0, 'h' },
	{ "period",     1, 0, 'p' },
	{ "spantime",   1, 0, 's' },
	{ "tol",	1, 0, 't' },
	{ NULL,	 0, 0,  0  }
};

void print_usage(void)
{
    fputs(
	("\n*** KERNEL-USER SPACE LATENCY CALIBRATIONS ***\n"
	 "\n"
	 "OPTIONS:\n"
	 "  -h, --help\n"
	 "      print usage\n"
	 "  -p <period (us)>, --period <period (us)>\n"
	 "      the period, in microseconds, of the hard real time calibrator task, default 200 (us)\n"
	 "  -s <spantime (s)>, --spantime <spantime (s)>\n"
	 "      the duration, in seconds, of the requested calibration, default 1 (s)\n"
	 "  -t <conv. tol. (ns)>, --tol <conv. tol. (ns)>\n"
	 "      the acceptable tolerance, in nanoseconds, within which the latency must stay, default 100 (ns)\n"
	 "\n")
	, stderr);
}

static int period = 200 /* us */, loops = 1 /* s */;

static inline int sign(int v) { return v > 0 ? 1 : (v < 0 ? -1 : 0); }
static int user_latency;
static RT_TASK *calmng;

int user_calibrator(long loops)
{
	RTIME expected;
	double s = 0;

 	if (!rt_thread_init(nam2num("USRCAL"), 0, 0, SCHED_FIFO, 0xF)) {
		printf("*** CANNOT INIT USER LATENCY CALIBRATOR TASK ***\n");
		return 1;
	}
	mlockall(MCL_CURRENT | MCL_FUTURE);

	rt_make_hard_real_time();
	expected = rt_get_tscnt() + 10*period;
	rt_task_make_periodic(NULL, expected, period);
	while(loops--) {
		expected += period;
		rt_task_wait_period();
		user_latency += rt_get_tscnt() - expected;
		s += 3.14;
	}
	rt_make_soft_real_time();
	rt_task_resume(calmng);


	rt_thread_delete(NULL);
	return s;
}

int main(int argc, char *argv[])
{
	int kern_latency, UserLatency = 0, KernLatency = 0, tol = 100;

	while (1) {
		int c;
		if ((c = getopt_long(argc, argv, "hp:t:l:", options, NULL)) < 0) {
			break;
		}
		switch(c) {
			case 'h': { print_usage();	 return 0; }
			case 'p': { period = atoi(optarg);    break; }
			case 't': { loops  = atoi(optarg);    break; }
			case 'l': { tol    = atoi(optarg);    break; }
		}
	}

	system("/sbin/insmod \"" HAL_SCHED_PATH "\"/rtai_hal.ko >/dev/null 2>&1");
	system("/sbin/insmod \"" HAL_SCHED_PATH "\"/rtai_sched.ko >/dev/null 2>&1");

 	if (!(calmng = rt_thread_init(nam2num("CALMNG"), 10, 0, SCHED_FIFO, 0xF)) ) {
		printf("*** CANNOT INIT CALIBRATION TASK ***\n");
		return 1;
	}
	printf("\n* CALIBRATING SCHEDULING LATENCIES FOR %d (s):", loops);
	loops  = (loops*1000000 + period/2)/period;
	printf(" PERIOD %d (us), LOOPS %d. *\n", period, loops);

	start_rt_timer(0);
	period = nano2count(1000*period);

	printf("\n* KERNEL SPACE. *\n");
	do {
		kern_latency = kernel_calibrator(period, loops, KernLatency);

		kern_latency = (kern_latency + loops/2)/loops;
		kern_latency = sign(kern_latency)*count2nano(abs(kern_latency));
#if CONFIG_RTAI_BUSY_TIME_ALIGN
		printf("* KERNEL SPACE RETURN DELAY: %d (ns), ADD TO THE ONE CONFIGURED. *\n", kern_latency);
#else
		printf("* KERNEL SPACE SCHED LATENCY: %d (ns), ADD TO THE ONE CONFIGURED. *\n", kern_latency);
#endif

		KernLatency += kern_latency;
		printf("* KERNEL SPACE LATENCY: %d. *\n", KernLatency);
	}	while (abs(kern_latency) > tol && !CONFIG_RTAI_BUSY_TIME_ALIGN);

	printf("\n* USER SPACE. *\n");
	do {
		kernel_calibrator(period, loops, -UserLatency);
		rt_thread_create((void *)user_calibrator, (void *)loops, 0);
		rt_task_suspend(calmng);

		user_latency = (user_latency + loops/2)/loops;
		user_latency = sign(user_latency)*count2nano(abs(user_latency));
#if CONFIG_RTAI_BUSY_TIME_ALIGN
		printf("* USER SPACE RETURN DELAY: %d (ns), ADD TO THE ONE CONFIGURED. *\n", user_latency);
#else
		printf("* USER SPACE SCHED LATENCY: %d (ns), ADD TO THE ONE CONFIGURED. *\n", user_latency);
#endif
		UserLatency += user_latency;
		printf("* USER SPACE LATENCY: %d. *\n", UserLatency);
	}	while (abs(user_latency) > tol && !CONFIG_RTAI_BUSY_TIME_ALIGN);

	stop_rt_timer();
	rt_thread_delete(NULL);
	printf("\n");

	system("/sbin/rmmod rtai_sched >/dev/null 2>&1");
	system("/sbin/rmmod rtai_hal >/dev/null 2>&1");

	return 0;
}
