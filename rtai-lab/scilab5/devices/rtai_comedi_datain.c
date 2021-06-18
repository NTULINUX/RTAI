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
extern int ComediDev_AIInUse[];

#ifdef SOFTCALIB
int get_softcal_coef(char *devName, unsigned subdev, unsigned channel,
  unsigned range, unsigned direction, double *coef);
#endif

struct ADCOMDev{
  void * dev;
  unsigned int index;
  int subdev;
  unsigned int channel;
  unsigned int range;
  unsigned int aref;
  double range_min;
  double range_max;
  lsampl_t maxdata;
  int use_softcal;
  double coefficients[5];
};

static void init(scicos_block *block)
{
  struct ADCOMDev * comdev = (struct ADCOMDev *) malloc(sizeof(struct ADCOMDev));
  *block->work = (void *)comdev;

  char devName[15];
  char board[50];
  comedi_krange krange;

  comdev->channel = block->ipar[0];
  comdev->range = block->ipar[1];
  comdev->aref = block->ipar[2];
  comdev->index = block->ipar[3];

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

  if ((comdev->subdev = comedi_find_subdevice_by_type(comdev->dev, COMEDI_SUBD_AI, 0)) < 0) {
    fprintf(stderr, "Comedi find_subdevice failed (No analog input)\n");
    comedi_close(comdev->dev);
    exit_on_error();
    return;
  }
  if (!ComediDev_AIInUse[comdev->index] && comedi_lock(comdev->dev, comdev->subdev) < 0) {
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
  if ((comedi_get_krange(comdev->dev, comdev->subdev, comdev->channel, comdev->range, &krange)) < 0) {
    fprintf(stderr, "Comedi get_range failed for subdevice %d\n", comdev->subdev);
    comedi_unlock(comdev->dev, comdev->subdev);
    comedi_close(comdev->dev);
    exit_on_error();
    return;
  }
#ifdef SOFTCALIB
  int flags;
  if ((flags = comedi_get_subdevice_flags(comdev->dev, comdev->subdev)) < 0) {
    fprintf(stderr, "Comedi get_subdevice_flags failed for subdevice %d\n", comdev->subdev);
  } else {
    if (flags & SDF_SOFT_CALIBRATED) {/* board uses software calibration */
      if ((comdev->use_softcal = get_softcal_coef(devName, comdev->subdev,
                comdev->channel, comdev->range, 0, comdev->coefficients)) == 0)
        fprintf(stderr, "No software calibration found for AI Channel %d\n",comdev->channel);
    }
  }
#endif

  ComediDev_InUse[comdev->index]++;
  ComediDev_AIInUse[comdev->index]++;
  comdev->maxdata = comedi_get_maxdata(comdev->dev, comdev->subdev, comdev->channel);
  comdev->range_min = (double)(krange.min)*1.e-6;
  comdev->range_max = (double)(krange.max)*1.e-6;
  printf("AI Channel %d - Range : %1.2f [V] - %1.2f [V]%s\n\n", comdev->channel,
    comdev->range_min, comdev->range_max, (comdev->use_softcal)?" Software calibration used":"");
}

static void inout(scicos_block *block)
{
  struct ADCOMDev * comdev = (struct ADCOMDev *) (*block->work);

  lsampl_t data;
  double x = 0;
  double term = 1;
  double *y = block->outptr[0];

  comedi_data_read(comdev->dev, comdev->subdev, comdev->channel, comdev->range, comdev->aref, &data);
  if (comdev->use_softcal) {
    unsigned i;
    for(i = 0; i <= 4; ++i) {
      x += comdev->coefficients[i+1] * term;
      term *= data - comdev->coefficients[0];
    }
  } else {
    x = comdev->range_min + data*(comdev->range_max - comdev->range_min)/comdev->maxdata;
  }
  y[0] = x;
}

static void end(scicos_block *block)
{
  struct ADCOMDev * comdev = (struct ADCOMDev *) (*block->work);

  if (comdev->dev) {
    int index = comdev->index;
    ComediDev_InUse[index]--;
    ComediDev_AIInUse[index]--;
    if (!ComediDev_AIInUse[index]) {
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

void rt_comedi_datain(scicos_block *block,int flag)
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
