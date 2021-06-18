/*
COPYRIGHT (C) 2002  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2002  Paolo Mantegazza (mantegazza@aero.polimi.it)
 
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


#define MY_NUPAR1      5
#define MBX_BUF_SIZE  5000
static MBX *mbx1, *mbx2;
static MBX *lmbx1, *lmbx2;

static volatile int c1, c2;
static struct { float t; float h[2]; } th1, th2;

static float mat1[4][3] = { { 1, 1, 1 }, {2, 2, 2 }, { 3, 3, 3 }, { 4, 4, 4 } };
static float mat2[3][2] = { { 1, 1 }, { 2, 2 }, { 3, 3 } };

#include <errno.h>
#include "/home/mante/scilab-2.6/routines/machine.h"

/*---------------------------------------- Actuators */ 
void 
pid_actuator(flag,nport,nevprt,t,u,nu)
     /*
      * To be customized for standalone execution
      * flag  : specifies the action to be done
      * nport : specifies the  index of the Super Bloc 
      *         regular input (The input ports are numbered 
      *         from the top to the bottom ) 
      * nevprt: indicates if an activation had been received
      *         0 = no activation
      *         1 = activation
      * t     : the current time value
      * u     : the vector inputs value
      * nu    : the input  vector size
      */
     integer *flag,*nevprt,*nport;
     integer *nu;

     double  *t, u[];
{
  int k;

  switch (*nport) {
  case 1 :/* Port number 1 ----------*/
    /* skeleton to be customized */
    switch (*flag) {
    case 2 : 
      if(*nevprt>0) {/* get the input value */
  	/*for (k=0;k<*nu;k++) {????=u[k];} */
        th1.h[0] = u[0];
	if (++c1 == 2) {
		th1.t = *t;
		rt_mbx_send_if(mbx1, &th1, sizeof(th1));
		rt_mbx_send_if(lmbx1, mat1, sizeof(mat1));
		c1 = 0;
	}
      } 
      break;
    case 4 : /* actuator initialisation */
      /* do whatever you want to initialize the actuator */
      break;
    case 5 : /* actuator ending */
      /* do whatever you want to end the actuator */
      break;
    }
  break;
  case 2 :/* Port number 2 ----------*/
    /* skeleton to be customized */
    switch (*flag) {
    case 2 : 
      if(*nevprt>0) {/* get the input value */
  	/*for (k=0;k<*nu;k++) {????=u[k];} */
	th1.h[1] = u[0];
	if (++c1 == 2) {
		th1.t = *t;
		rt_mbx_send_if(mbx1, &th1, sizeof(th1));
		rt_mbx_send_if(lmbx1, mat1, sizeof(mat1));
		c1 = 0;
	}
      } 
      break;
    case 4 : /* actuator initialisation */
      /* do whatever you want to initialize the actuator */
      {
	char name[7];
	rtRegisterScope("PIDSCOPE1", 2);
	get_a_name(TargetMbxID, name);
#ifdef __KERNEL__
	mbx1 = RT_typed_named_mbx_init(0, 0, name, (MBX_BUF_SIZE/sizeof(th1))*sizeof(th1), FIFO_Q);
#else
	mbx1 = rt_mbx_init(nam2num(name), (MBX_BUF_SIZE/sizeof(th1))*sizeof(th1));
#endif
	rtRegisterLogData("LOG1", 3, 4);
	get_a_name(TargetLogMbxID, name);
	lmbx1 = rt_mbx_init(nam2num(name), (MBX_BUF_SIZE/sizeof(mat1))*sizeof(mat1));
      }
      break;
    case 5 : /* actuator ending */
      /* do whatever you want to end the actuator */
#ifdef __KERNEL__
	RT_named_mbx_delete(0, 0, mbx1);
#else
	rt_mbx_delete(mbx1);
#endif
	rt_mbx_delete(lmbx1);
      break;
    }
  break;
  case 3 :/* Port number 3 ----------*/
    /* skeleton to be customized */
    switch (*flag) {
    case 2 : 
      if(*nevprt>0) {/* get the input value */
  	/*for (k=0;k<*nu;k++) {????=u[k];} */
        th2.h[0] = u[0];
	if (++c2 == 2) {
		th2.t = *t;
		rt_mbx_send_if(mbx2, &th2, sizeof(th2));
		rt_mbx_send_if(lmbx2, mat2, sizeof(mat2));
		c2 = 0;
	}
      } 
      break;
    case 4 : /* actuator initialisation */
      /* do whatever you want to initialize the actuator */
      break;
    case 5 : /* actuator ending */
      /* do whatever you want to end the actuator */
      break;
    }
  break;
  case 4 :/* Port number 4 ----------*/
    /* skeleton to be customized */
    switch (*flag) {
    case 2 : 
      if(*nevprt>0) {/* get the input value */
  	/*for (k=0;k<*nu;k++) {????=u[k];} */
	th2.h[1] = u[0];
	if (++c2 == 2) {
		th2.t = *t;
		rt_mbx_send_if(mbx2, &th2, sizeof(th2));
		rt_mbx_send_if(lmbx2, mat2, sizeof(mat2));
		c2 = 0;
	}
      } 
      break;
    case 4 : /* actuator initialisation */
      /* do whatever you want to initialize the actuator */
      {
	char name[7];
	rtRegisterScope("PIDSCOPE2", 2);
	get_a_name(TargetMbxID, name);
#ifdef __KERNEL__
	mbx2 = RT_typed_named_mbx_init(0, 0, name, (MBX_BUF_SIZE/sizeof(th2))*sizeof(th2), FIFO_Q);
#else
	mbx2 = rt_mbx_init(nam2num(name), (MBX_BUF_SIZE/sizeof(th2))*sizeof(th2));
#endif
	rtRegisterLogData("LOG2", 2, 3);
	get_a_name(TargetLogMbxID, name);
	lmbx2 = rt_mbx_init(nam2num(name), (MBX_BUF_SIZE/sizeof(mat2))*sizeof(mat2));
      }
      break;
    case 5 : /* actuator ending */
      /* do whatever you want to end the actuator */
#ifdef __KERNEL__
	RT_named_mbx_delete(0, 0, mbx2);
#else
	rt_mbx_delete(mbx2);
#endif
	rt_mbx_delete(lmbx2);
      break;
    }
  break;
  }
}

static int node, port;
static RT_TASK *task;
static unsigned long resume_misses;

/*---------------------------------------- Sensor */ 
void 
pid_sensor(flag,nport,nevprt,t,y,ny)
     /*
      * To be customized for standalone execution
      * flag  : specifies the action to be done
      * nport : specifies the  index of the Super Bloc 
      *         regular input (The input ports are numbered 
      *         from the top to the bottom ) 
      * nevprt: indicates if an activation had been received
      *         0 = no activation
      *         1 = activation
      * t     : the current time value
      * y     : the vector outputs value
      * ny    : the output  vector size
      */
     integer *flag,*nevprt,*nport;
     integer *ny;

     double  *t, y[];
{
  int k;
  switch (*flag) {
  case 1 : /* set the ouput value */
    /* for (k=0;k<*ny;k++) {y[k]=????;}*/
	y[0] = UPAR1[0]*sin(UPAR1[1]**t + UPAR1[2]);
        if (task) {
		if (!rt_waiting_return(node, port)) {
			RT_task_resume(node, -port, task);
		} else {
			++resume_misses;
			if (resume_misses > 50 && !(resume_misses%50)) {
				rt_printk("PREVIOUS RESUME NOT RECEIVED YET\n");
			}
		}
	}
    break;
  case 2 : /* Update internal discrete state if any */
    break;
  case 4 : /* sensor initialisation */
    /* do whatever you want to initialize the sensor */
	if (!InternTimer) {
		TimingEventArg  = (unsigned long)rt_BaseRateTask;
		WaitTimingEvent = (void *)rt_task_suspend;
		SendTimingEvent = (void *)rt_task_resume;
	}
	NUPAR1 = MY_NUPAR1;
	UPAR1 = (double *)malloc(MY_NUPAR1*sizeof(double));
	memset(UPAR1, 0, MY_NUPAR1*sizeof(double));
	UPAR1[0] = 1.0;
	UPAR1[1] = 62.8;
    break;
  case 5 : /* sensor ending */
    /* do whatever you want to end the sensor */
	free(UPAR1);
	if (task) {
		task = 0;
		rt_release_port(node, port);
	}
    break;
  }
}
/*---------------------------------------- callback at user params updates */ 
void 
pid_upar_update(int index)
{
	if (index == 3) {
	        if ((node = udn2nl(UPAR1[3])) > 0) {
		        if (port <= 0) {
				int p;
			        if ((p = rt_request_port(node)) > 0) {
					port = p;
				}
			}
		        if (port > 0 && !task) {
				RT_TASK *host;
				unsigned int msg = 'b';
				if ((host = RT_get_adr(node, port, "aaaaaa"))) {
					RT_rpc(node, port, host, msg, (void *)&task);
				}
			}
        	} else {
			task = 0;
		}
	}
}
