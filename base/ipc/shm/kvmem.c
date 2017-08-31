/*
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/slab.h>

#include <rtai_shm.h>

#ifndef VM_RESERVED
#define VM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)
#endif

static __inline__ int vm_remap_page_range(struct vm_area_struct *vma, unsigned long from, unsigned long to)
{
	vma->vm_flags |= VM_RESERVED;
	return vm_insert_page(vma, from, vmalloc_to_page((void *)to));
}

static __inline__ int km_remap_page_range(struct vm_area_struct *vma, unsigned long from, unsigned long to, unsigned long size)
{
	vma->vm_flags |= VM_RESERVED;
	return mm_remap_page_range(vma, from, virt_to_phys((void *)to), size, PAGE_SHARED);
}

/* allocate user space mmapable block of memory in kernel space */
void *rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	if ((mem = vmalloc(size))) {
		adr = (unsigned long)mem;
		while (size > 0) {
//			mem_map_reserve(virt_to_page(UVIRT_TO_KVA(adr)));
			SetPageReserved(vmalloc_to_page((void *)adr));
			adr  += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
	return mem;
}

void rvfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if ((adr = (unsigned long)mem)) {
		while (size > 0) {
//			mem_map_unreserve(virt_to_page(UVIRT_TO_KVA(adr)));
			ClearPageReserved(vmalloc_to_page((void *)adr));
			adr  += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
		vfree(mem);
	}
}

/* this function will map (fragment of) rvmalloc'ed memory area to user space */
int rvmmap(void *mem, unsigned long memsize, struct vm_area_struct *vma)
{
	unsigned long pos, size, offset;
	unsigned long start  = vma->vm_start;

	/* this is not time critical code, so we check the arguments */
	/* vma->vm_offset HAS to be checked (and is checked)*/
	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) {
		return -EFAULT;
	}
	offset = vma->vm_pgoff << PAGE_SHIFT;
	size = vma->vm_end - start;
	if ((size + offset) > memsize) {
		return -EFAULT;
	}
	pos = (unsigned long)mem + offset;
	if (pos%PAGE_SIZE || start%PAGE_SIZE || size%PAGE_SIZE) {
		return -EFAULT;
	}
	while (size > 0) {
//		if (mm_remap_page_range(vma, start, kvirt_to_pa(pos), PAGE_SIZE, PAGE_SHARED)) {
		if (vm_remap_page_range(vma, start, pos)) {
			return -EAGAIN;
		}
		start += PAGE_SIZE;
		pos   += PAGE_SIZE;
		size  -= PAGE_SIZE;
	}
	return 0;
}

/* allocate user space mmapable block of memory in kernel space */
void *rkmalloc(int *msize, int suprt)
{
	unsigned long mem, adr, size;

	if (*msize <= KMALLOC_LIMIT) {
		mem = (unsigned long)kmalloc(*msize, suprt);
	} else {
		mem = (unsigned long)__get_free_pages(suprt, get_order(*msize));
	}
	if (mem) {
		adr  = PAGE_ALIGN(mem);
		size = *msize -= (adr - mem);
		while (size > 0) {
//			mem_map_reserve(virt_to_page(adr));
			SetPageReserved(virt_to_page(adr));
			adr  += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
	return (void *)mem;
}

void rkfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if ((adr = (unsigned long)mem)) {
		unsigned long sz = size;
		adr  = PAGE_ALIGN((unsigned long)mem);
		while (size > 0) {
//			mem_map_unreserve(virt_to_page(adr));
			ClearPageReserved(virt_to_page(adr));
			adr  += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
		if (sz <= KMALLOC_LIMIT) {
			kfree(mem);
		} else {
			free_pages((unsigned long)mem, get_order(sz));
		}
	}
}

/* this function will map an rkmalloc'ed memory area to user space */
int rkmmap(void *mem, unsigned long memsize, struct vm_area_struct *vma)
{
	unsigned long pos, size, offset;
	unsigned long start  = vma->vm_start;

	/* this is not time critical code, so we check the arguments */
	/* vma->vm_offset HAS to be checked (and is checked)*/
	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) {
		return -EFAULT;
	}
	offset = vma->vm_pgoff << PAGE_SHIFT;
	size = vma->vm_end - start;
	if ((size + offset) > memsize) {
		return -EFAULT;
	}
	pos = (unsigned long)mem + offset;
	if (pos%PAGE_SIZE || start%PAGE_SIZE || size%PAGE_SIZE) {
		return -EFAULT;
	}
//	if (mm_remap_page_range(vma, start, virt_to_phys((void *)pos), size, PAGE_SHARED)) {
	if (km_remap_page_range(vma, start, pos, size)) {
		return -EAGAIN;
	}
	return 0;
}
