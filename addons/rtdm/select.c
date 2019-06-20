/*!\file nucleus/select.c
 * \brief file descriptors events multiplexing.
 * \author Gilles Chanteperdrix
 *
 * Copyright (C) 2008 Efixo <gilles.chanteperdrix@xenomai.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \ingroup select
 */

/*!
 * \ingroup nucleus
 * \defgroup select File descriptors events multiplexing services.
 *
 * File descriptors events multiplexing services.
 *
 * This module implements the services needed for implementing the posix
 * "select" service, or any other events multiplexing services.
 *
 * Following the implementation of the posix select service, this module defines
 * three types of events:
 * - \a XNSELECT_READ meaning that a file descriptor is ready for reading;
 * - \a XNSELECT_WRITE meaning that a file descriptor is ready for writing;
 * - \a XNSELECT_EXCEPT meaning that a file descriptor received an exceptional
 *   event.
 *
 * It works by defining two structures:
 * - a @a struct @a xnselect structure, which should be added to every file
 * descriptor for every event type (read, write, or except);
 * - a @a struct @a xnselector structure, the selection structure,  passed by
 * the thread calling the xnselect service, where this service does all its
 * housekeeping.
 *@{*/

#include <linux/types.h>
#include <linux/bitops.h>	/* For hweight_long */

#include <rtdm/select.h>

#ifdef CONFIG_RTAI_RTDM_SELECT

static xnqueue_t xnselectors;
static int xnselect_apc;

#define link2binding(baddr, memb)				\
	container_of(baddr, struct xnselect_binding, memb)

/**
 * Initialize a @a struct @a xnselect structure.
 *
 * This service must be called to initialize a @a struct @a xnselect structure
 * before it is bound to a selector by the means of xnselect_bind().
 *
 * @param select_block pointer to the xnselect structure to be initialized
 */
void xnselect_init(struct xnselect *select_block)
{
	initq(&select_block->bindings);
}
EXPORT_SYMBOL_GPL(xnselect_init);

static inline int xnselect_wakeup(struct xnselector *selector)
{
	return xnsynch_flush(&selector->synchbase, 0) == XNSYNCH_RESCHED;
}

/**
 * Bind a file descriptor (represented by its @a xnselect structure) to a
 * selector block.
 *
 * @param select_block pointer to the @a struct @a xnselect to be bound;
 *
 * @param binding pointer to a newly allocated (using xnmalloc) @a struct
 * @a xnselect_binding;
 *
 * @param selector pointer to the selector structure;
 *
 * @param type type of events (@a XNSELECT_READ, @a XNSELECT_WRITE, or @a
 * XNSELECT_EXCEPT);
 *
 * @param index index of the file descriptor (represented by @a select_block) in the bit fields used by the @a selector structure;
 *
 * @param state current state of the file descriptor>.
 *
 * @a select_block must have been initialized with xnselect_init(),
 * the @a xnselector structure must have been initialized with
 * xnselector_init(), @a binding may be uninitialized.
 *
 * This service must be called with nklock locked, irqs off. For this reason,
 * the @a binding parameter must have been allocated by the caller outside the
 * locking section.
 *
 * @retval -EINVAL if @a type or @a index is invalid;
 * @retval 0 otherwise.
 */
int xnselect_bind(struct xnselect *select_block,
		  struct xnselect_binding *binding,
		  struct xnselector *selector,
		  unsigned type,
		  unsigned index,
		  unsigned state)
{
	if (type >= XNSELECT_MAX_TYPES || index > __FD_SETSIZE)
		return -EINVAL;

	binding->selector = selector;
	binding->fd = select_block;
	binding->type = type;
	binding->bit_index = index;
	inith(&binding->link);
	inith(&binding->slink);

	appendq(&selector->bindings, &binding->slink);
	appendq(&select_block->bindings, &binding->link);
	__FD_SET__(index, &selector->fds[type].expected);
	if (state) {
		__FD_SET__(index, &selector->fds[type].pending);
		if (xnselect_wakeup(selector))
			xnpod_schedule();
	} else
		__FD_CLR__(index, &selector->fds[type].pending);

	return 0;
}
EXPORT_SYMBOL_GPL(xnselect_bind);

/* Must be called with nklock locked irqs off */
int __xnselect_signal(struct xnselect *select_block, unsigned state)
{
	xnholder_t *holder;
	int resched;

	for(resched = 0, holder = getheadq(&select_block->bindings);
	    holder; holder = nextq(&select_block->bindings, holder)) {
		struct xnselect_binding *binding;
		struct xnselector *selector;

		binding = link2binding(holder, link);

		selector = binding->selector;
		if (state) {
			if (!__FD_ISSET__(binding->bit_index,
					&selector->fds[binding->type].pending)) {
				__FD_SET__(binding->bit_index,
					 &selector->fds[binding->type].pending);
				if (xnselect_wakeup(selector))
					resched = 1;
			}
		} else
			__FD_CLR__(binding->bit_index,
				 &selector->fds[binding->type].pending);
	}

	return resched;
}
EXPORT_SYMBOL_GPL(__xnselect_signal);

/**
 * Destroy the @a xnselect structure associated with a file descriptor.
 *
 * Any binding with a @a xnselector block is destroyed.
 *
 * @param select_block pointer to the @a xnselect structure associated with a file descriptor
 */
void xnselect_destroy(struct xnselect *select_block)
{
	xnholder_t *holder;
	int resched = 0;
	spl_t s;

	xnlock_get_irqsave(&nklock, s);
	while ((holder = getq(&select_block->bindings))) {
		struct xnselect_binding *binding;
		struct xnselector *selector;

		binding = link2binding(holder, link);
		selector = binding->selector;

		__FD_CLR__(binding->bit_index,
			 &selector->fds[binding->type].expected);
		if (!__FD_ISSET__(binding->bit_index,
				&selector->fds[binding->type].pending)) {
			__FD_SET__(binding->bit_index,
				 &selector->fds[binding->type].pending);
			if (xnselect_wakeup(selector))
				resched = 1;
		}
		removeq(&selector->bindings, &binding->slink);
		xnlock_put_irqrestore(&nklock, s);

		xnfree(binding);

		xnlock_get_irqsave(&nklock, s);
	}
	if (resched)
		xnpod_schedule();
	xnlock_put_irqrestore(&nklock, s);
}
EXPORT_SYMBOL_GPL(xnselect_destroy);

static unsigned
fd_set_andnot(fd_set *result, fd_set *first, fd_set *second, unsigned n)
{
	unsigned i, not_empty = 0;

	for (i = 0; i < __FDELT__(n); i++)
		if((result->fds_bits[i] =
		    first->fds_bits[i] & ~(second->fds_bits[i])))
			not_empty = 1;

	if (i < __FDSET_LONGS__
	    && (result->fds_bits[i] =
		first->fds_bits[i] & ~(second->fds_bits[i]) & (__FDMASK__(n) - 1)))
		not_empty = 1;

	return not_empty;
}

static unsigned
fd_set_and(fd_set *result, fd_set *first, fd_set *second, unsigned n)
{
	unsigned i, not_empty = 0;

	for (i = 0; i < __FDELT__(n); i++)
		if((result->fds_bits[i] =
		    first->fds_bits[i] & second->fds_bits[i]))
			not_empty = 1;

	if (i < __FDSET_LONGS__
	    && (result->fds_bits[i] =
		first->fds_bits[i] & second->fds_bits[i] & (__FDMASK__(n) - 1)))
		not_empty = 1;

	return not_empty;
}

static void fd_set_zeropad(fd_set *set, unsigned n)
{
	unsigned i;

	i = __FDELT__(n);

	if (i < __FDSET_LONGS__)
		set->fds_bits[i] &= (__FDMASK__(n) - 1);

	for(i++; i < __FDSET_LONGS__; i++)
		set->fds_bits[i] = 0;
}

static unsigned fd_set_popcount(fd_set *set, unsigned n)
{
	unsigned count = 0, i;

	for (i = 0; i < __FDELT__(n); i++)
		if (set->fds_bits[i])
			count += hweight_long(set->fds_bits[i]);

	if (i < __FDSET_LONGS__ && (set->fds_bits[i] & (__FDMASK__(n) - 1)))
		count += hweight_long(set->fds_bits[i] & (__FDMASK__(n) - 1));

	return count;
}

/**
 * Initialize a selector structure.
 *
 * @param selector The selector structure to be initialized.
 *
 * @retval 0
 */
int xnselector_init(struct xnselector *selector)
{
	unsigned i;

	xnsynch_init(&selector->synchbase, XNSYNCH_FIFO, NULL);
	for (i = 0; i < XNSELECT_MAX_TYPES; i++) {
		__FD_ZERO__(&selector->fds[i].expected);
		__FD_ZERO__(&selector->fds[i].pending);
	}
	initq(&selector->bindings);
	return 0;
}
EXPORT_SYMBOL_GPL(xnselector_init);

/**
 * Check the state of a number of file descriptors, wait for a state change if
 * no descriptor is ready.
 *
 * @param selector structure to check for pending events
 * @param out_fds The set of descriptors with pending events if a strictly positive number is returned, or the set of descriptors not yet bound if -ECHRNG is returned;
 * @param in_fds the set of descriptors which events should be checked
 * @param nfds the highest-numbered descriptor in any of the @a in_fds sets, plus 1;
 * @param timeout the timeout, whose meaning depends on @a timeout_mode, note
 * that xnselect() pass @a timeout and @a timeout_mode unchanged to
 * xnsynch_sleep_on, so passing a relative value different from XN_INFINITE as a
 * timeout with @a timeout_mode set to XN_RELATIVE, will cause a longer sleep
 * than expected if the sleep is interrupted.
 * @param timeout_mode the mode of @a timeout.
 *
 * @retval -EINVAL if @a nfds is negative;
 * @retval -ECHRNG if some of the descriptors passed in @a in_fds have not yet
 * been registered with xnselect_bind(), @a out_fds contains the set of such
 * descriptors;
 * @retval -EINTR if @a xnselect was interrupted while waiting;
 * @retval 0 in case of timeout.
 * @retval the number of file descriptors having received an event.
 */
int xnselect(struct xnselector *selector,
	     fd_set *out_fds[XNSELECT_MAX_TYPES],
	     fd_set *in_fds[XNSELECT_MAX_TYPES],
	     int nfds,
	     xnticks_t timeout, xntmode_t timeout_mode)
{
	unsigned i, not_empty = 0;
	xnthread_t *curr;
	spl_t s;

	if ((unsigned) nfds > __FD_SETSIZE)
		return -EINVAL;

	curr = xnpod_current_thread();

	for (i = 0; i < XNSELECT_MAX_TYPES; i++)
		if (out_fds[i])
			fd_set_zeropad(out_fds[i], nfds);

	xnlock_get_irqsave(&nklock, s);
	for (i = 0; i < XNSELECT_MAX_TYPES; i++)
		if (out_fds[i]
		    && fd_set_andnot(out_fds[i], in_fds[i],
				     &selector->fds[i].expected, nfds))
			not_empty = 1;
	xnlock_put_irqrestore(&nklock, s);

	if (not_empty)
		return -ECHRNG;

	xnlock_get_irqsave(&nklock, s);
	for (i = 0; i < XNSELECT_MAX_TYPES; i++)
		if (out_fds[i]
		    && fd_set_and(out_fds[i], in_fds[i],
				  &selector->fds[i].pending, nfds))
			not_empty = 1;

	while (!not_empty) {
		xnsynch_sleep_on(&selector->synchbase, timeout, timeout_mode);

		for (i = 0; i < XNSELECT_MAX_TYPES; i++)
			if (out_fds[i]
			    && fd_set_and(out_fds[i], in_fds[i],
					  &selector->fds[i].pending, nfds))
				not_empty = 1;

		if (xnthread_test_info(curr, XNBREAK | XNTIMEO))
			break;
	}
	xnlock_put_irqrestore(&nklock, s);

	if (not_empty) {
		unsigned count;

		for (count = 0, i = 0; i < XNSELECT_MAX_TYPES; i++)
			if (out_fds[i])
				count += fd_set_popcount(out_fds[i], nfds);

		return count;
	}

	if (xnthread_test_info(curr, XNBREAK))
		return -EINTR;

	return 0; /* Timeout */
}
EXPORT_SYMBOL_GPL(xnselect);

/**
 * Destroy a selector block.
 *
 * All bindings with file descriptor are destroyed.
 *
 * @param selector the selector block to be destroyed
 */
void xnselector_destroy(struct xnselector *selector)
{
	spl_t s;

	inith(&selector->destroy_link);
	xnlock_get_irqsave(&nklock, s);
	appendq(&xnselectors, &selector->destroy_link);
	__rthal_apc_schedule(xnselect_apc);
	xnlock_put_irqrestore(&nklock, s);
}
EXPORT_SYMBOL_GPL(xnselector_destroy);

static void xnselector_destroy_loop(void *cookie)
{
	struct xnselector *selector;
	xnholder_t *holder;
	int resched;
	spl_t s;

	xnlock_get_irqsave(&nklock, s);
	while ((holder = getq(&xnselectors))) {
		selector = container_of(holder, struct xnselector, destroy_link);
		while ((holder = getq(&selector->bindings))) {
			struct xnselect_binding *binding;
			struct xnselect *fd;

			binding = link2binding(holder, slink);
			fd = binding->fd;
			removeq(&fd->bindings, &binding->link);
			xnlock_put_irqrestore(&nklock, s);

			xnfree(binding);

			xnlock_get_irqsave(&nklock, s);
		}
		resched =
			xnsynch_destroy(&selector->synchbase) == XNSYNCH_RESCHED;
		xnlock_put_irqrestore(&nklock, s);

		xnfree(selector);
		if (resched)
			xnpod_schedule();

		xnlock_get_irqsave(&nklock, s);
	}
	xnlock_put_irqrestore(&nklock, s);
}

int xnselect_mount(void)
{
	initq(&xnselectors);
	xnselect_apc = rthal_apc_alloc("xnselectors_destroy",
				       xnselector_destroy_loop, NULL);
	if (xnselect_apc < 0)
		return xnselect_apc;

	return 0;
}

int xnselect_umount(void)
{
	rthal_apc_free(xnselect_apc);
	return 0;
}

#endif /* CONFIG_RTAI_RTDM_SELECT */

/*@}*/
