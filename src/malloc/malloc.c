/*!\file malloc.c
 * \brief Dynamic memory allocation services.
 *
 * Copyright (C) 2007 Paolo Mantegazza <mantegazza@aero.polimi.it>.
 * Copyright (C) 2019 Alec Ari <neotheuser@ymail.com>.
 * Specific following parts as copyrighted/licensed by their authors. 
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
 * \ingroup shm
 */

/*!
 * @addtogroup shm
 *@{*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/slab.h>

#include <rtai_config.h>
#include <asm/rtai.h>
#include <rtai_malloc.h>
#include <rtai_shm.h>

int rtai_global_heap_size = RTHEAP_GLOBALSZ;
RTAI_MODULE_PARM(rtai_global_heap_size, int);

void *rtai_global_heap_adr = NULL;

rtheap_t rtai_global_heap;	/* Global system heap */

static void *alloc_extent(u_long size, int suprt)
{
	caddr_t p;
	if (!suprt) {
		if ((p = (caddr_t)vmalloc(size))) {
			unsigned long _p = (unsigned long)p;
//			printk("RTAI[malloc]: vmalloced extent %p, size %lu.\n", p, size);
			for (; size > 0; size -= PAGE_SIZE, _p += PAGE_SIZE) {
//				mem_map_reserve(virt_to_page(UVIRT_TO_KVA(_p)));
				SetPageReserved(vmalloc_to_page((void *)_p));
			}
		}
	} else {
		if (size <= KMALLOC_LIMIT) {
			p = kmalloc(size, suprt);
		} else {
			p = (void*)__get_free_pages(suprt, get_order(size));
		}
//		p = (caddr_t)kmalloc(size, suprt);
//		printk("RTAI[malloc]: kmalloced extent %p, size %lu.\n", p, size);
	}
	if (p) {
		memset(p, 0, size);
	}
	return p;
}

static void free_extent(void *p, u_long size, int suprt)
{
	if (!suprt) {
		unsigned long _p = (unsigned long)p;

//		printk("RTAI[malloc]: vfreed extent %p, size %lu.\n", p, size);
		for (; size > 0; size -= PAGE_SIZE, _p += PAGE_SIZE) {
//			mem_map_unreserve(virt_to_page(UVIRT_TO_KVA(_p)));
			ClearPageReserved(vmalloc_to_page((void *)_p));
		}
		vfree(p);
	} else {
//		printk("RTAI[malloc]: kfreed extent %p, size %lu.\n", p, size);
//		kfree(p);
		if (size <= KMALLOC_LIMIT) {
			kfree(p);
		} else {
			free_pages((unsigned long)p, get_order(size));
		}
	}
}

#ifndef CONFIG_RTAI_USE_TLSF
#define CONFIG_RTAI_USE_TLSF 0
#endif

#if CONFIG_RTAI_USE_TLSF

static size_t init_memory_pool(size_t mem_pool_size, void *mem_pool);
static inline void *malloc_ex(size_t size, void *mem_pool);
static inline void free_ex(void *ptr, void *mem_pool);

static void init_extent(rtheap_t *heap, rtextent_t *extent)
{
	INIT_LIST_HEAD(&extent->link);
	extent->membase = (caddr_t)extent + sizeof(rtextent_t);
	extent->memlim  = (caddr_t)extent + heap->extentsize;
}

int rtheap_init(rtheap_t *heap, void *heapaddr, u_long heapsize, u_long pagesize, int suprt)
{
	rtextent_t *extent;

	INIT_LIST_HEAD(&heap->extents);
	spin_lock_init(&heap->lock);

	heap->extentsize = heapsize;
	if (!heapaddr && suprt) {
		if (heapsize <= KMALLOC_LIMIT || (heapaddr = alloc_extent(heapsize, suprt)) == NULL) {
			heap->extentsize = KMALLOC_LIMIT;
			heapaddr = NULL;
		}
	}

	if (heapaddr) {
		extent = (rtextent_t *)heapaddr;
		init_extent(heap, extent);
		list_add_tail(&extent->link, &heap->extents);
		return init_memory_pool(heapsize - sizeof(rtextent_t), heapaddr + sizeof(rtextent_t)) < 0 ? RTHEAP_NOMEM : 0;
	} else {
		u_long init_size = 0;
		while (init_size < heapsize) {
			if (!(extent = (rtextent_t *)alloc_extent(heap->extentsize, suprt)) || init_memory_pool(heap->extentsize - sizeof(rtextent_t), (void *)extent + sizeof(rtextent_t)) < 0) {
				struct list_head *holder, *nholder;
				list_for_each_safe(holder, nholder, &heap->extents) {
					extent = list_entry(holder, rtextent_t, link);
					free_extent(extent, heap->extentsize, suprt);
				}
				return RTHEAP_NOMEM;
			}
			init_extent(heap, extent);
			list_add_tail(&extent->link, &heap->extents);
			init_size += heap->extentsize;
		}
	}
	return 0;
}

void rtheap_destroy(rtheap_t *heap, int suprt)
{
	struct list_head *holder, *nholder;
	list_for_each_safe(holder, nholder, &heap->extents) {
		free_extent(list_entry(holder, rtextent_t, link), heap->extentsize, suprt);
	}
}

void *rtheap_alloc(rtheap_t *heap, u_long size, int mode)
{
	void *adr = NULL;
	struct list_head *holder;
	unsigned long flags;

	if (!size) {
		return NULL;
	}

	flags = rt_spin_lock_irqsave(&heap->lock);
	list_for_each(holder, &heap->extents) {
		if ((adr = malloc_ex(size, list_entry(holder, rtextent_t, link)->membase)) != NULL) {
			break;
		}
	}
	rt_spin_unlock_irqrestore(flags, &heap->lock);
	return adr;
}

int rtheap_free(rtheap_t *heap, void *block)
{
	unsigned long flags;
	struct list_head *holder;

	flags = rt_spin_lock_irqsave(&heap->lock);
	list_for_each(holder, &heap->extents) {
		rtextent_t *extent;
		extent = list_entry(holder, rtextent_t, link);
		if ((caddr_t)block < extent->memlim && (caddr_t)block >= extent->membase) {
			free_ex(block, extent->membase);
			break;
		}
	}
	rt_spin_unlock_irqrestore(flags, &heap->lock);
	return 0;
}

/******************************** BEGIN TLSF ********************************/

/* 
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 2.4.2
 *
 * Written by Miguel Masmano Tello <mimastel@doctor.upv.es>
 *
 * Thanks to Ismael Ripoll for his suggestions and reviews
 *
 * Copyright (C) 2008, 2007, 2006, 2005, 2004
 *
 * This code is released using a dual license strategy: GPL/LGPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of the GNU General Public License Version 2.0
 * Released under the terms of the GNU Lesser General Public License Version 2.1
 *
 * Adaption to RTAI by Paolo Mantegazza <mantegazza@aero.polimi.it>,
 * with TLSF downloaded from: http://rtportal.upv.es/rtmalloc/.
 *
 */

/*
 * Code contributions:
 *
 * (Jul 28 2007)  Herman ten Brugge <hermantenbrugge@home.nl>:
 *
 * - Add 64 bit support. It now runs on x86_64 and solaris64.
 * - I also tested this on vxworks/32and solaris/32 and i386/32 processors.
 * - Remove assembly code. I could not measure any performance difference 
 *   on my core2 processor. This also makes the code more portable.
 * - Moved defines/typedefs from tlsf.h to tlsf.c
 * - Changed MIN_BLOCK_SIZE to sizeof (free_ptr_t) and BHDR_OVERHEAD to 
 *   (sizeof (bhdr_t) - MIN_BLOCK_SIZE). This does not change the fact 
 *    that the minumum size is still sizeof 
 *   (bhdr_t).
 * - Changed all C++ comment style to C style. (// -> /.* ... *./)
 * - Used ls_bit instead of ffs and ms_bit instead of fls. I did this to 
 *   avoid confusion with the standard ffs function which returns 
 *   different values.
 * - Created set_bit/clear_bit fuctions because they are not present 
 *   on x86_64.
 * - Added locking support + extra file target.h to show how to use it.
 * - Added get_used_size function (REMOVED in 2.4)
 * - Added rtl_realloc and rtl_calloc function
 * - Implemented realloc clever support.
 * - Added some test code in the example directory.
 *        
 *
 * (Oct 23 2006) Adam Scislowicz: 
 *
 * - Support for ARMv5 implemented
 *
 */

/*#define USE_SBRK        (0) */
/*#define USE_MMAP        (0) */

#ifndef TLSF_USE_LOCKS
#define	TLSF_USE_LOCKS 	(0)
#endif

#ifndef TLSF_STATISTIC
#define	TLSF_STATISTIC 	(1)
#endif

#ifndef USE_MMAP
#define	USE_MMAP 	(0)
#endif

#ifndef USE_SBRK
#define	USE_SBRK 	(0)
#endif


#if TLSF_USE_LOCKS
#include "target.h"
#else
#define TLSF_CREATE_LOCK(_unused_)   do{}while(0)
#define TLSF_DESTROY_LOCK(_unused_)  do{}while(0) 
#define TLSF_ACQUIRE_LOCK(_unused_)  do{}while(0)
#define TLSF_RELEASE_LOCK(_unused_)  do{}while(0)
#endif

#if TLSF_STATISTIC
#define	TLSF_ADD_SIZE(tlsf, b) do {									\
		tlsf->used_size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;	\
		if (tlsf->used_size > tlsf->max_size) {						\
			tlsf->max_size = tlsf->used_size;						\
		} 										\
	} while(0)

#define	TLSF_REMOVE_SIZE(tlsf, b) do {								\
		tlsf->used_size -= (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;	\
	} while(0)
#else
#define	TLSF_ADD_SIZE(tlsf, b)	     do{}while(0)
#define	TLSF_REMOVE_SIZE(tlsf, b)    do{}while(0)
#endif

#if USE_MMAP || USE_SBRK
#include <unistd.h>
#endif

#if USE_MMAP
#include <sys/mman.h>
#endif

#if !defined(__GNUC__)
#ifndef __inline__
#define __inline__
#endif
#endif

/* The  debug functions  only can  be used  when _DEBUG_TLSF_  is set. */
#ifndef _DEBUG_TLSF_
#define _DEBUG_TLSF_  (0)
#endif

/*************************************************************************/
/* Definition of the structures used by TLSF */


/* Some IMPORTANT TLSF parameters */
/* Unlike the preview TLSF versions, now they are statics */
#define BLOCK_ALIGN (sizeof(void *) * 2)

#define MAX_FLI		(30)
#define MAX_LOG2_SLI	(5)
#define MAX_SLI		(1 << MAX_LOG2_SLI)     /* MAX_SLI = 2^MAX_LOG2_SLI */

#define FLI_OFFSET	(6)     /* tlsf structure just will manage blocks bigger */
/* than 128 bytes */
#define SMALL_BLOCK	(128)
#define REAL_FLI	(MAX_FLI - FLI_OFFSET)
#define MIN_BLOCK_SIZE	(sizeof (free_ptr_t))
#define BHDR_OVERHEAD	(sizeof (bhdr_t) - MIN_BLOCK_SIZE)
#define TLSF_SIGNATURE	(0x2A59FA59)

#define	PTR_MASK	(sizeof(void *) - 1)
#undef BLOCK_SIZE
#define BLOCK_SIZE	(0xFFFFFFFF - PTR_MASK)

#define GET_NEXT_BLOCK(_addr, _r) ((bhdr_t *) ((char *) (_addr) + (_r)))
#define	MEM_ALIGN		  ((BLOCK_ALIGN) - 1)
#define ROUNDUP_SIZE(_r)          (((_r) + MEM_ALIGN) & ~MEM_ALIGN)
#define ROUNDDOWN_SIZE(_r)        ((_r) & ~MEM_ALIGN)
#define ROUNDUP(_x, _v)           ((((~(_x)) + 1) & ((_v)-1)) + (_x))

#define BLOCK_STATE	(0x1)
#define PREV_STATE	(0x2)

/* bit 0 of the block size */
#define FREE_BLOCK	(0x1)
#define USED_BLOCK	(0x0)

/* bit 1 of the block size */
#define PREV_FREE	(0x2)
#define PREV_USED	(0x0)


#define DEFAULT_AREA_SIZE (1024*10)

#if USE_MMAP
#define PAGE_SIZE (getpagesize())
#endif

#define PRINT_MSG(fmt, args...) rt_printk(fmt, ## args)
#define ERROR_MSG(fmt, args...) rt_printk(fmt, ## args)

typedef unsigned int u32_t;     /* NOTE: Make sure that this type is 4 bytes long on your computer */
typedef unsigned char u8_t;     /* NOTE: Make sure that this type is 1 byte on your computer */

typedef struct free_ptr_struct {
    struct bhdr_struct *prev;
    struct bhdr_struct *next;
} free_ptr_t;

typedef struct bhdr_struct {
    /* This pointer is just valid if the first bit of size is set */
    struct bhdr_struct *prev_hdr;
    /* The size is stored in bytes */
    size_t size;                /* bit 0 indicates whether the block is used and */
    /* bit 1 allows to know whether the previous block is free */
    union {
        struct free_ptr_struct free_ptr;
        u8_t buffer[1];         /*sizeof(struct free_ptr_struct)]; */
    } ptr;
} bhdr_t;

/* This structure is embedded at the beginning of each area, giving us
 * enough information to cope with a set of areas */

typedef struct area_info_struct {
    bhdr_t *end;
    struct area_info_struct *next;
} area_info_t;

typedef struct TLSF_struct {
    /* the TLSF's structure signature */
    u32_t tlsf_signature;

#if TLSF_USE_LOCKS
    TLSF_MLOCK_T lock;
#endif

#if TLSF_STATISTIC
    /* These can not be calculated outside tlsf because we
     * do not know the sizes when freeing/reallocing memory. */
    size_t used_size;
    size_t max_size;
#endif

    /* A linked list holding all the existing areas */
    area_info_t *area_head;

    /* the first-level bitmap */
    /* This array should have a size of REAL_FLI bits */
    u32_t fl_bitmap;

    /* the second-level bitmap */
    u32_t sl_bitmap[REAL_FLI];

    bhdr_t *matrix[REAL_FLI][MAX_SLI];
} tlsf_t;


/******************************************************************/
/**************     Helping functions    **************************/
/******************************************************************/
static __inline__ void tlsf_set_bit(int nr, u32_t * addr);
static __inline__ void tlsf_clear_bit(int nr, u32_t * addr);
static __inline__ int ls_bit(int x);
static __inline__ int ms_bit(int x);
static __inline__ void MAPPING_SEARCH(size_t * _r, int *_fl, int *_sl);
static __inline__ void MAPPING_INSERT(size_t _r, int *_fl, int *_sl);
static __inline__ bhdr_t *FIND_SUITABLE_BLOCK(tlsf_t * _tlsf, int *_fl, int *_sl);
static __inline__ bhdr_t *process_area(void *area, size_t size);
#if USE_SBRK || USE_MMAP
static __inline__ void *get_new_area(size_t * size);
#endif

static const int table[] = {
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
    4, 4,
    4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5,
    5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6,
    6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6,
    6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7
};

static __inline__ int ls_bit(int i)
{
    unsigned int a;
    unsigned int x = i & -i;

    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
    return table[x >> a] + a;
}

static __inline__ int ms_bit(int i)
{
    unsigned int a;
    unsigned int x = (unsigned int) i;

    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
    return table[x >> a] + a;
}

static __inline__ void tlsf_set_bit(int nr, u32_t * addr)
{
    addr[nr >> 5] |= 1 << (nr & 0x1f);
}

static __inline__ void tlsf_clear_bit(int nr, u32_t * addr)
{
    addr[nr >> 5] &= ~(1 << (nr & 0x1f));
}

static __inline__ void MAPPING_SEARCH(size_t * _r, int *_fl, int *_sl)
{
    int _t;

    if (*_r < SMALL_BLOCK) {
        *_fl = 0;
        *_sl = *_r / (SMALL_BLOCK / MAX_SLI);
    } else {
        _t = (1 << (ms_bit(*_r) - MAX_LOG2_SLI)) - 1;
        *_r = *_r + _t;
        *_fl = ms_bit(*_r);
        *_sl = (*_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
        /*if ((*_fl -= FLI_OFFSET) < 0) // FL wil be always >0!
         *_fl = *_sl = 0;
         */
        *_r &= ~_t;
    }
}

static __inline__ void MAPPING_INSERT(size_t _r, int *_fl, int *_sl)
{
    if (_r < SMALL_BLOCK) {
        *_fl = 0;
        *_sl = _r / (SMALL_BLOCK / MAX_SLI);
    } else {
        *_fl = ms_bit(_r);
        *_sl = (_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
    }
}


static __inline__ bhdr_t *FIND_SUITABLE_BLOCK(tlsf_t * _tlsf, int *_fl, int *_sl)
{
    u32_t _tmp = _tlsf->sl_bitmap[*_fl] & (~0 << *_sl);
    bhdr_t *_b = NULL;

    if (_tmp) {
        *_sl = ls_bit(_tmp);
        _b = _tlsf->matrix[*_fl][*_sl];
    } else {
        *_fl = ls_bit(_tlsf->fl_bitmap & (~0 << (*_fl + 1)));
        if (*_fl > 0) {         /* likely */
            *_sl = ls_bit(_tlsf->sl_bitmap[*_fl]);
            _b = _tlsf->matrix[*_fl][*_sl];
        }
    }
    return _b;
}


#define EXTRACT_BLOCK_HDR(_b, _tlsf, _fl, _sl) {					\
		_tlsf -> matrix [_fl] [_sl] = _b -> ptr.free_ptr.next;		\
		if (_tlsf -> matrix[_fl][_sl])								\
			_tlsf -> matrix[_fl][_sl] -> ptr.free_ptr.prev = NULL;	\
		else {														\
			tlsf_clear_bit (_sl, &_tlsf -> sl_bitmap [_fl]);				\
			if (!_tlsf -> sl_bitmap [_fl])							\
				tlsf_clear_bit (_fl, &_tlsf -> fl_bitmap);				\
		}															\
		_b -> ptr.free_ptr = (free_ptr_t) {NULL, NULL};				\
	}


#define EXTRACT_BLOCK(_b, _tlsf, _fl, _sl) {							\
		if (_b -> ptr.free_ptr.next)									\
			_b -> ptr.free_ptr.next -> ptr.free_ptr.prev = _b -> ptr.free_ptr.prev; \
		if (_b -> ptr.free_ptr.prev)									\
			_b -> ptr.free_ptr.prev -> ptr.free_ptr.next = _b -> ptr.free_ptr.next; \
		if (_tlsf -> matrix [_fl][_sl] == _b) {							\
			_tlsf -> matrix [_fl][_sl] = _b -> ptr.free_ptr.next;		\
			if (!_tlsf -> matrix [_fl][_sl]) {							\
				tlsf_clear_bit (_sl, &_tlsf -> sl_bitmap[_fl]);				\
				if (!_tlsf -> sl_bitmap [_fl])							\
					tlsf_clear_bit (_fl, &_tlsf -> fl_bitmap);				\
			}															\
		}																\
		_b -> ptr.free_ptr = (free_ptr_t) {NULL, NULL};					\
	}

#define INSERT_BLOCK(_b, _tlsf, _fl, _sl) {								\
		_b -> ptr.free_ptr = (free_ptr_t) {NULL, _tlsf -> matrix [_fl][_sl]}; \
		if (_tlsf -> matrix [_fl][_sl])									\
			_tlsf -> matrix [_fl][_sl] -> ptr.free_ptr.prev = _b;		\
		_tlsf -> matrix [_fl][_sl] = _b;								\
		tlsf_set_bit (_sl, &_tlsf -> sl_bitmap [_fl]);						\
		tlsf_set_bit (_fl, &_tlsf -> fl_bitmap);								\
	}

#if USE_SBRK || USE_MMAP
static __inline__ void *get_new_area(size_t * size) 
{
    void *area;

#if USE_SBRK
    area = sbrk(0);
    if (sbrk(*size) != ((void *) ~0))
        return area;
#endif

#if USE_MMAP
    *size = ROUNDUP(*size, PAGE_SIZE);
    if ((area = mmap(0, *size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) != MAP_FAILED)
        return area;
#endif
    return ((void *) ~0);
}
#endif

static __inline__ bhdr_t *process_area(void *area, size_t size)
{
    bhdr_t *b, *lb, *ib;
    area_info_t *ai;

    ib = (bhdr_t *) area;
    ib->size =
        (sizeof(area_info_t) <
         MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(sizeof(area_info_t)) | USED_BLOCK | PREV_USED;
    b = (bhdr_t *) GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
    b->size = ROUNDDOWN_SIZE(size - 3 * BHDR_OVERHEAD - (ib->size & BLOCK_SIZE)) | USED_BLOCK | PREV_USED;
    b->ptr.free_ptr.prev = b->ptr.free_ptr.next = 0;
    lb = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    lb->prev_hdr = b;
    lb->size = 0 | USED_BLOCK | PREV_FREE;
    ai = (area_info_t *) ib->ptr.buffer;
    ai->next = 0;
    ai->end = lb;
    return ib;
}

/******************************************************************/
/******************** Begin of the allocator code *****************/
/******************************************************************/

/******************************************************************/
size_t init_memory_pool(size_t mem_pool_size, void *mem_pool)
{
/******************************************************************/
    char *mp = NULL;
    tlsf_t *tlsf;
    bhdr_t *b, *ib;

    if (!mem_pool || !mem_pool_size || mem_pool_size < sizeof(tlsf_t) + BHDR_OVERHEAD * 8) {
        ERROR_MSG("init_memory_pool (): memory_pool invalid\n");
        return -1;
    }

    if (((unsigned long) mem_pool & PTR_MASK)) {
        ERROR_MSG("init_memory_pool (): mem_pool must be aligned to a word\n");
        return -1;
    }
    tlsf = (tlsf_t *) mem_pool;
    /* Check if already initialised */
    if (tlsf->tlsf_signature == TLSF_SIGNATURE) {
        mp = mem_pool;
        b = GET_NEXT_BLOCK(mp, ROUNDUP_SIZE(sizeof(tlsf_t)));
        return b->size & BLOCK_SIZE;
    }

    mp = mem_pool;

    /* Zeroing the memory pool */
    memset(mem_pool, 0, sizeof(tlsf_t));

    tlsf->tlsf_signature = TLSF_SIGNATURE;

    TLSF_CREATE_LOCK(&tlsf->lock);

    ib = process_area(GET_NEXT_BLOCK
                      (mem_pool, ROUNDUP_SIZE(sizeof(tlsf_t))), ROUNDDOWN_SIZE(mem_pool_size - sizeof(tlsf_t)));
    b = GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
    free_ex(b->ptr.buffer, tlsf);
    tlsf->area_head = (area_info_t *) ib->ptr.buffer;

#if TLSF_STATISTIC
    tlsf->used_size = mem_pool_size - (b->size & BLOCK_SIZE);
    tlsf->max_size = tlsf->used_size;
#endif

    return (b->size & BLOCK_SIZE);
}

/******************************************************************/
void *malloc_ex(size_t size, void *mem_pool)
{
/******************************************************************/
    tlsf_t *tlsf = (tlsf_t *) mem_pool;
    bhdr_t *b, *b2, *next_b;
    int fl, sl;
    size_t tmp_size;

    size = (size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(size);

    /* Rounding up the requested size and calculating fl and sl */
    MAPPING_SEARCH(&size, &fl, &sl);

    /* Searching a free block, recall that this function changes the values of fl and sl,
       so they are not longer valid when the function fails */
    b = FIND_SUITABLE_BLOCK(tlsf, &fl, &sl);
#if USE_MMAP || USE_SBRK
    if (!b) {
        size_t area_size;
        void *area;
        /* Growing the pool size when needed */
        area_size = size + BHDR_OVERHEAD * 8;   /* size plus enough room for the requered headers. */
        area_size = (area_size > DEFAULT_AREA_SIZE) ? area_size : DEFAULT_AREA_SIZE;
        area = get_new_area(&area_size);        /* Call sbrk or mmap */
        if (area == ((void *) ~0))
            return NULL;        /* Not enough system memory */
        add_new_area(area, area_size, mem_pool);
        /* Rounding up the requested size and calculating fl and sl */
        MAPPING_SEARCH(&size, &fl, &sl);
        /* Searching a free block */
        b = FIND_SUITABLE_BLOCK(tlsf, &fl, &sl);
    }
#endif
    if (!b)
        return NULL;            /* Not found */

    EXTRACT_BLOCK_HDR(b, tlsf, fl, sl);

    /*-- found: */
    next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    /* Should the block be split? */
    tmp_size = (b->size & BLOCK_SIZE) - size;
    if (tmp_size >= sizeof(bhdr_t)) {
        tmp_size -= BHDR_OVERHEAD;
        b2 = GET_NEXT_BLOCK(b->ptr.buffer, size);
        b2->size = tmp_size | FREE_BLOCK | PREV_USED;
        next_b->prev_hdr = b2;
        MAPPING_INSERT(tmp_size, &fl, &sl);
        INSERT_BLOCK(b2, tlsf, fl, sl);

        b->size = size | (b->size & PREV_STATE);
    } else {
        next_b->size &= (~PREV_FREE);
        b->size &= (~FREE_BLOCK);       /* Now it's used */
    }

    TLSF_ADD_SIZE(tlsf, b);

    return (void *) b->ptr.buffer;
}

/******************************************************************/
void free_ex(void *ptr, void *mem_pool)
{
/******************************************************************/
    tlsf_t *tlsf = (tlsf_t *) mem_pool;
    bhdr_t *b, *tmp_b;
    int fl = 0, sl = 0;

    if (!ptr) {
        return;
    }
    b = (bhdr_t *) ((char *) ptr - BHDR_OVERHEAD);
    b->size |= FREE_BLOCK;

    TLSF_REMOVE_SIZE(tlsf, b);

    b->ptr.free_ptr = (free_ptr_t) { NULL, NULL};
    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    if (tmp_b->size & FREE_BLOCK) {
        MAPPING_INSERT(tmp_b->size & BLOCK_SIZE, &fl, &sl);
        EXTRACT_BLOCK(tmp_b, tlsf, fl, sl);
        b->size += (tmp_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
    }
    if (b->size & PREV_FREE) {
        tmp_b = b->prev_hdr;
        MAPPING_INSERT(tmp_b->size & BLOCK_SIZE, &fl, &sl);
        EXTRACT_BLOCK(tmp_b, tlsf, fl, sl);
        tmp_b->size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
        b = tmp_b;
    }
    MAPPING_INSERT(b->size & BLOCK_SIZE, &fl, &sl);
    INSERT_BLOCK(b, tlsf, fl, sl);

    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    tmp_b->size |= PREV_FREE;
    tmp_b->prev_hdr = b;
}

unsigned long tlsf_get_used_size(rtheap_t *heap) {
#if TLSF_STATISTIC
        struct list_head *holder;
        list_for_each(holder, &heap->extents) { break; }
	return ((tlsf_t *)(list_entry(holder, rtextent_t, link)->membase))->used_size;
#else
	return 0;
#endif
}

/******************************** END TLSF ********************************/

#else /* !CONFIG_RTAI_USE_TLSF */

/***************************** BEGIN BSD GPMA ********************************/

/*!\file malloc.c
 * \brief Dynamic memory allocation services.
 *
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
 * Dynamic memory allocation services lifted and adapted from the
 * Xenomai nucleus.
 *
 * This file implements the RTAI dynamic memory allocator based on the
 * algorithm described in "Design of a General Purpose Memory
 * Allocator for the 4.3BSD Unix Kernel" by Marshall K. McKusick and
 * Michael J. Karels.
 *
 * \ingroup shm
 */

/*!
 * @addtogroup shm
 *@{*/

static void init_extent (rtheap_t *heap, rtextent_t *extent)
{
	caddr_t freepage;
	int n, lastpgnum;

	INIT_LIST_HEAD(&extent->link);

	/* The page area starts right after the (aligned) header. */
	extent->membase = (caddr_t)extent + heap->hdrsize;
	lastpgnum = heap->npages - 1;

	/* Mark each page as free in the page map. */
	for (n = 0, freepage = extent->membase; n < lastpgnum; n++, freepage += heap->pagesize) {
		*((caddr_t *)freepage) = freepage + heap->pagesize;
		extent->pagemap[n] = RTHEAP_PFREE;
	}
	*((caddr_t *)freepage) = NULL;
	extent->pagemap[lastpgnum] = RTHEAP_PFREE;
	extent->memlim = freepage + heap->pagesize;

	/* The first page starts the free list of a new extent. */
	extent->freelist = extent->membase;
}

/*! 
 * \fn int rtheap_init(rtheap_t *heap,
                       void *heapaddr,
		       u_long heapsize,
		       u_long pagesize,
		       int suprt);
 * \brief Initialize a memory heap.
 *
 * Initializes a memory heap suitable for dynamic memory allocation
 * requests.  The heap manager can operate in two modes, whether
 * time-bounded if the heap storage area and size are statically
 * defined at initialization time, or dynamically extendable at the
 * expense of a less deterministic behaviour.
 *
 * @param heap The address of a heap descriptor the memory manager
 * will use to store the allocation data.  This descriptor must always
 * be valid while the heap is active therefore it must be allocated in
 * permanent memory.
 *
 * @param heapaddr The address of a statically-defined heap storage
 * area. If this parameter is non-zero, all allocations will be made
 * from the given area in fully time-bounded mode. In such a case, the
 * heap is non-extendable. If a null address is passed, the heap
 * manager will attempt to extend the heap each time a memory
 * starvation is encountered. In the latter case, the heap manager
 * will request additional chunks of core memory to Linux when needed,
 * voiding the real-time guarantee for the caller.
 *
 * @param heapsize If heapaddr is non-zero, heapsize gives the size in
 * bytes of the statically-defined storage area. Otherwise, heapsize
 * defines the standard length of each extent that will be requested
 * to Linux when a memory starvation is encountered for the heap.
 * heapsize must be a multiple of pagesize and lower than 16
 * Mbytes. Depending on the Linux allocation service used, requests
 * for extent memory might be limited in size. For instance, heapsize
 * must be lower than 128Kb for kmalloc()-based allocations. In the
 * current implementation, heapsize must be large enough to contain an
 * internal header. The following formula gives the size of this
 * header: hdrsize = (sizeof(rtextent_t) + ((heapsize -
 * sizeof(rtextent_t))) / (pagesize + 1) + 15) & ~15;
 *
 * @param pagesize The size in bytes of the fundamental memory page
 * which will be used to subdivide the heap internally. Choosing the
 * right page size is important regarding performance and memory
 * fragmentation issues, so it might be a good idea to take a look at
 * http://docs.FreeBSD.org/44doc/papers/kernmalloc.pdf to pick the
 * best one for your needs. In the current implementation, pagesize
 * must be a power of two in the range [ 8 .. 32768] inclusive.
 *
 * @return 0 is returned upon success, or one of the following
 * error codes:
 * - RTHEAP_PARAM is returned whenever a parameter is invalid.
 * - RTHEAP_NOMEM is returned if no initial extent can be allocated
 * for a dynamically extendable heap (i.e. heapaddr == NULL).
 *
 * Side-effect: This routine does not call the rescheduling procedure.
 *
 * Context: This routine must be called on behalf of a thread context.
 */

int rtheap_init (rtheap_t *heap, void *heapaddr, u_long heapsize, u_long pagesize, int suprt)
{
	u_long hdrsize, pmapsize, shiftsize, pageshift;
	rtextent_t *extent;
	int n;

	/*
	 * Perform some parametrical checks first.
	 * Constraints are:
	 * PAGESIZE must be >= 2 ** MINLOG2.
	 * PAGESIZE must be <= 2 ** MAXLOG2.
	 * PAGESIZE must be a power of 2.
	 * HEAPSIZE must be large enough to contain the static part of an
	 * extent header.
	 * HEAPSIZE must be a multiple of PAGESIZE.
	 * HEAPSIZE must be lower than RTHEAP_MAXEXTSZ.
	 */
	if ((pagesize < (1 << RTHEAP_MINLOG2)) ||
	    (pagesize > (1 << RTHEAP_MAXLOG2)) ||
	    (pagesize & (pagesize - 1)) != 0 ||
	    heapsize <= sizeof(rtextent_t) ||
	    heapsize > RTHEAP_MAXEXTSZ ||
	    (heapsize & (pagesize - 1)) != 0) {
		return RTHEAP_PARAM;
	}

	/* Determine the page map overhead inside the given extent
	   size. We need to reserve a byte in a page map for each page
	   which is addressable into this extent. The page map is itself
	   stored in the extent space, right after the static part of its
	   header, and before the first allocatable page. */
	pmapsize = ((heapsize - sizeof(rtextent_t)) * sizeof(u_char)) / (pagesize + sizeof(u_char));

	/* The overall header size is: static_part + page_map rounded to
	   the minimum alignment size. */
	hdrsize = (sizeof(rtextent_t) + pmapsize + RTHEAP_MINALIGNSZ - 1) & ~(RTHEAP_MINALIGNSZ - 1);

	/* An extent must contain at least two addressable pages to cope
	   with allocation sizes between pagesize and 2 * pagesize. */
	if (hdrsize + 2 * pagesize > heapsize) {
		return RTHEAP_PARAM;
	}

	/* Compute the page shiftmask from the page size (i.e. log2 value). */
	for (pageshift = 0, shiftsize = pagesize; shiftsize > 1; shiftsize >>= 1, pageshift++);

	heap->pagesize   = pagesize;
	heap->pageshift  = pageshift;
	heap->hdrsize    = hdrsize;

	heap->extentsize = heapsize;
	if (!heapaddr && suprt) {
		if (heapsize <= KMALLOC_LIMIT || (heapaddr = alloc_extent(heapsize, suprt)) == NULL) {
			heap->extentsize = KMALLOC_LIMIT;
			heapaddr = NULL;
		}
	}

	heap->npages     = (heap->extentsize - hdrsize) >> pageshift;
	heap->maxcont    = heap->npages*pagesize;
	heap->flags      =
	heap->ubytes     = 0;
	INIT_LIST_HEAD(&heap->extents);
	spin_lock_init(&heap->lock);

	for (n = 0; n < RTHEAP_NBUCKETS; n++) {
		heap->buckets[n] = NULL;
	}

	if (heapaddr) {
		extent = (rtextent_t *)heapaddr;
		init_extent(heap, extent);
		list_add_tail(&extent->link, &heap->extents);
	} else {
		u_long init_size = 0;
		while (init_size < heapsize) {
			if (!(extent = (rtextent_t *)alloc_extent(heap->extentsize, suprt))) {
				struct list_head *holder, *nholder;
				list_for_each_safe(holder, nholder, &heap->extents) {
					extent = list_entry(holder, rtextent_t, link);
					free_extent(extent, heap->extentsize, suprt);
				}
				return RTHEAP_NOMEM;
			}
			init_extent(heap, extent);
			list_add_tail(&extent->link, &heap->extents);
			init_size += heap->extentsize;
		}
	}
	return 0;
}

/*! 
 * \fn void rtheap_destroy(rtheap_t *heap);
 * \brief Destroys a memory heap.
 *
 * Destroys a memory heap. Dynamically allocated extents are returned
 * to Linux.
 *
 * @param heap The descriptor address of the destroyed heap.
 *
 * Side-effect: This routine does not call the rescheduling procedure.
 *
 * Context: This routine must be called on behalf of a thread context.
 */

void rtheap_destroy (rtheap_t *heap, int suprt)
{
	struct list_head *holder, *nholder;

	list_for_each_safe(holder, nholder, &heap->extents) {
		free_extent(list_entry(holder, rtextent_t, link), heap->extentsize, suprt);
	}
}

/*
 * get_free_range() -- Obtain a range of contiguous free pages to
 * fulfill an allocation of 2 ** log2size. Each extent is searched,
 * and a new one is allocated if needed, provided the heap is
 * extendable. Must be called with the heap lock set.
 */

static caddr_t get_free_range (rtheap_t *heap,
			       u_long bsize,
			       int log2size,
			       int mode)
{
    caddr_t block, eblock, freepage, lastpage, headpage, freehead = NULL;
    u_long pagenum, pagecont, freecont;
    struct list_head *holder;
    rtextent_t *extent;

    list_for_each(holder,&heap->extents) {

	extent = list_entry(holder,rtextent_t,link);
	freepage = extent->freelist;

	while (freepage != NULL)
	    {
	    headpage = freepage;
    	    freecont = 0;

	    /* Search for a range of contiguous pages in the free page
	       list of the current extent. The range must be 'bsize'
	       long. */
	    do
		{
		lastpage = freepage;
		freepage = *((caddr_t *)freepage);
		freecont += heap->pagesize;
		}
	    while (freepage == lastpage + heap->pagesize && freecont < bsize);

	    if (freecont >= bsize)
		{
		/* Ok, got it. Just update the extent's free page
		   list, then proceed to the next step. */

		if (headpage == extent->freelist)
		    extent->freelist = *((caddr_t *)lastpage);
		else   
		    *((caddr_t *)freehead) = *((caddr_t *)lastpage);

		goto splitpage;
		}

	    freehead = lastpage;
	    }
    }

    /* No available free range in the existing extents so far. If we
       cannot extend the heap, we have failed and we are done with
       this request. */

    return NULL;

splitpage:

    /* At this point, headpage is valid and points to the first page
       of a range of contiguous free pages larger or equal than
       'bsize'. */

    if (bsize < heap->pagesize)
	{
	/* If the allocation size is smaller than the standard page
	   size, split the page in smaller blocks of this size,
	   building a free list of free blocks. */

	for (block = headpage, eblock = headpage + heap->pagesize - bsize;
	     block < eblock; block += bsize)
	    *((caddr_t *)block) = block + bsize;

	*((caddr_t *)eblock) = NULL;
	}
    else   
        *((caddr_t *)headpage) = NULL;

    pagenum = (headpage - extent->membase) >> heap->pageshift;

    /* Update the extent's page map.  If log2size is non-zero
       (i.e. bsize <= 2 * pagesize), store it in the first page's slot
       to record the exact block size (which is a power of
       two). Otherwise, store the special marker RTHEAP_PLIST,
       indicating the start of a block whose size is a multiple of the
       standard page size, but not necessarily a power of two.  In any
       case, the following pages slots are marked as 'continued'
       (PCONT). */

    extent->pagemap[pagenum] = log2size ? log2size : RTHEAP_PLIST;

    for (pagecont = bsize >> heap->pageshift; pagecont > 1; pagecont--)
	extent->pagemap[pagenum + pagecont - 1] = RTHEAP_PCONT;

    return headpage;
}

/*! 
 * \fn void *rtheap_alloc(rtheap_t *heap, u_long size, int flags);
 * \brief Allocate a memory block from a memory heap.
 *
 * Allocates a contiguous region of memory from an active memory heap.
 * Such allocation is guaranteed to be time-bounded if the heap is
 * non-extendable (see rtheap_init()). Otherwise, it might trigger a
 * dynamic extension of the storage area through an internal request
 * to the Linux allocation service (kmalloc/vmalloc).
 *
 * @param heap The descriptor address of the heap to get memory from.
 *
 * @param size The size in bytes of the requested block. Sizes lower
 * or equal to the page size are rounded either to the minimum
 * allocation size if lower than this value, or to the minimum
 * alignment size if greater or equal to this value. In the current
 * implementation, with MINALLOC = 16 and MINALIGN = 16, a 15 bytes
 * request will be rounded to 16 bytes, and a 17 bytes request will be
 * rounded to 32.
 *
 * @param flags A set of flags affecting the operation. Unless
 * RTHEAP_EXTEND is passed and the heap is extendable, this service
 * will return NULL without attempting to extend the heap dynamically
 * upon memory starvation.
 *
 * @return The address of the allocated region upon success, or NULL
 * if no memory is available from the specified non-extendable heap,
 * or no memory can be obtained from Linux to extend the heap.
 *
 * Side-effect: This routine does not call the rescheduling procedure.
 *
 * Context: This routine can always be called on behalf of a thread
 * context. It can also be called on behalf of an IST context if the
 * heap storage area has been statically-defined at initialization
 * time (see rtheap_init()).
 */

void *rtheap_alloc (rtheap_t *heap, u_long size, int mode)

{
    u_long bsize, flags;
    caddr_t block;
    int log2size;

    if (size == 0)
	return NULL;

    if (size <= heap->pagesize)
	/* Sizes lower or equal to the page size are rounded either to
	   the minimum allocation size if lower than this value, or to
	   the minimum alignment size if greater or equal to this
	   value. In other words, with MINALLOC = 15 and MINALIGN =
	   16, a 15 bytes request will be rounded to 16 bytes, and a
	   17 bytes request will be rounded to 32. */
	{
	if (size <= RTHEAP_MINALIGNSZ)
	    size = (size + RTHEAP_MINALLOCSZ - 1) & ~(RTHEAP_MINALLOCSZ - 1);
	else
	    size = (size + RTHEAP_MINALIGNSZ - 1) & ~(RTHEAP_MINALIGNSZ - 1);
	}
    else
	/* Sizes greater than the page size are rounded to a multiple
	   of the page size. */
	size = (size + heap->pagesize - 1) & ~(heap->pagesize - 1);

    /* It becomes more space efficient to directly allocate pages from
       the free page list whenever the requested size is greater than
       2 times the page size. Otherwise, use the bucketed memory
       blocks. */

    if (size <= heap->pagesize * 2)
	{
	/* Find the first power of two greater or equal to the rounded
	   size. The log2 value of this size is also computed. */

	for (bsize = (1 << RTHEAP_MINLOG2), log2size = RTHEAP_MINLOG2;
	     bsize < size; bsize <<= 1, log2size++)
	    ; /* Loop */

	flags = rt_spin_lock_irqsave(&heap->lock);

	block = heap->buckets[log2size - RTHEAP_MINLOG2];

	if (block == NULL)
	    {
	    block = get_free_range(heap,bsize,log2size,mode);

	    if (block == NULL)
		goto release_and_exit;
	    }

	heap->buckets[log2size - RTHEAP_MINLOG2] = *((caddr_t *)block);
	heap->ubytes += bsize;
	}
    else
        {
        if (size > heap->maxcont)
            return NULL;

	flags = rt_spin_lock_irqsave(&heap->lock);

	/* Directly request a free page range. */
	block = get_free_range(heap,size,0,mode);

	if (block)   
	    heap->ubytes += size;
	}

release_and_exit:

    rt_spin_unlock_irqrestore(flags,&heap->lock);

    return block;
}

/*! 
 * \fn int rtheap_free(rtheap_t *heap, void *block);
 * \brief Release a memory block to a memory heap.
 *
 * Releases a memory region to the memory heap it was previously
 * allocated from.
 *
 * @param heap The descriptor address of the heap to release memory
 * to.
 *
 * @param block The address of the region to release returned by a
 * previous call to rtheap_alloc().
 *
 * @return 0 is returned upon success, or RTHEAP_PARAM is returned
 * whenever the block is not a valid region of the specified heap.
 *
 * Side-effect: This routine does not call the rescheduling procedure.
 *
 * Context: This routine can be called on behalf of a thread or IST
 * context
 */

int rtheap_free (rtheap_t *heap, void *block)

{
    u_long pagenum, pagecont, boffset, bsize, flags;
    caddr_t freepage, lastpage, nextpage, tailpage;
    rtextent_t *extent = NULL;
    struct list_head *holder;
    int log2size, npages;

    flags = rt_spin_lock_irqsave(&heap->lock);

    /* Find the extent from which the returned block is
       originating. If the heap is non-extendable, then a single
       extent is scanned at most. */

    list_for_each(holder,&heap->extents) {

        extent = list_entry(holder,rtextent_t,link);

	if ((caddr_t)block >= extent->membase &&
	    (caddr_t)block < extent->memlim)
	    break;
    }

    if (!holder)
	goto unlock_and_fail;

    /* Compute the heading page number in the page map. */
    pagenum = ((caddr_t)block - extent->membase) >> heap->pageshift;
    boffset = ((caddr_t)block - (extent->membase + (pagenum << heap->pageshift)));

    switch (extent->pagemap[pagenum])
	{
	case RTHEAP_PFREE: /* Unallocated page? */
	case RTHEAP_PCONT:  /* Not a range heading page? */

unlock_and_fail:

	    rt_spin_unlock_irqrestore(flags,&heap->lock);
	    return RTHEAP_PARAM;

	case RTHEAP_PLIST:

	    npages = 1;

	    while (npages < heap->npages &&
		   extent->pagemap[pagenum + npages] == RTHEAP_PCONT)
		npages++;

	    bsize = npages * heap->pagesize;

	    /* Link all freed pages in a single sub-list. */

	    for (freepage = (caddr_t)block,
		     tailpage = (caddr_t)block + bsize - heap->pagesize;
		 freepage < tailpage; freepage += heap->pagesize)
		*((caddr_t *)freepage) = freepage + heap->pagesize;

	    /* Mark the released pages as free in the extent's page map. */

	    for (pagecont = 0; pagecont < npages; pagecont++)
		extent->pagemap[pagenum + pagecont] = RTHEAP_PFREE;

	    /* Return the sub-list to the free page list, keeping
	       an increasing address order to favor coalescence. */
    
	    for (nextpage = extent->freelist, lastpage = NULL;
		 nextpage != NULL && nextpage < (caddr_t)block;
		 lastpage = nextpage, nextpage = *((caddr_t *)nextpage))
		; /* Loop */

	    *((caddr_t *)tailpage) = nextpage;

	    if (lastpage)
		*((caddr_t *)lastpage) = (caddr_t)block;
	    else
		extent->freelist = (caddr_t)block;

	    break;

	default:

	    log2size = extent->pagemap[pagenum];
	    bsize = (1 << log2size);

	    if ((boffset & (bsize - 1)) != 0) /* Not a block start? */
		goto unlock_and_fail;

	    /* Return the block to the bucketed memory space. */

	    *((caddr_t *)block) = heap->buckets[log2size - RTHEAP_MINLOG2];
	    heap->buckets[log2size - RTHEAP_MINLOG2] = block;

	    break;
	}

    heap->ubytes -= bsize;

    rt_spin_unlock_irqrestore(flags,&heap->lock);

    return 0;
}

/*
 * IMPLEMENTATION NOTES:
 *
 * The implementation follows the algorithm described in a USENIX
 * 1988 paper called "Design of a General Purpose Memory Allocator for
 * the 4.3BSD Unix Kernel" by Marshall K. McKusick and Michael
 * J. Karels. You can find it at various locations on the net,
 * including http://docs.FreeBSD.org/44doc/papers/kernmalloc.pdf.
 * A minor variation allows this implementation to have 'extendable'
 * heaps when needed, with multiple memory extents providing autonomous
 * page address spaces. When the non-extendable form is used, the heap
 * management routines show bounded worst-case execution time.
 *
 * The data structures hierarchy is as follows:
 *
 * HEAP {
 *      block_buckets[]
 *      extent_queue -------+
 * }                        |
 *                          V
 *                       EXTENT #1 {
 *                              <static header>
 *                              page_map[npages]
 *                              page_array[npages][pagesize]
 *                       } -+
 *                          |
 *                          |
 *                          V
 *                       EXTENT #n {
 *                              <static header>
 *                              page_map[npages]
 *                              page_array[npages][pagesize]
 *                       }
 */

/*@}*/

/****************************** END BSD GPMA ********************************/

#endif /* CONFIG_RTAI_USE_TLSF */

int __rtai_heap_init (void)
{
	rtai_global_heap_size = (rtai_global_heap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	if (rtheap_init(&rtai_global_heap, NULL, rtai_global_heap_size, PAGE_SIZE, 0)) {
		printk(KERN_INFO "RTAI[malloc]: failed to initialize the global heap (size=%d bytes).\n", rtai_global_heap_size);
		return 1;
	}
	rtai_global_heap_adr = rtai_global_heap.extents.next;
	printk(KERN_INFO "RTAI[malloc]: global heap size = %d bytes, <%s>.\n", rtai_global_heap_size, CONFIG_RTAI_USE_TLSF ? "TLSF" : "BSD");
	return 0;
}

void __rtai_heap_exit (void)
{
	rtheap_destroy(&rtai_global_heap, 0);
	printk("RTAI[malloc]: unloaded.\n");
}

#ifndef CONFIG_RTAI_MALLOC_BUILTIN
module_init(__rtai_heap_init);
module_exit(__rtai_heap_exit);
#endif /* !CONFIG_RTAI_MALLOC_BUILTIN */

EXPORT_SYMBOL(rtheap_init);
EXPORT_SYMBOL(rtheap_destroy);
EXPORT_SYMBOL(rtheap_alloc);
EXPORT_SYMBOL(rtheap_free);
EXPORT_SYMBOL(rtai_global_heap);
EXPORT_SYMBOL(rtai_global_heap_adr);
EXPORT_SYMBOL(rtai_global_heap_size);
