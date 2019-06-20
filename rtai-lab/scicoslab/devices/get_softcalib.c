/*
    Function for reading software calibration and getting coefficients
    Partly based on tutorial example #2 from Comedilib.

    Copyright (C) 2009 Guillaume Millet <millet@isir.fr>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation, version 2.1
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, see <http://www.gnu.org/licenses/>.


*/

#ifdef SOFTCALIB

#include <assert.h>
#include <stdlib.h>
#include <comedilib.h>

int get_softcal_coef(char *devName, unsigned subdev, unsigned channel,
  unsigned range, unsigned direction, double *coef)
{
  int retval, i;
  comedi_polynomial_t polynomial;

  comedi_t* device = comedi_open(devName);
  if (device == NULL) {
    comedi_perror(devName);
    return 0;
  }

  char *calibration_file_path = comedi_get_default_calibration_path(device);

  /* parse a calibration file which was produced by the
    comedi_soft_calibrate program */
  comedi_calibration_t* parsed_calibration =
    comedi_parse_calibration_file(calibration_file_path);
  free(calibration_file_path);
  if (parsed_calibration == NULL) {
    comedi_perror("comedi_parse_calibration_file");
    return 0;
  }

  /* get the comedi_polynomial_t for the subdevice/channel/range
    we are interested in */
  retval = comedi_get_softcal_converter(subdev, channel, range, direction,
    parsed_calibration, &polynomial);
  comedi_cleanup_calibration(parsed_calibration);
  if (retval < 0) {
    comedi_perror("comedi_get_softcal_converter");
    return 0;
  }

  /* copy the coefficients */
  coef[0] = polynomial.expansion_origin;
  assert(polynomial.order < COMEDI_MAX_NUM_POLYNOMIAL_COEFFICIENTS);
  for (i = 0; i <= polynomial.order; ++i) {
    coef[i+1] = polynomial.coefficients[i];
  }
  return 1;
}
#endif
