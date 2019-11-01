/**
 * @ingroup shm
 * @file
 *
 * Interface of the @ref shm "RTAI SHM module".
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * @note Copyright &copy; 2019 Alec Ari <neotheuser@ymail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
ACKNOWLEDGMENTS:
- The suggestion and the code for mmapping at a user specified address is due to  Trevor Woolven (trevw@zentropix.com).
*/


#ifndef _RTAI_SHM_H
#define _RTAI_SHM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup shm
 *@{*/

#define GLOBAL_HEAP_ID  0x9ac6d9e7  // nam2num("RTGLBH");

#define USE_VMALLOC     0
#define USE_GFP_KERNEL  1
#define USE_GFP_ATOMIC  2
#define USE_GFP_DMA     3

/**
 * Allocate a chunk of memory to be shared inter-intra kernel modules and 
 * Linux processes.
 *
 * @internal
 * 
 * rtai_kalloc is used to allocate shared memory from kernel space.
 * 
 * @param name is an unsigned long identifier;
 * 
 * @param size is the amount of required shared memory;
 * 
 * rtai_kmalloc is a legacy helper macro, the real job is carried out by a
 * call to rt_shm_alloc() with the same name, size and with vmalloc support.
 * This function should not be used in newly developed applications. See 
 * rt_shm_alloc for more details.
 *
 * @returns a valid address on succes, 0 on failure.
 *
 */

#define rtai_kmalloc(name, size) \
	rt_shm_alloc(name, size, USE_VMALLOC)  // legacy

/**
 * Free a chunk of shared memory being shared inter-intra kernel modules and 
 * Linux processes.
 *
 * rtai_kfree is used to free a shared memory chunk from kernel space.
 *
 * @param name is the unsigned long identifier used when the memory was
 * allocated;
 *
 * rtai_kfree is a legacy helper macro, the real job is carried out by a
 * call to rt_shm_free with the same name. This function should not be used 
 * in newly developed applications. See rt_shm_free for more details.
 *
 * @returns the size of the succesfully freed memory, 0 on failure.
 *
 */

#define rtai_kfree(name) \
	rt_shm_free(name)  // legacy

#if defined(__KERNEL__)

#include <linux/module.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

#define UVIRT_TO_KVA(adr)  uvirt_to_kva(pgd_offset_k(adr), (adr))

static inline int remap_page_range(struct vm_area_struct *vma, unsigned long uvaddr, unsigned long paddr, unsigned long size, pgprot_t prot)
{
	return remap_pfn_range(vma, uvaddr, paddr >> PAGE_SHIFT, size, prot);
}

#include <rtai.h>

#include <rtai_malloc.h>

static inline unsigned long uvirt_to_kva(pgd_t *pgd, unsigned long adr)
{
	if (!pgd_none(*pgd) && !pgd_bad(*pgd)) {
		pmd_t *pmd;
		pmd = pmd_offset(pud_offset((void *)pgd, adr), adr);
		if (!pmd_none(*pmd)) {
			pte_t *ptep, pte;
			ptep = pte_offset_kernel(pmd, adr);
			pte = *ptep;
			if (pte_present(pte)) {
				return (((unsigned long)page_address(pte_page(pte))) | (adr & (PAGE_SIZE - 1)));
			}
		}
	}
	return 0UL;
}

static inline unsigned long kvirt_to_pa(unsigned long adr)
{
	return virt_to_phys((void *)uvirt_to_kva(pgd_offset_k(adr), adr));
}

int __rtai_shm_init(void);

void __rtai_shm_exit(void);

void *rt_shm_alloc(unsigned long name,
		   int size,
		   int suprt);

#define rt_shm_alloc_adr(adr, name, size) \
	rt_shm_alloc(name, size, suprt)

RTAI_SYSCALL_MODE int rt_shm_free(unsigned long name);

void *rt_heap_open(unsigned long name,
		   int size,
		   int suprt);

#define rt_heap_open_adr(adr, name, size, suprt) \
	rt_heap_open(name, size, suprt)

RTAI_SYSCALL_MODE void *rt_halloc(int size);

RTAI_SYSCALL_MODE void rt_hfree(void *addr);

RTAI_SYSCALL_MODE void *rt_named_halloc(unsigned long name, int size);

RTAI_SYSCALL_MODE void rt_named_hfree(void *addr);

void *rt_named_malloc(unsigned long name,
		      int size);

void rt_named_free(void *addr);

void *rvmalloc(unsigned long size);

void rvfree(void *mem,
	    unsigned long size);

int rvmmap(void *mem,
	   unsigned long memsize,
	   struct vm_area_struct *vma);

void *rkmalloc(int *size,
	       int suprt);

void rkfree(void *mem,
	    unsigned long size);

int rkmmap(void *mem,
	   unsigned long memsize,
	   struct vm_area_struct *vma);

#else /* !__KERNEL__ */

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <rtai_lxrt.h>

//#define SHM_USE_LXRT

#define RTAI_SHM_DEV  "/dev/rtai_shm"

RTAI_PROTO (void *, _rt_shm_alloc, (void *start, unsigned long name, int size, int suprt, int isheap))
{
	int hook;
	void *adr = NULL;

	if ((hook = open(RTAI_SHM_DEV, O_RDWR)) <= 0) {
		return NULL;
	} else {
		struct { unsigned long name; long arg, suprt; } arg = { name, size, suprt };
#ifdef SHM_USE_LXRT
		if ((size = rtai_lxrt(BIDX, SIZARG, SHM_ALLOC, &arg).i[LOW])) {
#else
		if ((size = ioctl(hook, SHM_ALLOC, (unsigned long)(&arg)))) {
#endif
			if ((adr = mmap(start, size, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_LOCKED, hook, 0)) == MAP_FAILED) {;
#ifdef SHM_USE_LXRT
				rtai_lxrt(BIDX, sizeof(name), SHM_FREE, &name);
#else
				ioctl(hook, SHM_FREE, &name);
#endif
			} else if (isheap) {
				arg.arg = (unsigned long)adr;
#ifdef SHM_USE_LXRT
				rtai_lxrt(BIDX, SIZARG, HEAP_SET, &arg);
#else
				ioctl(hook, HEAP_SET, &arg);
#endif
			}
		}
	}
	close(hook);
	return adr;
}

#define rt_shm_alloc(name, size, suprt)  \
	_rt_shm_alloc(0, name, size, suprt, 0)

#define rt_heap_open(name, size, suprt)  \
	_rt_shm_alloc(0, name, size, suprt, 1)

/**
 * Allocate a chunk of memory to be shared inter-intra kernel modules and 
 * Linux processes.
 *
 * @internal
 * 
 * rtai_malloc is used to allocate shared memory from user space.
 * 
 * @param name is an unsigned long identifier;
 * 
 * @param size is the amount of required shared memory;
 * 
 * rtai_malloc is a legacy helper macro, the real job is carried out by a
 * call to rt_shm_alloc() with the same name, size and with vmalloc support.
 * This function should not be used in newly developed applications. See
 * rt_shm_alloc fro more details.
 *
 * @returns a valid address on succes, on failure: 0 if it was unable to 
 * allocate any memory, MAP_FAILED if it was possible to allocate the
 * required memory but failed to mmap it to user space, in which case the
 * allocated memory is freed anyhow.
 *
 */

#define rtai_malloc(name, size)  \
	_rt_shm_alloc(0, name, size, USE_VMALLOC, 0)  // legacy

/**
 * Allocate a chunk of memory to be shared inter-intra kernel modules and Linux
 * processes.
 *
 * rt_shm_alloc_adr is used to allocate in user space.
 *
 * @param start_address is a user desired address where the allocated memory
 * should be mapped in user space;
 *
 * @param name is an unsigned long identifier;
 * 
 * @param size is the amount of required shared memory.
 *
 * @param suprt is the kernel allocation method to be used, it can be:
 * - USE_VMALLOC, use vmalloc;
 * - USE_GFP_KERNEL, use kmalloc with GFP_KERNEL;
 * - USE_GFP_ATOMIC, use kmalloc with GFP_ATOMIC;
 * - USE_GFP_DMA, use kmalloc with GFP_DMA.
 *
 * Since @c name can be a clumsy identifier, services are provided to
 * convert 6 characters identifiers to unsigned long, and vice versa.
 *
 * @see the functions nam2num() and num2nam().
 *
 * It must be remarked that only the very first call does a real allocation, 
 * any subsequent call to allocate with the same name from anywhere will just
 * increase the usage count and map the area to user space, or return the
 * related pointer to the already allocated space in kernel space. The function 
 * returns a pointer to the allocated memory, appropriately mapped to the memory
 * space in use. So if one is really sure that the named shared memory has been 
 * allocated already parameters size and suprt are not used and can be 
 * assigned any value.
 *
 * @note If the same process calls rtai_malloc_adr and rtai_malloc() twice in
 * the same process it get a zero return value on the second call.
 *
 * @returns a valid address on succes, on failure: 0 if it was unable to
 * allocate any memory, MAP_FAILED if it was possible to allocate the
 * required memory but failed to mmap it to user space, in which case the
 * allocated memory is freed anyhow.
 *
 */

#define rt_shm_alloc_adr(start_address, name, size, suprt)  \
	_rt_shm_alloc(start_address, name, size, suprt, 0)

#define rt_heap_open_adr(start, name, size, suprt)  \
	_rt_shm_alloc(start, name, size, suprt, 1)

/**
 * Allocate a chunk of memory to be shared inter-intra kernel modules and 
 * Linux processes.
 *
 * @internal
 * 
 * rtai_malloc_adr is used to allocate shared memory from user space.
 *
 * @param start_address is the adr were the shared memory should be mapped.
 * 
 * @param name is an unsigned long identifier;
 * 
 * @param size is the amount of required shared memory;
 * 
 * rtai_malloc_adr is a legacy helper macro, the real job is carried out by a
 * call to rt_shm_alloc_adr() with the same name, size and with vmalloc support.
 * This function should not be used in newly developed applications. See
 * rt_shm_alloc_adr for more details.
 *
 * @returns a valid address on succes, 0 on failure.
 *
 */

#define rtai_malloc_adr(start_address, name, size)  \
	_rt_shm_alloc(start_address, name, size, USE_VMALLOC, 0)  // legacy

RTAI_PROTO(int, rt_shm_free, (unsigned long name))
{
	int hook, size;
	struct { void *nameadr; } arg = { &name };
	if ((hook = open(RTAI_SHM_DEV, O_RDWR)) <= 0) {
		return 0;
	}
// no SHM_FREE needed, we release it all and munmap will do it through 
// the vma close operation provided by shm.c
#ifdef SHM_USE_LXRT
	if ((size = rtai_lxrt(BIDX, SIZARG, SHM_SIZE, &arg).i[LOW])) {
#else
	if ((size = ioctl(hook, SHM_SIZE, (unsigned long)&arg))) {
#endif
		if (munmap((void *)name, size)) {
			size = 0;
		}
	}
	close(hook);
	return size;
}

/**
 * Free a chunk of shared memory being shared inter-intra 
 * kernel modules and Linux processes.
 *
 * rtai_free is used to free a shared memory chunk from user space.
 *
 * @param name is the unsigned long identifier used when the memory was
 * allocated;
 *
 * @param adr is not used.
 *
 * rtai_free is a legacy helper macro, the real job is carried out by a
 * call to rt_shm_free with the same name. This function should not be used 
 * in newly developed applications. See rt_shm_alloc_adr for more details.
 *
 * @returns the size of the succesfully freed memory, 0 on failure.
 *
 */

#define rtai_free(name, adr)  \
	rt_shm_free(name)  // legacy

RTAI_PROTO(void *, rt_halloc, (int size))
{
	struct { long size; } arg = { size };
	return rtai_lxrt(BIDX, SIZARG, HEAP_ALLOC, &arg).v[LOW];
}

RTAI_PROTO(void, rt_hfree, (void *addr))
{
	struct { void *addr; } arg = { addr };
	rtai_lxrt(BIDX, SIZARG, HEAP_FREE, &arg);
}

RTAI_PROTO(void *, rt_named_halloc, (unsigned long name, int size))
{
	struct { unsigned long name; long size; } arg = { name, size };
	return rtai_lxrt(BIDX, SIZARG, HEAP_NAMED_ALLOC, &arg).v[LOW];
}

RTAI_PROTO(void, rt_named_hfree, (void *addr))
{
	struct { void *addr; } arg = { addr };
	rtai_lxrt(BIDX, SIZARG, HEAP_NAMED_FREE, &arg);
}

RTAI_PROTO(void *, rt_malloc, (int size))
{
	struct { long size; } arg = { size };
	return rtai_lxrt(BIDX, SIZARG, MALLOC, &arg).v[LOW];
}

RTAI_PROTO(void, rt_free, (void *addr))
{
	struct { void *addr; } arg = { addr };
	rtai_lxrt(BIDX, SIZARG, FREE, &arg);
}

RTAI_PROTO(void *, rt_named_malloc, (unsigned long name, int size))
{
	struct { unsigned long name; long size; } arg = { name, size };
	return rtai_lxrt(BIDX, SIZARG, NAMED_MALLOC, &arg).v[LOW];
}

RTAI_PROTO(void, rt_named_free, (void *addr))
{
	struct { void *addr; } arg = { addr };
	rtai_lxrt(BIDX, SIZARG, NAMED_FREE, &arg);
}

#endif /* __KERNEL__ */

/**
 * Close a real time group heap being shared inter-intra kernel modules and
 * Linux processes.
 *
 * @internal
 * 
 * rt_heap_close is used to close a previously opened real time group heap.
 *
 * @param name is the unsigned long identifier used to identify the heap.
 *
 * @param adr is not used.
 *
 * Analogously to what done by any allocation function this group real time
 * heap closing call have just the effect of decrementing a usage count, 
 * unmapping any user space heap being closed, till the last is done, as that 
 * is the one the really closes the group heap, freeing any allocated memory.
 *
 * @returns the size of the succesfully freed heap, 0 on failure.
 *
 */

#define rt_heap_close(name, adr)  rt_shm_free(name)

// aliases in use already, different heads different choices
#define rt_heap_init         rt_heap_open
#define rt_heap_create       rt_heap_open
#define rt_heap_acquire      rt_heap_open
#define rt_heap_init_adr     rt_heap_open_adr
#define rt_heap_create_adr   rt_heap_open_adr
#define rt_heap_acquire_adr  rt_heap_open_adr

#define rt_heap_delete       rt_heap_close
#define rt_heap_destroy      rt_heap_close
#define rt_heap_release      rt_heap_close

// these have no aliases, and never will

/**
 * Open the global real time heap to be shared inter-intra kernel modules and 
 * Linux processes.
 *
 * @internal
 * 
 * rt_global_heap_open is used to open the global real time heap.
 * 
 * The global heap is created by the shared memory module and its opening is
 * needed in user space to map it to the process address space. In kernel
 * space opening the global heap in a task is not required but should be done
 * anyhow, both for symmetry and to register its usage.
 *
 */

#define rt_global_heap_open()  rt_heap_open(GLOBAL_HEAP_ID, 0, 0)

/**
 * Close the global real time heap being shared inter-intra kernel modules and 
 * Linux processes.
 *
 * @internal
 * 
 * rt_global_heap_close is used to close the global real time heap.
 * 
 * Closing a global heap in user space has just the effect of deregistering
 * its use and unmapping the related memory from a process address space.
 * In kernel tasks just the deregistration is performed.
 * The global real time heap is destroyed just a the rmmoding of the shared
 * memory module.
 *
 */

#define rt_global_heap_close()  rt_heap_close(GLOBAL_HEAP_ID, 0)

/*@}*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_RTAI_SHM_H */
