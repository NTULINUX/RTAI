# Copyright (C) 2008 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


# watch dog


WD_NOTHING = 1		# Not recommended
WD_RESYNC  = 2		# Good for occasional overruns
WD_DEBUG   = 3		# Good for debugging oneshot tasks
WD_STRETCH = 4		# Good for overrunning tasks
WD_SLIP    = 5		# Good for infinite loops
WD_SUSPEND = 6	 	# Good for most things
WD_KILL    = 7 		# Death sentence

WD_SET_GRACE    = 1
WD_SET_GRACEDIV = 2 
WD_SET_SAFETY   = 3 
WD_SET_POLICY   = 4
WD_SET_SLIP     = 5
WD_SET_STRETCH  = 6
WD_SET_LIMIT    = 7 

rt_wdset_grace = rtai.rt_wdset_grace

rt_wdset_gracediv = rtai.rt_wdset_gracediv

rt_wdset_policy = rtai.rt_wdset_policy

rt_wdset_slip = rtai.rt_wdset_slip

rt_wdset_stretch = rtai.rt_wdset_stretch

rt_wdset_limit = rtai.rt_wdset_limit

rt_wdset_safety = rtai.rt_wdset_safety
