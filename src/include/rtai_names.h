/*
 * Copyright (C) 2000  POSEIDON CONTROLS INC <pcloutier@poseidoncontrols.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _RTAI_NAMES_H
#define _RTAI_NAMES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

pid_t rt_Name_attach(const char *name);

pid_t rt_Name_locate(const char *host,
		     const char *name);

int rt_Name_detach(pid_t pid);

void rt_boom(void);

void rt_stomp(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_RTAI_NAMES_H */
