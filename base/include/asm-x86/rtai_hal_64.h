/**
 *   @ingroup hal
 *   @file
 *
 *   ARTI -- RTAI-compatible Adeos-based Real-Time Interface. Based on
 *   the original RTAI layer for x86.
 *
 *   Original RTAI/x86 layer implementation: \n
 *   Copyright &copy; 2005-2015 Paolo Mantegazza, \n
 *   Copyright &copy; 2000 Steve Papacharalambous, \n
 *   Copyright &copy; 2000 Stuart Hughes, \n
 *   and others.
 *
 *   RTAI/x86 rewrite over Adeos: \n
 *   Copyright &copy 2002 Philippe Gerum.
 *
 *   Porting to x86_64 architecture:
 *   Copyright &copy; 2005-2015 Paolo Mantegazza, \n
 *   Copyright &copy; 2005      Daniele Gasperini \n
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation,  either version 2 of the License, or 
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @addtogroup hal
 *@{*/


#ifndef _RTAI_ASM_X86_64_HAL_H
#define _RTAI_ASM_X86_64_HAL_H

static __inline__ unsigned long ffnz (unsigned long word) {
    /* Derived from bitops.h's ffs() */
    __asm__("bsfq %1, %0"
	    : "=r" (word)
	    : "r"  (word));
    return word;
}

static inline unsigned long long rtai_ulldiv(unsigned long long ull, unsigned long uld, unsigned long *r)
{
	if (r) *r = ull%uld;
	return ull/uld;
}

static inline long rtai_imuldiv (long i, long mult, long div) {

    /* Returns (int)i = (int)i*(int)(mult)/(int)div. */
    
    int dummy;

    __asm__ __volatile__ ( \
	"mulq %%rdx\t\n" \
	"divq %%rcx\t\n" \
	: "=a" (i), "=d" (dummy)
       	: "a" (i), "d" (mult), "c" (div));

    return i;
}

static inline long long rtai_llimd(long long ll, long mult, long div) {
	return rtai_imuldiv(ll, mult, div);
}

#if 0 // Let's try the patch support, 32-64 bits independent
static inline unsigned long long rtai_rdtsc(void)
{
     unsigned int __a,__d;
     asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
     return ((unsigned long)__a) | (((unsigned long)__d)<<32);
}

#else
#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC) && defined(CONFIG_RTAI_TUNE_TSC_SYNC)
extern long long rtai_tsc_ofst[];
#define rtai_rdtsc() ({ unsigned long long t; ipipe_read_tsc(t); t + rtai_tsc_ofst[rtai_cpuid()]; })
#else
#define rtai_rdtsc() ({ unsigned long long t; ipipe_read_tsc(t); t; })
#endif
#endif

#endif /* !_RTAI_ASM_X86_64_HAL_H */
