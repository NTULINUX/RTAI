/*!\file select.h
 * \brief file descriptors events multiplexing header.
 * \author Gilles Chanteperdrix
 *
 * Copyright (C) 2008 Efixo <gilles.chanteperdrix@laposte.net>
 * Copyright (C) 2014-2016 Alec Ari <neotheuser@ymail.com>
 *
 * Rtai is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * Rtai is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rtai; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * \ingroup select
 */

#ifndef XNSELECT_H
#define XNSELECT_H

/*! \addtogroup select
 *@{*/

#include "xn.h"

#define XNSELECT_READ      0
#define XNSELECT_WRITE     1
#define XNSELECT_EXCEPT    2
#define XNSELECT_MAX_TYPES 3

struct xnselector {
	xnsynch_t synchbase;
	struct fds {
		fd_set expected;
		fd_set pending;
	} fds [XNSELECT_MAX_TYPES];
	xnholder_t destroy_link;
	xnqueue_t bindings; /* only used by xnselector_destroy */
};

#define __NFDBITS__	(8 * sizeof(unsigned long))
#define __FDSET_LONGS__	(__FD_SETSIZE/__NFDBITS__)
#define	__FDELT__(d)	((d) / __NFDBITS__)
#define	__FDMASK__(d)	(1UL << ((d) % __NFDBITS__))

static inline void __FD_SET__(unsigned long __fd, __kernel_fd_set *__fdsetp)
{
	unsigned long __tmp = __fd / __NFDBITS__;
	unsigned long __rem = __fd % __NFDBITS__;
	__fdsetp->fds_bits[__tmp] |= (1UL<<__rem);
}

static inline void __FD_CLR__(unsigned long __fd, __kernel_fd_set *__fdsetp)
{
	unsigned long __tmp = __fd / __NFDBITS__;
	unsigned long __rem = __fd % __NFDBITS__;
	__fdsetp->fds_bits[__tmp] &= ~(1UL<<__rem);
}

static inline int __FD_ISSET__(unsigned long __fd, const __kernel_fd_set *__p)
{
	unsigned long __tmp = __fd / __NFDBITS__;
	unsigned long __rem = __fd % __NFDBITS__;
	return (__p->fds_bits[__tmp] & (1UL<<__rem)) != 0;
}

static inline void __FD_ZERO__(__kernel_fd_set *__p)
{
	unsigned long *__tmp = __p->fds_bits;
	int __i;

	__i = __FDSET_LONGS__;
	while (__i) {
		__i--;
		*__tmp = 0;
		__tmp++;
	}
}

struct xnselector;
#define DECLARE_XNSELECT(name)
#define xnselect_init(block)
#define xnselect_bind(select_block,binding,selector,type,bit_index,state) \
	({ -EBADF; })
#define xnselect_signal(block, state) ({ int __ret = 0; __ret; })
#define xnselect_destroy(block)

/*@}*/

#endif /* XNSELECT_H */
