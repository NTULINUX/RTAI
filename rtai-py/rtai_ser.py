# Copyright (C) 2010 Paolo Mantegazza <mantegazza@aero.polimi.it>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

from rtai_def import *


# serial port


TTY0 = 0
TTY1 = 1
COM1 = TTY0
COM2 = TTY1

RT_SP_NO_HAND_SHAKE = 0x00
RT_SP_DSR_ON_TX     = 0x01
RT_SP_HW_FLOW       = 0x02

RT_SP_PARITY_EVEN   = 0x18
RT_SP_PARITY_NONE   = 0x00
RT_SP_PARITY_ODD    = 0x08
RT_SP_PARITY_HIGH   = 0x28
RT_SP_PARITY_LOW    = 0x38

RT_SP_FIFO_DISABLE  = 0x00
RT_SP_FIFO_SIZE_1   = 0x00
RT_SP_FIFO_SIZE_4   = 0x40
RT_SP_FIFO_SIZE_8   = 0x80
RT_SP_FIFO_SIZE_14  = 0xC0

RT_SP_DTR           = 0x01
RT_SP_RTS           = 0x02

RT_SP_CTS           = 0x10
RT_SP_DSR           = 0x20
RT_SP_RI            = 0x40
RT_SP_DCD           = 0x80

RT_SP_BUFFER_FULL   = 0x01
RT_SP_OVERRUN_ERR   = 0x02
RT_SP_PARITY_ERR    = 0x04
RT_SP_FRAMING_ERR   = 0x08
RT_SP_BREAK         = 0x10
RT_SP_BUFFER_OVF    = 0x20

RT_SP_FIFO_SIZE_DEFAULT = RT_SP_FIFO_SIZE_8

DELAY_FOREVER = c_longlong(0x3FFFFFFFFFFFFFFF)

rtai.rt_spopen.argtypes = [c_int, c_int, c_int, c_int, c_int, c_int, c_int]
rtai.rt_spopen.restype = c_int
rt_spopen = rtai.rt_spopen

rtai.rt_spclose.argtypes = [c_int]
rtai.rt_spclose.restype = c_int
rt_spclose = rtai.rt_spclose

rtai.rt_spread.argtypes = [c_int, c_void_p, c_int]
rtai.rt_spread.restype = c_int
rt_spread = rtai.rt_spread

rtai.rt_spevdrp.argtypes = [c_int, c_void_p, c_int]
rtai.rt_spevdrp.restype = c_int
rt_spevdro = rtai.rt_spevdrp

rtai.rt_spwrite.argtypes = [c_int, c_void_p, c_int]
rtai.rt_spwrite.restype = c_int
rt_spwrite = rtai.rt_spwrite

rtai.rt_spread_timed.argtypes = [c_int, c_void_p, c_int, c_longlong]
rtai.rt_spread_timed.restype = c_int
rt_spread_timed = rtai.rt_spread_timed

rtai.rt_spwrite_timed.argtypes = [c_int, c_void_p, c_int, c_longlong]
rtai.rt_spwrite_timed.restype = c_int
rt_spwrite_timed = rtai.rt_spwrite_timed

rtai.rt_spclear_rx.argtypes = [c_int]
rtai.rt_spclear_rx.restype = c_int
rt_spclear_rx = rtai.rt_spclear_rx

rtai.rt_spclear_tx.argtypes = [c_int]
rtai.rt_spclear_tx.restype = c_int
rt_spclear_tx = rtai.rt_spclear_tx

rtai.rt_spget_msr.argtypes = [c_int, c_int]
rtai.rt_spget_msr.restype = c_int
rt_spget_msr = rtai.rt_spget_msr

rtai.rt_spset_mcr.argtypes = [c_int, c_int, c_int]
rtai.rt_spset_mcr.restype = c_int
rt_spset_mcr = rtai.rt_spset_mcr

rtai.rt_spget_err.argtypes = [c_int]
rtai.rt_spget_err.restype = c_int
rt_spget_err = rtai.rt_spget_err

rtai.rt_spset_mode.argtypes = [c_int, c_int]
rtai.rt_spset_mode.restype = c_int
rt_spset_mode = rtai.rt_spset_mode

rtai.rt_spset_fifotrig.argtypes = [c_int, c_int]
rtai.rt_spset_fifotrig.restype = c_int
rt_spset_fifotrig = rtai.rt_spset_fifotrig

rtai.rt_spget_rxavbs.argtypes = [c_int]
rtai.rt_spget_rxavbs.restype = c_int
rt_spget_rxavbs = rtai.rt_spget_rxavbs

rtai.rt_spget_txfrbs.argtypes = [c_int]
rtai.rt_spget_txfrbs.restype = c_int
rt_spget_txfrbs = rtai.rt_spget_txfrbs

rtai.rt_spset_thrs.argtypes = [c_int, c_int, c_int]
rtai.rt_spset_thrs.restype = c_int
rt_spset_thrs = rtai.rt_spset_thrs

rtai.rt_spwait_usr_callback.argtypes = [c_int, c_void_p]
rt_spwait_usr_callback = rtai.rt_spwait_usr_callback

rtai.rt_spset_callback_fun.argtypes = [c_int, c_void_p, c_int, c_int]
rtai.rt_spset_callback_fun.restype = c_int
rt_spset_callback_fun = rtai.rt_spset_callback_fun

rtai.rt_spset_err_callback_fun.argtypes = [c_int, c_void_p]
rtai.rt_spset_err_callback_fun.restype = c_int
rt_spset_err_callback_fun = rtai.rt_spset_err_callback_fun
