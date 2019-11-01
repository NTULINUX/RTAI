/*
 * Copyright (C) 2001,2002,2003,2004 Philippe Gerum <rpm@xenomai.org>.
 *
 * RTAI is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * RTAI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RTAI; if not, see <http://www.gnu.org/licenses/>.
 *
 * This file provides the interface to the RTAI dynamic memory
 * allocator based on the algorithm described in "Design of a General
 * Purpose Memory Allocator for the 4.3BSD Unix Kernel" by Marshall
 * K. McKusick and Michael J. Karels.
 */

#ifndef _RTAI_MALLOC_H
#define _RTAI_MALLOC_H

#include <rtai_types.h>

#ifdef __KERNEL__

#ifndef __cplusplus

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/spinlock.h>

/*
 * LIMITS:
 *
 * Minimum page size is 2 ** RTHEAP_MINLOG2 (must be large enough to
 * hold a pointer).
 *
 * Maximum page size is 2 ** RTHEAP_MAXLOG2.
 *
 * Minimum block size equals the minimum page size.
 *
 * Requested block size smaller than the minimum block size is
 * rounded to the minimum block size.
 *
 * Requested block size larger than 2 times the page size is rounded
 * to the next page boundary and obtained from the free page
 * list. So we need a bucket for each power of two between
 * RTHEAP_MINLOG2 and RTHEAP_MAXLOG2 inclusive, plus one to honor
 * requests ranging from the maximum page size to twice this size.
 */

#define	RTHEAP_MINLOG2    4
#define	RTHEAP_MAXLOG2    22
#define	RTHEAP_MINALLOCSZ (1 << RTHEAP_MINLOG2)
#define	RTHEAP_MINALIGNSZ RTHEAP_MINALLOCSZ
#define	RTHEAP_NBUCKETS   (RTHEAP_MAXLOG2 - RTHEAP_MINLOG2 + 2)
#define	RTHEAP_MAXEXTSZ   0x7FFFFFFF
#define RTHEAP_GLOBALSZ   (CONFIG_RTAI_MALLOC_HEAPSZ * 1024)

#define RTHEAP_PFREE   0
#define RTHEAP_PCONT   1
#define RTHEAP_PLIST   2

#define KMALLOC_LIMIT  (128 * 1024)

#define RTHEAP_NOMEM   (-1)
#define RTHEAP_PARAM   (-2)

typedef struct rtextent {

    struct list_head link;

    caddr_t membase,	/* Base address of the page array */
	    memlim,	/* Memory limit of page array */
	    freelist;	/* Head of the free page list */

    u_char pagemap[1];	/* Beginning of page map */

} rtextent_t;

/* Creation flag */
#define RTHEAP_EXTENDABLE 0x1

/* Allocation flags */
#define RTHEAP_EXTEND    0x1

typedef struct rtheap {

    spinlock_t lock;

    int flags;

    u_long extentsize,
           pagesize,
           pageshift,
	   hdrsize,
	   npages,	/* Number of pages per extent */
	   ubytes,
           maxcont;

    struct list_head extents;

    caddr_t buckets[RTHEAP_NBUCKETS];

} rtheap_t;

#else /* __cplusplus */

struct rtheap;

typedef struct rtheap rtheap_t;

#endif /* !__cplusplus */

extern rtheap_t rtai_global_heap;

#define rtheap_page_size(heap)   ((heap)->pagesize)
#define rtheap_page_count(heap)  ((heap)->npages)
#define rtheap_used_mem(heap)    ((heap)->ubytes)

#ifdef CONFIG_RTAI_MALLOC
#define rt_malloc(sz)  rtheap_alloc(&rtai_global_heap, sz, 0)
#define rt_free(p)     rtheap_free(&rtai_global_heap, p)
#else
#define rt_malloc(sz)  kmalloc(sz, GFP_KERNEL)
#define rt_free(p)     kfree(p)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int __rtai_heap_init(void);

void __rtai_heap_exit(void);

int rtheap_init(rtheap_t *heap, void *heapaddr, u_long heapsize, u_long pagesize, int suprt);

void rtheap_destroy(rtheap_t *heap, int suprt);

void *rtheap_alloc(rtheap_t *heap, u_long size, int flags);

int rtheap_free(rtheap_t *heap, void *block);

#ifdef __cplusplus
}
#endif

#endif /* __KERNEL__ */

#endif /* !_RTAI_MALLOC_H */
