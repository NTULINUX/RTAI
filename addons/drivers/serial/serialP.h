/*
COPYRIGHT (C) 2002-2008  Paolo Mantegazza (mantegazza@aero.polimi.it)
COPYRIGHT (C) 2002-2008  Giuseppe Renoldi (giuseppe@renoldi.org)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <http://www.gnu.org/licenses/>.

*/


#ifndef RTAI_SPDRV_HW_H
#define RTAI_SPDRV_HW_H

#define RTAI_SPDRV_NAME "RTAI_SPDRV"

#define CONFIG_SIZE  40   // likely abundant, twice the max # of serial ports
#define SPBUFSIZ     512  // bufsize will be (1 << SPBUFMSB)
#define RT_SP_CONFIG_INIT  { 0x3f8, 4, 0x2f8, 3, }

#ifdef CONFIG_SMP
static DEFINE_SPINLOCK(splock); 
#define chip_atomic_bgn(flags)  do { rtai_save_flags_and_cli(flags); rt_spin_lock(&splock); } while (0)
#define chip_atomic_end(flags)  do { rt_spin_unlock(&splock); rtai_restore_flags(flags); } while (0)
#define buf_atomic_bgn(flags, mbx)  do { flags = rt_spin_lock_irqsave(&(mbx)->lock); } while (0)
#define buf_atomic_end(flags, mbx)  do { rt_spin_unlock_irqrestore(flags, &(mbx)->lock); } while (0)
#else
#define chip_atomic_bgn(flags)  do { rtai_save_flags_and_cli(flags); } while (0)
#define chip_atomic_end(flags)  do { rtai_restore_flags(flags); } while (0)
#define buf_atomic_bgn(flags, mbx)  do { rtai_save_flags_and_cli(flags); } while (0)
#define buf_atomic_end(flags, mbx)  do { rtai_restore_flags(flags); } while (0)
#endif

#define ENABLE_SP(irq)  rt_enable_irq(irq)

#define RT_SP_BASE_BAUD  115200

#define RT_SP_RXB  0x00
#define RT_SP_TXB  0x00
#define RT_SP_IER  0x01
#define RT_SP_IIR  0x02
#define RT_SP_FCR  0x02
#define RT_SP_LCR  0x03
#define RT_SP_MCR  0x04
#define RT_SP_LSR  0x05
#define RT_SP_MSR  0x06
#define RT_SP_DLL  0x00
#define RT_SP_DLM  0x01

#define MCR_DTR	  0x01
#define MCR_RTS   0x02
#define MCR_OUT1  0x04
#define MCR_OUT2  0x08
#define MCR_LOOP  0x10
#define MCR_AFE   0x20

#define IER_ERBI   0x01  // Enable Received Data Available Interrupt
#define IER_ETBEI  0x02  // Enable Transmitter Holding Register Empty Interrupt
#define IER_ELSI   0x04  // Enable Receiver Line Status Interrupt
#define IER_EDSSI  0x08  // Enable MODEM Status Interrupt

#define MSR_DELTA_CTS  0x01
#define MSR_DELTA_DSR  0x02
#define MSR_TERI       0x04
#define MSR_DELTA_DCD  0x08
#define MSR_CTS        0x10
#define MSR_DSR        0x20
#define MSR_RI         0x40
#define MSR_DCD        0x80

#define LSR_DATA_READY   0x01
#define LSR_OVERRUN_ERR  0x02
#define LSR_PARITY_ERR   0x04
#define LSR_FRAMING_ERR  0x08
#define LSR_BREAK        0x10
#define LSR_THRE         0x20
#define LSR_TEMT         0x40

#define FCR_FIFO_ENABLE        0x01
#define FCR_INPUT_FIFO_RESET   0x02
#define FCR_OUTPUT_FIFO_RESET  0x04

struct rt_spmbx {
	int frbs, avbs, fbyte, lbyte;
	char *bufadr;
	spinlock_t lock;
};

struct rt_spct_t {
	int opened;	
	int base_adr, irq;
	int mode, fifotrig;
	int ier, mcr;
	int error;
	unsigned long just_onew, just_oner;
	int tx_fifo_depth;
	struct rt_spmbx ibuf, obuf;
	void (*callback_fun)(int, int);
	volatile int rxthrs, txthrs;
	void (*err_callback_fun)(int);
	RT_TASK *callback_task;
	unsigned long callback_fun_usr;
	unsigned long err_callback_fun_usr;
	volatile unsigned long call_usr;
	SEM txsem, rxsem;
	struct rt_spct_t *next;
};

#endif /* RTAI_SPDRV_HW_H */
