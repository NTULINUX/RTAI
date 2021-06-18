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
#include <rtai_fifos.h>

static void init(scicos_block *block)
{
  int sizeData = sizeof(float)*(block->nin+1);
  int dim = block->ipar[1]/sizeData*sizeData;
  rtf_create(block->ipar[0],dim);
  rtf_reset(block->ipar[0]);
}

static void inout(scicos_block *block)
{
  double *u;
  int ntraces=block->nin;
  struct {
    float t;
    float u[ntraces];
  } data;
  int i;

  data.t=(float) get_scicos_time();
  for (i = 0; i < ntraces; i++) {
    u=block->inptr[i];
    data.u[i] = (float) u[0];
  }
  rtf_put(block->ipar[0],&data, sizeof(data));
}

static void end(scicos_block *block)
{
  rtf_destroy(block->ipar[0]);
  printf("FIFO %d closed\n",block->ipar[0]);
}



void rt_fifoout(scicos_block *block,int flag)
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


