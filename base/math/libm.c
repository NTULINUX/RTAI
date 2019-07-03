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

EXPORT_SYMBOL(sin);
EXPORT_SYMBOL(cos);
EXPORT_SYMBOL(tan);
EXPORT_SYMBOL(sqrt);
EXPORT_SYMBOL(fabs);
EXPORT_SYMBOL(atan);
EXPORT_SYMBOL(atan2);
EXPORT_SYMBOL(asin);
EXPORT_SYMBOL(acos);
EXPORT_SYMBOL(exp);
EXPORT_SYMBOL(pow);
EXPORT_SYMBOL(fmax);
EXPORT_SYMBOL(fmod);

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

