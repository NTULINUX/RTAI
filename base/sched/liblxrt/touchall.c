/*
 * Copyright (C) Tomasz Motylewski <motyl@stan.chemie.unibas.ch>
 * Copyright (C) Pierre Cloutier   <pcloutier@poseidoncontrols.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the version 2 of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <malloc.h>

#if 0
#define TOUCH_BUFSIZE 256
#define GROW_STACK (64*1024)
#define GROW_HEAP  (64*1024)
#define STR_SIZE 16

void touch_area(void *begin, size_t len, int writeable) {
	volatile char *ptr = begin;
	int i, page_size;
	volatile int tmp;
	
	page_size = getpagesize();
	for(i=0;i<len;i+=page_size) {
		tmp=ptr[i];
	/*	printf("R:%p",ptr+i); */
		if(writeable) {
			ptr[i]=tmp;
	/*		printf("W:%p",ptr+i); */
		}
	}
}

int touch_all(void) {
	FILE *maps;
	unsigned long start,end,flags,size;
	char perms[STR_SIZE],dev[STR_SIZE];
	char buf[TOUCH_BUFSIZE];
	
	maps=fopen("/proc/self/maps","r");
	if(!maps) {
		perror("touch_all");
		return -1;
	}
	while(fgets(buf,TOUCH_BUFSIZE-1,maps)) {
		if(sscanf(buf,"%8lx-%8lx %15s %lx %15s %lu",&start,&end,perms,&flags,dev,&size)<2)
			continue;
		if(perms[1] == 'w')
			touch_area((void*)start,end-start,1);
		else
			touch_area((void*)start,end-start,0);
	}
	fclose(maps);
	return 0;
}

int lock_all(int stk, int heap) {
	extern void dump_malloc_stats(void);
	int err, n, i;
	unsigned long *pt;
	char stack[stk ? stk : GROW_STACK];
	stack[0] = ' ';

/*
    glibc (2.1.3) limits and default values:

	dynamically tunable      *
	mmap threshold      128k *
    mmap threshold max  512k  
	mmap max           1024k *
	trim threshold      128k *
	top pad               0k *	
	heap min             32k
	heap max           1024k
*/

    n = heap / 65536 + 1;
	if ( n > (sizeof(stack) / sizeof(int))) {
		printf("heap too large\n");
		exit(-1);
	}
	
    err = mallopt(M_MMAP_THRESHOLD, 512*1024);
	if (!err) {
		printf("mallopt(M_MMAP_THRESHOLD, heap) failed\n");
		exit(-1);
	}	 

	err = mallopt(M_TOP_PAD,        heap ? heap : GROW_HEAP);
    if (!err) {
		printf("mallopt(M_TOP_PAD, heap) failed\n");
		exit(-1);
    }

	if(mlockall(MCL_CURRENT|MCL_FUTURE)) {
		perror("mlockall");
		exit(-1);
	}

	touch_all();

    pt = (unsigned long *) stack;
	for(i=0; i<n ; i++, pt++) *pt = (unsigned long) malloc(65536);
    pt = (unsigned long *) stack;
	for(i=0; i<n ; i++, pt++) free((void *) *pt);

	return 0;
}

void dump_malloc_stats(void)
{
	struct mallinfo mi;
	extern int rtai_print_to_screen(const char *fmt, ...);
//	memset(&mi, 0, sizeof(mi));	
	mi = mallinfo();

	rtai_print_to_screen("\ntotal space allocated from system %d\n", mi.arena);
	rtai_print_to_screen("number of non-inuse chunks        %d\n", mi.ordblks);
	rtai_print_to_screen("number of mmapped regions         %d\n", mi.hblks);
	rtai_print_to_screen("total space in mmapped regions    %d\n", mi.hblkhd);
	rtai_print_to_screen("total allocated space             %d\n", mi.uordblks);
	rtai_print_to_screen("total non-inuse space             %d\n", mi.fordblks);
	rtai_print_to_screen("top-most, releasable space        %d\n", mi.keepcost);
}

/*
int main(int argc, char *argv[]) {
	sleep(5);
	lock_all(0,0);
	dump_malloc_stats();
	sleep(5);
	return 0;
}
*/
#endif
