/*
 * Copyright (C) 2002-2008 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * Copyright (C) 2002      Giuseppe Renoldi <giuseppe@renoldi.org>
 * Copyright (C) 2002      Rich Walker <rw@shadow.org.uk>, fixed support for UART 16450   
 * Copyright (C) 2003      Richard Brunelle <rbrunelle@envitech.com>, fixed ISR hard flow 
 * Copyright (C) 2008      Renato Castello <zx81@gmx.net>, support for shared interrupts  
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


/* Nov. 2002, Rich Walker <rw@shadow.org.uk>, fixed support for UART 16450   */
/* Jan. 2003, Richard Brunelle <rbrunelle@envitech.com>, fixed ISR hard flow */
/* Apr. 2008, Renato Castello <zx81@gmx.net>, support for shared interrupts  */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include <asm/uaccess.h> 
#include <asm/io.h>

#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtai_serial.h>
#include "serialP.h"

MODULE_AUTHOR("Paolo Mantegazza, Giuseppe Renoldi, Renato Castello.");
MODULE_DESCRIPTION("RTAI real time serial ports driver with multiport support");
MODULE_LICENSE("GPL");

static unsigned long spconfig[2*CONFIG_SIZE] = RT_SP_CONFIG_INIT;
static int spconfig_size = 2*CONFIG_SIZE; 
RTAI_MODULE_PARM_ARRAY(spconfig, ulong, &spconfig_size, 2*CONFIG_SIZE);
struct base_adr_irq_s { unsigned long base_adr, irq; } *sp_config = (void *)spconfig; 

static int spbufsiz = SPBUFSIZ;
RTAI_MODULE_PARM(spbufsiz, int);

static int sptxdepth[CONFIG_SIZE] = { 0 };
static int sptxdepth_size = CONFIG_SIZE; 
RTAI_MODULE_PARM_ARRAY(sptxdepth, int, &sptxdepth_size, CONFIG_SIZE);

struct rt_spct_t *spct;

static int spcnt;	// number of available serial ports
static int spbuflow;	// received buffer low level threshold for 
                        // RTS-CTS hardware flow control    
static int spbufhi;	// received buffer high level threshold for
                        // RTS-CTS hardware flow control
static int spbufull;	// threshold for receive buffer to have the
			// buffer full error

static int max_opncnt;

#define CHECK_SPINDX(indx)  do { if (indx >= spcnt) return -ENODEV; } while (0)


static void mbx_init(struct rt_spmbx *mbx);

/*
 * rt_spclear_rx
 *
 * Clear all received chars in buffer and inside UART FIFO
 *
 * Arguments: 
 * 		tty		serial port number
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 * 		-EACCES		if rx occupied
 *
 */ 
RTAI_SYSCALL_MODE int rt_spclear_rx(unsigned int tty)
{
//	static void mbx_init(struct rt_spmbx *mbx);
	unsigned long flags;
 	struct rt_spct_t *p;

	CHECK_SPINDX(tty);
	if (!test_and_set_bit(0, &(p = spct + tty)->just_oner)) {
		mbx_init(&(p = spct + tty)->ibuf);
		chip_atomic_bgn(flags);
		if (p->fifotrig) {
			outb(inb(p->base_adr + RT_SP_FCR) | FCR_INPUT_FIFO_RESET, p->base_adr + RT_SP_FCR);
		}
		if (p->mode & RT_SP_HW_FLOW) {
			outb(p->mcr |= MCR_RTS, p->base_adr + RT_SP_MCR);
		}
		chip_atomic_end(flags);
		clear_bit(0, &p->just_oner);
		return 0;
	}
    return -EACCES;
}


/*
 * rt_spclear_tx
 *
 * Clear all chars in buffer to be transmitted and inside TX UART FIFO
 *
 * Arguments: 
 * 		tty		serial port number
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 * 		-EACCES		if tx occupied
 *
 */ 
RTAI_SYSCALL_MODE int rt_spclear_tx(unsigned int tty)
{
//	static void mbx_init(struct rt_spmbx *mbx);
	unsigned long flags;
	struct rt_spct_t *p;

	CHECK_SPINDX(tty);
	if (!test_and_set_bit(0, &(p = spct + tty)->just_onew)) {
		mbx_init(&(p = spct + tty)->obuf);
		chip_atomic_bgn(flags);
		outb(p->ier &= ~IER_ETBEI, p->base_adr | RT_SP_IER);
		if (p->fifotrig) {
			outb(inb(p->base_adr + RT_SP_FCR) | FCR_OUTPUT_FIFO_RESET, p->base_adr + RT_SP_FCR);
		}
		chip_atomic_end(flags);
		clear_bit(0, &p->just_onew);
		return 0;
    }
	return -EACCES;
}


/*
 * rt_spset_mode
 *
 * Set the handshaking mode for serial line
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 * 		mode		RT_SP_NO_HAND_SHAKE 
 * 				RT_SP_DSR_ON_TX 	transmitter enabled if DSR active
 * 				RT_SP_HW_FLOW		RTS-CTS flow control
 * 					
 *				Note: RT_SP_DSR_ON_TX and RT_SP_HW_FLOW can
 *				      be ORed together 
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 *
 */ 
RTAI_SYSCALL_MODE int rt_spset_mode(unsigned int tty, int mode)
{
	CHECK_SPINDX(tty);
	if ((mode & (RT_SP_DSR_ON_TX|RT_SP_HW_FLOW)) != mode) {
		return -EINVAL;
	}
	// Enable MODEM status interrupt if RT_SP_DSR_ON_TX or RT_SP_HW_FLOW mode
	outb ((spct[tty].mode = mode) == RT_SP_NO_HAND_SHAKE ? (spct[tty].ier &= ~IER_EDSSI) : (spct[tty].ier |= IER_EDSSI), spct[tty].base_adr + RT_SP_IER);
	return 0;
}


/*
 * rt_spset_fifotrig
 *
 * Set the trigger level for UART RX FIFO
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 *		fifotrig	RT_SP_FIFO_DISABLE
 *      			RT_SP_FIFO_SIZE_1
 *      			RT_SP_FIFO_SIZE_4
 *      			RT_SP_FIFO_SIZE_8
 *      			RT_SP_FIFO_SIZE_14
 *      			RT_SP_FIFO_SIZE_DEFAULT
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 * 		-EINVAL		if wrong fifotrig value
 *
 */ 
RTAI_SYSCALL_MODE int rt_spset_fifotrig(unsigned int tty, int fifotrig)
{
	CHECK_SPINDX(tty);
	if ((fifotrig & 0xC0) != fifotrig) {
		return -EINVAL;
	}
	outb(spct[tty].fifotrig ? spct[tty].fifotrig | FCR_FIFO_ENABLE : 0, spct[tty].base_adr + RT_SP_FCR);
	return 0;
}


/*
 * rt_spset_mcr
 *
 * Set MODEM Control Register MCR
 * -> DTR, RTS bits
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 *		mask		RT_SP_DTR | RT_SP_RTS
 *      			
 *		setbits		0 -> reset bits in mask
 *      			1 -> set bits in mask
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 * 		-EINVAL		if wrong mask value
 *
 */ 
RTAI_SYSCALL_MODE int rt_spset_mcr(unsigned int tty, int mask, int setbits)
{
	CHECK_SPINDX(tty);
	if ((mask & (RT_SP_DSR_ON_TX|RT_SP_HW_FLOW)) != mask) {
		return -EINVAL;
	}
	outb(setbits ? (spct[tty].mcr |= mask) : (spct[tty].mcr &= ~mask), spct[tty].base_adr + RT_SP_MCR);
	return 0;
}


/*
 * rt_spget_msr
 *
 * Get MODEM Status Register MSR
 * -> CTS, DSR, RI, DCD 
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 *		mask		RT_SP_CTS | RT_SP_DSR | RT_SP_RI | RT_SP_DCD
 *      			
 * Return Value:
 * 		masked MSR register value
 * 		or
 * 		-ENODEV		if wrong tty number
 *
 */ 
RTAI_SYSCALL_MODE int rt_spget_msr(unsigned int tty, int mask)
{
	CHECK_SPINDX(tty);
	return inb(spct[tty].base_adr + RT_SP_MSR) & 0xF0 & mask;
}


/*
 * rt_spget_err
 *
 * Return and reset last error detected by UART inside interrupt 
 * service routine
 *
 * Arguments: 
 * 		tty		serial port number
 *      			
 * Return Value:
 * 		last error condition code
 * 		
 * 		This condition code can be one of this values:
 * 		RT_SP_BUFFER_FULL
 * 		RT_SP_BUFFER_OVF
 * 		RT_SP_OVERRUN_ERR
 * 		RT_SP_PARITY_ERR
 * 		RT_SP_FRAMING_ERR
 * 		RT_SP_BREAK
 *
 */ 
RTAI_SYSCALL_MODE int rt_spget_err(unsigned int tty)
{
   	int tmp;

	CHECK_SPINDX(tty);
	tmp = spct[tty].error;
	spct[tty].error = 0;
	return tmp;
}


static void mbx_init(struct rt_spmbx *mbx)
{
	unsigned long flags;
	spin_lock_init(&mbx->lock);
	buf_atomic_bgn(flags, mbx);
	mbx->frbs = spbufsiz;
	mbx->fbyte = mbx->lbyte = mbx->avbs = 0;
	buf_atomic_end(flags, mbx);
}

#define MOD_SIZE(indx) ((indx) < spbufsiz ? (indx) : (indx) - spbufsiz)

#define KSPACE(adr)  ((unsigned long)adr > PAGE_OFFSET)

static inline int mbxput(struct rt_spmbx *mbx, char **msg, int msg_size)
{
	unsigned long flags;
	int tocpy, kspace = KSPACE(*msg);

	while (msg_size > 0 && mbx->frbs) {
		if ((tocpy = spbufsiz - mbx->lbyte) > msg_size) {
			tocpy = msg_size;
		}
		if (tocpy > mbx->frbs) {
			tocpy = mbx->frbs;
		}
		if (kspace) {
			memcpy(mbx->bufadr + mbx->lbyte, *msg, tocpy);
		} else {
			rt_copy_from_user(mbx->bufadr + mbx->lbyte, *msg, tocpy);
		}
		buf_atomic_bgn(flags, mbx);
		mbx->frbs -= tocpy;
		mbx->avbs += tocpy;
		buf_atomic_end(flags, mbx);
		msg_size  -= tocpy;
		*msg      += tocpy;
		mbx->lbyte = MOD_SIZE(mbx->lbyte + tocpy);
	}
	return msg_size;
}

static inline int mbxget(struct rt_spmbx *mbx, char **msg, int msg_size)
{
	unsigned long flags;
	int tocpy, kspace = KSPACE(*msg);

	while (msg_size > 0 && mbx->avbs) {
		if ((tocpy = spbufsiz - mbx->fbyte) > msg_size) {
			tocpy = msg_size;
		}
		if (tocpy > mbx->avbs) {
			tocpy = mbx->avbs;
		}
		if (kspace) {
			memcpy(*msg, mbx->bufadr + mbx->fbyte, tocpy);
		} else {
			rt_copy_from_user(*msg, mbx->bufadr + mbx->fbyte, tocpy);
		}
		buf_atomic_bgn(flags, mbx);
		mbx->frbs  += tocpy;
		mbx->avbs  -= tocpy;
		buf_atomic_end(flags, mbx);
		msg_size  -= tocpy;
		*msg      += tocpy;
		mbx->fbyte = MOD_SIZE(mbx->fbyte + tocpy);
	}
	return msg_size;
}

static inline int mbxevdrp(struct rt_spmbx *mbx, char **msg, int msg_size)
{
	int tocpy, fbyte, avbs, kspace = KSPACE(*msg);

	fbyte = mbx->fbyte;
	avbs  = mbx->avbs;
	while (msg_size > 0 && avbs) {
		if ((tocpy = spbufsiz - fbyte) > msg_size) {
			tocpy = msg_size;
		}
		if (tocpy > avbs) {
			tocpy = avbs;
		}
		if (kspace) {
			memcpy(*msg, mbx->bufadr + fbyte, tocpy);
		} else {
			rt_copy_from_user(*msg, mbx->bufadr + fbyte, tocpy);
		}
		avbs     -= tocpy;
		msg_size -= tocpy;
		*msg     += tocpy;
		fbyte = MOD_SIZE(fbyte + tocpy);
	}
	return msg_size;
}


/*
 * rt_spwrite
 *
 * Send one or more bytes 
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 * 		msg		pointer to the chars to send
 *
 *		msg_size	abs(msg_size) is the number of bytes to write.
 * 				If <0, write bytes only if possible to write them all together.
 *      			
 * Return Value:
 * 		number of chars NOT sent
 * 		or
 * 		-ENODEV		if wrong tty number
 *
 */ 
RTAI_SYSCALL_MODE int rt_spwrite(unsigned int tty, char *msg, int msg_size)
{
	struct rt_spct_t *p;
	CHECK_SPINDX(tty);
	if (!test_and_set_bit(0, &(p = spct + tty)->just_onew)) {
		if (msg_size > 0 || (msg_size = -msg_size) <= p->obuf.frbs) {
			msg_size = mbxput(&p->obuf, &msg, msg_size);
			// enable Transmitter Holding Register Empty to 
			// start transmission if not running already
			outb(p->ier |= IER_ETBEI, p->base_adr + RT_SP_IER);
		}
		clear_bit(0, &p->just_onew);
	}
	return msg_size;
}


/*
 * rt_spread
 *
 * Get one or more bytes from what received removing them
 * from buffer
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 * 		msg		pointer to the buffer where to put chars received
 *
 *		msg_size	abs(msg_size) is the number of bytes to read. 
 *				If >0, read all the bytes up to msg_size 
 *				If <0, read bytes only if possible to read them all together.
 *      			
 * Return Value:
 * 		number of chars NOT get (respect to what requested)
 * 		or
 * 		-ENODEV		if wrong tty number
 *
 */ 
RTAI_SYSCALL_MODE int rt_spread(unsigned int tty, char *msg, int msg_size)
{
	struct rt_spct_t *p;
	CHECK_SPINDX(tty);
	if (!test_and_set_bit(0, &(p = spct + tty)->just_oner)) {
		if (msg_size > 0 || (msg_size = -msg_size) <= p->ibuf.avbs) {
			msg_size = mbxget(&p->ibuf, &msg, msg_size);
			// if RTS-CTS hardware flow control enabled and received
			// buffer not full more than spbufhi threshold enable RTS
			if ((p->mode & RT_SP_HW_FLOW) && p->ibuf.frbs > spbufhi) {
				outb(p->mcr |= MCR_RTS, p->base_adr + RT_SP_MCR);
			}
		}
		clear_bit(0, &p->just_oner);
	}
	return msg_size;
}


/*
 * rt_spevdrp
 *
 * Get one or more bytes from what received WITHOUT removing them
 * from buffer
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 * 		msg		pointer to the buffer where to put chars received
 *
 *		msg_size	abs(msg_size) is the number of bytes to read. 
 *				If >0, read all the bytes up to msg_size 
 *				If <0, read bytes only if possible to read them all together.
 *      			
 * Return Value:
 * 		number of chars NOT get (respect to what requested)
 * 		or
 * 		-ENODEV		if wrong tty number
 *
 */ 
RTAI_SYSCALL_MODE int rt_spevdrp(unsigned int tty, char *msg, int msg_size)
{
	struct rt_spct_t *p;

	CHECK_SPINDX(tty);
	if (!test_and_set_bit(0, &(p = spct + tty)->just_oner)) {
		if (msg_size > 0 || (msg_size = -msg_size) <= p->ibuf.avbs) {
			msg_size = mbxevdrp(&p->ibuf, &msg, msg_size);
		}
		clear_bit(0, &p->just_oner);
	}
	return msg_size;
}


/*
 * rt_spwrite_timed
 *
 * Send one or more bytes with timeout
 *
 * Arguments: 
 * 		tty		serial port number.
 * 		
 * 		msg		pointer to the chars to send.
 *
 *		msg_size	number of bytes to send.
 *
 *		delay		timeout in internal count unit,
 *				use DELAY_FOREVER for a blocking send.
 *      			
 * Return Value:
 * 		-ENODEV, if wrong tty number;
 * 		msg_size, if < 0 or another writer is already using tty;
 * 		one of the semaphores error messages;
 * 		0, message sent succesfully.
 *
 */ 
RTAI_SYSCALL_MODE int rt_spwrite_timed(unsigned int tty, char *msg, int msg_size, RTIME delay)
{
	struct rt_spct_t *p;
	CHECK_SPINDX(tty);
	if (msg_size > 0) {
		if (!test_and_set_bit(0, &(p = spct + tty)->just_onew)) {
			if (msg_size > p->obuf.frbs) {
				int semret;
				p->txsem.count = 0;
				p->txthrs = -msg_size;
				semret = rt_sem_wait_timed(&p->txsem, delay);
				p->txthrs = 0;
				if (semret) {
					clear_bit(0, &p->just_onew);
					return semret;
				}
			}
			mbxput(&p->obuf, &msg, msg_size);
			outb(p->ier |= IER_ETBEI, p->base_adr + RT_SP_IER);
			clear_bit(0, &p->just_onew);
			return 0;
		}
	}
	return msg_size;
}


/*
 * rt_spread_timed
 *
 * Receive one or more bytes with timeout
 *
 * Arguments: 
 * 		tty		serial port number.
 * 		
 * 		msg		pointer to the chars to receive.
 *
 *		msg_size	the number of bytes to receive.
 *
 *		delay		timeout in internal count unit,
 *				use DELAY_FOREVER for a blocking receive.
 *      			
 * Return Value:
 * 		-ENODEV, if wrong tty number;
 * 		msg_size, if < 0 or another reader is already using tty;
 * 		one of the semaphores error messages;
 * 		0, message received succesfully.
 *
 */ 
RTAI_SYSCALL_MODE int rt_spread_timed(unsigned int tty, char *msg, int msg_size, RTIME delay)
{
	struct rt_spct_t *p;
	CHECK_SPINDX(tty);
	if (msg_size > 0) {
		if (!test_and_set_bit(0, &(p = spct + tty)->just_oner)) {
			if (msg_size > p->ibuf.avbs) {
				int semret;
				p->rxsem.count = 0;
				p->rxthrs = -msg_size;
				semret = rt_sem_wait_timed(&p->rxsem, delay);
				p->rxthrs = 0;
				if (semret) {
					clear_bit(0, &p->just_oner);
					return semret;
				}
			}
			mbxget(&p->ibuf, &msg, msg_size);
			if ((p->mode & RT_SP_HW_FLOW) && p->ibuf.frbs > spbufhi) {
				outb(p->mcr |= MCR_RTS, p->base_adr + RT_SP_MCR);
			}
			clear_bit(0, &p->just_oner);
			return 0;
		}
	}
	return msg_size;
}


/*
 * rt_spget_rxavbs
 *
 * Get how many chars are in receive buffer
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 * Return Value:
 * 		number of chars in receive buffer
 * 		or
 * 		-ENODEV		if wrong tty number
 *
 */ 
RTAI_SYSCALL_MODE int rt_spget_rxavbs(unsigned int tty)
{
	CHECK_SPINDX(tty);
	return spct[tty].ibuf.avbs;
}


/*
 * rt_spget_txfrbs
 *
 * Get how many chars are in transmit buffer waiting to be sent by UART
 *
 * Arguments: 
 * 		tty		serial port number
 * 		
 * Return Value:
 * 		number of chars in transmit buffer
 * 		or
 * 		-ENODEV		if wrong tty number
 *
 */ 
RTAI_SYSCALL_MODE int rt_spget_txfrbs(unsigned int tty)
{
	CHECK_SPINDX(tty);
	return spct[tty].obuf.frbs;
}

static inline int rt_spget_irq(struct rt_spct_t *p, unsigned char *c)
{
	unsigned long flags;
	if (p->obuf.avbs) {
		*c = p->obuf.bufadr[p->obuf.fbyte];
		buf_atomic_bgn(flags, &p->obuf);
		p->obuf.avbs--;
		p->obuf.frbs++;
		buf_atomic_end(flags, &p->obuf);
		p->obuf.fbyte = MOD_SIZE(p->obuf.fbyte + 1);
		return 0;
	}
	return -ENOSPC;
}

static inline int rt_spset_irq(struct rt_spct_t *p, unsigned char ch)
{
	unsigned long flags;
	if (p->ibuf.frbs) {
		p->ibuf.bufadr[p->ibuf.lbyte] = ch;
		buf_atomic_bgn(flags, &p->ibuf);
		p->ibuf.frbs--;
		p->ibuf.avbs++;
		buf_atomic_end(flags, &p->ibuf);
		p->ibuf.lbyte = MOD_SIZE(p->ibuf.lbyte + 1);
		return 0;
	}
	return -ENOSPC;
}

/* Extended to support shared interrupts, by: Renato Castello <zx81@gmx.net>, */
/* with a lot of rewrites by: Paolo Mantegazza <mantegazza@aero.polimi.it>    */

static int rt_spisr(int irq, struct rt_spct_t *pp)
{
	struct { unsigned char er, cb, txs, rxs; } todo[max_opncnt];
{
	unsigned int base_adr;
	int          data_to_tx;
	int          toFifo;
	int 	     txed, rxed;	
	unsigned char data, iir, msr, lsr, error;
	struct rt_spct_t *p = pp;
	int i = 0;
	
	while (1) {
		base_adr = p->base_adr;
		toFifo   = p->tx_fifo_depth;
		rxed = txed = 0;	
		todo[i].txs = todo[i].rxs = todo[i].cb = error = 0;
	
		iir = inb(base_adr + RT_SP_IIR);	
		do {
		//rt_printk("rt_spisr irq=%d, iir=0x%02x\n",irq,iir);	
			switch (iir & 0x0f) {
				case 0x06: // Receiver Line Status
				   // Overrun Error or Parity Error or 
				   // Framing Error or Break Interrupt
				
				if ((lsr = inb(base_adr + RT_SP_LSR)) & 0x1e) {
					error &= ~0x1e;
					error |= (lsr & 0x1e);
				}	
				break;
	
				case 0x04: // Received Data Available
				case 0x0C: // Character Timeout Indication
				rxed = 1;
				/* get available data from base_adr */
				do {
					if (rt_spset_irq(p, inb(base_adr + RT_SP_RXB))) {
						error |= RT_SP_BUFFER_OVF;
					}
					if ((lsr = inb(base_adr + RT_SP_LSR)) & 0x1e) {
						error &= ~0x1e;
						error |= (lsr & 0x1e);
					}
				} while (LSR_DATA_READY & lsr);
				// check for received buffer full
				if (p->ibuf.frbs < spbufull) {
					error |= RT_SP_BUFFER_FULL;
				}
				// if RTS-CTS hardware flow control, check if received
				// buffer is full enough to stop removing RTS signal
				if ((p->mode & RT_SP_HW_FLOW) && p->ibuf.frbs < spbuflow) {
					// disable RTS	
					outb(p->mcr &= ~MCR_RTS, p->base_adr + RT_SP_MCR);
				}
				break;
	
				case 0x02: // Transmitter Holding Register Empty
	
				/* if possible, put data to base_adr */
				msr = inb(base_adr + RT_SP_MSR);
				if ( (p->mode == RT_SP_NO_HAND_SHAKE) || 
					 ((p->mode == RT_SP_DSR_ON_TX) && (MSR_DSR & msr)) || 
					 ((p->mode == RT_SP_HW_FLOW) && (MSR_CTS & msr)) ||
					 ((p->mode == (RT_SP_HW_FLOW|RT_SP_DSR_ON_TX)) &&
                                          (MSR_CTS & msr) && (MSR_DSR & msr)) ) {
			    	// if there are data to transmit 
					if (!(data_to_tx = rt_spget_irq(p, &data))) {
						txed = 1;	
						do {
//							rt_printk("->%c] ",data);	
							outb(data, base_adr + RT_SP_TXB);
						} while ((--toFifo > 0) && !(data_to_tx = rt_spget_irq(p, &data)));
					}
					if (data_to_tx) {
		            	/* no more data in output buffer, disable Transmitter
			               Holding Register Empty Interrupt */
						outb(p->ier &= ~IER_ETBEI, base_adr + RT_SP_IER);
					}
				}
				break;
	
				case 0x00: // MODEM Status
				msr = inb(base_adr + RT_SP_MSR);
				break;
	
				default:
				break;
			}
		} while (!((iir = inb(base_adr + RT_SP_IIR)) & 1) );

	    /* controls on buffer full and RTS clear on hardware flow control */
		if (p->ibuf.frbs < spbufull) {
			error = RT_SP_BUFFER_FULL;
		}
		if ((p->mode & RT_SP_HW_FLOW) && p->ibuf.frbs < spbuflow) {
			outb(p->mcr &= ~MCR_RTS, p->base_adr + RT_SP_MCR);
		}

		rxed = rxed && p->rxthrs && p->ibuf.avbs >= abs(p->rxthrs) ? p->rxthrs : 0;
		txed = txed && p->txthrs && p->obuf.frbs >= abs(p->txthrs) ? p->txthrs : 0;
		todo[i].er = p->error = error;
		todo[i].cb = (rxed || txed);
		if (rxed < 0 && p->rxsem.count < 0) {
			p->rxthrs = 0;
			todo[i].rxs = 1;
		}	
		if (txed < 0 && p->txsem.count < 0) {
			p->txthrs = 0;
			todo[i].txs = 1;
		}
		rtai_sti();
		if (!(p = p->next)) {
			break;
		}
		i++;
		rtai_cli();
	}
}	
	ENABLE_SP(irq);
{
	int tsk, i = 0;
	do {
		if (todo[i].er) {
			if (pp->err_callback_fun) {
				(pp->err_callback_fun)(pp->error = todo[i].er);
			}
			tsk = 1;
		} else {
			tsk = 0;
		}
		if (todo[i].cb) {
			if (pp->callback_fun) {
				(pp->callback_fun)(pp->ibuf.avbs, pp->obuf.frbs);
			}
			tsk |= 2;
		}
		if (todo[i].rxs && todo[i].txs) {
			if (pp->rxsem.queue.next->task->priority < pp->txsem.queue.next->task->priority) {
				rt_sem_signal(&pp->rxsem);
				rt_sem_signal(&pp->txsem);
			} else {
				rt_sem_signal(&pp->txsem);
				rt_sem_signal(&pp->rxsem);
			}
			goto again;
                }
		if (todo[i].rxs) {
			rt_sem_signal(&pp->rxsem);
			goto again;
		}
		if (todo[i].txs) {
			rt_sem_signal(&pp->txsem);
			goto again;
		}
		if (pp->callback_task && tsk) {
			pp->call_usr = tsk;
			rt_task_resume(pp->callback_task);
		}
again:
		i++;
	} while ((pp = pp->next));
}
	return 0;
}

/*
 * rt_spopen
 *
 * Open the serial port
 *
 * Arguments: 
 * 		tty		serial port number
 *
 * 		baud		50 .. 115200
 *
 *      numbits		5,6,7,8
 *      
 *      stopbits	1,2
 *					
 *      
 *      parity		RT_SP_PARITY_NONE
 *      		RT_SP_PARITY_EVEN
 *      		RT_SP_PARITY_ODD
 *      		RT_SP_PARITY_HIGH
 *      		RT_SP_PARITY_LOW
 *      			
 * 	mode		RT_SP_NO_HAND_SHAKE
 * 			RT_SP_DSR_ON_TX
 * 			RT_SP_HW_FLOW
 *
 *      fifotrig	RT_SP_FIFO_DISABLE
 *      		RT_SP_FIFO_SIZE_1
 *      		RT_SP_FIFO_SIZE_4
 *      		RT_SP_FIFO_SIZE_8
 *      		RT_SP_FIFO_SIZE_14
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 * 		-EINVAL		if wrong parameter value
 * 		-EADDRINUSE	if trying to open an openend port
 *
 */ 
RTAI_SYSCALL_MODE int rt_spopen(unsigned int tty, unsigned int baud, unsigned int numbits,
              unsigned int stopbits, unsigned int parity, int mode,
			  int fifotrig)
{
	struct rt_spct_t *p;
	int base_adr;

	CHECK_SPINDX(tty);
	if ( baud < 50 || baud > 115200 ||
	     (mode & 0x03) != mode ||
	     (parity & 0x38) != parity ||
		 stopbits < 1 || stopbits > 2 ||
		 numbits < 5 || numbits > 8 ||
	     (fifotrig & 0xC0) != fifotrig )
		return -EINVAL;	

	if ((p = spct + tty)->opened)
		return -EADDRINUSE;
	
	// disable interrupt
	rt_disable_irq(p->irq);

	// disable all UART interrupts
	outb(0x00, (base_adr = p->base_adr) + RT_SP_IER);

	// reset possible pending interrupts
	inb(base_adr + RT_SP_IIR);
	inb(base_adr + RT_SP_LSR);
	inb(base_adr + RT_SP_RXB);
	inb(base_adr + RT_SP_MSR);

	// set Divisor Latch Access Bit (DLAB) for Baud Rate setting
	outb(0x80, base_adr + RT_SP_LCR);
	// set baud rate
	outb((RT_SP_BASE_BAUD/baud) & 0xFF, base_adr + RT_SP_DLL);
	outb((RT_SP_BASE_BAUD/baud) >> 8, base_adr + RT_SP_DLM);

	// set numbits, stopbits and parity and reset DLAB 
	outb((numbits - 5) | ((stopbits - 1) << 2) | (parity & 0x38), base_adr + RT_SP_LCR);

	// DTR, RTS, OUT1 and OUT2 active
	// and initialize p->mcr register
	outb(p->mcr = MCR_DTR | MCR_RTS | MCR_OUT1 | MCR_OUT2, base_adr + RT_SP_MCR);

	// Enable MODEM status interrupt if RT_SP_DSR_ON_TX or RT_SP_HW_FLOW mode
	// and initialize p->ier register
	outb((p->mode = mode) == RT_SP_NO_HAND_SHAKE ? (p->ier = IER_ERBI | IER_ELSI) : (p->ier = IER_ERBI | IER_ELSI | IER_EDSSI), base_adr + RT_SP_IER);

	if (fifotrig >= 0) {
		p->fifotrig = fifotrig;
	}

	// Enable fifo 
	outb(FCR_FIFO_ENABLE | p->fifotrig, base_adr + RT_SP_FCR);
	// reset error 
	p->error = 0;
	// initialize received and transmit buffers
	mbx_init(&p->ibuf);
	mbx_init(&p->obuf);
	p->just_oner = p->just_onew = 0;
	// remember that it is opened
	spct[tty].callback_task = 0;
	p->opened = 1;
	// enable interrupts
	rt_enable_irq(p->irq);
	return 0;
}


/*
 * rt_spclose
 *
 * Close the serial port
 *
 * Arguments: 
 * 		tty		serial port number
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 *
 */ 
RTAI_SYSCALL_MODE int rt_spclose(unsigned int tty)
{
	int base_adr=spct[tty].base_adr;
	
	CHECK_SPINDX(tty);
	// disable interrupt
//	rt_disable_irq(spct[tty].irq);
	// disable interrupt generation for UART
	outb(0, base_adr + RT_SP_IER);
	// reset possible pending interrupts
	inb(base_adr + RT_SP_IIR);
	inb(base_adr + RT_SP_LSR);
	inb(base_adr + RT_SP_RXB);
	inb(base_adr + RT_SP_MSR);
	// disable DTR, RTS
	outb(spct[tty].mcr = MCR_OUT1 | MCR_OUT2, base_adr + RT_SP_MCR);
	spct[tty].opened = 0;
	if (spct[tty].callback_task) {
		RT_TASK *task = spct[tty].callback_task;
		spct[tty].callback_task = 0;
		rt_task_resume(task);
	}
	return 0;
}


/*
 * rt_spset_thrs
 *
 * Open the serial port
 *
 * Arguments: 
 * 		tty		serial port number
 *
 * 		rxthrs		number of chars in receive buffer trhreshold
 *
 * 		txthrs		number of free chars in transmit buffer threshold
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 * 		-EINVAL		if wrong parameter value
 *
 */ 
RTAI_SYSCALL_MODE int rt_spset_thrs(unsigned int tty, int rxthrs, int txthrs)
{
	CHECK_SPINDX(tty);
	if (rxthrs < 0 || txthrs < 0) {
		return -EINVAL;
	}
	spct[tty].rxthrs = rxthrs;
	spct[tty].txthrs = txthrs;
	return 0;
}

/*
 * rt_spset_callback_fun
 *
 * Define the callback function to be called when the chars in the receive
 * buffer or the free chars in the transmit buffer have reached the 
 * specified thresholds
 *
 * Arguments: 
 * 		tty		serial port number
 *
 * 		callback_fun	pointer to the callback function
 * 				the two int parameters passed to this funcion
 * 				are the number 
 *
 * 		rxthrs		number of chars in receive buffer trhreshold
 *
 * 		txthrs		number of free chars in transmit buffer threshold
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 * 		-EINVAL		if wrong parameter value
 *
 */ 
long rt_spset_callback_fun(unsigned int tty, void (*callback_fun)(int, int), 
                          int rxthrs, int txthrs)
{
	long prev_callback_fun;

	CHECK_SPINDX(tty);
	if (rt_spset_thrs(tty, rxthrs, txthrs)) {
		return -EINVAL;
	}
	prev_callback_fun = (long)spct[tty].callback_fun;
	spct[tty].callback_fun = callback_fun;
	return prev_callback_fun;
}


/*
 * rt_spset_err_callback_fun
 *
 * Define the callback function to be called when the interrupt 
 * service routine detect an error
 *
 * Arguments: 
 * 		tty		serial port number
 *
 * 		err_callback_fun 	pointer to the callback function
 * 					the int parameter passed to this funcion
 * 					will contain the error code 
 *
 * Return Value:
 * 		0		if success
 * 		-ENODEV		if wrong tty number
 * 		-EINVAL		if wrong parameter value
 *
 */ 
long rt_spset_err_callback_fun(unsigned int tty, void (*err_callback_fun)(int))
{
	long prev_err_callback_fun;

	CHECK_SPINDX(tty);
	prev_err_callback_fun = (long)spct[tty].err_callback_fun;
	spct[tty].err_callback_fun = err_callback_fun;
	return prev_err_callback_fun;
}


/*
 * rt_spset_callback_fun_usr
 *
 * Define the callback function to be called from user space
 *
 */ 
RTAI_SYSCALL_MODE int rt_spset_callback_fun_usr(unsigned int tty, unsigned long callback_fun_usr, int rxthrs, int txthrs, int code, void *task)
{
	int prev_callback_fun_usr;

	CHECK_SPINDX(tty);
	if (!spct[tty].opened) {
		return -ENODEV;
	}
	if (code && !task) {
		return !spct[tty].callback_task ? EINVAL : -EINVAL;
	}
	if ((!code && !task) || rt_spset_thrs(tty, rxthrs, txthrs)) {
		return -EINVAL;
	}
	if (task) {
		spct[tty].callback_task = task;
	}
	prev_callback_fun_usr = (int)spct[tty].callback_fun_usr;
	spct[tty].callback_fun_usr = callback_fun_usr;
	return prev_callback_fun_usr;
}


/*
 * rt_spset_err_callback_fun_usr
 *
 * Define the err callback function to be called from user space
 *
 */ 
RTAI_SYSCALL_MODE int rt_spset_err_callback_fun_usr(unsigned int tty, unsigned long err_callback_fun_usr, int dummy1, int dummy2, int code, void *task)
{
	int prev_err_callback_fun_usr;

	CHECK_SPINDX(tty);
	if (!spct[tty].opened) {
		return -ENODEV;
	}
	if (code && !task) {
		return !spct[tty].callback_task ? EINVAL : -EINVAL;
	}
	if (!code && !task) {
		return -EINVAL;
	}
	if (task) {
		spct[tty].callback_task = task;
	}
	prev_err_callback_fun_usr = (int)spct[tty].err_callback_fun_usr;
	spct[tty].callback_fun_usr = err_callback_fun_usr;
	return prev_err_callback_fun_usr;
}


/*
 * rt_spwait_usr_callback
 *
 * Wait for user space callback
 *
 */ 
RTAI_SYSCALL_MODE void rt_spwait_usr_callback(unsigned int tty, unsigned long *retvals)
{
	rt_task_suspend(spct[tty].callback_task);
	retvals[0] = spct[tty].call_usr | 2 ? spct[tty].callback_fun_usr : 0;
	retvals[1] = spct[tty].call_usr | 1 ? spct[tty].err_callback_fun_usr : 0;
	retvals[2] = spct[tty].ibuf.avbs;
	retvals[3] = spct[tty].obuf.frbs;
	retvals[4] = spct[tty].error;
	retvals[5] = spct[tty].opened;
	return;
}

static struct rt_fun_entry rtai_spdrv_fun[] = {
	[_SPOPEN]              = { 0, rt_spopen},
	[_SPCLOSE]             = { 0, rt_spclose},
	[_SPREAD]              = { 0, rt_spread },
	[_SPEVDRP]             = { 0, rt_spevdrp },
	[_SPWRITE]             = { 0, rt_spwrite },
	[_SPCLEAR_RX]          = { 0, rt_spclear_rx },
	[_SPCLEAR_TX]          = { 0, rt_spclear_tx },
	[_SPGET_MSR]           = { 0, rt_spget_msr },
	[_SPSET_MCR]           = { 0, rt_spset_mcr },
	[_SPGET_ERR]           = { 0, rt_spget_err },
	[_SPSET_MODE]          = { 0, rt_spset_mode },
	[_SPSET_FIFOTRIG]      = { 0, rt_spset_fifotrig },
	[_SPGET_RXAVBS]        = { 0, rt_spget_rxavbs },
	[_SPGET_TXFRBS]        = { 0, rt_spget_txfrbs },
	[_SPSET_THRS]          = { 0, rt_spset_thrs },
	[_SPSET_CALLBACK]      = { 0, rt_spset_callback_fun_usr },
	[_SPSET_ERR_CALLBACK]  = { 0, rt_spset_err_callback_fun_usr },
	[_SPWAIT_USR_CALLBACK] = { UW1(2, 3), rt_spwait_usr_callback },
	[_SPREAD_TIMED]        = { UW1(2, 3), rt_spread_timed },
	[_SPWRITE_TIMED]       = { UR1(2, 3), rt_spwrite_timed }
};

/* Extended to support shared interrupts, by: Renato Castello <zx81@gmx.net> */

int __rtai_serial_init(void)
{
	int i, k, newirq;
	struct rt_spct_t *p;
		
	for (spcnt = 0; spcnt < CONFIG_SIZE; spcnt++) {
		if (!sp_config[spcnt].base_adr || !sp_config[spcnt].irq) {
			break;
		}
		printk("TTY_INDX: %d, PORT: %lx, IRQ %ld.\n", spcnt, sp_config[spcnt].base_adr, sp_config[spcnt].irq);
	}
	printk("# OF PORTS: %d, BUFFER SIZE: %d.\n", spcnt, spbufsiz);
	if (!(spct = kmalloc(spcnt*sizeof(struct rt_spct_t), GFP_KERNEL))) {
		printk("IMPOSSIBLE TO KMALLOC RT_SPDRV CONTROL TABLES.\n");
		return -ENOMEM;
	}
	memset(spct, 0, spcnt*sizeof(struct rt_spct_t));

	for (i = 0; i < spcnt; i++) {
		spct[i].next = NULL;
		for (k = i + 1; k < spcnt; k++) {
			if (sp_config[i].irq == sp_config[k].irq) {
				spct[i].next = &spct[k];
				break;
			}
		}
 		for (newirq = k = 0; k < spcnt; k++) {
			if (sp_config[i].irq == sp_config[k].irq) {
 	    			newirq++;
 			}
 		}
		if (max_opncnt < newirq) {
			max_opncnt = newirq;
 		}
	}

	for (i = 0; i < spcnt; i++) {
		newirq = 1;
		for (k = i - 1; k >= 0 ; k--) {
			if (sp_config[i].irq == sp_config[k].irq) {
				newirq = 0;
			}
		}
		if (newirq) {
			rt_disable_irq(sp_config[i].irq);
			if (rt_request_irq(sp_config[i].irq, (void *)rt_spisr, (void *)(spct + i), 1)) {
				printk("IRQ NOT AVAILABLE (TTY INDEX: %d).\n", i);
				newirq = 0;
			}
		    
			p = &spct[i];
			do {
				if (!newirq || !(p->ibuf.bufadr = kmalloc(2*spbufsiz, GFP_KERNEL))) {
					int retval;
					if (!newirq) {
						retval = -ENODEV;
					} else {
						retval = -ENOMEM;
						printk("NO MEMORY AVAILABLE FOR READ/WRITE BUFFERS (TTY INDEX: %d).\n", k);
					}
					for (k = 0; k < spcnt; k++) {
						if (spct[k].ibuf.bufadr) {
					  		rt_release_irq(sp_config[k].irq);
							release_region(sp_config[k].base_adr, 8);
							kfree(spct[k].ibuf.bufadr);
						}
					}
					kfree(spct);
					return retval;
				} else {
					p->obuf.bufadr = p->ibuf.bufadr + spbufsiz;
				}
			} while ((p = p->next));
		}

		if (request_region(sp_config[i].base_adr, 8, RTAI_SPDRV_NAME) == NULL) {
			release_region(sp_config[i].base_adr, 8);
			request_region(sp_config[i].base_adr, 8, RTAI_SPDRV_NAME);
		}
		spct[i].callback_task = 0;
		spct[i].opened = 0;
		// set base address for UART
		spct[i].base_adr = sp_config[i].base_adr;
		// set irq 
		spct[i].irq      = sp_config[i].irq;
		// set default values for RX FIFO trigger level
		spct[i].fifotrig = RT_SP_FIFO_SIZE_DEFAULT;
		// sef default thresholds for callback function 
		spct[i].rxthrs = spct[i].txthrs = 0;
		if (!(spct[i].tx_fifo_depth = sptxdepth[i])) {
			spct[i].tx_fifo_depth = 16;
		}
		rt_sem_init(&spct[i].txsem, 0);
		rt_sem_init(&spct[i].rxsem, 0);
	}
	
	spbuflow = spbufsiz / 3;
	spbufhi  = spbufsiz - spbuflow;
	spbufull = spbufsiz / 10;

	if (set_rt_fun_ext_index(rtai_spdrv_fun, FUN_EXT_RTAI_SP)) {
		rt_printk("%d is a wrong index module for lxrt.\n", FUN_EXT_RTAI_SP);
		return -EACCES;
	}			

	return 0;
}

void __rtai_serial_exit(void)
{
	int i;

	reset_rt_fun_ext_index(rtai_spdrv_fun, FUN_EXT_RTAI_SP);

	for (i = 0; i < spcnt; i++) {
		if (spct[i].opened) {
			rt_spclose(spcnt);
		}
		if (spct[i].callback_task) {
			rt_task_resume(spct[i].callback_task);
		}
  		rt_release_irq(spct[i].irq);
		release_region(spct[i].base_adr, 8);
		kfree(spct[i].ibuf.bufadr);
		rt_sem_delete(&spct[i].txsem);
		rt_sem_delete(&spct[i].rxsem);
		printk("removed IRQ %d\n", spct[i].irq);
	}
	kfree(spct);
}

module_init(__rtai_serial_init);
module_exit(__rtai_serial_exit);

EXPORT_SYMBOL(rt_spclear_rx);
EXPORT_SYMBOL(rt_spclear_tx);
EXPORT_SYMBOL(rt_spget_err);
EXPORT_SYMBOL(rt_spget_msr);
EXPORT_SYMBOL(rt_spset_mcr);
EXPORT_SYMBOL(rt_spset_mode);
EXPORT_SYMBOL(rt_spset_fifotrig);
EXPORT_SYMBOL(rt_spopen);
EXPORT_SYMBOL(rt_spclose);
EXPORT_SYMBOL(rt_spread);
EXPORT_SYMBOL(rt_spevdrp);
EXPORT_SYMBOL(rt_spwrite);
EXPORT_SYMBOL(rt_spset_thrs);
EXPORT_SYMBOL(rt_spset_callback_fun);
EXPORT_SYMBOL(rt_spget_rxavbs);
EXPORT_SYMBOL(rt_spget_txfrbs);
EXPORT_SYMBOL(rt_spset_err_callback_fun);
EXPORT_SYMBOL(rt_spset_callback_fun_usr);
EXPORT_SYMBOL(rt_spset_err_callback_fun_usr);
EXPORT_SYMBOL(rt_spwait_usr_callback);
EXPORT_SYMBOL(rt_spread_timed);
EXPORT_SYMBOL(rt_spwrite_timed);
