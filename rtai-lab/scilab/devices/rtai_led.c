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

#define MAX_RTAI_LEDS               1000
#define MBX_RTAI_LED_SIZE           5000

extern char *TargetLedMbxID;

void par_getstr(char * str, int par[], int init, int len);

static void init(scicos_block *block)
{
  char ledName[20];
  char name[7];
  int nch  = GetNin(block);
  int * ipar = GetIparPtrs(block);
  MBX *mbx;

  par_getstr(ledName,ipar,1,ipar[0]);
  rtRegisterLed(ledName,nch);
  get_a_name(TargetLedMbxID,name);

  mbx = (MBX *) RT_typed_named_mbx_init(0,0,name,MBX_RTAI_LED_SIZE/sizeof(unsigned int)*sizeof(unsigned int),FIFO_Q);
  if(mbx == NULL) {
    fprintf(stderr, "Cannot init mailbox\n");
    exit_on_error();
  }

  *(block->work) = mbx;

}

static void inout(scicos_block *block)
{
  MBX * mbx = *(block->work);
  int i;
  unsigned int led_mask = 0;
  int nleds  = GetNin(block);

  double *u;
  for (i = 0; i < nleds; i++) {
    u = block->inptr[i];
    if (u[0] > 0.1) {
      led_mask += (1 << i);
    } else {
      led_mask += (0 << i);
    }
  }
  RT_mbx_send_if(0, 0, mbx, &led_mask, sizeof(led_mask));

}

static void end(scicos_block *block)
{
  int * ipar = GetIparPtrs(block);

  char ledName[10];
  MBX * mbx = *(block->work);
  RT_named_mbx_delete(0, 0, mbx);
  par_getstr(ledName,ipar,1,ipar[0]);
  printf("Led %s closed\n",ledName);

}

void rtled(scicos_block *block,int flag)
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


