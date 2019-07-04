/*
 *  rtai/libm/libm.c - module wrapper for SunSoft/FreeBSD/MacOX/uclibc libm
 *  RTAI - Real-Time Application Interface
 *  Copyright (C) 2001 David A. Schleef <ds@schleef.org>
 * 
 * Dave's idea modified (2013) by Paolo Mantegazza <mantegazza@aero.polimi.it>, 
 * so to use just the standard GPLed glibc, with the added possibility of 
 * calling both the float and double version of libm.a functions.
 * 
 * Copyright (C) 2019 Alec Ari <neotheuser@ymail.com>
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

void __stack_chk_fail(void)
{
	panic("rtai_math.ko stack-protector: Kernel stack is corrupted in: %p\n",
	__builtin_return_address(0));
}

int signgam;

#include "export_musl.h"

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

int __rtai_math_init(void)
{
	printk(KERN_INFO "RTAI[math]: loaded, using MUSL.\n");
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
