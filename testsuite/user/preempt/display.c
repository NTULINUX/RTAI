/*
COPYRIGHT (C) 1999-2008 Paolo Mantegazza (mantegazza@aero.polimi.it)

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
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <signal.h>
#include <unistd.h>

#include <rtai_mbx.h>

static volatile int end;

static void endme (int dummy) { end = 1; }

int main(int argc,char *argv[])
{
	MBX *mbx;
        struct pollfd pollkb;
	struct sample { long min, max, avrg, jitters[2]; } samp;
	int n = 0;

	setlinebuf(stdout);
	
	pollkb.fd = 0;
	pollkb.events = POLLIN;

	signal(SIGHUP,  endme);
	signal(SIGINT,  endme);
	signal(SIGKILL, endme);
	signal(SIGTERM, endme);
	signal(SIGALRM, endme);

        if (!rt_thread_init(nam2num("DSPLY"), 0, 0, SCHED_FIFO, 0xF)) {
                printf("CANNOT INIT DISPLAY TASK\n");
                exit(1);
        }

	if (!(mbx = rt_get_adr(nam2num("MBX")))) {
		fprintf(stderr, "Error opening mbx in display\n");
		exit(1);
	}

	printf("RTAI Testsuite - LXRT preempt (all data in nanoseconds)\n");

	while (!end) {
		if ((n++ % 21) == 0) {
			printf("RTH|%12s|%12s|%12s|%12s|%12s\n", "lat min","lat avg","lat max","jit fast","jit slow");
		}
		rt_mbx_receive(mbx, &samp, sizeof(samp));
		printf("RTD|%12ld|%12ld|%12ld|%12ld|%12ld\n", samp.min, samp.avrg, samp.max, samp.jitters[0], samp.jitters[1]);
		fflush(stdout);
		if (poll(&pollkb, 1, 1) > 0) {
			getchar();
			break;
                }
        }
	return 0;
}
