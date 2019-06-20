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

void par_getstr(char * str, int par[], int init, int len);

static void init(scicos_block *block)
{
  double *y = GetRealOutPortPtrs(block,1);
 
  y[0] = 0.0;
}

static void inout(scicos_block *block)
{
  double v;
  double * rpar =  GetRparPtrs(block);
  double t = get_scicos_time();
  double *y = GetRealOutPortPtrs(block,1);

  if (t<rpar[4]) y[0]=0.0;
  else {
    v=(t-rpar[4])/rpar[1];
    v=(v - (int) v) * rpar[1];
    if(v < rpar[2]) y[0] = rpar[3]+rpar[0];
    else            y[0] = rpar[3];
  }
}

static void end(scicos_block *block)
{
  double *y = GetRealOutPortPtrs(block,1);

  y[0] = 0.0;
}

void rtsquare(scicos_block *block,int flag)
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


