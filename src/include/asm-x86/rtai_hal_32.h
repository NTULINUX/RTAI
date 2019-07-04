/**
 *   @ingroup hal
 *   @file
 *
 *   ARTI -- RTAI-compatible Adeos-based Real-Time Interface. Based on
 *   the original RTAI layer for x86.
 *
 *   Original RTAI/x86 layer implementation: \n
 *   Copyright &copy; 2000-2015 Paolo Mantegazza, \n
 *   Copyright &copy; 2000 Steve Papacharalambous, \n
 *   Copyright &copy; 2000 Stuart Hughes, \n
 *   and others.
 *
 *   RTAI/x86 rewrite over Adeos: \n
 *   Copyright &copy 2002 Philippe Gerum.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or 
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


#ifndef _RTAI_ASM_X86_32_HAL_H
#define _RTAI_ASM_X86_32_HAL_H

static __inline__ unsigned long ffnz (unsigned long word) {
    /* Derived from bitops.h's ffs() */
    __asm__("bsfl %1, %0"
	    : "=r" (word)
	    : "r"  (word));
    return word;
}

/* do_div below taken from Linux */
#ifndef do_div
#define do_div(n,base) ({ \
	unsigned long __upper, __low, __high, __mod, __base; \
	__base = (base); \
	asm("":"=a" (__low), "=d" (__high):"A" (n)); \
	__upper = __high; \
	if (__high) { \
		__upper = __high % (__base); \
		__high = __high / (__base); \
	} \
	asm("divl %2":"=a" (__low), "=d" (__mod):"rm" (__base), "0" (__low), "1" (__upper)); \
	asm("":"=A" (n):"a" (__low),"d" (__high)); \
	__mod; \
})
#endif

static inline unsigned long long rtai_ulldiv (unsigned long long ull, unsigned long uld, unsigned long *r)
{
	if (r) {
		*r = do_div(ull, uld);
	} else {
		do_div(ull, uld);
	}
	return ull;
}
#endif

static inline int rtai_imuldiv (int i, int mult, int div) {

    /* Returns (int)i = (int)i*(int)(mult)/(int)div. */
    
    int dummy;

    __asm__ __volatile__ ( \
	"mull %%edx\t\n" \
	"div %%ecx\t\n" \
	: "=a" (i), "=d" (dummy)
       	: "a" (i), "d" (mult), "c" (div));

    return i;
}

static inline long long rtai_llimd(long long ll, int mult, int div) {

    /* Returns (long long)ll = (int)ll*(int)(mult)/(int)div. */

    __asm__ __volatile ( \
	"movl %%edx,%%ecx\t\n" \
	"mull %%esi\t\n" \
	"movl %%eax,%%ebx\n\t" \
	"movl %%ecx,%%eax\t\n" \
        "movl %%edx,%%ecx\t\n" \
        "mull %%esi\n\t" \
	"addl %%ecx,%%eax\t\n" \
	"adcl $0,%%edx\t\n" \
        "divl %%edi\n\t" \
        "movl %%eax,%%ecx\t\n" \
        "movl %%ebx,%%eax\t\n" \
	"divl %%edi\n\t" \
	"sal $1,%%edx\t\n" \
        "cmpl %%edx,%%edi\t\n" \
        "movl %%ecx,%%edx\n\t" \
	"jge 1f\t\n" \
        "addl $1,%%eax\t\n" \
        "adcl $0,%%edx\t\n" \
	"1:\t\n" \
	: "=A" (ll) \
	: "A" (ll), "S" (mult), "D" (div) \
	: "%ebx", "%ecx");

    return ll;
}

#ifdef CONFIG_X86_TSC

//#define CONFIG_RTAI_DIAG_TSC_SYNC
#if 0 // Let's try the patch support, 32-64 bits independent

#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC) && defined(CONFIG_RTAI_TUNE_TSC_SYNC)
extern long long rtai_tsc_ofst[];
#define rtai_rdtsc() ({ unsigned long long t; __asm__ __volatile__( "rdtsc" : "=A" (t)); t + rtai_tsc_ofst[rtai_cpuid()]; })
#else
#define rtai_rdtsc() ({ unsigned long long t; __asm__ __volatile__( "rdtsc" : "=A" (t)); t; })
#endif

#else

#if defined(CONFIG_SMP) && defined(CONFIG_RTAI_DIAG_TSC_SYNC) && defined(CONFIG_RTAI_TUNE_TSC_SYNC)
extern long long rtai_tsc_ofst[];
#define rtai_rdtsc() ({ unsigned long long t; ipipe_read_tsc(t); t - rtai_tsc_ofst[rtai_cpuid()]; })
#else
#define rtai_rdtsc() ({ unsigned long long t; ipipe_read_tsc(t); t; })
#endif

#endif

#endif /* !_RTAI_ASM_X86_32_HAL_H */
