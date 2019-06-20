/*
COPYRIGHT (C) 2000  Emanuele Bianchi (bianchi@aero.polimi.it)
COPYRIGHT (C) 2000  Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/poll.h>

#include <rtai_sem.h>
#include <rtai_msg.h>

#define LOOPS  1000
#define NR_RT_TASKS 10
#define taskname(x) (1000 + (x))

static pthread_t thread[NR_RT_TASKS];

static RT_TASK *mytask[NR_RT_TASKS];

static SEM *sem;

static int volatile hrt[NR_RT_TASKS], change, end;

static int indx[NR_RT_TASKS];       

static void *thread_fun(void *arg)
{
	int mytask_indx;
	unsigned long msg;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	mytask_indx = ((int *)arg)[0];
	if (!(mytask[mytask_indx] = rt_thread_init(taskname(mytask_indx), 0, 0,
SCHED_FIFO, 0x1))) {
		printf("CANNOT INIT TASK %u\n", taskname(mytask_indx));
		exit(1);
	}

	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
	hrt[mytask_indx] = 1;
	while (!end) {
		switch (change) {
			case 0:
				rt_task_suspend(mytask[mytask_indx]);
				break;
			case 1:
				rt_sem_wait(sem);
				break;
			case 2:
				rt_return(rt_receive(NULL, &msg), 0);
				break;
		}
	}
	rt_make_soft_real_time();

	rt_task_delete(mytask[mytask_indx]);
	hrt[mytask_indx] = 0;

	return (void*)0;
}

static void msleep(int ms)
{
	poll(NULL, 0, ms);
}

int main(void)
{
	RTIME tsr, tss, tsm;
	RT_TASK *mainbuddy;
	int i, k, s;       
	unsigned long msg;

	printf("\n\nWait for it ...\n");
	if (!(mainbuddy = rt_thread_init(nam2num("MASTER"), 1000, 0, SCHED_FIFO, 0x1))) {
		printf("CANNOT INIT TASK %lu\n", nam2num("MASTER"));
		exit(1);
	}

	sem = rt_sem_init(nam2num("SEMAPH"), 1); 
	change =  0;
	
	for (i = 0; i < NR_RT_TASKS; i++) {
		indx[i] = i;
		if (!(thread[i] = rt_thread_create(thread_fun, indx + i, 0))) {
			printf("ERROR IN CREATING THREAD %d\n", indx[i]);
			exit(1);
 		}       
 	} 

	do {
		msleep(50);
		s = 0;	
		for (i = 0; i < NR_RT_TASKS; i++) {
			s += hrt[i];
		}
	} while (s != NR_RT_TASKS);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	
	rt_make_hard_real_time();
	tsr = rt_get_cpu_time_ns();
	for (i = 0; i < LOOPS; i++) {
		for (k = 0; k < NR_RT_TASKS; k++) {
			rt_task_resume(mytask[k]);
		} 
	} 
	tsr = rt_get_cpu_time_ns() - tsr;

	change = 1;

	for (k = 0; k < NR_RT_TASKS; k++) {
		rt_task_resume(mytask[k]);
	} 

	tss = rt_get_cpu_time_ns();
	for (i = 0; i < LOOPS; i++) {
		for (k = 0; k < NR_RT_TASKS; k++) {
	        	rt_sem_signal(sem);
		}
	}
	tss = rt_get_cpu_time_ns() - tss;

	change = 2;

	for (k = 0; k < NR_RT_TASKS; k++) {
		rt_sem_signal(sem);
	}

	tsm = rt_get_cpu_time_ns();
	for (i = 0; i < LOOPS; i++) {
		for (k = 0; k < NR_RT_TASKS; k++) {
			rt_rpc(mytask[k], 0, &msg);
		}
	}
	tsm = rt_get_cpu_time_ns() - tsm;

	rt_make_soft_real_time();

	printf("\n\nFOR %d TASKS: ", NR_RT_TASKS);
	printf("TIME %d (ms), SUSP/RES SWITCHES %d, ", (int)(tsr/1000000), 2*NR_RT_TASKS*LOOPS);
	printf("SWITCH TIME %d (ns)\n", (int)(tsr/(2*NR_RT_TASKS*LOOPS)));

	printf("\nFOR %d TASKS: ", NR_RT_TASKS);
	printf("TIME %d (ms), SEM SIG/WAIT SWITCHES %d, ", (int)(tss/1000000), 2*NR_RT_TASKS*LOOPS);
	printf("SWITCH TIME %d (ns)\n", (int)(tss/(2*NR_RT_TASKS*LOOPS)));

	printf("\nFOR %d TASKS: ", NR_RT_TASKS);
	printf("TIME %d (ms), RPC/RCV-RET SWITCHES %d, ", (int)(tsm/1000000), 2*NR_RT_TASKS*LOOPS);
	printf("SWITCH TIME %d (ns)\n\n", (int)(tsm/(2*NR_RT_TASKS*LOOPS)));

	fflush(stdout);

	end = 1;
	for (i = 0; i < NR_RT_TASKS; i++) {
		rt_rpc(mytask[i], 0, &msg);
	} 
	do {
		msleep(50);
		s = 0;	
		for (i = 0; i < NR_RT_TASKS; i++) {
			s += hrt[i];
		}
	} while (s);

	rt_sem_delete(sem);
	rt_task_delete(mainbuddy);
	for (i = 0; i < NR_RT_TASKS; i++) {
		rt_thread_join(thread[i]);
	}
	
	return 0;
}
