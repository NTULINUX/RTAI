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
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <rtai_netrpc.h>
#include <rtai_mbx.h>

void exit_on_error(void);
void par_getstr(char * str, int par[], int init, int len);

struct MbxSif{
  char mbxName[20];
  MBX * mbx;
  long tNode;
  long tPort;
};

static void init(scicos_block *block)
{
  char str[20];
  struct MbxSif * mbx = (struct MbxSif *) malloc(sizeof(struct MbxSif));
  int nch=block->nin;
  par_getstr(str,block->ipar,2,block->ipar[0]);
  strcpy(mbx->mbxName,str);
  par_getstr(str,block->ipar,2+block->ipar[0],block->ipar[1]);

  struct sockaddr_in addr;

  if(!strcmp(str,"0")) {
    mbx->tNode = 0;
    mbx->tPort = 0;
  }
  else {
    inet_aton(str, &addr.sin_addr);
    mbx->tNode = addr.sin_addr.s_addr;
    while ((mbx->tPort = rt_request_port(mbx->tNode)) <= 0
           && mbx->tPort != -EINVAL);
  }

  mbx->mbx = (MBX *) RT_typed_named_mbx_init(mbx->tNode,mbx->tPort,mbx->mbxName,nch*sizeof(double),FIFO_Q);

  if(mbx->mbx == NULL) {
    fprintf(stderr, "Error in getting %s mailbox address\n", mbx->mbxName);
    exit_on_error();
  }

  *block->work=(void *) mbx;
}

static void inout(scicos_block *block)
{
  struct MbxSif * mbx = (struct MbxSif *) (*block->work);
  int ntraces = block->nin;
  double *u;
  struct{
    double u[ntraces];
  } data;
  int i;

  for(i=0;i<ntraces;i++){
    u =  block->inptr[i];
    data.u[i] = u[0];
  }
  RT_mbx_send_if(mbx->tNode, mbx->tPort,mbx->mbx,&data,sizeof(data));
}

static void end(scicos_block *block)
{
  struct MbxSif * mbx = (struct MbxSif *) (*block->work);

  RT_named_mbx_delete(mbx->tNode, mbx->tPort,mbx->mbx);
  printf("MBX %s closed\n",mbx->mbxName);
  if(mbx->tNode){
    rt_release_port(mbx->tNode,mbx->tPort);
  }
  free(mbx);
}

void rtai_mbx_send_if(scicos_block *block,int flag)
{
  if (flag==1){          /* get input */
    inout(block);
  }
  else if (flag==5){     /* termination */ 
    end(block);
  }
  else if (flag ==4){    /* initialisation */
    init(block);
  }
}


