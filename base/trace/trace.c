/*
 * Copyright (C) 2000 Karim Yaghmour <karym@opersys.com>
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

/*****************************************************************
 * File : rtai_tracer.c
 * Description :
 *    Contains the code for the RTAI tracing driver.
 * Author :
 *    karym@opersys.com
 * Date :
 *    16/05/00, Initial typing.
 * Note :
 *****************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <rtai_trace.h>

MODULE_LICENSE("GPL");

/* Local variables */
static int        rt_tracer_registered = 0;   /* Is there a tracer registered */
tracer_call       rt_tracer = NULL;           /* The registered tracer */

/****************************************************
 * Function: rt_register_tracer()
 * Description:
 *   Register the tracer to RTAI
 * Return values :
 *   0, all is OK
 *   -1, tracer already registered
 ****************************************************/
int rt_register_tracer(tracer_call pmTraceFunction)
{
  /* Is there a tracer already registered */
  if(rt_tracer_registered == 1)
    return -1;

  /* Set the tracer to the one being passed by the caller */
  rt_tracer = pmTraceFunction;

  /* Remember that a tracer is now registered */
  rt_tracer_registered = 1;

  /* Tell the caller that everything went fine */
  return 0;
}

/***************************************************
 * Function: rt_unregister_tracer()
 * Description:
 *   Unregister the currently registered tracer.
 * Return values :
 *   0, all is OK
 *   -ENOMEDIUM, there isn't a registered tracer
 *   -ENXIO, unregestering wrong tracer
 ***************************************************/
int rt_unregister_tracer(tracer_call pmTraceFunction)
{
  /* Is there a tracer already registered */
  if(rt_tracer_registered == 0)
    /* Nothing to unregister */
    return -ENOMEDIUM;

  /* Is it the tracer that was registered */
  if(rt_tracer == pmTraceFunction)
    /* There isn't any tracer in here */
    rt_tracer_registered = 0;
  else
    return -ENXIO;

  /* Reset tracer function */
  rt_tracer = NULL;

  /* Reset the registered flag */
  rt_tracer_registered = 0;

  /* Tell the caller that everything went OK */
  return 0;
}

/*******************************************************
 * Function: rt_trace_event()
 * Description:
 *   Trace an event
 * Parameters :
 *   pmEventID, the event's ID (check out rtai_trace.h)
 *   pmEventStruct, the structure describing the event
 * Return values :
 *   0, all is OK
 *   -ENOMEDIUM, there isn't a registered tracer
 *   -EBUSY, tracing hasn't started yet
 *******************************************************/
int rt_trace_event(uint8_t  pmEventID, 
		   void*    pmEventStruct)
{
  /* Is there a tracer registered */
  if(rt_tracer_registered != 1)
    return -ENOMEDIUM;

  /* Call the tracer */
  return (rt_tracer(pmEventID, pmEventStruct));
}

/*******************************************************
 * Function: __rtai_trace_init()
 * Description:
 *   Initializes the RTAI trace module
 * Parameters :
 *   NONE
 * Return values :
 *   0, all is OK
 *******************************************************/
int __rtai_trace_init(void)
{
  return 0;
}

/*******************************************************
 * Function: __rtai_trace_exit()
 * Description:
 *   Cleans-up the RTAI trace module
 * Parameters :
 *   NONE
 * Return values :
 *   NONE
 *******************************************************/
void __rtai_trace_exit(void)
{
}

module_init(__rtai_trace_init);
module_exit(__rtai_trace_exit);
