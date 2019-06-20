/*
COPYRIGHT (C) 2006  Roberto Bucher (roberto.bucher@supsi.ch)

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

#define MAX_RTAI_METERS               1000
#define MBX_RTAI_METER_SIZE           5000

extern char *TargetMeterMbxID;

void par_getstr(char * str, int par[], int init, int len);

static void init(scicos_block *block)
{
  int * ipar = GetIparPtrs(block);

  char meterName[20];
  char name[7];
  MBX *mbx;

  par_getstr(meterName,ipar,1,ipar[0]);
  rtRegisterMeter(meterName,1);
  get_a_name(TargetMeterMbxID,name);

  mbx = (MBX *) RT_typed_named_mbx_init(0,0,name,MBX_RTAI_METER_SIZE/sizeof(float)*sizeof(float),FIFO_Q);
  if(mbx == NULL) {
    fprintf(stderr, "Cannot init mailbox\n");
    exit_on_error();
  }

  *block->work = mbx;
}

static void inout(scicos_block *block)
{
  double *u = block->inptr[0];
  MBX * mbx = *(block->work);
  float data;
  data = (float) u[0];
  RT_mbx_send_if(0, 0, mbx, &data, sizeof(data));
}

static void end(scicos_block *block)
{
  char meterName[10];
  int * ipar = GetIparPtrs(block);
  MBX * mbx = (MBX *) (*block->work);

  RT_named_mbx_delete(0, 0, mbx);
  par_getstr(meterName,ipar,1,ipar[0]);
  printf("Meter %s closed\n",meterName);
}

void rtmeter(scicos_block *block,int flag)
{
  if (flag==1){          /* set output */
    inout(block);
  }
  else if (flag==5){     /* termination */ 
    end(block);
  }
  else if (flag ==4){    /* initialisation */
    init(block);
  }
}


