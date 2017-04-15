/*
 * COPYRIGHT (C) 2001  Paolo Mantegazza <mantegazza@aero.polimi.it>
 *               2002  Robert Schwebel  <robert@schwebel.de>
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
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <rtai_fifos.h>

static int end;

static void endme(int dummy) { end = 1;}

int main(int argc, char *argv[])
{
	int fd0;
	char line[256];
	char nm[RTF_NAMELEN+1];
	FILE *procfile;
	long long max = -1000000000, min = 1000000000;
	struct sample { long long min; long long max; int index, ovrn; } samp;
	int n = 0;

	setlinebuf(stdout);

	signal(SIGINT, endme);

	if ((procfile = fopen("/proc/rtai/latency_calibrate", "r")) == NULL) {
		printf("Warning: Error opening /proc/rtai/latency_calibrate\n");
		printf("         Couldn't get infos about the module's state.\n");
	} else {
		while (fgets(line,256,procfile) != NULL) {printf("%s",line);}
		fclose(procfile);
	}

	if ((fd0 = open(rtf_getfifobyminor(3,nm,sizeof(nm)), O_RDONLY)) < 0) {
		fprintf(stderr, "Error opening %s\n",nm);
		exit(1);
	}

	printf("RTAI Testsuite - KERNEL latency (all data in nanoseconds)\n");

	while (!end) {
		if ((n++ % 21)==0) {
#if 0
			time(&timestamp); 
			tm_timestamp=localtime(&timestamp);
			printf("%04d/%02d/%0d %02d:%02d:%02d\n", tm_timestamp->tm_year+1900, tm_timestamp->tm_mon+1, tm_timestamp->tm_mday, tm_timestamp->tm_hour, tm_timestamp->tm_min, tm_timestamp->tm_sec);
#endif
			printf("RTH|%11s|%11s|%11s|%11s|%11s|%11s\n", "lat min", "ovl min", "lat avg", "lat max", "ovl max", "overruns");
		}
		read(fd0, &samp, sizeof(samp));
		if (min > samp.min) min = samp.min;
		if (max < samp.max) max = samp.max;
		printf("RTD|%11lld|%11lld|%11d|%11lld|%11lld|%11d\n", samp.min, min, samp.index, samp.max, max, samp.ovrn);
		fflush(stdout);
	}

	close(fd0);

	return 0;
}
