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

void exit_on_error(void);
void par_getstr(char * str, int par[], int init, int len);

static void init(scicos_block *block)
{
  FILE * fp;
  double * pData;
  char filename[30];
  int i;
  int npts=block->ipar[0];

  if(npts==0) {
    fprintf(stderr, "Error - Data length is 0!\n");
    exit_on_error();
  }

  pData=(double *) calloc(npts,sizeof(double));

  par_getstr(filename,block->ipar,3,block->ipar[2]);
  fp=fopen(filename,"r");
  if(fp!=NULL){
    block->ipar[1]=0;
    for(i=0;i<npts;i++) {
      if(feof(fp)) break;
      fscanf(fp,"%lf",&(pData[i]));
    }
    fclose(fp);
  }
  else{
    fprintf(stderr, "File %s not found!\n",filename);
    exit_on_error();
  }
  *block->work=(void *) pData;
}

static void inout(scicos_block *block)
{
  double *y = block->outptr[0];
  double * pData=(double *) *block->work;
  y[0]=pData[block->ipar[1]];
  block->ipar[1] = ++(block->ipar[1]) % block->ipar[0];
}

static void end(scicos_block *block)
{
  double * pData=(double *) *block->work;
  double *y = block->outptr[0];
  y[0]=0.0;
  free(pData);
  printf("EXTDATA closed\n");
}



void rtextdata(scicos_block *block,int flag)
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


