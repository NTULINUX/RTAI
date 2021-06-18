/**
 * @ingroup shm
 * @ingroup lxrt
 * @ingroup tasklets
 * @file
 *
 * Conversion between 6 characters strings and unsigned long identifiers.
 * 
 * Convert a 6 characters string to un unsigned long, and vice versa, to be used
 * as an dentifier for RTAI services, symmetrically available in user and kernel
 * space, e.g. @ref shm "shared memory" and @ref lxrt "LXRT and LXRT-INFORMED".
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2013 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_NAM2NUM_H
#define _RTAI_NAM2NUM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <rtai_types.h>

#ifdef __KERNEL__

#include <linux/ctype.h>
#include <linux/string.h>
#define NAM2NUM_PROTO(type, name, arglist)  static inline type name arglist

#else

#include <ctype.h>
#include <string.h>
#define NAM2NUM_PROTO  RTAI_PROTO

#endif

#define MAX_NAM2NUM 4096000003UL  // nam2num("$$$$$$") + 2

/**
 * Convert a 6 characters string to an unsigned long.
 *
 * Converts a 6 characters string name containing an alpha numeric identifier 
 * to its corresponding unsigned long identifier.
 *
 * @param name is the name to be converted.
 *
 * Allowed characters are:
 * -  english letters (no difference between upper and lower case, the latter 
 *    will always be translated to upper case);
 * -  decimal digits;
 * -  '_', '@', '.' and another character of your choice, the latter will 
 *    always be treated as a $ and converted back as such by num2nam().
 *
 * @return the unsigned long associated with @a name.
 */
NAM2NUM_PROTO(unsigned long, nam2num, (const char *name))
{
        unsigned long retval = 0;
	int c, i;

	for (i = 0; i < 6; i++) {
		if (!(c = name[i])) {
			break;
		}
		if (islower(c)) {
			c +=  (10 - 'a');
		} else if (isupper(c)) {
			c += (10 - 'A');
		} else if (isdigit(c)) {
			c -= '0';
		} else {
			c = c == '_' ? 36 : c == '@' ? 37 : c == '.' ? 38 : 39;
		}
		retval = retval*40 + c;
	}
	return i > 0 ? retval + 2 : 0xFFFFFFFF;
}

/**
 * Convert an unsigned long identifier back to its corresponding 6 characters 
 * string.
 *
 * @param num is the unsigned long identifier whose alphanumeric name string has
 * to be evaluated;
 * 
 * @param name is a pointer to a 6 characters buffer where the identifier will
 * be returned. Recall to dimension it at least to 7.
 */
NAM2NUM_PROTO(void, num2nam, (unsigned long num, char *name))
{
        int c, i, k, q; 
	if (num >= MAX_NAM2NUM) {
		strncpy(name, "|null|", 7);
		return;
	}
        i = 5; 
	num -= 2;
	while (num && i >= 0) {
		q = num/40;
		c = num - q*40;
		num = q;
		if (c < 36) {
			name[i--] = c > 9 ? c + 'A' - 10 : c + '0';
		} else {
			name[i--] = c == 36 ? '_' : c == 37 ? '@' : c == 38 ? '.' : '$';
		}
	}
	for (k = 0; i < 5; k++) {
		name[k] = name[++i];
	}
	name[k] = 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_RTAI_NAM2NUM_H */
