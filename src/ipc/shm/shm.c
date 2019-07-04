/**
 * @ingroup shm
 * @file
 *
 * Implementation of the @ref shm "RTAI SHM module".
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2004 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


/**
 * @defgroup shm Unified RTAI real-time memory management.
 * 
 *@{*/

#define RTAI_SHM_MISC_MINOR  254 // The same minor used to mknod for major 10.

#include <linux/version.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>

#include <rtai_trace.h>
#include <rtai_schedcore.h>
#include <rtai_registry.h>
#include "rtai_shm.h"

MODULE_LICENSE("GPL");

#define ALIGN2PAGE(adr)   ((void *)PAGE_ALIGN((unsigned long)adr))
#define RT_SHM_OP_PERM()  (!(_rt_whoami()->is_hard))

static int SUPRT[] = { 0, GFP_KERNEL, GFP_ATOMIC, GFP_DMA };

static inline void *_rt_shm_alloc(unsigned long name, int size, int suprt)
{
	void *adr;

//	suprt = USE_GFP_ATOMIC; // to force some testing
	if (!(adr = rt_get_adr_cnt(name)) && size > 0 && suprt >= 0 && RT_SHM_OP_PERM()) {
		size = ((size - 1) & PAGE_MASK) + PAGE_SIZE;
		if ((adr = suprt ? rkmalloc(&size, SUPRT[suprt]) : rvmalloc(size))) {
			if (!rt_register(name, adr, suprt ? -size : size, 0)) {
				if (suprt) {
                                        rkfree(adr, size);
                                } else {
                                        rvfree(adr, size);
                                }
				return 0;
			}
			memset(ALIGN2PAGE(adr), 0, size);
		}
	}
	return ALIGN2PAGE(adr);
}

static inline int _rt_shm_free(unsigned long name, int size)
{
	void *adr;

	if (size && (adr = rt_get_adr(name))) {
		if (RT_SHM_OP_PERM()) {
			if (!rt_drg_on_name_cnt(name) && name != GLOBAL_HEAP_ID) {
				if (size < 0) {
					rkfree(adr, -size);
				} else {
					rvfree(adr, size);
				}
			}
		}
		return abs(size);
	}
	return 0;
}

/**
 * Allocate a chunk of memory to be shared inter-intra kernel modules and Linux
 * processes.
 *
 * @internal
 * 
 * rt_shm_alloc is used to allocate shared memory.
 * 
 * @param name is an unsigned long identifier;
 * 
 * @param size is the amount of required shared memory;
 * 
 * @param suprt is the kernel allocation method to be used, it can be:
 * - USE_VMALLOC, use vmalloc;
 * - USE_GFP_KERNEL, use kmalloc with GFP_KERNEL;
 * - USE_GFP_ATOMIC, use kmalloc with GFP_ATOMIC;
 * - USE_GFP_DMA, use kmalloc with GFP_DMA.
 * 
 * Since @a name can be a clumsy identifier, services are provided to
 * convert 6 characters identifiers to unsigned long, and vice versa.
 * 
 * @see nam2num() and num2nam().
 * 
 * It must be remarked that only the very first call does a real allocation, 
 * any following call to allocate with the same name from anywhere will just 
 * increase the usage count and maps the area to the user space, or return 
 * the related pointer to the already allocated space in kernel space. 
 * In any case the functions return a pointer to the allocated memory, 
 * appropriately mapped to the memory space in use. So if one is really sure
 * that the named shared memory has been allocated already parameters size 
 * and suprt are not used and can be assigned any value.
 *
 * @returns a valid address on succes, 0 on failure.
 *
 */

void *rt_shm_alloc(unsigned long name, int size, int suprt)
{
	TRACE_RTAI_SHM(TRACE_RTAI_EV_SHM_KMALLOC, name, size, 0);
	return _rt_shm_alloc(name, size, suprt);
}

static RTAI_SYSCALL_MODE int rt_shm_alloc_usp(unsigned long name, int size, int suprt)
{
	TRACE_RTAI_SHM(TRACE_RTAI_EV_SHM_MALLOC, name, size, current->pid);

	if (_rt_shm_alloc(name, size, suprt)) {
		rtai_tskext(current, TSKEXT1) = (void *)name;
		return abs(rt_get_type(name));
	}
	return 0;
}

/**
 * Free a chunk of shared memory being shared inter-intra kernel modules and
 * Linux processes.
 *
 * @internal
 * 
 * rt_shm_free is used to free a previously allocated shared memory.
 *
 * @param name is the unsigned long identifier used when the memory was
 * allocated;
 *
 * Analogously to what done by all the named allocation functions the freeing 
 * calls have just the effect of decrementing a usage count, unmapping any 
 * user space shared memory being freed, till the last is done, as that is the 
 * one the really frees any allocated memory.
 *
 * @returns the size of the succesfully freed memory, 0 on failure.
 *
 */

RTAI_SYSCALL_MODE int rt_shm_free(unsigned long name)
{
	TRACE_RTAI_SHM(TRACE_RTAI_EV_SHM_KFREE, name, 0, 0);
	return _rt_shm_free(name, rt_get_type(name));
}

static RTAI_SYSCALL_MODE int rt_shm_size(unsigned long *arg)
{
	int size;
	struct vm_area_struct *vma;

	size = abs(rt_get_type(*arg));
	for (vma = (current->mm)->mmap; vma; vma = vma->vm_next) {
		if (vma->vm_private_data == (void *)*arg && (vma->vm_end - vma->vm_start) == size) {
			*arg = vma->vm_start;
			return size;
		}
	}
	return 0;
}

static void rtai_shm_vm_open(struct vm_area_struct *vma)
{
	rt_get_adr_cnt((unsigned long)vma->vm_private_data);
}

static void rtai_shm_vm_close(struct vm_area_struct *vma)
{
	_rt_shm_free((unsigned long)vma->vm_private_data, rt_get_type((unsigned long)vma->vm_private_data));
}

static struct vm_operations_struct rtai_shm_vm_ops = {
	open:  	rtai_shm_vm_open,
	close: 	rtai_shm_vm_close
};

#ifdef CONFIG_RTAI_MALLOC
static RTAI_SYSCALL_MODE void rt_set_heap(unsigned long, void *);
#endif

#ifdef HAVE_UNLOCKED_IOCTL
static long rtai_shm_f_ioctl(struct file *file, unsigned int cmd, unsigned long uarg)
#else
static int rtai_shm_f_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long uarg)
#endif
{
	switch (cmd) {
#ifdef CONFIG_RTAI_USE_STACK_ARGS
		case SHM_ALLOC: {
			TRACE_RTAI_SHM(TRACE_RTAI_EV_SHM_MALLOC, ((unsigned long *)uarg)[0], cmd, current->pid);
			return rt_shm_alloc_usp(((unsigned long *)uarg)[0], ((long *)uarg)[1], ((long *)uarg)[2]);
		}
		case SHM_FREE: {
			TRACE_RTAI_SHM(TRACE_RTAI_EV_SHM_FREE, uarg, cmd, current->pid);
			return _rt_shm_free(uarg, rt_get_type(uarg));	
		}
		case SHM_SIZE: {
			TRACE_RTAI_SHM(TRACE_RTAI_EV_SHM_GET_SIZE, uarg, cmd, current->pid);
			return rt_shm_size((unsigned long *)((unsigned long *)uarg)[0]);
		}
#ifdef CONFIG_RTAI_MALLOC
		case HEAP_SET: {
			rt_set_heap(((unsigned long *)uarg)[0], (void *)((unsigned long *)uarg)[1]);
			return 0;
		}
#endif
#else
		case SHM_ALLOC: {
			struct { unsigned long name; long arg, suprt; } arg;
			rt_copy_from_user(&arg, (void *)uarg, sizeof(arg));
			return rt_shm_alloc_usp(arg.name, arg.arg, arg.suprt);
		}
		case SHM_FREE: {
			unsigned long arg;
			rt_get_user(arg, (unsigned long *)uarg);
			return _rt_shm_free(arg, rt_get_type(arg));	
		}
		case SHM_SIZE: {
			unsigned long arg;
			rt_get_user(arg, (unsigned long *)uarg);
			rt_get_user(arg, (unsigned long *)arg);
			return rt_shm_size(&arg);
		}
#ifdef CONFIG_RTAI_MALLOC
		case HEAP_SET: {
			struct { unsigned long name; long arg; } arg;
			rt_copy_from_user(&arg, (void *)uarg, sizeof(arg));
			rt_set_heap(arg.name, (void *)arg.arg);
			return 0;
		}
#endif
#endif
	}
	return 0;
}

static int rtai_shm_f_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long name;
	int size;
	if (!vma->vm_ops) {
		vma->vm_ops = &rtai_shm_vm_ops;
		vma->vm_flags |= VM_LOCKED;
		name = (unsigned long)(vma->vm_private_data = rtai_tskext(current, TSKEXT1));
		rtai_tskext(current, TSKEXT1) = rtai_tskext(current, TSKEXT0) ? current : NULL;
		return (size = rt_get_type(name)) < 0 ? rkmmap(ALIGN2PAGE(rt_get_adr(name)), -size, vma) : rvmmap(rt_get_adr(name), size, vma);
	}
	return -EFAULT;
}

static struct file_operations rtai_shm_fops = {
#ifdef HAVE_UNLOCKED_IOCTL
	unlocked_ioctl:	rtai_shm_f_ioctl,
#else
	ioctl:		rtai_shm_f_ioctl,
#endif
	mmap:		rtai_shm_f_mmap
};

static struct miscdevice rtai_shm_dev = 
	{ RTAI_SHM_MISC_MINOR, "rtai_shm", &rtai_shm_fops };

#ifdef CONFIG_RTAI_MALLOC

static inline void *_rt_halloc(int size, struct rt_heap_t *heap)
{
	void *mem_ptr = NULL;

	if ((mem_ptr = rtheap_alloc(heap->heap, size, 0))) {
		mem_ptr = heap->uadr + (mem_ptr - heap->kadr);
	}
	return mem_ptr;
}

static inline void _rt_hfree(void *addr, struct rt_heap_t *heap)
{
	rtheap_free(heap->heap, heap->kadr + (addr - heap->uadr));
}

#define GLOBAL    0
#define SPECIFIC  1

/**
 * Allocate a chunk of the global real time heap in kernel/user space. Since 
 * it is not named there is no chance of retrieving and sharing it elsewhere.
 *
 * @internal
 * 
 * rt_malloc is used to allocate a non sharable piece of the global real time 
 * heap.
 *
 * @param size is the size of the requested memory in bytes;
 *
 * @returns the pointer to the allocated memory, 0 on failure.
 *
 */

/**
 * Free a chunk of the global real time heap.
 *
 * @internal
 * 
 * rt_free is used to free a previously allocated chunck of the global real 
 * time heap.
 *
 * @param addr is the addr of the memory to be freed.
 *
 */

/**
 * Allocate a chunk of the global real time heap in kernel/user space. Since 
 * it is named it can be retrieved and shared everywhere.
 *
 * @internal
 * 
 * rt_named_malloc is used to allocate a sharable piece of the global real 
 * time heap.
 *
 * @param name is an unsigned long identifier;
 * 
 * @param size is the amount of required shared memory;
 * 
 * Since @a name can be a clumsy identifier, services are provided to
 * convert 6 characters identifiers to unsigned long, and vice versa.
 * 
 * @see nam2num() and num2nam().
 * 
 * It must be remarked that only the very first call does a real allocation,
 * any subsequent call to allocate with the same name will just increase the
 * usage count and return the appropriate pointer to the already allocated 
 * memory having the same name. So if one is really sure that the named chunk 
 * has been allocated already the size parameter is not used and can be 
 * assigned any value.
 *
 * @returns a valid address on succes, 0 on failure.
 *
 */

void *rt_named_malloc(unsigned long name, int size)
{
	void *mem_ptr;

	if ((mem_ptr = rt_get_adr_cnt(name))) {
		return mem_ptr;
	}
	if ((mem_ptr = _rt_halloc(size, &rt_smp_linux_task->heap[GLOBAL]))) {
		if (rt_register(name, mem_ptr, IS_HPCK, 0)) {
                        return mem_ptr;
                }
                rt_hfree(mem_ptr);
	}
	return NULL;
}

/**
 * Free a named chunk of the global real time heap. 
 *
 * @internal
 * 
 * rt_named_free is used to free a previously allocated chunk of the global
 * real time heap.
 *
 * @param adr is the addr of the memory to be freed.
 *
 * Analogously to what done by all the named allocation functions the freeing 
 * calls of named memory chunks have just the effect of decrementing its usage
 * count, any shared piece of the global heap being freed only when the last 
 * is done, as that is the one the really frees any allocated memory.
 * So one must be carefull not to use rt_free on a named global heap chunk, 
 * since it will force its unconditional immediate freeing.
 *
 */

void rt_named_free(void *adr)
{
	unsigned long name;

	name = rt_get_name(adr);
	if (!rt_drg_on_name_cnt(name)) {
		_rt_hfree(adr, &rt_smp_linux_task->heap[GLOBAL]);
	}
}

/* 
 * we must care of this because LXRT callable functions are set as non 
 * blocking, so they are called directly.
 */
#define RTAI_TASK(return_instr) \
do { \
	if (!(task = _rt_whoami())->is_hard) { \
		if (!(task = rtai_tskext_t(current, TSKEXT0))) { \
			return_instr; \
		} \
	} \
} while (0)

static inline void *rt_halloc_typed(int size, int htype)
{
	RT_TASK *task;

	RTAI_TASK(return NULL);
	return _rt_halloc(size, &task->heap[htype]);
}

static inline void rt_hfree_typed(void *addr, int htype)
{
	RT_TASK *task;

	RTAI_TASK(return);
	_rt_hfree(addr, &task->heap[htype]);
}

static inline void *rt_named_halloc_typed(unsigned long name, int size, int htype)
{
	RT_TASK *task;
	void *mem_ptr;

	RTAI_TASK(return NULL);
	if ((mem_ptr = rt_get_adr_cnt(name))) {
		return task->heap[htype].uadr + (mem_ptr - task->heap[htype].kadr);
	}
	if ((mem_ptr = _rt_halloc(size, &task->heap[htype]))) {
		if (rt_register(name, task->heap[htype].kadr + (mem_ptr - task->heap[htype].uadr), IS_HPCK, 0)) {
                        return mem_ptr;
                }
		_rt_hfree(mem_ptr, &task->heap[htype]);
	}
	return NULL;
}

static inline void rt_named_hfree_typed(void *adr, int htype)
{
	RT_TASK *task;
	unsigned long name;

	RTAI_TASK(return);
	name = rt_get_name(task->heap[htype].kadr + (adr - task->heap[htype].uadr));
	if (!rt_drg_on_name_cnt(name)) {
		_rt_hfree(adr, &task->heap[htype]);
	}
}

/**
 * Allocate a chunk of a group real time heap in kernel/user space. Since 
 * it is not named there is no chance to retrieve and share it elsewhere.
 *
 * @internal
 * 
 * rt_halloc is used to allocate a non sharable piece of a group real time 
 * heap.
 *
 * @param size is the size of the requested memory in bytes;
 *
 * A process/task must have opened the real time group heap to use and can use
 * just one real time group heap. Be careful and avoid opening more than one 
 * group real time heap per process/task. If more than one is opened then just 
 * the last will used.
 *
 * @returns the pointer to the allocated memory, 0 on failure.
 *
 */

RTAI_SYSCALL_MODE void *rt_halloc(int size)
{
	return rt_halloc_typed(size, SPECIFIC);
}

/**
 * Free a chunk of a group real time heap.
 *
 * @internal
 * 
 * rt_hfree is used to free a previously allocated chunck of a group real 
 * time heap.
 *
 * @param adr is the addr of the memory to be freed.
 *
 */

RTAI_SYSCALL_MODE void rt_hfree(void *adr)
{
	rt_hfree_typed(adr, SPECIFIC);
}

/**
 * Allocate a chunk of a group real time heap in kernel/user space. Since 
 * it is named it can be retrieved and shared everywhere among the group 
 * peers, i.e all processes/tasks that have opened the same group heap.
 *
 * @internal
 * 
 * rt_named_halloc is used to allocate a sharable piece of a group real 
 * time heap.
 *
 * @param name is an unsigned long identifier;
 * 
 * @param size is the amount of required shared memory;
 * 
 * Since @a name can be a clumsy identifier, services are provided to
 * convert 6 characters identifiers to unsigned long, and vice versa.
 * 
 * @see nam2num() and num2nam().
 * 
 * A process/task must have opened the real time group heap to use and can use
 * just one real time group heap. Be careful and avoid opening more than one
 * group real time heap per process/task. If more than one is opened then just
 * the last will used. It must be remarked that only the very first call does 
 * a real allocation, any subsequent call with the same name will just 
 * increase the usage count and receive the appropriate pointer to the already
 * allocated memory having the same name.
 *
 * @returns a valid address on succes, 0 on failure.
 *
 */

RTAI_SYSCALL_MODE void *rt_named_halloc(unsigned long name, int size)
{
	return rt_named_halloc_typed(name, size, SPECIFIC);
}

/**
 * Free a chunk of a group real time heap. 
 *
 * @internal
 * 
 * rt_named_hfree is used to free a previously allocated chunk of the global
 * real time heap.
 *
 * @param adr is the address of the memory to be freed.
 *
 * Analogously to what done by all the named allocation functions the freeing 
 * calls of named memory chunks have just the effect of decrementing a usage
 * count, any shared piece of the global heap being freed only when the last 
 * is done, as that is the one the really frees any allocated memory.
 * So one must be carefull not to use rt_hfree on a named global heap chunk, 
 * since it will force its unconditional immediate freeing.
 *
 */

RTAI_SYSCALL_MODE void rt_named_hfree(void *adr)
{
	rt_named_hfree_typed(adr, SPECIFIC);
}

extern rtheap_t  rtai_global_heap;
extern void     *rtai_global_heap_adr;
extern int       rtai_global_heap_size;

static RTAI_SYSCALL_MODE void *rt_malloc_usp(int size)
{
	return rtai_global_heap_adr ? rt_halloc_typed(size, GLOBAL) : NULL;
}

static RTAI_SYSCALL_MODE void rt_free_usp(void *adr)
{
	if (rtai_global_heap_adr) {
		rt_hfree_typed(adr, GLOBAL);
	}
}

static RTAI_SYSCALL_MODE void *rt_named_malloc_usp(unsigned long name, int size)
{
	return rtai_global_heap_adr ? rt_named_halloc_typed(name, size, GLOBAL) : NULL;
}

static RTAI_SYSCALL_MODE void rt_named_free_usp(void *adr)
{
	if (rtai_global_heap_adr) {
		rt_named_hfree_typed(adr, GLOBAL);
	}
}

static RTAI_SYSCALL_MODE void rt_set_heap(unsigned long name, void *adr)
{
	void *heap, *hptr;
	int size;
	RT_TASK *task;

	heap = rt_get_adr(name);
	hptr = ALIGN2PAGE(heap);
	size = ((abs(rt_get_type(name)) - sizeof(rtheap_t) - (hptr - heap)) & PAGE_MASK);
	heap = hptr + size;
	if (!atomic_cmpxchg((atomic_t *)hptr, 0, name)) {
		rtheap_init(heap, hptr, size, PAGE_SIZE, 0);
	}
	RTAI_TASK(return);
	if (name == GLOBAL_HEAP_ID) {
		task->heap[GLOBAL].heap = &rtai_global_heap;
		task->heap[GLOBAL].kadr = rtai_global_heap_adr;
		task->heap[GLOBAL].uadr = adr;
	} else {
		task->heap[SPECIFIC].heap = heap;
		task->heap[SPECIFIC].kadr = hptr;
		task->heap[SPECIFIC].uadr = adr;
	}
}

/**
 * Open/create a named group real time heap to be shared inter-intra kernel 
 * modules and Linux processes.
 *
 * @internal
 * 
 * rt_heap_open is used to allocate open/create a shared real time heap.
 * 
 * @param name is an unsigned long identifier;
 * 
 * @param size is the amount of required shared memory;
 * 
 * @param suprt is the kernel allocation method to be used, it can be:
 * - USE_VMALLOC, use vmalloc;
 * - USE_GFP_KERNEL, use kmalloc with GFP_KERNEL;
 * - USE_GFP_ATOMIC, use kmalloc with GFP_ATOMIC;
 * - USE_GFP_DMA, use kmalloc with GFP_DMA.
 *
 * Since @a name can be a clumsy identifier, services are provided to
 * convert 6 characters identifiers to unsigned long, and vice versa.
 * 
 * @see nam2num() and num2nam().
 * 
 * It must be remarked that only the very first open does a real allocation, 
 * any subsequent one with the same name from anywhere will just map the area 
 * to the user space, or return the related pointer to the already allocated 
 * memory in kernel space. In any case the functions return a pointer to the 
 * allocated memory, appropriately mapped to the memory space in use.
 * Be careful and avoid opening more than one group heap per process/task, if 
 * more than one is opened then just the last will used.
 *
 * @returns a valid address on succes, 0 on failure.
 *
 */

void *rt_heap_open(unsigned long name, int size, int suprt)
{
	void *adr;
	if ((adr = rt_shm_alloc(name, ((size - 1) & PAGE_MASK) + PAGE_SIZE + sizeof(rtheap_t), suprt))) {
		rt_set_heap(name, adr);
		return adr;
	}
	return 0;
}

#endif

struct rt_native_fun_entry rt_shm_entries[] = {
        { { 0, rt_shm_alloc_usp },		SHM_ALLOC },
        { { 0, rt_shm_free },			SHM_FREE },
        { { 0, rt_shm_size },			SHM_SIZE },
#ifdef CONFIG_RTAI_MALLOC
        { { 0, rt_set_heap },			HEAP_SET},
        { { 0, rt_halloc },			HEAP_ALLOC },
        { { 0, rt_hfree },			HEAP_FREE },
        { { 0, rt_named_halloc },		HEAP_NAMED_ALLOC },
        { { 0, rt_named_hfree },		HEAP_NAMED_FREE },
        { { 0, rt_malloc_usp },			MALLOC },
        { { 0, rt_free_usp },			FREE },
        { { 0, rt_named_malloc_usp },		NAMED_MALLOC },
        { { 0, rt_named_free_usp },		NAMED_FREE },
#endif
        { { 0, 0 },				000 }
};

extern int set_rt_fun_entries(struct rt_native_fun_entry *entry);
extern void reset_rt_fun_entries(struct rt_native_fun_entry *entry);

int __rtai_shm_init (void)
{
	if (misc_register(&rtai_shm_dev) < 0) {
		printk("***** UNABLE TO REGISTER THE SHARED MEMORY DEVICE (miscdev minor: %d) *****\n", RTAI_SHM_MISC_MINOR);
		return -EBUSY;
	}
#ifdef CONFIG_RTAI_MALLOC
#ifdef CONFIG_RTAI_MALLOC_VMALLOC
	rt_register(GLOBAL_HEAP_ID, rtai_global_heap_adr, rtai_global_heap_size, 0);
	rt_smp_linux_task->heap[GLOBAL].heap = &rtai_global_heap;
	rt_smp_linux_task->heap[GLOBAL].kadr =
	rt_smp_linux_task->heap[GLOBAL].uadr = rtai_global_heap_adr;
#else
	printk("***** WARNING: GLOBAL HEAP NEITHER SHARABLE NOR USABLE FROM USER SPACE (use the vmalloc option for RTAI malloc) *****\n");
#endif
#endif
	return set_rt_fun_entries(rt_shm_entries);
}

void __rtai_shm_exit (void)
{
	extern int max_slots;
        int slot;
        struct rt_registry_entry entry;

#ifdef CONFIG_RTAI_MALLOC_VMALLOC
	rt_drg_on_name_cnt(GLOBAL_HEAP_ID);
#endif
	for (slot = 1; slot <= max_slots; slot++) {
		if (rt_get_registry_slot(slot, &entry)) {
			if (abs(entry.type) >= PAGE_SIZE) {
        			char name[8];
				while (_rt_shm_free(entry.name, entry.type));
                        	num2nam(entry.name, name);
	                        rt_printk("\nSHM_CLEANUP_MODULE releases: '%s':0x%lx:%lu (%d).\n", name, entry.name, entry.name, entry.type);
                        }
		}
	}
	reset_rt_fun_entries(rt_shm_entries);
	misc_deregister(&rtai_shm_dev);
	return;
}

/*@}*/

#ifndef CONFIG_RTAI_SHM_BUILTIN
module_init(__rtai_shm_init);
module_exit(__rtai_shm_exit);
#endif /* !CONFIG_RTAI_SHL_BUILTIN */

EXPORT_SYMBOL(rt_shm_alloc);
EXPORT_SYMBOL(rt_shm_free);
#ifdef CONFIG_RTAI_MALLOC
EXPORT_SYMBOL(rt_named_malloc);
EXPORT_SYMBOL(rt_named_free);
EXPORT_SYMBOL(rt_halloc);
EXPORT_SYMBOL(rt_hfree);
EXPORT_SYMBOL(rt_named_halloc);
EXPORT_SYMBOL(rt_named_hfree);
EXPORT_SYMBOL(rt_heap_open);
#endif
