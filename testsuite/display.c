/*
 * COPYRIGHT (C) 2000  Paolo Mantegazza (mantegazza@aero.polimi.it)
 * COPYRIGHT (C) 2019  Alec Ari         (neotheuser@ymail.com)
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
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/poll.h>

#include <rtai_mbx.h>
#include <rtai_msg.h>

static RT_TASK *task;

static volatile int end;
void endme(int sig) { end = 1; }

int main(void)
{
	unsigned int msg;
	MBX *mbx;
	struct sample { long long min; long long max; int index, ovrn; } samp;
	long long max = -1000000000, min = 1000000000;
	int n = 0;
	struct pollfd pfd = { fd: 0, events: POLLIN|POLLERR|POLLHUP, revents: 0 };

	signal(SIGHUP,  endme);
	signal(SIGINT,  endme);
	signal(SIGKILL, endme);
	signal(SIGTERM, endme);
	signal(SIGALRM, endme);
	setlinebuf(stdout);

	if (!(task = rt_task_init(nam2num("LATCHK"), 20, 0, 0))) {
		printf("CANNOT INIT MASTER TASK\n");
		exit(1);
	}

	if (!(mbx = rt_get_adr(nam2num("LATMBX")))) {
		printf("CANNOT FIND MAILBOX\n");
		exit(1);
	}

	printf("\nRTAI testsuite and calibration tool (all data in nanoseconds)\n");
	printf("# period = %i (ns) \n", 100000);
	printf("# average time = %i (s)\n\n", (int)1);

	while (!end) {
		if ((n++ % 21) == 0) {
			printf("RTH|%11s|%11s|%11s|%11s|%11s|%11s\n", "lat min", "ovl min", "lat avg","lat max","ovl max", "overruns");
		}
		rt_mbx_receive(mbx, &samp, sizeof(samp));
		if (min > samp.min) min = samp.min;
		if (max < samp.max) max = samp.max;
		printf("RTD|%11lld|%11lld|%11d|%11lld|%11lld|%11d\n", samp.min, min, samp.index, samp.max, max, samp.ovrn);
		if ((poll(&pfd, 1, 20) > 0 && (pfd.revents & (POLLIN | POLLERR | POLLHUP))) || end) {
			break;
		}
	}

	if (rt_get_adr(nam2num("LATCAL"))) {
		rt_rpc(rt_get_adr(nam2num("LATCAL")), msg, &msg);
	}
	rt_task_delete(task);
	exit(0);
}
