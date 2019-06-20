/*
 * Copyright (C) 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#if 0  // B E G I N  U N U S E D

#ifndef _RTAI_ASM_I386_SHM_H
#define _RTAI_ASM_I386_SHM_H

#ifndef __KERNEL__

#include <asm/rtai_vectors.h>

static inline long long rtai_shmrq(int srq, unsigned long args)
{
	long long retval;
	RTAI_DO_TRAP(RTAI_SYS_VECTOR, retval, srq, args);
	return retval;
}

#endif /* __KERNEL__ */

#include <asm/pgtable.h>
#include <asm/io.h>
#include <rtai_wrappers.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define VMALLOC_VMADDR(x) ((unsigned long)(x))
#endif /* >= 2.6.0 */

/* convert virtual user memory address to physical address */
/* (virt_to_phys only works for kmalloced kernel memory) */

static inline unsigned long uvirt_to_kva(pgd_t *pgd, unsigned long adr)
{
	if (!pgd_none(*pgd) && !pgd_bad(*pgd)) {
		pmd_t *pmd;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
		pmd = pmd_offset(pgd, adr);
#else /* >= 2.6.11 */
		pmd = pmd_offset(pud_offset(pgd, adr), adr);
#endif /* < 2.6.11 */
		if (!pmd_none(*pmd)) {
			pte_t *ptep, pte;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			ptep = pte_offset(pmd, adr);
#else /* >= 2.6.0 */
			ptep = pte_offset_kernel(pmd, adr);
#endif /* < 2.6.0 */
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

static inline unsigned long uvirt_to_bus(unsigned long adr)
{
	return virt_to_bus((void *)uvirt_to_kva(pgd_offset(current->mm, adr), adr));
}

static inline unsigned long kvirt_to_bus(unsigned long adr)
{
	unsigned long va;

	va = VMALLOC_VMADDR(adr);
	return virt_to_bus((void *)uvirt_to_kva(pgd_offset_k(va), va));
}

#endif  /* !_RTAI_ASM_I386_SHM_H */

#endif // E N D  O F  U N U S E D

