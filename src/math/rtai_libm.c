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

MODULE_LICENSE("GPL");

double acos(double x);
double asin(double x);
double atan(double x);
double atan2(double y, double x);
double ceil(double x);
double cos(double x);
double exp(double x);
double fabs(double x);
double floor(double x);
double fmax(double x, double y);
double fmin(double x, double y);
double fmod(double x, double y);
double pow(double x, double y);
double round(double x);
double scalbn(double x, int n);
double sin(double x);
double sqrt(double x);
double tan(double x);

EXPORT_SYMBOL(acos);
EXPORT_SYMBOL(asin);
EXPORT_SYMBOL(atan);
EXPORT_SYMBOL(atan2);
EXPORT_SYMBOL(ceil);
EXPORT_SYMBOL(cos);
EXPORT_SYMBOL(exp);
EXPORT_SYMBOL(fabs);
EXPORT_SYMBOL(floor);
EXPORT_SYMBOL(fmax);
EXPORT_SYMBOL(fmin);
EXPORT_SYMBOL(fmod);
EXPORT_SYMBOL(pow);
EXPORT_SYMBOL(round);
EXPORT_SYMBOL(scalbn);
EXPORT_SYMBOL(sin);
EXPORT_SYMBOL(sqrt);
EXPORT_SYMBOL(tan);

int __rtai_math_init(void)
{
	printk(KERN_INFO "RTAI[math]: loaded, using musl libm.\n");
	return 0;
}

void __rtai_math_exit(void)
{
	printk(KERN_INFO "RTAI[math]: unloaded.\n");
}

module_init(__rtai_math_init);
module_exit(__rtai_math_exit);
