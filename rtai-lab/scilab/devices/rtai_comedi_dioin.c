/*
COPYRIGHT (C) 2006 Roberto Bucher (roberto.bucher@supsi.ch)
COPYRIGHT (C) 2009 Guillaume Millet (millet@isir.fr)

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

#include <rtai_comedi.h>
#include "rtmain.h"

extern void *ComediDev[];
extern int ComediDev_InUse[];
extern int ComediDev_DIOInUse[];

struct DICOMDev{
  void * dev;
  unsigned int index;
  unsigned int channel;
  int subdev;
  int subdev_type;
  double threshold;
};

static void init(scicos_block *block)
{
  struct DICOMDev * comdev = (struct DICOMDev *) malloc(sizeof(struct DICOMDev));
  *block->work = (void *) comdev;

  char devName[15];
  char board[50];

  comdev->channel = block->ipar[0];
  comdev->index = block->ipar[1];
  comdev->subdev_type = -1;

  sprintf(devName,"/dev/comedi%d", comdev->index);
  if (!ComediDev[comdev->index]) {
    comdev->dev = comedi_open(devName);
    if (!(comdev->dev)) {
      fprintf(stderr, "COMEDI %s open failed\n", devName);
      exit_on_error();
      return;
    }
    rt_comedi_get_board_name(comdev->dev, board);
    printf("COMEDI %s (%s) opened.\n\n", devName, board);
    ComediDev[comdev->index] = comdev->dev;
  } else
    comdev->dev = ComediDev[comdev->index];

  if ((comdev->subdev = comedi_find_subdevice_by_type(comdev->dev, COMEDI_SUBD_DI, 0)) < 0) {
    if ((comdev->subdev = comedi_find_subdevice_by_type(comdev->dev, COMEDI_SUBD_DIO, 0)) < 0) {
      fprintf(stderr, "Comedi find_subdevice failed (No digital Input or I/O)\n");
      comedi_close(comdev->dev);
      exit_on_error();
      return;
    } else
      comdev->subdev_type = COMEDI_SUBD_DIO;
  } else
    comdev->subdev_type = COMEDI_SUBD_DI;

  if (!ComediDev_DIOInUse[comdev->index] && comedi_lock(comdev->dev, comdev-> subdev) < 0) {
    fprintf(stderr, "Comedi lock failed for subdevice %d\n", comdev->subdev);
    comedi_close(comdev->dev);
    exit_on_error();
    return;
  }

  if (comdev->channel >= comedi_get_n_channels(comdev->dev, comdev->subdev)) {
    fprintf(stderr, "Comedi channel not available for subdevice %d\n", comdev->subdev);
    comedi_unlock(comdev->dev, comdev->subdev);
    comedi_close(comdev->dev);
    exit_on_error();
    return;
  }

  if (comdev->subdev_type == COMEDI_SUBD_DIO && comedi_dio_config(comdev->dev,
      comdev->subdev, comdev->channel, COMEDI_INPUT) < 0) {
    fprintf(stderr, "Comedi DIO config failed for subdevice %d\n", comdev->subdev);
    comedi_unlock(comdev->dev, comdev->subdev);
    comedi_close(comdev->dev);
    exit_on_error();
    return;
  }

  ComediDev_InUse[comdev->index]++;
  ComediDev_DIOInUse[comdev->index]++;
  printf("%s Channel %d\n\n", (comdev->subdev_type == COMEDI_SUBD_DIO)?"DIO Input":"DI", comdev->channel);
}

static void inout(scicos_block *block)
{
  struct DICOMDev * comdev = (struct DICOMDev *) (*block->work);
  unsigned int bit;
  double *y = block->outptr[0];

  comedi_dio_read(comdev->dev, comdev->subdev, comdev->channel, &bit);
  y[0] = (double) bit;
}

static void end(scicos_block *block)
{
  struct DICOMDev * comdev = (struct DICOMDev *) (*block->work);

  if (comdev->dev) {
    int index = comdev->index;
    ComediDev_InUse[index]--;
    ComediDev_DIOInUse[index]--;
    if (!ComediDev_DIOInUse[index]) {
      comedi_unlock(comdev->dev, comdev->subdev);
    }
    if (!ComediDev_InUse[index]) {
      comedi_close(comdev->dev);
      printf("\nCOMEDI /dev/comedi%d closed.\n\n", index);
      ComediDev[index] = NULL;
    }
  }
  free(comdev);
}

void rt_comedi_dioin(scicos_block *block,int flag)
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
