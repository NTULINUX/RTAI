#if 1

/*
 *  rtai/libm/libm.c - module wrapper for SunSoft/FreeBSD/MacOX/uclibc libm
 *  RTAI - Real-Time Application Interface
 *  Copyright (C) 2001 David A. Schleef <ds@schleef.org>
 * 
 * Dave's idea modified (2013) by Paolo Mantegazza <mantegazza@aero.polimi.it>, 
 * so to use just the standard GPLed glibc, with the added possibility of 
 * calling both the float and double version of libm.a functions, complex
 * support included.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include "rtai_math.h"

MODULE_LICENSE("GPL");

#if CONFIG_RTAI_MATH_LIBM_TO_USE < 1 || CONFIG_RTAI_MATH_LIBM_TO_USE > 3
#error "***** 0 < CONFIG_RTAI_MATH_LIBM_TO_USE < 4 *****"
#endif

/***** Begin of libc entries needed by libm *****/

int stderr = 2;

#define kerrno_adr (&kerrno)

int *__getreent(void)
{
	return kerrno_adr;
}

int *_impure_ptr(void)
{
	return kerrno_adr;
}

int *__errno(void)
{
	return kerrno_adr;
}

int *__errno_location(void)
{
	return kerrno_adr;
}

void __assert_fail(const char *assertion, const char *file, int line, const char *function)
{
	printk("An '__assert_fail' assertion has been called.\n");
}

int fputs(const char *s, void *stream)
{ 
	return printk("%s\n", s);
}

#define generic_echo(buf, size) \
do { \
	char str[size + 1]; \
	memcpy(str, buf, size); \
	str[size] = 0; \
	return printk("%s\n", str); \
} while (0)

size_t fwrite(const void *ptr, size_t size, size_t nmemb, void *stream)
{
	generic_echo(ptr, size*nmemb);
}

ssize_t write(int fildes, const void *buf, size_t nbytes)
{
	generic_echo(buf, nbytes);
}

/***** End of libc entries needed by libm *****/

#if 1
void __stack_chk_fail(void)
{
	panic("rtai_math.ko stack-protector: Kernel stack is corrupted in: %p\n",
	__builtin_return_address(0));
}
#endif

int signgam;

#if CONFIG_RTAI_MATH_LIBM_TO_USE == 1 // NEWLIB
#include "export_newlib.h"
char using[] = "NEWLIB";
#endif // NEWLIB

#if CONFIG_RTAI_MATH_LIBM_TO_USE == 2 // UCLIBC-NG
#include "export_uclibc.h"
char using[] = "UCLIBC-NG";
#ifdef CONFIG_RTAI_MATH_KCOMPLEX
#ifdef CONFIG_X86_32
#if 1
#include "libm_cmplx.hc.gsl"
#else
#include "libm_cmplx.hc.musl"
#endif
#else
asmlinkage double _Complex clog(double _Complex x)
{
	return log(cabs(x)) + I*carg(x);
}
asmlinkage float _Complex clogf(float _Complex x)
{
	return logf(cabsf(x)) + I*cargf(x);
}
#endif
#endif
#endif // UCLIBC-NG

#if CONFIG_RTAI_MATH_LIBM_TO_USE == 3 // MUSL
#include "export_musl.h"
char using[] = "MUSL";
#ifdef CONFIG_RTAI_MATH_KCOMPLEX
// Hopefully a provisional fix. Till it is understood why a plain call of 
// the ones in MUSL libc.a keeps segfaulting for X86_32 (aka i386)
asmlinkage double _Complex cpow(double _Complex x, double _Complex y)
{
	return cexp(y*clog(x));
}

asmlinkage float _Complex cpowf(float _Complex x, float _Complex y)
{
	return cexpf(y*clogf(x));
}
// We have to provide them, till they are fixed in MUSL
asmlinkage double _Complex cacosh(double _Complex z)
{
	z = cacos(z);
	return cimag(z) > 0 ? __builtin_complex(cimag(z), -creal(z)) : __builtin_complex(-cimag(z), creal(z));
}
asmlinkage float _Complex cacoshf(float _Complex z)
{
	z = cacosf(z);
	return cimagf(z) > 0 ? __builtin_complex(cimagf(z), -crealf(z)) : __builtin_complex(-cimagf(z), crealf(z));
}
#endif
// Export gamma function not found in MUSL libc.a 
double gamma(double x)
{
	return lgamma(x);
}
EXPORT_SYMBOL(gamma);

double gamma_r(double x, int *signgamp)
{
	return lgamma_r(x, signgamp);
}
EXPORT_SYMBOL(gamma_r);

float gammaf(float x)
{
	return lgammaf(x);
}
EXPORT_SYMBOL(gammaf);

float gammaf_r(float x, int *signgamp)
{
	return lgammaf_r(x, signgamp);
}
EXPORT_SYMBOL(gammaf_r);
#endif // MUSL

int __rtai_math_init(void)
{
	printk(KERN_INFO "RTAI[math]: loaded, using %s.\n", using);
	return 0;
}

void __rtai_math_exit(void)
{
	printk(KERN_INFO "RTAI[math]: unloaded.\n");
}

module_init(__rtai_math_init);
module_exit(__rtai_math_exit);
  
EXPORT_SYMBOL(__fpclassify);
EXPORT_SYMBOL(__fpclassifyf);
EXPORT_SYMBOL(__signbit);
EXPORT_SYMBOL(__signbitf);

char *d2str(double d, int dgt, char *str)
{
	const int MAXDGT = 17;
	int e, i;
	unsigned long long l;
	double p;

	if (d < 0) {
		d = -d;
		str[0] = '-';
	} else {
		str[0] = '+';
	}
	str[1] = '0';
	str[2] = '.';
	i = fpclassify(d);
	if (i == FP_ZERO || i == FP_SUBNORMAL) {
		memset(&str[3], '0', dgt);
		strcpy(&str[dgt + 3], "e+00");
		return str;
	}
	if (i == FP_NAN) {
		strcpy(str, "NaN");
		return str;
	}
	if (i == FP_INFINITE) {
		strcpy(str, "Inf");
		return str;
	}
	if (dgt <= 0) {
		dgt = 1;
	} else if (dgt > MAXDGT) {
		dgt = MAXDGT;
	}
	e = log10(d);
	p = pow(10, MAXDGT - e);
	l = d*p + 0.5*pow(10, MAXDGT - dgt + (d >= 1));
	sprintf(&str[3], "%llu", l);
	i = dgt + 3;
	str[i] = 'e';
	if (e < 0) {
		e = -e;
		str[i + 1] = '-';
	} else {
		str[i + 1] = '+';
	}
	
	i = i + sprintf(&str[i + 2], "%d", e + (l/p >= 1));
	str[i + 2] = 0;
	return str;
}
EXPORT_SYMBOL(d2str);

#if defined(CONFIG_RTAI_MATH_KCOMPLEX) && (defined(_RTAI_EXPORT_NEWLIB_H) || defined(_RTAI_EXPORT_MUSL_H) || defined(_RTAI_EXPORT_UCLIBC_H))

char *cd2str(complex double cd, int dgt, char *str)
{
	int i;
	d2str(__real__ cd, dgt, str);
	i = strlen(str);
	str[i] = ' ';
#if 0 // two options to display the imaginary part
	d2str(__imag__ cd, dgt, &str[i + 4]);
	str[i + 1] = str[i + 4];
	str[i + 2] = ' ';
	str[i + 3] = 'j';
	str[i + 4] = '*';
#else
	d2str(__imag__ cd, dgt, &str[i + 1]);
	str[i = strlen(str)] = 'j';
	str[i + 1] = 0;
#endif
	return str;
}
EXPORT_SYMBOL(cd2str);

#if CONFIG_RTAI_MATH_LIBM_TO_USE == 1 || CONFIG_RTAI_MATH_LIBM_TO_USE == 2 || CONFIG_RTAI_MATH_LIBM_TO_USE == 3 // i.e. them all
asmlinkage double _Complex clog(double _Complex x);
asmlinkage double _Complex clog10(double _Complex x)
{
	const double one_over_lnof10 = 0.4342944819032518276454794132;
	return clog(x)*one_over_lnof10;
}
asmlinkage float _Complex clogf(float _Complex x);
asmlinkage float _Complex clog10f(float _Complex x)
{
	const float one_over_lnof10 = 0.4342944819032518276454794132;
	return clogf(x)*one_over_lnof10;
}
#endif

EXPORT_SYMBOL(cabs);
EXPORT_SYMBOL(cabsf);
EXPORT_SYMBOL(carg);
EXPORT_SYMBOL(cargf);
EXPORT_SYMBOL(cexp);
EXPORT_SYMBOL(cexpf);
EXPORT_SYMBOL(clog);
EXPORT_SYMBOL(clogf);
EXPORT_SYMBOL(clog10);
EXPORT_SYMBOL(clog10f);
#if CONFIG_RTAI_MATH_LIBM_TO_USE != 2 || defined(CONFIG_X86_32)
EXPORT_SYMBOL(cacos);
EXPORT_SYMBOL(cacosf);
EXPORT_SYMBOL(cacosh);
EXPORT_SYMBOL(cacoshf);
EXPORT_SYMBOL(casin);
EXPORT_SYMBOL(casinf);
EXPORT_SYMBOL(casinh);
EXPORT_SYMBOL(casinhf);
EXPORT_SYMBOL(catan);
EXPORT_SYMBOL(catanf);
EXPORT_SYMBOL(catanh);
EXPORT_SYMBOL(catanhf);
EXPORT_SYMBOL(ccos);
EXPORT_SYMBOL(ccosf);
EXPORT_SYMBOL(ccosh);
EXPORT_SYMBOL(ccoshf);
EXPORT_SYMBOL(cimag);
EXPORT_SYMBOL(cimagf);
EXPORT_SYMBOL(conj);
EXPORT_SYMBOL(conjf);
EXPORT_SYMBOL(cpow);
EXPORT_SYMBOL(cpowf);
EXPORT_SYMBOL(cproj);
EXPORT_SYMBOL(cprojf);
EXPORT_SYMBOL(creal);
EXPORT_SYMBOL(crealf);
EXPORT_SYMBOL(csin);
EXPORT_SYMBOL(csinf);
EXPORT_SYMBOL(csinh);
EXPORT_SYMBOL(csinhf);
EXPORT_SYMBOL(csqrt);
EXPORT_SYMBOL(csqrtf);
EXPORT_SYMBOL(ctan);
EXPORT_SYMBOL(ctanf);
EXPORT_SYMBOL(ctanh);
EXPORT_SYMBOL(ctanhf);
#endif

#if 1
// It is called, even if long double is not supported here; let's track it, to track what has to be done
double _Complex __mulxc3(long double a, long double b, long double c, long double d)
{
	printk(KERN_WARNING "***** __mulxc3 called by %s *****\n", using);
	return __muldc3((double)a, (double)b, (double)c, (double)d);
}
#endif

#define USE_COMPILER_QUALITY_CODE

#ifndef USE_COMPILER_QUALITY_CODE

/* 
 * This is the naive, but faster, approach.
 * No checks and readjustments for NAN and INFINITE
 * results.
 * If you want something closer to what a compiler
 * does, use the LLVM compiler infrastructure below.
 */

double _Complex __muldc3(double a, double b, double c, double d)
{
	double _Complex z;
	__real__ z = a*c - b*d; 
	__imag__ z = a*d + b*c; 
	return z;
}

float _Complex __mulsc3(float a, float b, float c, float d)
{
	float _Complex z;
	__real__ z = a*c - b*d; 
	__imag__ z = a*d + b*c; 
	return z;
}

double _Complex __divdc3(double a, double b, double c, double d)
{
	double _Complex z;
	double dn = c*c + d*d;
	__real__ z = (a*c + b*d)/dn; 
	__imag__ z = (-a*d + b*c)/dn; 
	return z;
}

float _Complex __divsc3(float a, float b, float c, float d)
{
	float _Complex z;
	float dn = c*c + d*d;
	__real__ z = (a*c + b*d)/dn; 
	__imag__ z = (-a*d + b*c)/dn; 
	return z;
}

#else

/* 
 * This is the LLVM compiler infrastructure.
 * It is an undeniably more serious, compiler quality, 
 * code, though it has some more overhead.
 */

/*
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 */

/* Returns: the product of a + ib and c + id */

double _Complex
__muldc3(double __a, double __b, double __c, double __d)
{
    double __ac = __a * __c;
    double __bd = __b * __d;
    double __ad = __a * __d;
    double __bc = __b * __c;
    double _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (isnan(__real__ z) && isnan(__imag__ z))
    {
        int __recalc = 0;
        if (isinf(__a) || isinf(__b))
        {
            __a = copysign(isinf(__a) ? 1 : 0, __a);
            __b = copysign(isinf(__b) ? 1 : 0, __b);
            if (isnan(__c))
                __c = copysign(0, __c);
            if (isnan(__d))
                __d = copysign(0, __d);
            __recalc = 1;
        }
        if (isinf(__c) || isinf(__d))
        {
            __c = copysign(isinf(__c) ? 1 : 0, __c);
            __d = copysign(isinf(__d) ? 1 : 0, __d);
            if (isnan(__a))
                __a = copysign(0, __a);
            if (isnan(__b))
                __b = copysign(0, __b);
            __recalc = 1;
        }
        if (!__recalc && (isinf(__ac) || isinf(__bd) ||
                          isinf(__ad) || isinf(__bc)))
        {
            if (isnan(__a))
                __a = copysign(0, __a);
            if (isnan(__b))
                __b = copysign(0, __b);
            if (isnan(__c))
                __c = copysign(0, __c);
            if (isnan(__d))
                __d = copysign(0, __d);
            __recalc = 1;
        }
        if (__recalc)
        {
            __real__ z = INFINITY * (__a * __c - __b * __d);
            __imag__ z = INFINITY * (__a * __d + __b * __c);
        }
    }
    return z;
}

float _Complex
__mulsc3(float __a, float __b, float __c, float __d)
{
    float __ac = __a * __c;
    float __bd = __b * __d;
    float __ad = __a * __d;
    float __bc = __b * __c;
    float _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (isnan(__real__ z) && isnan(__imag__ z))
    {
        int __recalc = 0;
        if (isinf(__a) || isinf(__b))
        {
            __a = copysignf(isinf(__a) ? 1 : 0, __a);
            __b = copysignf(isinf(__b) ? 1 : 0, __b);
            if (isnan(__c))
                __c = copysignf(0, __c);
            if (isnan(__d))
                __d = copysignf(0, __d);
            __recalc = 1;
        }
        if (isinf(__c) || isinf(__d))
        {
            __c = copysignf(isinf(__c) ? 1 : 0, __c);
            __d = copysignf(isinf(__d) ? 1 : 0, __d);
            if (isnan(__a))
                __a = copysignf(0, __a);
            if (isnan(__b))
                __b = copysignf(0, __b);
            __recalc = 1;
        }
        if (!__recalc && (isinf(__ac) || isinf(__bd) ||
                          isinf(__ad) || isinf(__bc)))
        {
            if (isnan(__a))
                __a = copysignf(0, __a);
            if (isnan(__b))
                __b = copysignf(0, __b);
            if (isnan(__c))
                __c = copysignf(0, __c);
            if (isnan(__d))
                __d = copysignf(0, __d);
            __recalc = 1;
        }
        if (__recalc)
        {
            __real__ z = INFINITY * (__a * __c - __b * __d);
            __imag__ z = INFINITY * (__a * __d + __b * __c);
        }
    }
    return z;
}

double _Complex
__divdc3(double __a, double __b, double __c, double __d)
{
    int __ilogbw = 0;
    double __logbw = logb(fmax(fabs(__c), fabs(__d)));
    double __denom = __c * __c + __d * __d;
    double _Complex z;
    if (isfinite(__logbw))
    {
        __ilogbw = (int)__logbw;
        __c = scalbn(__c, -__ilogbw);
        __d = scalbn(__d, -__ilogbw);
    }
    __real__ z = scalbn((__a * __c + __b * __d) / __denom, -__ilogbw);
    __imag__ z = scalbn((__b * __c - __a * __d) / __denom, -__ilogbw);
    if (isnan(__real__ z) && isnan(__imag__ z))
    {
        if ((__denom == 0.0) && (!isnan(__a) || !isnan(__b)))
        {
            __real__ z = copysign(INFINITY, __c) * __a;
            __imag__ z = copysign(INFINITY, __c) * __b;
        }
        else if ((isinf(__a) || isinf(__b)) && isfinite(__c) && isfinite(__d))
        {
            __a = copysign(isinf(__a) ? 1.0 : 0.0, __a);
            __b = copysign(isinf(__b) ? 1.0 : 0.0, __b);
            __real__ z = INFINITY * (__a * __c + __b * __d);
            __imag__ z = INFINITY * (__b * __c - __a * __d);
        }
        else if (isinf(__logbw) && __logbw > 0.0 && isfinite(__a) && isfinite(__b))
        {
            __c = copysign(isinf(__c) ? 1.0 : 0.0, __c);
            __d = copysign(isinf(__d) ? 1.0 : 0.0, __d);
            __real__ z = 0.0 * (__a * __c + __b * __d);
            __imag__ z = 0.0 * (__b * __c - __a * __d);
        }
    }
    return z;
}


float _Complex
__divsc3(float __a, float __b, float __c, float __d)
{
    int __ilogbw = 0;
    float __logbw = logbf(fmaxf(fabsf(__c), fabsf(__d)));
    float __denom = __c * __c + __d * __d;
    float _Complex z;
    if (isfinite(__logbw))
    {
        __ilogbw = (int)__logbw;
        __c = scalbnf(__c, -__ilogbw);
        __d = scalbnf(__d, -__ilogbw);
    }
    __real__ z = scalbnf((__a * __c + __b * __d) / __denom, -__ilogbw);
    __imag__ z = scalbnf((__b * __c - __a * __d) / __denom, -__ilogbw);
    if (isnan(__real__ z) && isnan(__imag__ z))
    {
        if ((__denom == 0) && (!isnan(__a) || !isnan(__b)))
        {
            __real__ z = copysignf(INFINITY, __c) * __a;
            __imag__ z = copysignf(INFINITY, __c) * __b;
        }
        else if ((isinf(__a) || isinf(__b)) && isfinite(__c) && isfinite(__d))
        {
            __a = copysignf(isinf(__a) ? 1 : 0, __a);
            __b = copysignf(isinf(__b) ? 1 : 0, __b);
            __real__ z = INFINITY * (__a * __c + __b * __d);
            __imag__ z = INFINITY * (__b * __c - __a * __d);
        }
        else if (isinf(__logbw) && __logbw > 0 && isfinite(__a) && isfinite(__b))
        {
            __c = copysignf(isinf(__c) ? 1 : 0, __c);
            __d = copysignf(isinf(__d) ? 1 : 0, __d);
            __real__ z = 0 * (__a * __c + __b * __d);
            __imag__ z = 0 * (__b * __c - __a * __d);
        }
    }
    return z;
}

#endif

#endif

#else 

/*
rtai/libm/libm.c - module wrapper for SunSoft/FreeBSD/MacOX/uclibc libm
RTAI - Real-Time Application Interface
Copyright (C) 2001   David A. Schleef <ds@schleef.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <rtai_math.h>

MODULE_LICENSE("GPL");

int libm_errno;

static int verbose = 1;

int __rtai_math_init(void)
{
	if(verbose){
		printk(KERN_INFO "RTAI[math]: loaded.\n");
	}
	return 0;
}

void __rtai_math_exit(void)
{
	if(verbose){
		printk(KERN_INFO "RTAI[math]: unloaded.\n");
	}
}

#ifndef CONFIG_RTAI_MATH_BUILTIN
module_init(__rtai_math_init);
module_exit(__rtai_math_exit);
#endif /* CONFIG_RTAI_MATH_BUILTIN */

#ifdef CONFIG_KBUILD
EXPORT_SYMBOL(acos);
EXPORT_SYMBOL(asin);
EXPORT_SYMBOL(atan);
EXPORT_SYMBOL(atan2);
EXPORT_SYMBOL(ceil);
EXPORT_SYMBOL(copysign);
EXPORT_SYMBOL(cos);
EXPORT_SYMBOL(cosh);
EXPORT_SYMBOL(exp);
EXPORT_SYMBOL(expm1);
EXPORT_SYMBOL(fabs);
EXPORT_SYMBOL(floor);
EXPORT_SYMBOL(fmod);
EXPORT_SYMBOL(frexp);
EXPORT_SYMBOL(log);
EXPORT_SYMBOL(log10);
EXPORT_SYMBOL(modf);
EXPORT_SYMBOL(pow);
EXPORT_SYMBOL(scalbn);
EXPORT_SYMBOL(sin);
EXPORT_SYMBOL(sinh);
EXPORT_SYMBOL(sqrt);
EXPORT_SYMBOL(tan);
EXPORT_SYMBOL(tanh);

#ifdef CONFIG_RTAI_MATH_C99
EXPORT_SYMBOL(acosh);
EXPORT_SYMBOL(asinh);
EXPORT_SYMBOL(atanh);
EXPORT_SYMBOL(cabs);
EXPORT_SYMBOL(cbrt);
EXPORT_SYMBOL(drem);
EXPORT_SYMBOL(erf);
EXPORT_SYMBOL(erfc);
EXPORT_SYMBOL(gamma);
EXPORT_SYMBOL(gamma_r);
EXPORT_SYMBOL(hypot);
EXPORT_SYMBOL(ilogb);
EXPORT_SYMBOL(j0);
EXPORT_SYMBOL(j1);
EXPORT_SYMBOL(jn);
EXPORT_SYMBOL(ldexp);
EXPORT_SYMBOL(lgamma);
EXPORT_SYMBOL(lgamma_r);
EXPORT_SYMBOL(log1p);
EXPORT_SYMBOL(logb);
EXPORT_SYMBOL(matherr);
EXPORT_SYMBOL(nearbyint);
EXPORT_SYMBOL(nextafter);
EXPORT_SYMBOL(remainder);
EXPORT_SYMBOL(rint);
EXPORT_SYMBOL(rinttol);
EXPORT_SYMBOL(round);
EXPORT_SYMBOL(roundtol);
EXPORT_SYMBOL(scalb);
EXPORT_SYMBOL(signgam);
EXPORT_SYMBOL(significand);
EXPORT_SYMBOL(trunc);
EXPORT_SYMBOL(y0);
EXPORT_SYMBOL(y1);
EXPORT_SYMBOL(yn);
EXPORT_SYMBOL(libm_errno);
#endif /* CONFIG_RTAI_MATH_C99 */
#endif /* CONFIG_KBUILD */

#endif
