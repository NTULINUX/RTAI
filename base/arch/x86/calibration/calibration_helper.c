/*
 * Copyright (C) 2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <calibrate.h>

int main(int argc, char *argv[])
{
	FILE *smis;
	char cmd[CMD_SIZE];
        int retval = 0;

	if (system("lspci -nn | grep \"ISA bridge\" >"SUPRT_STREAM));
	if ((smis = fopen(SUPRT_STREAM, "r"))) {
		int n;
		if ((n = fread(cmd, 1, sizeof(cmd), smis)) > 0) {
			char *p;
			p = strstr(cmd, "\n");
			p[0] = 0;
			printf("*** SETSMI found: \"%s\" ***\n", cmd);
			if (strstr(cmd, "Intel Corporation")) {
				if ((p = strstr(cmd, "[8086:"))) {
					int hal_smi_masked_bits, user_smi_device;
					sscanf(p + sizeof("[8086:") - 1, "%x", &user_smi_device);
					hal_smi_masked_bits = argc == 2 ? atoi(argv[1]) : 1;
					printf("*** SETSMI: \"insmod rtai_smi.ko hal_smi_masked_bits=0x%x user_smi_device=0x%x\" ***\n", hal_smi_masked_bits, user_smi_device);
					sprintf(cmd, "insmod ../modules/rtai_smi.ko hal_smi_masked_bits=0x%x user_smi_device=0x%x", hal_smi_masked_bits, user_smi_device);
					if (system(cmd));
					goto cont;
                                }
                        }
                }
        }
        printf("*** SETSMI: no Intel SMI chip found ***\n");
        retval = -1;
cont:
	fclose(smis);
	if (system("rm "SUPRT_STREAM));
        return retval;
}
