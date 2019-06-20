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
#include <math.h>
#include "rtmain.h"

extern void *ComediDev[];
extern int ComediDev_InUse[];
extern int ComediDev_AOInUse[];

#ifdef SOFTCALIB
int get_softcal_coef(char *devName, unsigned subdev, unsigned channel,
  unsigned range, unsigned direction, double *coef);
#endif

struct DACOMDev{
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
  struct DACOMDev * comdev = (struct DACOMDev *) malloc(sizeof(struct DACOMDev));
  *block->work = (void *)comdev;

  char devName[15];
  char board[50];
  comedi_krange krange;
  double s, u, term = 1;
  lsampl_t data;

  comdev->channel=block->ipar[0];
  comdev->range = block->ipar[1];
  comdev->aref = block->ipar[2];
  comdev->index = block->ipar[3];

  sprintf(devName,"/dev/comedi%d",comdev->index);
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

  if ((comdev->subdev = comedi_find_subdevice_by_type(comdev->dev, COMEDI_SUBD_AO, 0)) < 0) {
    fprintf(stderr, "Comedi find_subdevice failed (No analog output)\n");
    comedi_close(comdev->dev);
    exit_on_error();
    return;
  }
  if (!ComediDev_AOInUse[comdev->index] && comedi_lock(comdev->dev, comdev->subdev) < 0) {
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
                comdev->channel, comdev->range, 1, comdev->coefficients)) == 0)
        fprintf(stderr, "No software calibration found for AO Channel %d\n",comdev->channel);
    }
  }
#endif

  ComediDev_InUse[comdev->index]++;
  ComediDev_AOInUse[comdev->index]++;
  comdev->maxdata = comedi_get_maxdata(comdev->dev, comdev->subdev, comdev->channel);
  comdev->range_min = (double)(krange.min)*1.e-6;
  comdev->range_max = (double)(krange.max)*1.e-6;
  printf("AO Channel %d - Range : %1.2f [V] - %1.2f [V]%s\n\n", comdev->channel,
    comdev->range_min, comdev->range_max, (comdev->use_softcal)?" Software calibration used":"");

  u = 0.;
  if (comdev->use_softcal) {
    unsigned i;
    for(i = 0; i <= 4; ++i) {
      s += comdev->coefficients[i+1] * term;
      term *= u - comdev->coefficients[0];
    }
  } else {
    s = (u - comdev->range_min)/(comdev->range_max - comdev->range_min)*comdev->maxdata;
  }
  data = nearbyint(s);
  comedi_data_write(comdev->dev, comdev->subdev, comdev->channel, comdev->range, comdev->aref, data);
}

static void inout(scicos_block *block)
{
  struct DACOMDev * comdev = (struct DACOMDev *) (*block->work);

  double *u = block->inptr[0];
  double s, term = 1;
  lsampl_t data;

  if (comdev->use_softcal) {
    unsigned i;
    for(i = 0; i <= 4; ++i) {
      s += comdev->coefficients[i+1] * term;
      term *= u[0] - comdev->coefficients[0];
    }
  } else {
    s = (u[0] - comdev->range_min)/(comdev->range_max - comdev->range_min)*comdev->maxdata;
  }
  if (s < 0) {
    data = 0;
  } else if (s > comdev->maxdata) {
    data = comdev->maxdata;
  } else {
    data = nearbyint(s);
  }
  comedi_data_write(comdev->dev, comdev->subdev, comdev->channel, comdev->range, comdev->aref, data);
}

static void end(scicos_block *block)
{
  struct DACOMDev * comdev = (struct DACOMDev *) (*block->work);

  if (comdev->dev) {
    double s, term = 1;
    lsampl_t data;

    int index = comdev->index;

    if (comdev->use_softcal) {
      unsigned i;
      for(i = 0; i <= 4; ++i) {
	s += comdev->coefficients[i+1] * term;
	term *= 0.0 - comdev->coefficients[0];
      }
    } else {
      s = (0.0 - comdev->range_min)/(comdev->range_max - comdev->range_min)*comdev->maxdata;
    }
    data = nearbyint(s);

    comedi_data_write(comdev->dev, comdev->subdev, comdev->channel, comdev->range, comdev->aref, data);

    ComediDev_InUse[index]--;
    ComediDev_AOInUse[index]--;
    if (!ComediDev_AOInUse[index]) {
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

void rt_comedi_dataout(scicos_block *block,int flag)
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
