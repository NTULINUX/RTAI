/*
rtai/libm/libm.c - module wrapper for SunSoft/FreeBSD/MacOX/uclibc libm
RTAI - Real-Time Application Interface
Copyright (C) 2001   David A. Schleef <ds@schleef.org>
Copyright (C) 2014-2016 Alec Ari <neotheuser@ymail.com>
Special thanks to Jeff Epler for many fixes

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
#define __USE_MISC
#define __USE_SVID
#define __USE_ISOC99 1
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

module_init(__rtai_math_init);
module_exit(__rtai_math_exit);

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
EXPORT_SYMBOL(finite);
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
EXPORT_SYMBOL(matherr);
EXPORT_SYMBOL(nextafter);
EXPORT_SYMBOL(remainder);
EXPORT_SYMBOL(rint);
EXPORT_SYMBOL(scalb);
EXPORT_SYMBOL(signgam);
EXPORT_SYMBOL(significand);
EXPORT_SYMBOL(y0);
EXPORT_SYMBOL(y1);
EXPORT_SYMBOL(yn);
EXPORT_SYMBOL(libm_errno);
