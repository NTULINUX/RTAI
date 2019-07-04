/*
 * Copyright (C) 2016 Paolo Mantegazza <mantegazza@aero.polimi.it>
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
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include <rtai_lxrt.h>

static inline RTIME rt_rdtsc(void)
{
#ifdef __i386__
        unsigned long long t;
        __asm__ __volatile__ ("rdtsc" : "=A" (t));
       return t;
#else
        union { unsigned int __ad[2]; RTIME t; } t;
        __asm__ __volatile__ ("rdtsc" : "=a" (t.__ad[0]), "=d" (t.__ad[1]));
        return t.t;
#endif
}

#define DIAG_KF_LAT_EVAL 0
#define HINF 0
#define THETA ((double)0.1)
#define MAX_LOOPS CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES
#define R  ((double)2.0)
#define Q  0.0
#define P0 R
#define ALPHA ((double)1.0001)
static inline double kf_lat_eval(long period)
{
	int calok, loop;
	double xe, pe, q, r, xm; 
	double xp, pp, ppe, y, g;
	RTIME start_time, resume_time;

	xe = 0;
	pe = P0*P0; 
	q  = Q*Q;
	r  = R*R;
	xm = 1.0e9; 

#if DIAG_KF_LAT_EVAL
	rt_printk("INITIAL VALUES: xe %g, pe %g, q %g, r %g.\n", xe, pe, q, r);
#endif

	start_time = rt_rdtsc();
	resume_time = start_time + 5*period;
	rt_task_make_periodic(NULL, resume_time, period);
	for (calok = loop = 1; loop <= MAX_LOOPS; loop++) {
		resume_time += period;
		if (!rt_task_wait_period()) {
			y = (rt_rdtsc() - resume_time);
		} else {
			y = xe;
		}
		if (y < xm) xm = y;
#if !HINF
		xp  = xe;
		pp  = ALPHA*pe + q;
		g   = pp/(pp + r);
		xe  = xp + g*(y - xp);
		ppe = pe;
		pe  = (1.0 - g)*pp;
#else
		xp  = xe;
		g   = pe/(pe*(1.0 - THETA*r) + r);
		xe  = xp + g*(y - xp);
		ppe = pe;
		pe  = g*r + q;
#endif

#if DIAG_KF_LAT_EVAL
		rt_printk("loop %d, xp %g, xe %g, y %g, pp %g, pe %g, g %g, r %g.\n", loop, xp, xe, y, pp, pe, g, r);
#endif

		if (fabs((xe - xp)/xe) < 1.0e-3 && fabs((pe - ppe)/pe) < 1.0e-3) {
			if (calok++ > 250) break;
		} else {
			calok = 1;
		}
	}
	rt_printk("USER SPACE LATENCY ENDED AT CYCLE: %d, LATENCY = %g, VARIANCE = %g, GAIN = %g, LEAST = %g.\n", loop - 1 , xe,  pe, g, xm);
	return CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS == 1 ? xe : (CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS == 2 ? xm : (xe + xm)/2);
}

#if 1
int main(int argc, char **argv)
{
	RT_TASK *usrcal;
	int period, ulat = -1, klat = -1;
	FILE *file;

	period = atoi(argv[2]);
	if (period <= 0) {
		if (!period && !access(argv[1], F_OK)) {
			file = fopen(argv[1], "r");
			fscanf(file, "%d %d %d", &klat, &ulat, &period);
			fclose(file);
		}
		rt_sched_latencies(klat > 0 ? nano2count(klat) : klat, ulat > 0 ? nano2count(ulat) : ulat, period > 0 ? nano2count(period) : period);
	} else {
 		if (!(usrcal = rt_thread_init(nam2num("USRCAL"), 0, 0, SCHED_FIFO, 0x1))) {
			return 1;
		}
		if ((file = fopen(argv[1], "w"))) {
			klat = atoi(argv[3]);
			mlockall(MCL_CURRENT | MCL_FUTURE);
			rt_make_hard_real_time();
			ulat = kf_lat_eval(period);
			rt_make_soft_real_time();
			fprintf(file, "%lld %lld %lld\n", count2nano(klat), count2nano(ulat), count2nano(period));
			rt_sched_latencies(klat, ulat, period);
			fclose(file);
		}
		rt_thread_delete(usrcal);
	}
	return 0;
}
#else
int main(int argc, char **argv)
{
#define WARMUP 50
	RT_TASK *usrcal;
	RTIME start_time, resume_time;
	int loop, period, ulat = -1, klat = -1;
	long latency = 0, ovrns = 0;
	FILE *file;

	period = atoi(argv[2]);
	if (period <= 0) {
		if (!period && !access(argv[1], F_OK)) {
			file = fopen(argv[1], "r");
			fscanf(file, "%d %d %d", &klat, &ulat, &period);
			fclose(file);
		}
		rt_sched_latencies(klat > 0 ? nano2count(klat) : klat, ulat > 0 ? nano2count(ulat) : ulat, period > 0 ? nano2count(period) : period);
	} else {
 		if (!(usrcal = rt_thread_init(nam2num("USRCAL"), 0, 0, SCHED_FIFO, 0xF))) {
			return 1;
		}
		if ((file = fopen(argv[1], "w"))) {
			klat = atoi(argv[3]);
			mlockall(MCL_CURRENT | MCL_FUTURE);
			rt_make_hard_real_time();
			start_time = rt_rdtsc();
			resume_time = start_time + 5*period;
			rt_task_make_periodic(usrcal, resume_time, period);
			for (loop = 1; loop <= (MAX_LOOPS + WARMUP); loop++) {
				resume_time += period;
				if (!rt_task_wait_period()) {
					latency += (long)(rt_rdtsc() - resume_time);
					if (loop == WARMUP) {
						latency = 0;
					}
				} else {
					ovrns++;
				}
			}
			rt_make_soft_real_time();
			ulat = latency/MAX_LOOPS;
			fprintf(file, "%lld %lld %lld\n", count2nano(klat), count2nano(ulat), count2nano(period));
			rt_sched_latencies(klat, ulat, period);
			fclose(file);
		}
		rt_thread_delete(usrcal);
	}
	return 0;
}
#endif
