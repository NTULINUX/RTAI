/*
COPYRIGHT (C) 2003-2008 Paolo Mantegazza (mantegazza@aero.polimi.it)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <http://www.gnu.org/licenses/>.

*/


#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>

#include <rtai_sem.h>
#include <rtai_mbx.h>

#define CPUMAP 0x1

#define USEDFRAC 25  // in %

#define NAVRG 4000

#define FASTMUL  4

#define SLOWMUL  24

#if defined(CONFIG_UCLINUX) || defined(CONFIG_ARM) || defined(CONFIG_COLDFIRE)
#define TICK_TIME 1000000
#else
#define TICK_TIME 100000
#endif

static RT_TASK *Latency_Task;
static RT_TASK *Slow_Task;
static RT_TASK *Fast_Task;

static volatile	RTIME start;
static volatile int end, slowjit, fastjit;

static MBX *mbx;

static SEM *barrier;

static void endme (int dummy) { end = 1; }

static void *slow_fun(void *arg)
{
	int jit, period;
	RTIME expected;

	if (!(Slow_Task = rt_thread_init(nam2num("SLWTSK"), 3, 0, SCHED_FIFO, CPUMAP))) {
		printf("CANNOT INIT SLOW TASK\n");
		exit(1);
	}

	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
	rt_sem_wait_barrier(barrier);

	period = nano2count(SLOWMUL*TICK_TIME);
	expected = start + 9*nano2count(TICK_TIME);
	rt_task_make_periodic(Slow_Task, expected, period);

	while (!end) {  
		jit = abs(count2nano(rt_get_tscnt() - expected));
		if (jit > slowjit) {
			slowjit = jit;
		}
		rt_busy_sleep((SLOWMUL*TICK_TIME*USEDFRAC)/100);
		expected += period;
		rt_task_wait_period();                                        
	}

	rt_sem_wait_barrier(barrier);
	rt_make_soft_real_time();
	rt_thread_delete(Slow_Task);
	return 0;
}                                        

static void *fast_fun(void *arg) 
{                             
	int jit, period;
	RTIME expected;

	if (!(Fast_Task = rt_thread_init(nam2num("FSTSK"), 2, 0, SCHED_FIFO, CPUMAP))) {
		printf("CANNOT INIT FAST TASK\n");
		exit(1);
	}

	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
	rt_sem_wait_barrier(barrier);

	period = nano2count(FASTMUL*TICK_TIME);
	expected = start + 6*nano2count(TICK_TIME);
	rt_task_make_periodic(Fast_Task, expected, period);

	while (!end) {  
		jit = abs(count2nano(rt_get_tscnt() - expected));
		if (jit > fastjit) {
			fastjit = jit;
		}
		rt_busy_sleep((FASTMUL*TICK_TIME*USEDFRAC)/100);
		expected += period;
		rt_task_wait_period();                                        
	}                      

	rt_sem_wait_barrier(barrier);
	rt_make_soft_real_time();
	rt_thread_delete(Fast_Task);
	return 0;
}

static void *latency_fun(void *arg)
{
	struct sample { long min, max, avrg, jitters[2]; } samp;
	int diff;
	int skip;
	int average;
	int min_diff;
	int max_diff;
	int period;
	RTIME expected;

	min_diff = 1000000000;
	max_diff = -1000000000;
	if (!(Latency_Task = rt_thread_init(nam2num("LTCTSK"), 1, 0, SCHED_FIFO, CPUMAP))) {
		printf("CANNOT INIT LATENCY TASK\n");
		exit(1);
	}

	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
	rt_sem_wait_barrier(barrier);

	period = nano2count(TICK_TIME);
	expected = start + 3*period;
	rt_task_make_periodic(Latency_Task, expected, period);

	while (!end) {  
		average = 0;
		for (skip = 0; skip < NAVRG && !end; skip++) {
			expected += period;
			rt_task_wait_period();
			diff = (int)count2nano(rt_get_tscnt() - expected);
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
		rt_mbx_send_if(mbx, &samp, sizeof(samp));
	}

	rt_sem_wait_barrier(barrier);
	rt_make_soft_real_time();
	rt_thread_delete(Latency_Task);
	return 0;
}

static pthread_t latency_thread, fast_thread, slow_thread;

int main(void)
{
	RT_TASK *Main_Task;

	signal(SIGHUP,  endme);
	signal(SIGINT,  endme);
	signal(SIGKILL, endme);
	signal(SIGTERM, endme);
	signal(SIGALRM, endme);

	if (!(Main_Task = rt_thread_init(nam2num("MNTSK"), 0, 0, SCHED_FIFO, 0xF))) {
		printf("CANNOT INIT MAIN TASK\n");
		exit(1);
	}

	if (!(mbx = rt_mbx_init(nam2num("MBX"), 1000))) {
		printf("ERROR OPENING MBX\n");
		exit(1);
	}

	barrier = rt_sem_init(nam2num("PREMSM"), 4);
	latency_thread = rt_thread_create(latency_fun, NULL, 0);
	fast_thread    = rt_thread_create(fast_fun, NULL, 0);
	slow_thread    = rt_thread_create(slow_fun, NULL, 0);
	start = rt_get_tscnt() + nano2count(200000000);
	rt_sem_wait_barrier(barrier);

	pause();
	end = 1;

	rt_sem_wait_barrier(barrier);
	rt_thread_join(latency_thread);
	rt_thread_join(fast_thread);
	rt_thread_join(slow_thread);
	rt_mbx_delete(mbx);
	rt_sem_delete(barrier);
	rt_thread_delete(Main_Task);
	return 0;
}
