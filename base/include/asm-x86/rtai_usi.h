/*
 * Copyright (C) 1999-2013 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


#ifndef _RTAI_ASM_X86_USI_H
#define _RTAI_ASM_X86_USI_H

#define USI_SRQ_MASK  0xFFFFFFF0

#define _STARTUP_IRQ       1
#define _SHUTDOWN_IRQ      2
#define _ENABLE_IRQ        3
#define _DISABLE_IRQ       4
#define _MASK_AND_ACK_IRQ  5
#define _ACK_IRQ           6
#define _UNMASK_IRQ        7
#define _DISINT            8
#define _ENINT             9
#define _SAVE_FLAGS_CLI   10
#define _RESTORE_FLAGS    11

#ifdef __KERNEL__

#ifdef CONFIG_RTAI_USI

static void usi_cli(unsigned long arg, unsigned long *eflags) 
{
	clear_bit(RTAI_IFLAG, eflags);
}

static void usi_sti(unsigned long arg, unsigned long *eflags)
{
	set_bit(RTAI_IFLAG, eflags);
}

static unsigned long usi_save_flags_and_cli(unsigned long arg, unsigned long *eflags)
{
	unsigned long flags = *eflags;
	clear_bit(RTAI_IFLAG, eflags);
	return flags;
}

static void usi_restore_flags(unsigned long flags, unsigned long *eflags)
{
	if (test_bit(RTAI_IFLAG, &flags)) {
		set_bit(RTAI_IFLAG, eflags);
	} else {
		clear_bit(RTAI_IFLAG, eflags);
	}
}

static unsigned long (*usi_fun_entry[ ])(unsigned long, unsigned long *) = {
	[_STARTUP_IRQ]	    = (void *)rt_startup_irq,
	[_SHUTDOWN_IRQ]	    = (void *)rt_shutdown_irq,
	[_ENABLE_IRQ]	    = (void *)rt_enable_irq,
	[_DISABLE_IRQ]	    = (void *)rt_disable_irq,
	[_MASK_AND_ACK_IRQ] = (void *)rt_mask_and_ack_irq,
	[_ACK_IRQ]	    = (void *)rt_ack_irq,
	[_UNMASK_IRQ]       = (void *)rt_unmask_irq,
	[_DISINT]	    = (void *)usi_cli,
	[_ENINT]	    = (void *)usi_sti,
	[_SAVE_FLAGS_CLI]   = (void *)usi_save_flags_and_cli,
	[_RESTORE_FLAGS]    = (void *)usi_restore_flags
};

#ifdef CONFIG_RTAI_USE_STACK_ARGS
#define RTAI_USE_STACK_ARGS 1
#else
#define RTAI_USE_STACK_ARGS 0
#endif

#define IF_IS_A_USI_SRQ_CALL_IT(srq, args, retval, psr, retpath) \
	if (srq > USI_SRQ_MASK) { \
		if (RTAI_USE_STACK_ARGS) { \
			*retval = usi_fun_entry[srq & ~USI_SRQ_MASK](args, &(psr)); \
		} else { \
			rt_put_user(usi_fun_entry[srq & ~USI_SRQ_MASK](args, &(psr)), retval); \
		} \
		return retpath; \
	}
#else

#define IF_IS_A_USI_SRQ_CALL_IT(srq, args, retval, psr, retpath)

#endif

#endif /* __KERNEL__ */

#endif /* !_RTAI_ASM_X86_USI_H */
