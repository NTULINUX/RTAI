/*
  COPYRIGHT (C) 2002  Lorenzo Dozio (dozio@aero.polimi.it)
  COPYRIGHT (C) 2002  Paolo Mantegazza (mantegazza@aero.polimi.it)
  COPYRIGHT (C) 2002  Roberto Bucher (roberto.bucher@supsi.ch)
  COPYRIGHT (C) 2002  Daniele Gasperini (daniele.gasperini@elet.polimi.it)
  COPYRIGHT (C) 2002  Guillaume Millet <millet@isir.fr>

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <float.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/io.h>

#include <rtai_netrpc.h>
#include <rtai_msg.h>
#include <rtai_mbx.h>
#include <rtai_fifos.h>

#define RTAILAB_VERSION         "3.7.1"
#define MAX_ADR_SRCH      500
#define MAX_NAME_SIZE     256
#define MAX_SCOPES        100
#define MAX_LOGS          100
#define MAX_LEDS          100
#define MAX_METERS        100
#define HZ                100
#define POLL_PERIOD       100 // millisecs
#define MAX_NTARGETS		1000

#define rt_HostInterfaceTaskPriority	96
#define rt_MainTaskPriority	      97

#define MAX_DATA_SIZE           30

typedef struct rtTargetParamInfo {
  char modelName[MAX_NAME_SIZE];
  char blockName[MAX_NAME_SIZE];
  char paramName[MAX_NAME_SIZE];
  unsigned int nRows;
  unsigned int nCols;
  unsigned int dataType;
  unsigned int dataClass;
  double dataValue[MAX_DATA_SIZE];
} rtTargetParamInfo;

static sem_t err_sem;
static pthread_t  rt_HostInterfaceThread, rt_BaseRateThread;
static RT_TASK    *rt_MainTask, *rt_HostInterfaceTask, *rt_BaseRateTask;

static char *HostInterfaceTaskName = "IFTASK";
char *TargetMbxID                  = "RTS";
char *TargetLogMbxID            = "RTL";
char *TargetALogMbxID              = "RAL";
char *TargetLedMbxID            = "RTE";
char *TargetMeterMbxID	        = "RTM";
char *TargetSynchronoscopeMbxID = "RTY";

static volatile int CpuMap       = 0xF;
static volatile int UseHRT        = 1;
static volatile int WaitToStart   = 0;
static volatile int isRunning    = 0;
static volatile int verbose      = 0;
static volatile int endBaseRate   = 0;
static volatile int endInterface  = 0;
static volatile int stackinc     = 100000;
static volatile int ClockTick    = 1;
static volatile int InternTimer  = 1;
static RTIME rt_BaseRateTick;
static float FinalTime           = 0.0;

static volatile int endex;

static double TIME;
static struct { char *name; char **traceName; int ntraces; } rtaiScope[MAX_SCOPES];
static struct { char name[MAX_NAME_SIZE]; int nrow, ncol; } rtaiLogData[MAX_LOGS];
static struct { char name[MAX_NAME_SIZE]; int nrow, ncol; } rtaiALogData[MAX_LOGS];
static struct { char name[MAX_NAME_SIZE]; int nleds; } rtaiLed[MAX_LEDS];
static struct { char name[MAX_NAME_SIZE]; int nmeters; } rtaiMeter[MAX_METERS];

static char *loadParFile = NULL;
static char *saveParFile = NULL;
static int parChecksum;

#ifdef TASKDURATION
RTIME RTTSKinit=0, RTTSKper;
#endif

#define SS_DOUBLE  0
#define rt_SCALAR  0 

#define msleep(t)  do { poll(0, 0, t); } while (0)

#define MAX_COMEDI_DEVICES      11
#define MAX_COMEDI_COUNTERS      8

void *ComediDev[MAX_COMEDI_DEVICES];
int ComediDev_InUse[MAX_COMEDI_DEVICES] = {0};
int ComediDev_AIInUse[MAX_COMEDI_DEVICES] = {0};
int ComediDev_AOInUse[MAX_COMEDI_DEVICES] = {0};
int ComediDev_DIOInUse[MAX_COMEDI_DEVICES] = {0};
int ComediDev_CounterInUse[MAX_COMEDI_DEVICES][MAX_COMEDI_COUNTERS] = {{0}};

static void DummySend(void) { }

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

static inline void strncpyz(char *dest, const char *src, int n)
{
  strncpy(dest, src, n);
  dest[n - 1] = '\0';
}

// the following 2 functions are unsafe, in theory, but can be used anyhow 
// since it is very unlikely that two controllers will be started in parallel,
// moreover it is also possible to avoid using them

char *get_a_name(const char *root, char *name)
{
  unsigned long i;
  for (i = 0; i < MAX_ADR_SRCH; i++) {
    sprintf(name, "%s%ld", root, i);
    if (!rt_get_adr(nam2num(name))) {
      return name;
    }
  }
  return 0;
}

int rtRegisterScope(char *name, char **traceName, int n)
{
  int i;
  for (i = 0; i < MAX_SCOPES; i++) {
    if (!rtaiScope[i].ntraces) {
      rtaiScope[i].ntraces = n;
      rtaiScope[i].name = name;
      rtaiScope[i].traceName = traceName;
      int j;
/*       for(j=0; j<n; j++) { */
/*         printf("%s\n",rtaiScope[i].traceName[j]); */
/*       } */
      return 0;
    }
  }
  return -1;
}

int rtRegisterLed(const char *name, int n)
{
  int i;
  for (i = 0; i < MAX_LEDS; i++) {
    if (!rtaiLed[i].nleds) {
      rtaiLed[i].nleds = n;
      strncpyz(rtaiLed[i].name, name, MAX_NAME_SIZE);
      return 0;
    }
  }
  return -1;
}

int rtRegisterMeter(const char *name, int n)
{
  int i;
  for (i = 0; i < MAX_METERS; i++) {
    if (!rtaiMeter[i].nmeters) {
      rtaiMeter[i].nmeters = n;
      strncpyz(rtaiMeter[i].name, name, MAX_NAME_SIZE);
      return 0;
    }
  }
  return -1;
}

static int rtRegisterLogData(const char *name, int nrow, int ncol)
{
  int i;
  for (i = 0; i < MAX_SCOPES; i++) {
    if (!rtaiLogData[i].nrow) {
      rtaiLogData[i].nrow = nrow;
      rtaiLogData[i].ncol = ncol;
      strncpyz(rtaiLogData[i].name, name, MAX_NAME_SIZE);
      return 0;
    }
  }
  return -1;
}

static void grow_and_lock_stack(int inc)
{
  char c[inc];
  memset(c, 0, inc);
  mlockall(MCL_CURRENT | MCL_FUTURE);
}

static void (*WaitTimingEvent)(unsigned long);
static void (*SendTimingEvent)(unsigned long);
static unsigned long TimingEventArg;
	
#define XNAME(x,y)  x##y
#define NAME(x,y)   XNAME(x,y)

#define XSTR(x)    #x
#define STR(x)     XSTR(x)

/* #define MODELNAME  STR(MODELN) */
/* #include MODELNAME */

int NAME(MODEL,_useInternTimer)(void);
void rtextclk(void);
int NAME(MODEL,_init)(void);
int NAME(MODEL,_isr)(double);
int NAME(MODEL,_end)(void);
double NAME(MODEL,_get_tsamp)(void);
double NAME(MODEL,_get_tsamp_delay)(void);

extern int NTOTRPAR;
extern int NTOTIPAR;
extern double *RPAR[];
extern int *IPAR[];
extern int NRPAR;
extern int NIPAR;
extern char * strRPAR[];
extern char * strIPAR[];
extern int lenRPAR[];
extern int lenIPAR[];

double get_scicos_time(void)
{
  return(TIME);
}

static inline int rtModifyRParam(int i, int j, double *param)
{
  if (j >= 0 && j < lenRPAR[i]) {
    set_double(&RPAR[i][j], param);
/*     RPAR[i][j] = *param; */
    if (verbose) {
      printf("rpar_%d[%d] : %le.\n", i,j, RPAR[i][j]);
    }
    return 0;
  }
  return -1;
}

static inline int rtModifyIParam(int i, int j, int param)
{
  if (j >= 0 && j < lenIPAR[i]) {
    IPAR[i][j] = param;
    if (verbose) {
      printf("IPAR_%d[%d] : %d.\n", i, j, IPAR[i][j]);
    }
    return 0;
  }
  return -1;
}

static void *rt_BaseRate(void *args)
{
  char name[7];
  int i;
  static RTIME t0;

  for (i = 0; i < MAX_NTARGETS; i++) {
    sprintf(name,"BRT%d",i);
    if (!rt_get_adr(nam2num(name))) break;
  }
  if (!(rt_BaseRateTask = rt_task_init_schmod(nam2num(name), *((int *)args), 0, 0, SCHED_FIFO, CpuMap))) {
    fprintf(stderr,"Cannot init rt_BaseRateTask.\n");
    return (void *)1;
  }

  sem_post(&err_sem);

  iopl(3);
  rt_task_use_fpu(rt_BaseRateTask, 1);

  NAME(MODEL,_init)();
  grow_and_lock_stack(stackinc);
  if (UseHRT) {
    rt_make_hard_real_time();
  }

  rt_rpc(rt_MainTask,0,(void *) name); 
  t0 = rt_get_cpu_time_ns();
  rt_task_make_periodic(rt_BaseRateTask, rt_get_time() + rt_BaseRateTick, rt_BaseRateTick);
  while (!endBaseRate) {
#ifdef TASKDURATION
    RTTSKper=rt_get_cpu_time_ns()-RTTSKinit;
#endif
    WaitTimingEvent(TimingEventArg);

    if (endBaseRate) break;

    TIME = (rt_get_cpu_time_ns() - t0)*1.0E-9;
#ifdef TASKDURATION
    RTTSKinit=rt_get_cpu_time_ns();
#endif

    NAME(MODEL,_isr)(TIME);

  }
  if (UseHRT) {
    rt_make_soft_real_time();
  }
  NAME(MODEL,_end)();
  rt_task_delete(rt_BaseRateTask);

  return 0;
}

static inline void modify_any_param(int index, double param)
{
  int i,j,tot;
  if (index < NTOTRPAR) {
    i=0;
    tot=0;
    do {
      tot+=lenRPAR[i];
      i++;
    }while(tot<=index);
    i--;
    tot -= lenRPAR[i];
    j = index-tot; 
    rtModifyRParam(i, j, &param);
  } 
  else {
    index -= NTOTRPAR;
    i=0;
    tot=0;
    do{
      tot+=lenIPAR[i];
      i++;
    }while(tot<=index);
    i--;
    tot -= lenIPAR[i];
    j = index-tot; 
    rtModifyIParam(i, j, (int)param);
  }
}

static void *rt_HostInterface(void *args)
{
  RT_TASK *task;
  unsigned int Request;
  int Reply;
  long int len;

  if (!(rt_HostInterfaceTask = rt_task_init_schmod(nam2num(HostInterfaceTaskName), rt_HostInterfaceTaskPriority, 0, 0, SCHED_RR, 0xFF))) {
    fprintf(stderr,"Cannot init rt_HostInterfaceTask.\n");
    return (void *)1;
  }

  sem_post(&err_sem);

  while (!endInterface) {
      task = rt_receive(0, &Request);
      if (endInterface) break;
      switch (Request & 0xFF) {
      case 'c': {
	int i, j, Idx;
	rtTargetParamInfo rtParam;
	float samplingTime;

	strncpyz(rtParam.modelName, STR(MODEL), MAX_NAME_SIZE);
	rtParam.dataType  = SS_DOUBLE;
	rtParam.dataClass = rt_SCALAR;
	rtParam.nRows = 1;
	rtParam.nCols = 1;

	rt_return(task, (isRunning << 16) | ((NTOTRPAR + NTOTIPAR) & 0xFFFF));
	rt_receivex(task, &Request, 1, &len);
	rt_returnx(task, &rtParam, sizeof(rtParam));
					  
	for (i = 0; i < NRPAR; i++) {
	  sprintf(rtParam.blockName,"%s/%s",rtParam.modelName,strRPAR[i]);
	  if(i==0) Idx = 0;
	  else     Idx += lenRPAR[i-1];
	  for(j=0;j<lenRPAR[i];j++) {
	    rt_receivex(task, &Request, 1, &len);
	    sprintf(rtParam.paramName, "Value[%d]",j);
	    rtParam.dataValue[0] = RPAR[i][j];
	    rt_returnx(task, &rtParam, sizeof(rtParam));
	  }
	}
	for (i = 0; i < NIPAR; i++) {
	  sprintf(rtParam.blockName,"%s/%s",rtParam.modelName,strIPAR[i]);
	  if(i==0) Idx = 0;
	  else     Idx += lenIPAR[i-1];
	  for(j=0;j<lenIPAR[i];j++) {
	    rt_receivex(task, &Request, 1, &len);
	    sprintf(rtParam.paramName, "Value[%d]",j);
	    rtParam.dataValue[0] = IPAR[i][j];
	    rt_returnx(task, &rtParam, sizeof(rtParam));
	  }
	}

	while (1) {
	  rt_receivex(task, &Idx, sizeof(int), &len);
	  if (Idx < 0) {
	    rt_returnx(task, &Idx, sizeof(int));
	    break;
	  } else {
	    rt_returnx(task, &rtaiScope[Idx].ntraces, sizeof(int));
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    rt_returnx(task, rtaiScope[Idx].name, strlen(rtaiScope[Idx].name)+1); // return null terminated string
	    while (1) {
	      rt_receivex(task, &j, sizeof(int), &len);
				if(j < 0) break;
	      rt_returnx(task, rtaiScope[Idx].traceName[j], strlen(rtaiScope[Idx].traceName[j])+1); // return null terminated string
	    }
	    samplingTime = NAME(MODEL,_get_tsamp)();
	    rt_returnx(task, &samplingTime, sizeof(float));
	  }
	}
	while (1) {
	  rt_receivex(task, &Idx, sizeof(int), &len);
	  if (Idx < 0) {
	    rt_returnx(task, &Idx, sizeof(int));
	    break;
	  } else {
	    rt_returnx(task, &rtaiLogData[Idx].nrow, sizeof(int));
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    rt_returnx(task, &rtaiLogData[Idx].ncol, sizeof(int));
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    rt_returnx(task, rtaiLogData[Idx].name, MAX_NAME_SIZE);
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    samplingTime = NAME(MODEL,_get_tsamp)();
	    rt_returnx(task, &samplingTime, sizeof(float));
	  }
	}
	while (1) {
	  rt_receivex(task, &Idx, sizeof(int), &len);
	  if (Idx < 0) {
	    rt_returnx(task, &Idx, sizeof(int));
	    break;
	  } else {
	    rt_returnx(task, &rtaiALogData[Idx].nrow, sizeof(int));
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    rt_returnx(task, &rtaiALogData[Idx].ncol, sizeof(int));
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    rt_returnx(task, rtaiALogData[Idx].name, MAX_NAME_SIZE);
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    samplingTime = NAME(MODEL,_get_tsamp)();
	    rt_returnx(task, &samplingTime, sizeof(float));
	  }
	}
	while (1) {
	  rt_receivex(task, &Idx, sizeof(int), &len);
	  if (Idx < 0) {
	    rt_returnx(task, &Idx, sizeof(int));
	    break;
	  } else {
	    rt_returnx(task, &rtaiLed[Idx].nleds, sizeof(int));
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    rt_returnx(task, rtaiLed[Idx].name, MAX_NAME_SIZE);
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    samplingTime = NAME(MODEL,_get_tsamp)();
	    rt_returnx(task, &samplingTime, sizeof(float));
	  }
	}
	while (1) {
	  rt_receivex(task, &Idx, sizeof(int), &len);
	  if (Idx < 0) {
	    rt_returnx(task, &Idx, sizeof(int));
	    break;
	  } else {
	    rt_returnx(task, rtaiMeter[Idx].name, MAX_NAME_SIZE);
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    samplingTime = NAME(MODEL,_get_tsamp)();
	    rt_returnx(task, &samplingTime, sizeof(float));
	  }
	}
	while (1) {
	  rt_receivex(task, &Idx, sizeof(int), &len);
	  if (Idx < 0) {
	    rt_returnx(task, &Idx, sizeof(int));
	    break;
	  } else {
	    rt_returnx(task, "", MAX_NAME_SIZE);
	    rt_receivex(task, &Idx, sizeof(int), &len);
	    samplingTime = NAME(MODEL,_get_tsamp)();
	    rt_returnx(task, &samplingTime, sizeof(float));
	  }
	}
	break;
      }
      case 's': {
	rt_task_resume(rt_MainTask);
	rt_return(task, 1);
	break;
      }
      case 't': {
	endex = 1;
	rt_return(task, 0);
	break;
      }
      case 'p': {
	int index;
	double param;
	int mat_ind;

	rt_return(task, isRunning);
	rt_receivex(task, &index, sizeof(int), &len);
	Reply = 0;
	rt_returnx(task, &Reply, sizeof(int));
	rt_receivex(task, &param, sizeof(double), &len);
	Reply = 1;
	rt_returnx(task, &Reply, sizeof(int));
	rt_receivex(task, &mat_ind, sizeof(int), &len);

	modify_any_param(index, param);
	rt_returnx(task, &Reply, sizeof(int));
	break;			
      }
      case 'g': {
	int i, j, Idx=0;
	rtTargetParamInfo rtParam;

	strncpyz(rtParam.modelName, STR(MODEL), MAX_NAME_SIZE);
	rtParam.dataType  = SS_DOUBLE;
	rtParam.dataClass = rt_SCALAR;
	rtParam.nRows = 1;
	rtParam.nCols = 1;
	rt_return(task, isRunning);

	for (i = 0; i < NRPAR; i++) {
	  sprintf(rtParam.blockName,"%s/%s",rtParam.modelName,strRPAR[i]);
	  if(i==0) Idx = 0;
	  else     Idx += lenRPAR[i-1];
	  for(j=0;j<lenRPAR[i];j++) {
	    rt_receivex(task, &Request, 1, &len);
	    sprintf(rtParam.paramName, "Value[%d]",j);
	    rtParam.dataValue[0] = RPAR[i][j];
	    rt_returnx(task, &rtParam, sizeof(rtParam));
	  }
	}
	for (i = 0; i < NIPAR; i++) {
	  sprintf(rtParam.blockName,"%s/%s",rtParam.modelName,strIPAR[i]);
	  if(i==0) Idx = 0;
	  else     Idx += lenIPAR[i-1];
	  for(j=0;j<lenIPAR[i];j++) {
	    rt_receivex(task, &Request, 1, &len);
	    sprintf(rtParam.paramName, "Value[%d]",j);
	    rtParam.dataValue[0] = IPAR[i][j];
	    rt_returnx(task, &rtParam, sizeof(rtParam));
	  } 
	}

	break;
      }
      case 'd': {
	int ParamCnt;
	rt_return(task, isRunning);
	rt_receivex(task, &ParamCnt, sizeof(int), &len);
	Reply = 0;
	rt_returnx(task, &Reply, sizeof(int));
	{
	  struct {
	    int index;
	    int mat_ind;
	    double value;
	  } Params[ParamCnt];
	  int i;
	  rt_receivex(task, &Params, sizeof(Params), &len);
	  for (i = 0; i < ParamCnt; i++) {
	    modify_any_param(Params[i].index, Params[i].value);
	  }
	}
	Reply = 1;
	rt_returnx(task, &Reply, sizeof(int));
	break;
      }
      case 'm': {
	float time = TIME;
	rt_return(task, isRunning);
	rt_receivex(task, &Reply, sizeof(int), &len);
	rt_returnx(task, &time, sizeof(float));
	break;
      }
      case 'b': {
	rt_return(task, (unsigned int)rt_BaseRateTask);
	break;
      }
      default : {
	break;
      }
      }
    }
    rt_task_delete(rt_HostInterfaceTask);
    return 0;
  }

static int calculateParametersChecksum(void) {
	int checksum = NIPAR ^ NRPAR ^ NTOTIPAR ^ NTOTRPAR;
	int i,j;

  for(i=0; i<NIPAR; i++) {
    checksum ^= lenIPAR[i];
  }	
  for(i=0; i<NRPAR; i++) {
    checksum ^= lenRPAR[i];
  }

  for(i=0; i<NIPAR; i++) {
    for(j=0;j<lenIPAR[i];j++) {
      checksum ^= IPAR[i][j];
    }
  }

  for(i=0; i<NRPAR; i++) {
    for(j=0;j<lenRPAR[i];j++) {
      checksum ^= (int) (RPAR[i][j]*100);
    }
  }

  return checksum;
}

static void loadParametersFromFile(char* filename) {
  FILE *fd;
  if((fd = fopen(filename, "r")) == NULL) {
    fprintf(stderr,"Cannot open file %s, dropping load of parameters\n", filename);
    return;
  }
  char buf[30];
  if(fgets(buf, sizeof(buf), fd) == NULL) {
    fprintf(stderr,"Unable to read checksum from file, dropping load of parameters\n");
    fclose(fd);
    return;
  }
  if(parChecksum != (int)strtol(buf, NULL, 10)) {
    fprintf(stderr,"Checksum error, dropping load of parameters\n");
    fclose(fd);
    return;
  }
  int i,j;
  for(i=0; i<NIPAR; i++) {
    for(j=0;j<lenIPAR[i];j++){
      if(fgets(buf, sizeof(buf), fd) == NULL) {
        fprintf(stderr,"Error during loading IPAR\n");
        fclose(fd);
        return;
      }
      rtModifyIParam(i, j, (int)strtol(buf, NULL, 10));
    }
  }
  for(i=0; i<NRPAR; i++) {
    for(j=0;j<lenRPAR[i];j++){
      if(fgets(buf, sizeof(buf), fd) == NULL) {
        fprintf(stderr,"Error during loading RPAR\n");
        fclose(fd);
        return;
      }
      double d = strtod(buf, NULL);
      rtModifyRParam(i, j, &d);
    }
  }
  fclose(fd);
  fprintf(stdout,"Parameters loaded from %s\n", filename);
}

static void saveParametersToFile(char* filename) {
  FILE *fd;
  if((fd = fopen(filename, "w")) == NULL) {
    fprintf(stderr,"Cannot write to file %s, parameters will not be saved\n", filename);
    return;
  }
  fprintf(fd, "%d\n", parChecksum);
  int i,j;
  for(i=0; i<NIPAR; i++) {
    for(j=0;j<lenIPAR[i];j++){
      fprintf(fd, "%d\n", IPAR[i][j]);
    }
  }
  for(i=0; i<NRPAR; i++) {
    for(j=0;j<lenRPAR[i];j++){
      fprintf(fd, "%.15e\n", RPAR[i][j]);
    }
  }
  fclose(fd);
  fprintf(stdout,"Parameters saved to %s\n", filename);
}

static int rt_Main(int priority)
{
  SEM *hard_timers_cnt = NULL;
  char name[7];
  RTIME rt_BaseTaskPeriod;
  struct timespec err_timeout;
  int i;

  rt_allow_nonroot_hrt();

  for (i = 0; i < MAX_NTARGETS; i++) {
    sprintf(name,"MNT%d",i);
    if (!rt_get_adr(nam2num(name))) break;
  }

  if (!(rt_MainTask = rt_task_init_schmod(nam2num(name), rt_MainTaskPriority, 0, 0, SCHED_RR, 0xFF))) {
    fprintf(stderr,"Cannot init rt_MainTask.\n");
    return 1;
  }
  sem_init(&err_sem, 0, 0);

  printf("TARGET STARTS.\n");
  parChecksum = calculateParametersChecksum();
  if(loadParFile != NULL) {
    loadParametersFromFile(loadParFile);
  }
  pthread_create(&rt_HostInterfaceThread, NULL, rt_HostInterface, NULL);
  err_timeout.tv_sec = (long int)(time(NULL)) + 1;
  err_timeout.tv_nsec = 0;
  if ((sem_timedwait(&err_sem, &err_timeout)) != 0) {
    fprintf(stderr, "Target is terminated.\n");
    goto finish;
  }

  pthread_create(&rt_BaseRateThread, NULL, rt_BaseRate, &priority);
  err_timeout.tv_sec = (long int)(time(NULL)) + 1;
  err_timeout.tv_nsec = 0;
  if ((sem_timedwait(&err_sem, &err_timeout)) != 0) {
    endInterface = 1;
    rt_send(rt_HostInterfaceTask, 0);
    pthread_join(rt_HostInterfaceThread, NULL);
    fprintf(stderr, "Target is terminated.\n");
    goto finish;
  }

  rt_BaseTaskPeriod = (RTIME) (1e9*(NAME(MODEL,_get_tsamp)()));
  if (InternTimer < 2) {
    // The model have not been forced to use it's internal timer, so ask the model which timer scheme to use
    InternTimer = NAME(MODEL,_useInternTimer)();
  }
  if (InternTimer) {
    printf("Using internal timer\n");
    WaitTimingEvent = (void *)rt_task_wait_period;
    if (!(hard_timers_cnt = rt_get_adr(nam2num("HTMRCN")))) {
      if (!ClockTick) {
	rt_set_oneshot_mode();
	start_rt_timer(0);
	rt_BaseRateTick = nano2count(rt_BaseTaskPeriod);
      }
      else {
	rt_set_periodic_mode();
	rt_BaseRateTick = start_rt_timer(nano2count(rt_BaseTaskPeriod));
      }
      hard_timers_cnt = rt_sem_init(nam2num("HTMRCN"), 0);
    } 
    else {
      rt_BaseRateTick = nano2count(rt_BaseTaskPeriod);
      rt_sem_signal(hard_timers_cnt);
    }
  }
  else {
    printf("Using external clock event\n");
    WaitTimingEvent = (void *)rtextclk;
    SendTimingEvent = (void *)DummySend;
  }

  if (verbose) {
    printf("Model : %s .\n", STR(MODEL));
    printf("Executes on CPU map : %x.\n", CpuMap);
    printf("Sampling time : %e (s).\n", NAME(MODEL,_get_tsamp)());
  }
  {
    int msg;
    rt_receive(0, &msg);
  }

  if (WaitToStart) {
    if (verbose) {
      printf("Target is waiting to start ... ");
      fflush(stdout);
    }
    rt_task_suspend(rt_MainTask);
  }
  if (verbose) {
    printf("Target is running.\n");
  }
  rt_return(rt_BaseRateTask,0);
  isRunning = 1;
  while (!endex && (!FinalTime || TIME < FinalTime)) {
    msleep(POLL_PERIOD);
  }
  endBaseRate = 1;
  if (!InternTimer) {
    SendTimingEvent(TimingEventArg);
  }
  pthread_join(rt_BaseRateThread, NULL);
  isRunning = 0;
  endInterface = 1;
  rt_send(rt_HostInterfaceTask, 0);
  if (verbose) {
    printf("Target has been stopped.\n");
  }
  pthread_join(rt_HostInterfaceThread, NULL);
  if (InternTimer) {
    if (!rt_sem_wait_if(hard_timers_cnt)) {
      rt_sem_delete(hard_timers_cnt);
    }
  }
	
 finish:
  if(saveParFile != NULL) {
    saveParametersToFile(saveParFile);
  }
  sem_destroy(&err_sem);
  rt_task_delete(rt_MainTask);
  printf("TARGET ENDS.\n");
  return 0;
}

struct option options[] = {
  { "usage",      0, 0, 'u' },
  { "verbose",    0, 0, 'v' },
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
  { "internal",   0, 0, 'I' },
  { "oneshot",    0, 0, 'o' },
  { "stack",      1, 0, 'm' },
  { "loadPar",    1, 0, 'L' },
  { "savePar",    1, 0, 'S' }
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
         "      print rtmain version\n"
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
	 "  -a <alogid>, --idalog <logid>\n"
	 "      set the alog mailboxes identifier (default RAL)\n"
	 "  -t <meterid>, --idmeter <meterid>\n"
	 "      set the meter mailboxes identifier (default RTM)\n"
	 "  -d <ledid>, --idled <ledid>\n"
	 "      set the led mailboxes identifier (default RTE)\n"
	 "  -y <synchid>, --idsynch <synchid>\n"
	 "      set the synchronoscope mailboxes identifier (default RTY)\n"
	 "  -c <cpumap>, --cpumap <cpumap>\n"
	 "      (1 << cpunum) on which the RT-model runs (default: let RTAI choose)\n"
	 "  -I, --internal\n"
	 "      force RT-model to use internal timer instead of an external resume\n"
	 "  -o, --oneshot\n"
	 "      the hard timer will run in oneshot mode (default periodic)\n"
	 "  -m <stack>, --stack <stack>\n"
	 "      set a guaranteed stack size extension (default 30000)\n"
	 "  -L <filename>, --loadPar <filename>\n"
	 "      load parameters from filename\n"
	 "  -S <filename>, --savePar <filename>\n"
	 "      save parameters on exit to filename\n"
	 "\n")
	,stderr);
  exit(0);
}

static void endme(int dummy)
{
  signal(SIGINT, endme);
  signal(SIGTERM, endme);
  endex = 1;
}

void exit_on_error(void)
{
  endme(0);
}

int main(int argc, char *argv[])
{
  extern char *optarg;
  int c, donotrun = 0, priority = 0;

  signal(SIGINT, endme);
  signal(SIGTERM, endme);

  do {
    c = getopt_long(argc, argv, "IuvVswop:f:m:n:i:l:a:t:d:y:c:L:S:", options, NULL);
    switch (c) {
    case 'c':
      if ((CpuMap = atoi(optarg)) <= 0) {
	fprintf(stderr, "-> Invalid CPU map.\n");
	donotrun = 1;
      }
      break;
    case 'I':
      InternTimer = 2;
      break;
    case 'f':
      if (strstr(optarg, "inf")) {
	FinalTime = 0.0;
      } else if ((FinalTime = atof(optarg)) <= 0.0) {
	fprintf(stderr, "-> Invalid final time.\n");
	donotrun = 1;
      }
      break;
    case 'i':
      TargetMbxID = strdup(optarg);
      break;
    case 'l':
      TargetLogMbxID = strdup(optarg);
      break;
    case 'a':
      TargetALogMbxID = strdup(optarg);
      break;	
    case 't':
      TargetMeterMbxID = strdup(optarg);
      break;
    case 'd':
      TargetLedMbxID = strdup(optarg);
      break;
    case 'y':
      TargetSynchronoscopeMbxID = strdup(optarg);
      break;
    case 'm':
      if ((stackinc = atoi(optarg)) < 0 ) {
	fprintf(stderr, "-> Invalid stack expansion.\n");
	donotrun = 1;
      }
      break;
    case 'n':
      HostInterfaceTaskName = strdup(optarg);
      break;
    case 'p':
      if ((priority = atoi(optarg)) < 0) {
	fprintf(stderr, "-> Invalid priority value.\n");
	donotrun = 1;
      }
      break;
    case 's':
      UseHRT = 0;
      break;
    case 'o':
      ClockTick = 0;
      break;
    case 'u':
      print_usage();
      break;
    case 'V':
      fprintf(stderr, "Version %s.\n", RTAILAB_VERSION);
      return 0;
      break;
    case 'v':
      verbose = 1;
      break;
    case 'w':
      WaitToStart = 1;
      break;
    case 'L':
      loadParFile = strdup(optarg);
      break;
    case 'S':
      saveParFile = strdup(optarg);
      break;
    default:
      if (c >= 0) {
	donotrun = 1;
      }
      break;
    }
  } while (c >= 0);

  if (verbose) {
    printf("\nTarget settings\n");
    printf("===============\n");
    printf("  Real-time : %s\n", UseHRT ? "HARD" : "SOFT");	
    printf("  Timing    : %s / ", InternTimer ? "internal" : "external");
    printf("%s\n", ClockTick ? "periodic" : "oneshot");
    printf("  Priority  : %d\n", priority);
    if (FinalTime > 0) {
      printf("  Finaltime : %f [s]\n", FinalTime);
    } else {
      printf("  Finaltime : RUN FOREVER\n");
    }
    printf("  CPU map   : %x\n\n", CpuMap);
  }
  if (donotrun) {
    printf("ABORTED BECAUSE OF EXECUTION OPTIONS ERRORS.\n");
    return 1;
  }
  return rt_Main(priority);
}
