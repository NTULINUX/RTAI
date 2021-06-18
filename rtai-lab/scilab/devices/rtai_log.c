/*
COPYRIGHT (C) 2006  Roberto Bucher (roberto.bucher@supsi.ch)
COPYRIGHT (C) 2009  Holger Nahrstaedt

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

#include <machine.h>
#include <scicos_block4.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtai_netrpc.h>
#include <rtai_msg.h>
#include <rtai_mbx.h>
#include <string.h>
#include "rtmain.h"

#define MAX_RTAI_LOGS		1000
#define MBX_RTAI_LOG_SIZE	5000

extern char *TargetLogMbxID;

void par_getstr(char * str, int par[], int init, int len);

static void init(scicos_block *block)
{
  char logName[20];
  char name[7];
  int mu = GetInPortCols(block,1);
  int nu = GetInPortRows(block,1);
  int * ipar = GetIparPtrs(block);
  MBX *mbx;

  int i;
  if (ipar[0]>20)
       ipar[0]=20;

  par_getstr(logName,ipar,1,ipar[0]);
  rtRegisterLogData(logName,nu,mu);
  get_a_name(TargetLogMbxID,name);
  mbx = (MBX *) RT_typed_named_mbx_init(0,0,name,(MBX_RTAI_LOG_SIZE/(nu*mu*sizeof(float)))*(nu*mu*sizeof(float)),FIFO_Q);
  if(mbx == NULL) {
    fprintf(stderr, "Cannot init mailbox\n");
    exit_on_error();
  }

  *(block->work) = mbx;
}

static void inout(scicos_block *block)
{
  double *u;
  int i;
  MBX * mbx = *(block->work);
  int nu = GetInPortRows(block,1);
  int mu = GetInPortCols(block,1);
  int   datasize=nu*mu;
    struct {
      float u[datasize];
    } data;

  u=GetRealInPortPtrs(block,1);
  for (i = 0; i < (nu*mu); i++) {
    data.u[i] = (float)u[i];
  }
  RT_mbx_send_if(0, 0, mbx, &data, sizeof(data));
}

static void end(scicos_block *block)
{
  char logName[20];
  int * ipar = GetIparPtrs(block);
  MBX * mbx = *(block->work);
  RT_named_mbx_delete(0, 0, mbx);
  par_getstr(logName,ipar,1,ipar[0]);
  printf("Log %s closed\n",logName);
}

void rtlog(scicos_block *block,int flag)
{
  if (flag==1){          
    inout(block);
  }
  else if (flag==5){     /* termination */ 
    end(block);
  }
  else if (flag ==4){    /* initialisation */
    init(block);
  }
}


