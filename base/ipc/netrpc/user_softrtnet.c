/*
COPYRIGHT (C) 2017  Paolo Mantegazza (mantegazza@aero.polimi.it)

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
#include <pthread.h>

#include <asm/rtai_srq.h>

static FILE *strm;

void *send_fun(void *arg)
{
	fprintf(strm, "USER SPACE SEND TASK BEGINS AND GOES TO KERNEL.\n"); fflush(strm);
	rtai_srq(((int *)arg)[0], 1);
	fprintf(strm, "USER SPACE SEND TASK ENDS.\n"); fflush(strm);
	return NULL;
}

int main(int argc, char **argv)
{
	int srq;
	pthread_t send_thread;
	strm = fopen(argv[1], "w+");
	fprintf(strm, "USER SPACE RECV TASK BEGINS AND OPENS SRQ.\n"); fflush(strm);
	srq = rtai_open_srq(0xbadface1);
	if (srq <= 0) {
		fprintf(strm, "MAIN-RECEIVER, NO SRQ AVAILABLE %d.\n", srq); fflush(strm);
		return -1;
	}
	fprintf(strm, "USER SPACE RECV TASK GOT SRQ %d AND CREATES USER SPACE SEND TASK.\n", srq); fflush(strm);
	pthread_create(&send_thread, NULL, send_fun, &srq);
	fprintf(strm, "USER SPACE RECV TASK GOES TO KERNEL.\n"); fflush(strm);
	rtai_srq(srq, 0);
	pthread_join(send_thread, NULL);
	fprintf(strm, "USER SPACE RECV TASK ENDS.\n"); fflush(strm);
	fclose(strm);
	return 0;
}
