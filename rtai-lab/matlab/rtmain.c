/*
  COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
  COPYRIGHT (C) 2003  Paolo Mantegazza (mantegazza@aero.polimi.it)
  COPYRIGHT (C) 2003  Roberto Bucher (roberto.bucher@supsi.ch)
  COPYRIGHT (C) 2003  Daniele Gasperini (daniele.gasperini@elet.polimi.it)

  COPYRIGHT (C) 2009  Henrik Slotholt (rtai@slotholt.net)

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

#define _XOPEN_SOURCE	600

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/io.h>
#include <getopt.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>

#include "tmwtypes.h"
#include "rtmodel.h"
#include "rt_sim.h"
#include "rt_nonfinite.h"
#include "mdl_info.h"
#include "bio_sig.h"
#include "simstruc.h"

#include <rtai_netrpc.h>
#include <rtai_msg.h>
#include <rtai_mbx.h>
#include <rtai_fifos.h>

#define EXPAND_CONCAT(name1,name2)	name1 ## name2
#define CONCAT(name1,name2)		EXPAND_CONCAT(name1,name2)
#define RT_MODEL			CONCAT(MODEL,_rtModel)

#if TID01EQ == 1
#define FIRST_TID 1
#else
#define FIRST_TID 0
#endif
#ifndef RT
# error "must define RT"
#endif
#ifndef MODEL
# error "must define MODEL"
#endif
#ifndef NUMST
# error "must define number of sample times, NUMST"
#endif
#ifndef NCSTATES
# error "must define NCSTATES"
#endif

extern void MdlInitializeSizes(void);
extern void MdlInitializeSampleTimes(void);
extern void MdlStart(void);
extern void MdlOutputs(int_T tid);
extern void MdlUpdate(int_T tid);
extern void MdlTerminate(void);
#define INITIALIZE_SIZES(M)		MdlInitializeSizes()
#define INITIALIZE_SAMPLE_TIMES(M)	MdlInitializeSampleTimes()
#define START(M)			MdlStart()
#define OUTPUTS(M,tid)			MdlOutputs(tid)
#define UPDATED(M,tid)			MdlUpdate(tid)
#define TERMINATE(M)			MdlTerminate()
#if NCSTATES > 0
extern void rt_ODECreateIntegrationData(RTWSolverInfo *si);
extern void rt_ODEUpdateContinuousStates(RTWSolverInfo *si);
#else
#define rt_ODECreateIntegrationData(si)		rtsiSetSolverName(si,"FixedStepDiscrete")
#define rt_ODEUpdateContinuousStates(si)	rtsiSetT(si,rtsiGetSolverStopTime(si))
#endif

extern RT_MODEL *MODEL(void);
static RT_MODEL *rtM;

#define RTAILAB_VERSION         "3.5.1"
#define MAX_NTARGETS		1000
#define MAX_NAMES_SIZE		256
#define RUN_FOREVER		-1.0
#define POLL_PERIOD		100000000
#define CONNECT_TO_TARGET	0
#define DISCONNECT_TO_TARGET	1
#define START_TARGET		2
#define STOP_TARGET		3
#define UPDATE_PARAM		4

#define MAX_DATA_SIZE		30

#define rt_MainTaskPriority		97
#define rt_HostInterfaceTaskPriority	96

typedef struct rtTargetParamInfo {
  char modelName[MAX_NAMES_SIZE];
  char blockName[MAX_NAMES_SIZE];
  char paramName[MAX_NAMES_SIZE];
  unsigned int nRows;
  unsigned int nCols;
  unsigned int dataType;
  unsigned int dataClass;
  double dataValue[MAX_DATA_SIZE];
} rtTargetParamInfo;

static sem_t err_sem;
static RT_TASK *rt_MainTask;

static pthread_t rt_HostInterfaceThread;
static RT_TASK *rt_HostInterfaceTask;

static pthread_t rt_BaseRateThread;
static RT_TASK *rt_BaseRateTask;

#ifdef MULTITASKING
static pthread_t *rt_SubRateThreads;
static RT_TASK **rt_SubRateTasks;
#endif

char *HostInterfaceTaskName     = "IFTASK";
char *TargetScopeMbxID	        = "RTS";
char *TargetALogMbxID           = "RAL";
char *TargetLogMbxID            = "RTL";
char *TargetLedMbxID            = "RTE";
char *TargetMeterMbxID	        = "RTM";
char *TargetSynchronoscopeMbxID = "RTY";

static volatile int Verbose       = 0;
static volatile int UseHRT        = 1;
static volatile int WaitToStart   = 0;
static volatile int IsRunning     = 0;
static volatile int InternalTimer = 1;
static volatile int ClockTick     = 1;
static volatile int CpuMap	  = 0xF;
static volatile int StackInc	  = 30000;
static volatile int endBaseRate   = 0;
static volatile int endInterface  = 0;
static float FinalTime            = RUN_FOREVER;
static RTIME rt_BaseRateTick;
#ifdef MULTITASKING
static volatile int endSubRate    = 0;
#endif

static volatile int endex;

#ifdef TASKDURATION
RTIME RTTSKinit=0, RTTSKend=0;
#endif

#define MAX_RTAI_SCOPES	1000
#define MAX_RTAI_LOGS	1000
#define MAX_RTAI_LEDS	1000
#define MAX_RTAI_METERS	1000
#define MAX_RTAI_SYNCHS	1000
SimStruct *rtaiScope[MAX_RTAI_SCOPES];
SimStruct *rtaiALog[MAX_RTAI_LOGS];
SimStruct *rtaiLog[MAX_RTAI_LOGS];
SimStruct *rtaiLed[MAX_RTAI_LEDS];
SimStruct *rtaiMeter[MAX_RTAI_METERS];
SimStruct *rtaiSynchronoscope[MAX_RTAI_SYNCHS];

#define MAX_COMEDI_DEVICES        10	

void *ComediDev[MAX_COMEDI_DEVICES];
int ComediDev_InUse[MAX_COMEDI_DEVICES] = {0};
int ComediDev_AIInUse[MAX_COMEDI_DEVICES] = {0};
int ComediDev_AOInUse[MAX_COMEDI_DEVICES] = {0};
int ComediDev_DIOInUse[MAX_COMEDI_DEVICES] = {0};

static void (*WaitTimingEvent)(unsigned long);
static void (*SendTimingEvent)(unsigned long);
static unsigned long TimingEventArg;
static void DummyWait(void) { }
static void DummySend(void) { }

#define XSTR(x)    #x
#define STR(x)     XSTR(x)


static inline void strncpyz(char *dest, const char *src, size_t n)
{
  if (src != NULL) {
    strncpy(dest, src, n);
    n = strlen(src);
  } else
    n = 0;
		
  dest[n] = '\0';
}

static inline int set_double(double *to, double *from)
{
	if (to && from) {
		if (sizeof(unsigned long) == 8) {
			to[0] = from[0];
		} else { /* What below is hacked from /usr/src/linux/include/asm-i386/system.h */
			unsigned long l = ((unsigned long *)from)[0];
			unsigned long h = ((unsigned long *)from)[1];
			__asm__ __volatile__ ("1: movl (%0), %%eax; movl 4(%0), %%edx; lock; cmpxchg8b (%0); jnz 1b" : : "D"(to), "b"(l), "c"(h) : "ax", "dx", "memory");
		}
		return 1;
	}
	return 0;
}

#define RT_MODIFY_PARAM_VALUE_IF(rtcase, rttype) \
	case rtcase: \
		for (paramIdx = 0; paramIdx < nParams; paramIdx++) { \
			rttype *param = (rttype *)(pMap[mapOffset + paramIdx]); \
			switch (ptinfoGetClass(ptRec)) { \
				case rt_SCALAR: \
					set_double(param, (double *)_newVal); \
					if (Verbose) { \
						printf("%s : %G\n", mmiGetBlockTuningBlockName(mmi,i), *param); \
					} \
					break; \
				case rt_VECTOR: \
					param[matIdx] = ((double *) _newVal)[0]; \
					if (Verbose) { \
						for (colIdx = 0; colIdx < nCols; colIdx++) { \
							printf("%s : %G\n", mmiGetBlockTuningBlockName(mmi,i), param[colIdx]); \
						} \
					} \
					break; \
				case rt_MATRIX_ROW_MAJOR: \
					param[matIdx] = ((double *) _newVal)[0]; \
					if (Verbose) { \
						for (rowIdx = 0; rowIdx < nRows; rowIdx++) { \
							for(colIdx = 0; colIdx < nCols; colIdx++){ \
								printf("%s : %G\n", mmiGetBlockTuningBlockName(mmi,i), param[rowIdx*nCols+colIdx]); \
							} \
						} \
					} \
					break; \
				case rt_MATRIX_COL_MAJOR: \
					param[matIdx] = ((double *) _newVal)[0]; \
					if (Verbose) { \
						for (rowIdx = 0; rowIdx < nRows; rowIdx++) { \
							for(colIdx = 0; colIdx < nCols; colIdx++) { \
								printf("%s : %G\n", mmiGetBlockTuningBlockName(mmi,i), param[colIdx*nRows+rowIdx]); \
							} \
						} \
					} \
					break; \
				default: \
					return(1); \
			} \
		} \
		break;

int_T rt_ModifyParameterValue(void *mpi, int i, int matIdx, void *_newVal)
{
  ModelMappingInfo *mmi;
  ParameterTuning *ptRec;
  void * const     *pMap;
  uint_T nRows;
  uint_T nCols;
  uint_T nParams;
  uint_T mapOffset;
  uint_T paramIdx;
  uint_T rowIdx;
  uint_T colIdx;

  mmi   = (ModelMappingInfo *)mpi;
  ptRec = (ParameterTuning*)mmiGetBlockTuningParamInfo(mmi,i);
  pMap  = mmiGetParametersMap(mmi);
  nRows = ptinfoGetNumRows(ptRec);
  nCols = ptinfoGetNumCols(ptRec);
  nParams   = ptinfoGetNumInstances(ptRec);
  mapOffset = ptinfoGetParametersOffset(ptRec);

  switch (ptinfoGetDataTypeEnum(ptRec)) {
    RT_MODIFY_PARAM_VALUE_IF(SS_DOUBLE, real_T)
      RT_MODIFY_PARAM_VALUE_IF(SS_SINGLE, real32_T)
      RT_MODIFY_PARAM_VALUE_IF(SS_INT8, int8_T)
      RT_MODIFY_PARAM_VALUE_IF(SS_UINT8, uint8_T)
      RT_MODIFY_PARAM_VALUE_IF(SS_INT16, int16_T)
      RT_MODIFY_PARAM_VALUE_IF(SS_UINT16, uint16_T)
      RT_MODIFY_PARAM_VALUE_IF(SS_INT32, int32_T)
      RT_MODIFY_PARAM_VALUE_IF(SS_UINT32, uint32_T)
      RT_MODIFY_PARAM_VALUE_IF(SS_BOOLEAN, boolean_T)
      default:
    return(1);
  }

  return(0);
}

#define RT_GET_PARAM_INFO_IF(rtcase, rttype) \
	case rtcase: \
		for (paramIdx = 0; paramIdx < nParams; paramIdx++) { \
			rttype *param = (rttype *)(pMap[mapOffset + paramIdx]); \
			switch (ptinfoGetClass(ptRec)) { \
				case rt_SCALAR: \
					rtpi->dataValue[0] = *param; \
					break; \
				case rt_VECTOR: \
					for (colIdx = 0; colIdx < nCols; colIdx++) { \
						rtpi->dataValue[colIdx] = param[colIdx]; \
					} \
					break; \
				case rt_MATRIX_ROW_MAJOR: \
					for (rowIdx = 0; rowIdx < nRows; rowIdx++) { \
						for (colIdx = 0; colIdx < nCols; colIdx++) { \
							rtpi->dataValue[rowIdx*nCols+colIdx] = param[rowIdx*nCols+colIdx]; \
						} \
					} \
					break; \
				case rt_MATRIX_COL_MAJOR: \
					for (rowIdx = 0; rowIdx < nRows; rowIdx++) { \
						for (colIdx = 0; colIdx < nCols; colIdx++) { \
							rtpi->dataValue[colIdx*nRows+rowIdx] = param[colIdx*nRows+rowIdx]; \
						} \
					} \
					break; \
				default: \
					return(1); \
			} \
		} \
		break;

int_T rt_GetParameterInfo(void *mpi, rtTargetParamInfo *rtpi, int i)
{
  ModelMappingInfo *mmi;
  ParameterTuning *ptRec;
  void * const     *pMap;
  uint_T nRows;
  uint_T nCols;
  uint_T nParams;
  uint_T mapOffset;
  uint_T paramIdx;
  uint_T rowIdx;
  uint_T colIdx;
  uint_T dataType;
  uint_T dataClass;

  mmi   = (ModelMappingInfo *)mpi;
	
  ptRec = (ParameterTuning*)mmiGetBlockTuningParamInfo(mmi,i);
  pMap  = mmiGetParametersMap(mmi);
  nRows = ptinfoGetNumRows(ptRec);
  nCols = ptinfoGetNumCols(ptRec);
  nParams   = ptinfoGetNumInstances(ptRec);
  mapOffset = ptinfoGetParametersOffset(ptRec);
  dataType  = ptinfoGetDataTypeEnum(ptRec);
  dataClass = ptinfoGetClass(ptRec);

  strncpyz(rtpi->modelName, STR(MODEL), MAX_NAMES_SIZE);
  strncpyz(rtpi->blockName, mmiGetBlockTuningBlockName(mmi,i), MAX_NAMES_SIZE);
  strncpyz(rtpi->paramName, mmiGetBlockTuningParamName(mmi,i), MAX_NAMES_SIZE);
  rtpi->dataType  = dataType;
  rtpi->dataClass = dataClass;
  rtpi->nRows = nRows;
  rtpi->nCols = nCols;

  switch (ptinfoGetDataTypeEnum(ptRec)) {
    RT_GET_PARAM_INFO_IF(SS_DOUBLE, real_T)
      RT_GET_PARAM_INFO_IF(SS_SINGLE, real32_T)
      RT_GET_PARAM_INFO_IF(SS_INT8, int8_T)
      RT_GET_PARAM_INFO_IF(SS_UINT8, uint8_T)
      RT_GET_PARAM_INFO_IF(SS_INT16, int16_T)
      RT_GET_PARAM_INFO_IF(SS_UINT16, uint16_T)
      RT_GET_PARAM_INFO_IF(SS_INT32, int32_T)
      RT_GET_PARAM_INFO_IF(SS_UINT32, uint32_T)
      RT_GET_PARAM_INFO_IF(SS_BOOLEAN, boolean_T)
      default:
    return(1);
  }

  return(0);
}

#ifdef MULTITASKING
static void *rt_SubRate(int *arg)
{
  int sample, i;
  char myname[7];
  int rt_SubRateTaskPriority;

  sample = arg[0];
  rt_SubRateTaskPriority = arg[1];

  rt_allow_nonroot_hrt();
  for (i = 0; i < MAX_NTARGETS; i++) {
    sprintf(myname, "TSR%d", i);
    if (!rt_get_adr(nam2num(myname))) break;
  }
  if (!(rt_SubRateTasks[sample] = rt_task_init_schmod(nam2num(myname), rt_SubRateTaskPriority, 0, 0, SCHED_FIFO, CpuMap))) {
    printf("Cannot init rt_SubRateTask #%d\n", sample);
    return (void *)1;
  }
  //	sem_post(&err_sem);

  iopl(3);
  rt_task_use_fpu(rt_SubRateTasks[sample], 1);
  rt_grow_and_lock_stack(StackInc);

  if (UseHRT) {
    rt_make_hard_real_time();
  }
  while (!endSubRate) {
    rt_task_suspend(rt_SubRateTasks[sample]);
    if (endSubRate) break;
    OUTPUTS(rtM, sample);
    UPDATED(rtM, sample);
    rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(rtM), rtmGetTimingData(rtM), sample);
  }
  if (UseHRT) {
    rt_make_soft_real_time();
  }

  rt_task_delete(rt_SubRateTasks[sample]);
  return (void *)(rt_SubRateTasks[sample] = 0);
}
#endif

static void *rt_BaseRate(void *args)
{
  real_T tnext;
  char myname[7];
  int i;
#ifdef MULTITASKING
  int_T  sample, *sampleHit = rtmGetSampleHitPtr(rtM);
#endif

  int rt_BaseRateTaskPriority = *((int *)args);

  rt_allow_nonroot_hrt();
  for (i = 0; i < MAX_NTARGETS; i++) {
    sprintf(myname, "TBR%d", i);
    if (!rt_get_adr(nam2num(myname))) break;
  }
  if (!(rt_BaseRateTask = rt_task_init_schmod(nam2num(myname), rt_BaseRateTaskPriority, 0, 0, SCHED_FIFO, CpuMap))) {
    printf("Cannot init rt_BaseRateTask\n");
    return (void *)1;
  }
  sem_post(&err_sem);

  iopl(3);
  rt_task_use_fpu(rt_BaseRateTask, 1);
  rt_grow_and_lock_stack(StackInc);
  if (UseHRT) {
    rt_make_hard_real_time();
  }
  rt_rpc(rt_MainTask,0,(void *)myname);
  rt_task_make_periodic(rt_BaseRateTask, rt_get_time() + rt_BaseRateTick, rt_BaseRateTick);

  while (!endBaseRate) {
#ifdef TASKDURATION
    RTTSKend=rt_get_cpu_time_ns();
#endif
    WaitTimingEvent(TimingEventArg);
    if (endBaseRate) break;

#ifdef TASKDURATION
    RTTSKinit=rt_get_cpu_time_ns();
#endif

#ifdef MULTITASKING
    tnext = rt_SimUpdateDiscreteEvents(rtmGetNumSampleTimes(rtM), rtmGetTimingData(rtM), rtmGetSampleHitPtr(rtM), rtmGetPerTaskSampleHitsPtr(rtM));
    rtsiSetSolverStopTime(rtmGetRTWSolverInfo(rtM), tnext);
    for (sample = FIRST_TID + 1; sample < NUMST; sample++) {
      if (sampleHit[sample]) {
	rt_task_resume(rt_SubRateTasks[sample]);
      }
    }
    OUTPUTS(rtM, FIRST_TID);
    UPDATED(rtM, FIRST_TID);
    if (rtmGetSampleTime(rtM, 0) == CONTINUOUS_SAMPLE_TIME) {
      rt_ODEUpdateContinuousStates(rtmGetRTWSolverInfo(rtM));
    } else {
      rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(rtM), rtmGetTimingData(rtM), 0);
    }
#if FIRST_TID == 1
    rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(rtM), rtmGetTimingData(rtM), 1);
#endif
#else
    tnext = rt_SimGetNextSampleHit();
    rtsiSetSolverStopTime(rtmGetRTWSolverInfo(rtM), tnext);
    OUTPUTS(rtM, 0);
    UPDATED(rtM, 0);
    rt_SimUpdateDiscreteTaskSampleHits(rtmGetNumSampleTimes(rtM), rtmGetTimingData(rtM), rtmGetSampleHitPtr(rtM), rtmGetTPtr(rtM));
    if (rtmGetSampleTime(rtM,0) == CONTINUOUS_SAMPLE_TIME) {
      rt_ODEUpdateContinuousStates(rtmGetRTWSolverInfo(rtM));
    }
#endif
  }

  if (UseHRT) {
    rt_make_soft_real_time();
  }
  rt_task_delete(rt_BaseRateTask);
  return 0;
}

static void *rt_HostInterface(void *args)
{
  RT_TASK *task;
  unsigned int IRequest;
  char Request;
  long len;

  rt_allow_nonroot_hrt();
  if (!(rt_HostInterfaceTask = rt_task_init_schmod(nam2num(HostInterfaceTaskName), rt_HostInterfaceTaskPriority, 0, 0, SCHED_FIFO, 0xFF))) {
    printf("Cannot init rt_HostInterfaceTask\n");
    return (void *)1;
  }
  sem_post(&err_sem);

  while (!endInterface) {
    task = rt_receive(0, &IRequest);
    Request = (char)(IRequest);
		
    if (endInterface) break;

    switch (Request) {

    case 'c': {
      ModelMappingInfo *MMI;
      uint_T nBlockParams;
      { int Reply;
      MMI = (ModelMappingInfo *)rtmGetModelMappingInfo(rtM);
      nBlockParams = mmiGetNumBlockParams(MMI);
      Reply = (IsRunning << 16) | (nBlockParams & 0xffff); /* max 2^16-1 blocks */
      rt_return(task, Reply);
      }
      { int i;
      rtTargetParamInfo rtParameters;
      rt_receivex(task, &rtParameters, sizeof(char), &len);
      rt_GetParameterInfo(MMI, &rtParameters, 0);
      rt_returnx(task, &rtParameters, sizeof(rtParameters));
					  
      for (i = 0; i < nBlockParams; i++) {
	rt_receivex(task, &rtParameters, sizeof(char), &len);
	rt_GetParameterInfo(MMI, &rtParameters, i);
	rt_returnx(task, &rtParameters, sizeof(rtParameters));
      } 
      }
      { int scopeIdx, Reply;
      float samplingTime;
      int ntraces;
      while (1) {
	rt_receivex(task, &scopeIdx, sizeof(int), &len);
	if (scopeIdx < 0) {
	  Reply = scopeIdx;
	  rt_returnx(task, &Reply, sizeof(int));
	  break;
	} else {
	  ntraces = ssGetNumInputPorts(rtaiScope[scopeIdx]);
	  rt_returnx(task, &ntraces, sizeof(int));
	  rt_receivex(task, &scopeIdx, sizeof(int), &len);
	  rt_returnx(task, (char *)ssGetModelName(rtaiScope[scopeIdx]), MAX_NAMES_SIZE*sizeof(char));
		while(1) {
	      int j;
	      rt_receivex(task, &j, sizeof(int), &len);
				if(j < 0) break;
	      char traceName[10];
				sprintf(traceName, "Trace %d", j+1);
	      rt_returnx(task, traceName, strlen(traceName)+1); // return null terminated string	    
	  }
	  samplingTime = ssGetSampleTime(rtaiScope[scopeIdx],0);
	  rt_returnx(task, &samplingTime, sizeof(float));
	}
      }
      }
      { int logIdx, Reply;
      float samplingTime;
      int nrow, ncol, *dim;
      while (1) {
	rt_receivex(task, &logIdx, sizeof(int), &len);
	if (logIdx < 0) {
	  Reply = logIdx;
	  rt_returnx(task, &Reply, sizeof(int));
	  break;
	} else {
	  dim = ssGetInputPortDimensions(rtaiLog[logIdx],0);
	  nrow = dim[0];
	  ncol = dim[1];
	  rt_returnx(task, &nrow, sizeof(int));
	  rt_receivex(task, &logIdx, sizeof(int), &len);
	  rt_returnx(task, &ncol, sizeof(int));
	  rt_receivex(task, &logIdx, sizeof(int), &len);
	  rt_returnx(task, (char *)ssGetModelName(rtaiLog[logIdx]), MAX_NAMES_SIZE*sizeof(char));
	  rt_receivex(task, &logIdx, sizeof(int), &len);
	  samplingTime = ssGetSampleTime(rtaiLog[logIdx],0);
	  rt_returnx(task, &samplingTime, sizeof(float));
	}
      }
      }
      { int alogIdx, Reply;  /* section added to support automatic log block  taken from log code */
      float samplingTime;
      int nrow, ncol, *dim;
      while (1) {
	rt_receivex(task, &alogIdx, sizeof(int), &len);
	if (alogIdx < 0) {
	  Reply = alogIdx;
	  rt_returnx(task, &Reply, sizeof(int));
	  break;
	} else {
	  dim = ssGetInputPortDimensions(rtaiALog[alogIdx],0);
	  nrow = dim[0];
	  ncol = dim[1];
	  rt_returnx(task, &nrow, sizeof(int));
	  rt_receivex(task, &alogIdx, sizeof(int), &len);
	  rt_returnx(task, &ncol, sizeof(int));
	  rt_receivex(task, &alogIdx, sizeof(int), &len);
	  rt_returnx(task, (char *)ssGetModelName(rtaiALog[alogIdx]), MAX_NAMES_SIZE*sizeof(char));
	  rt_receivex(task, &alogIdx, sizeof(int), &len);
	  samplingTime = ssGetSampleTime(rtaiALog[alogIdx],0);
	  rt_returnx(task, &samplingTime, sizeof(float));
	}
      }
      }
      { int ledIdx, Reply;
      float samplingTime;
      int n_leds;
      while (1) {
	rt_receivex(task, &ledIdx, sizeof(int), &len);
	if (ledIdx < 0) {
	  Reply = ledIdx;
	  rt_returnx(task, &Reply, sizeof(int));
	  break;
	} else {
	  n_leds = ssGetNumInputPorts(rtaiLed[ledIdx]);
	  rt_returnx(task, &n_leds, sizeof(int));
	  rt_receivex(task, &ledIdx, sizeof(int), &len);
	  rt_returnx(task, (char *)ssGetModelName(rtaiLed[ledIdx]), MAX_NAMES_SIZE*sizeof(char));
	  rt_receivex(task, &ledIdx, sizeof(int), &len);
	  samplingTime = ssGetSampleTime(rtaiLed[ledIdx],0);
	  rt_returnx(task, &samplingTime, sizeof(float));
	}
      }
      }
      { int meterIdx, Reply;
      float samplingTime;
      while (1) {
	rt_receivex(task, &meterIdx, sizeof(int), &len);
	if (meterIdx < 0) {
	  Reply = meterIdx;
	  rt_returnx(task, &Reply, sizeof(int));
	  break;
	} else {
	  rt_returnx(task, (char *)ssGetModelName(rtaiMeter[meterIdx]), MAX_NAMES_SIZE*sizeof(char));
	  rt_receivex(task, &meterIdx, sizeof(int), &len);
	  samplingTime = ssGetSampleTime(rtaiMeter[meterIdx],0);
	  rt_returnx(task, &samplingTime, sizeof(float));
	}
      }
      }
      { int synchIdx, Reply;
      float samplingTime;
      while (1) {
	rt_receivex(task, &synchIdx, sizeof(int), &len);
	if (synchIdx < 0) {
	  Reply = synchIdx;
	  rt_returnx(task, &Reply, sizeof(int));
	  break;
	} else {
	  rt_returnx(task, (char *)ssGetModelName(rtaiSynchronoscope[synchIdx]), MAX_NAMES_SIZE*sizeof(char));
	  rt_receivex(task, &synchIdx, sizeof(int), &len);
	  samplingTime = ssGetSampleTime(rtaiSynchronoscope[synchIdx],0);
	  rt_returnx(task, &samplingTime, sizeof(float));
	}
      }
      }
      break;
    }

    case 's': { int Reply = 1;

    rt_task_resume(rt_MainTask);
    rt_return(task, Reply);
    break;
    }

    case 't': { int Reply = 0;

    rt_return(task, Reply);
    endex = 1;
    break;
    }

    case 'p': { ModelMappingInfo *MMI;
    int index;
    int Reply;
    int matIdx;
    double newv;

    rt_return(task, IsRunning);
    Reply = 0;
    rt_receivex(task, &index, sizeof(int), &len);
    rt_returnx(task, &Reply, sizeof(int));
    Reply = 1;
    MMI = (ModelMappingInfo *)rtmGetModelMappingInfo(rtM);
    rt_receivex(task, &newv, sizeof(double), &len);
    rt_returnx(task, &Reply, sizeof(int));
    rt_receivex(task, &matIdx, sizeof(int), &len);
    rt_ModifyParameterValue(MMI, index, matIdx, &newv);
    rt_returnx(task, &Reply, sizeof(int));
    break;			
    }

    case 'g': { int i;
    uint_T nBlockParams;
    rtTargetParamInfo rtParameters;
    ModelMappingInfo *MMI;

    rt_return(task, IsRunning);
    MMI = (ModelMappingInfo *)rtmGetModelMappingInfo(rtM);
    nBlockParams = mmiGetNumBlockParams(MMI);
    for (i = 0; i < nBlockParams; i++) {
      rt_receivex(task, &rtParameters, sizeof(char), &len);
      rt_GetParameterInfo(MMI, &rtParameters, i);
      rt_returnx(task, &rtParameters, sizeof(rtParameters));
    } 
    break;			
    }

    case 'd': { int ParamCnt;
    int Reply;

    rt_return(task, IsRunning);
    Reply = 0;
    rt_receivex(task, &ParamCnt, sizeof(int), &len);
    rt_returnx(task, &Reply, sizeof(int));

    { int i;
    ModelMappingInfo *MMI;
    struct {
      int index;
      int mat_index;
      double value;
    } BatchParams[ParamCnt];

    Reply = 1;
    MMI = (ModelMappingInfo *)rtmGetModelMappingInfo(rtM);
    rt_receivex(task, &BatchParams, sizeof(BatchParams), &len);
    for (i = 0; i < ParamCnt; i++) {
      rt_ModifyParameterValue(MMI, BatchParams[i].index, BatchParams[i].mat_index, &BatchParams[i].value);
    }
    }
    rt_returnx(task, &ParamCnt, sizeof(int));
    break;
    }

    case 'm': { float ttime = (float)rtmGetT(rtM);
    int Reply;

    rt_return(task, IsRunning);
    rt_receivex(task, &Reply, sizeof(int), &len);
    rt_returnx(task, &ttime, sizeof(float));
    }

    default:
      break;
    }

  }

  rt_task_delete(rt_HostInterfaceTask);

  return 0;
}

static int_T rt_Main(RT_MODEL * (*model_name)(void), int_T priority)
{
  const char *status;
  RTIME rt_BaseTaskPeriod;
  struct timespec rt_MainTaskPollPeriod = { 0, POLL_PERIOD };
  struct timespec err_timeout;
  int msg, i;
  char myname[7];
  SEM *hard_timers_cnt = NULL;
#ifdef MULTITASKING
  int sample, *taskarg;
#endif

  rt_allow_nonroot_hrt();
  for (i = 0; i < MAX_NTARGETS; i++) {
    sprintf(myname, "TMN%d", i);
    if (!rt_get_adr(nam2num(myname))) break;
  }
  if (!(rt_MainTask = rt_task_init_schmod(nam2num(myname), rt_MainTaskPriority, 0, 0, SCHED_FIFO, 0xFF))) {
    fprintf(stderr, "Cannot init rt_MainTask\n");
    return 1;
  }
  sem_init(&err_sem, 0, 0);
  iopl(3);

  rt_InitInfAndNaN(sizeof(real_T));
  rtM = model_name();
  if (rtM == NULL) {
    fprintf(stderr, "Memory allocation error during target registration.\n");
    goto finish;
  }
  if (rtmGetErrorStatus(rtM) != NULL) {
    fprintf(stderr, "Error during target registration: %s.\n", rtmGetErrorStatus(rtM));
    goto finish;
  }

  if (FinalTime > 0.0 || FinalTime == RUN_FOREVER) {
    rtmSetTFinal(rtM, (real_T)FinalTime);
  }

  INITIALIZE_SIZES(rtM);
  INITIALIZE_SAMPLE_TIMES(rtM);

  status = rt_SimInitTimingEngine(rtmGetNumSampleTimes(rtM), rtmGetStepSize(rtM), rtmGetSampleTimePtr(rtM), rtmGetOffsetTimePtr(rtM), rtmGetSampleHitPtr(rtM), rtmGetSampleTimeTaskIDPtr(rtM), rtmGetTStart(rtM), &rtmGetSimTimeStep(rtM), &rtmGetTimingData(rtM));
  if (status != NULL) {
    fprintf(stderr, "Failed to initialize target sample time engine: %s.\n", status);
    goto finish;
  }

  rt_ODECreateIntegrationData(rtmGetRTWSolverInfo(rtM));

  START(rtM);
  if (rtmGetErrorStatus(rtM) != NULL) {
    fprintf(stderr, "Failed in target initialization.\n");
    TERMINATE(rtM);
    fprintf(stderr, "Target is terminated.\n");
    goto finish;
  }

  if ((pthread_create(&rt_HostInterfaceThread, NULL, rt_HostInterface, NULL)) != 0) {
    fprintf(stderr, "Failed to create HostInterfaceThread.\n");
    TERMINATE(rtM);
    fprintf(stderr, "Target is terminated.\n");
    goto finish;
  }
  err_timeout.tv_sec = (long int)(time(NULL)) + 1;
  err_timeout.tv_nsec = 0;
  if ((sem_timedwait(&err_sem, &err_timeout)) != 0) {
    TERMINATE(rtM);
    fprintf(stderr, "Target is terminated.\n");
    goto finish;
  }

#ifdef MULTITASKING
  rt_SubRateTasks   = malloc(NUMST*sizeof(RT_TASK *));
  rt_SubRateThreads = malloc(NUMST*sizeof(pthread_t));
  taskarg           = malloc(2*NUMST*sizeof(int));
  for (sample = FIRST_TID + 1; sample < NUMST; sample++) {
    taskarg[0] = sample;
    taskarg[1] = priority + sample;
    pthread_create(rt_SubRateThreads + sample, NULL, (void *)rt_SubRate, (void *)taskarg);
    taskarg += 2;
  }
#endif
  if ((pthread_create(&rt_BaseRateThread, NULL, rt_BaseRate, &priority)) != 0) {
    fprintf(stderr, "Failed to create BaseRateThread.\n");
    endInterface = 1;
    rt_send(rt_HostInterfaceTask, 0);
    pthread_join(rt_HostInterfaceThread, NULL);
    TERMINATE(rtM);
    fprintf(stderr, "Target is terminated.\n");
    goto finish;
  }
  err_timeout.tv_sec = (long int)(time(NULL)) + 1;
  err_timeout.tv_nsec = 0;
  if ((sem_timedwait(&err_sem, &err_timeout)) != 0) {
    endInterface = 1;
    rt_send(rt_HostInterfaceTask, 0);
    pthread_join(rt_HostInterfaceThread, NULL);
    TERMINATE(rtM);
    fprintf(stderr, "Target is terminated.\n");
    goto finish;
  }

  rt_BaseTaskPeriod = (RTIME)(1000000000.0*rtmGetStepSize(rtM));
  if (InternalTimer) {
    WaitTimingEvent = (void *)rt_task_wait_period;
    if (!(hard_timers_cnt = rt_get_adr(nam2num("HTMRCN")))) {
      if (!ClockTick) {
	rt_set_oneshot_mode();
	start_rt_timer(0);
	rt_BaseRateTick = nano2count(rt_BaseTaskPeriod);
      } else {
	rt_set_periodic_mode();
	rt_BaseRateTick = start_rt_timer(nano2count(rt_BaseTaskPeriod));
      }
      hard_timers_cnt = rt_sem_init(nam2num("HTMRCN"), 0);
    } else {
      rt_BaseRateTick = nano2count(rt_BaseTaskPeriod);
      rt_sem_signal(hard_timers_cnt);
    }
  } else {
    WaitTimingEvent = (void *)DummyWait;
    SendTimingEvent = (void *)DummySend;
  }

  if (Verbose) {
    int j;
    printf("\nTarget info\n");
    printf("===========\n");
    printf("  Model name             : %s\n", STR(MODEL));
    printf("  Base sample time       : %f [s]\n", rtmGetStepSize(rtM));
    printf("  Number of sample times : %d\n", rtmGetNumSampleTimes(rtM));
    for (j = 0; j < rtmGetNumSampleTimes(rtM); j++) {
      printf("  Sample Time %d          : %f [s]\n", j, rtmGetSampleTimePtr(rtM)[j]);
    }
    printf("\n");
  }

  if (WaitToStart) {
    if (Verbose) {
      printf("Target is waiting to start.\n");
    }
    rt_task_suspend(rt_MainTask);
  }
  rt_receive(0, &msg);
  rt_return(rt_BaseRateTask,0);
  IsRunning = 1;
  if (Verbose) {
    printf("Target is running.\n");
  }

  while (!endex) {
    if (rtmGetTFinal(rtM) != RUN_FOREVER && (rtmGetTFinal(rtM) - rtmGetT(rtM)) <= rtmGetT(rtM)*DBL_EPSILON) {
      if (Verbose) {
	printf("Final time occured.\n");
      }
      break;
    }
    if (rtmGetErrorStatus(rtM) != NULL) {
      fprintf(stderr, "%s.\n", rtmGetErrorStatus(rtM));
      break;
    }
    nanosleep(&rt_MainTaskPollPeriod, NULL);
  }

  endBaseRate = 1;
  if (!InternalTimer) {
    SendTimingEvent(TimingEventArg);
  }
  pthread_join(rt_BaseRateThread, NULL);

#ifdef MULTITASKING
  endSubRate = 1;
  for (sample = FIRST_TID + 1; sample < NUMST; sample++) {
    if (rt_SubRateTasks[sample]) {
      rt_task_resume(rt_SubRateTasks[sample]);
    }
    pthread_join(rt_SubRateThreads[sample], NULL);
  }
#endif

  IsRunning = 0;
  if (Verbose) {
    printf("Target is stopped.\n");
  }

  endInterface = 1;
  rt_send(rt_HostInterfaceTask, 0);
  pthread_join(rt_HostInterfaceThread, NULL);

  if (InternalTimer) {
    if (!rt_sem_wait_if(hard_timers_cnt)) {
      rt_sem_delete(hard_timers_cnt);
    }
  }
	
#ifdef MULTITASKING
  free(rt_SubRateTasks);
  free(rt_SubRateThreads);
#endif

  TERMINATE(rtM);
  printf("Target is terminated.\n");

 finish:
  sem_destroy(&err_sem);
  rt_task_delete(rt_MainTask);

  return 0;
}

static void endme(int dummy)
{
  signal(SIGINT, endme);
  signal(SIGTERM, endme);
  endex = 1;
}

struct option options[] = {
  { "usage",      0, 0, 'u' },
  { "verbose",    0, 0, 'v' },
  { "version",    0, 0, 'V' },
  { "soft",       0, 0, 's' },
  { "wait",       0, 0, 'w' },
  { "priority",   1, 0, 'p' },
  { "finaltime",  1, 0, 'f' },
  { "name",       1, 0, 'n' },
  { "idscope",    1, 0, 'i' },
  { "idlog",      1, 0, 'l' },
  { "idalog",     1, 0, 'a' },
  { "idmeter",    1, 0, 't' },
  { "idled",      1, 0, 'd' },
  { "idsynch",    1, 0, 'y' },
  { "cpumap",     1, 0, 'c' },
  { "external",   0, 0, 'e' },
  { "oneshot",    0, 0, 'o' },
  { "stack",      1, 0, 'm' }
};

void print_usage(void)
{
  fputs(
	("\nUsage:  'RT-model-name' [OPTIONS]\n"
	 "\n"
	 "OPTIONS:\n"
	 "  -u, --usage\n"
	 "      print usage\n"
	 "  -v, --verbose\n"
	 "      verbose output\n"
	 "  -V, --version\n"
	 "      print rt_main version\n"
	 "  -s, --soft\n"
	 "      run RT-model in soft real time (default hard RT)\n"
	 "  -w, --wait\n"
	 "      wait to start\n"
	 "  -p <priority>, --priority <priority>\n"
	 "      set the priority at which the RT-model's highest priority task will run (default 0)\n"
	 "  -f <finaltime>, --finaltime <finaltime>\n"
	 "      set the final time (default infinite)\n"
	 "  -n <ifname>, --name <ifname>\n"
	 "      set the name of the host interface task (default IFTASK)\n"
	 "  -i <scopeid>, --idscope <scopeid>\n"
	 "      set the scope mailboxes identifier (default RTS)\n"
	 "  -l <logid>, --idlog <logid>\n"
	 "      set the log mailboxes identifier (default RTL)\n"
	 "  -a <alogid>, --idalog <alogid>\n"
	 "      set the automatic log mailboxes identifier (default RAL)\n"
	 "  -t <meterid>, --idmeter <meterid>\n"
	 "      set the meter mailboxes identifier (default RTM)\n"
	 "  -d <ledid>, --idled <ledid>\n"
	 "      set the led mailboxes identifier (default RTE)\n"
	 "  -y <synchid>, --idsynch <synchid>\n"
	 "      set the synchronoscope mailboxes identifier (default RTY)\n"
	 "  -c <cpumap>, --cpumap <cpumap>\n"
	 "      (1 << cpunum) on which the RT-model runs (default: let RTAI choose)\n"
	 "  -e, --external\n"
	 "      RT-model timed by an external resume (default internal)\n"
	 "  -o, --oneshot\n"
	 "      the hard timer will run in oneshot mode (default periodic)\n"
	 "  -m <stack>, --stack <stack>\n"
	 "      set a guaranteed stack size extension (default 30000)\n"
	 "\n")
	,stderr);
  exit(0);
}

int main(int argc, char *argv[])
{
  int priority = 0;
  int c, option_index = 0;

  signal(SIGINT, endme);
  signal(SIGTERM, endme);

  while (1) {
    c = getopt_long(argc, argv, "uvVsweop:f:n:i:l:a:d:t:y:c:m:", options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 'v':
      Verbose = 1;
      break;
    case 'V':
      fputs("rt_main version " RTAILAB_VERSION "\n", stderr);
      exit(0);
    case 'p':
      if (isalpha(optarg[0])) {
	fprintf(stderr, "Invalid priority value\n");
	exit(1);
      }
      priority = atoi(optarg);
      if (priority < 0) {
	fprintf(stderr, "Invalid priority value\n");
	exit(1);
      }
      break;
    case 's':
      UseHRT = 0;
      break;
    case 'f':
      if (strstr(optarg, "inf")) {
	FinalTime = RUN_FOREVER;
      } else {
	if (isalpha(optarg[0])) {
	  fprintf(stderr, "Invalid final time value\n");
	  exit(1);
	}
	FinalTime = atof(optarg);
      }
      break;
    case 'w':
      WaitToStart = 1;
      break;
    case 'u':
      print_usage();
      exit(0);
    case 'n':
      HostInterfaceTaskName = strdup(optarg);
      break;
    case 'i':
      TargetScopeMbxID = strdup(optarg);
      break;
    case 'l':
      TargetLogMbxID = strdup(optarg);
      break;
    case 't':
      TargetMeterMbxID = strdup(optarg);
      break;
    case 'a':
      TargetALogMbxID = strdup(optarg);
      break;	
    case 'd':
      TargetLedMbxID = strdup(optarg);
      break;
    case 'y':
      TargetSynchronoscopeMbxID = strdup(optarg);
      break;
    case 'c':
      if (!(CpuMap = atoi(optarg))) {
	CpuMap = 0xF;
      }
      break;
    case 'e':
      InternalTimer = 0;
      break;
    case 'o':
      ClockTick = 0;
      break;
    case 'm':
      StackInc = atoi(optarg);
      break;
    default:
      break;
    }
  }

  if (Verbose) {
    printf("\nTarget settings\n");
    printf("===============\n");
    printf("  Real-time : %s\n", UseHRT ? "HARD" : "SOFT");	
    printf("  Timing    : %s / ", InternalTimer ? "internal" : "external");
    printf("%s\n", ClockTick ? "periodic" : "oneshot");
    printf("  Priority  : %d\n", priority);
    if (FinalTime > 0) {
      printf("  Finaltime : %f [s]\n", FinalTime);
    } else {
      printf("  Finaltime : RUN FOREVER\n");
    }
    printf("  CPU map   : %x\n\n", CpuMap);
  }

  return rt_Main(MODEL, priority);
}
