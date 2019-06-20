/*
  COPYRIGHT (C) 2002  Lorenzo Dozio (dozio@aero.polimi.it)

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


#define S_FUNCTION_NAME		sfun_rtai_led
#define S_FUNCTION_LEVEL	2

#ifdef MATLAB_MEX_FILE
#include "mex.h"
#endif
#include "simstruc.h"

#define NUMBER_OF_PARAMS	(2)
#define NUM_LEDS_PARAM		ssGetSFcnParam(S,0)
#define SAMPLE_TIME_PARAM	ssGetSFcnParam(S,1)
#define NUM_LEDS		((uint_T) mxGetPr(NUM_LEDS_PARAM)[0])
#define SAMPLE_TIME             ((real_T) mxGetPr(SAMPLE_TIME_PARAM)[0])

#ifndef MATLAB_MEX_FILE

#include <rtai_netrpc.h>
#include <rtai_msg.h>
#include <rtai_mbx.h>

#define MAX_RTAI_LEDS               1000
#define MBX_RTAI_LED_SIZE           5000
extern SimStruct *rtaiLed[];
extern char *TargetLedMbxID;

#endif

static void mdlInitializeSizes(SimStruct *S)
{
	uint_T i;
	uint_T nchan = NUM_LEDS;

	ssSetNumSFcnParams(S, NUMBER_OF_PARAMS);
	if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
		return;
	}
	for (i = 0; i < NUMBER_OF_PARAMS; i++) {
		ssSetSFcnParamNotTunable(S, i);
	}
	if (!ssSetNumInputPorts(S, nchan)) return;
	ssSetNumOutputPorts(S, 0);
	for (i = 0; i < nchan; i++) {
		ssSetInputPortWidth(S, i, 1);
	        ssSetInputPortDirectFeedThrough(S, i, 1);
	}
	ssSetNumContStates(S, 0);
	ssSetNumDiscStates(S, 0);
	ssSetNumSampleTimes(S, 1);
	ssSetNumPWork(S, 1);
}

static void mdlInitializeSampleTimes(SimStruct *S)
{
	ssSetSampleTime(S, 0, SAMPLE_TIME);
	ssSetOffsetTime(S, 0, 0.0);
}

#define MDL_START 
static void mdlStart(SimStruct *S)
{
#ifndef MATLAB_MEX_FILE
	char name[7];
	MBX *mbx;
	int i;

	for (i = 0; i < MAX_RTAI_LEDS; i++) {
		sprintf(name, "%s%d", TargetLedMbxID, i);
		if (!rt_get_adr(nam2num(name))) break;
	}
	rtaiLed[i] = S;
	if (!(mbx = (MBX *)rt_mbx_init(nam2num(name), (MBX_RTAI_LED_SIZE/(sizeof(unsigned int))*(sizeof(unsigned int)))))) {
		printf("Cannot init mailbox\n");
		exit(1);
	}
	ssGetPWork(S)[0]= (void *)mbx;
#endif
}

static void mdlOutputs(SimStruct *S, int_T tid)
{
	InputRealPtrsType uPtrs = ssGetInputPortRealSignalPtrs(S,0);
	unsigned int led_mask = 0;
	int i;

#ifndef MATLAB_MEX_FILE
	MBX *mbx = (MBX *)ssGetPWorkValue(S,0);
	for (i = 0; i < NUM_LEDS; i++) {
		if (*uPtrs[i] > 0.) {
			led_mask += (1 << i);
		} else {
			led_mask += (0 << i);
		}
	}
	rt_mbx_send_if(mbx, &led_mask, sizeof(led_mask));
#endif
}

static void mdlTerminate(SimStruct *S)
{
#ifndef MATLAB_MEX_FILE
	MBX *mbx = (MBX *)ssGetPWorkValue(S,0);
	rt_mbx_delete(mbx);
#endif
}

#ifdef  MATLAB_MEX_FILE
#include "simulink.c"
#else
#include "cg_sfun.h"
#endif
