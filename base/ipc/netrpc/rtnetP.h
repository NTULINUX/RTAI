/*
 * Copyright (C) 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef __RTNET_RTNETP_H
#define __RTNET_RTNETP_H

int soft_rt_bind(int s,
		 struct sockaddr *my_addr,
		 int addrlen);

int soft_rt_close(int s);

int soft_rt_recvfrom(int s,
		     void *buf,
		     int len,
		     unsigned int flags,
		     struct sockaddr *from,
		     long *fromlen);

int soft_rt_sendto(int s,
		   const void *buf,
		   int len,
		   unsigned int flags,
		   struct sockaddr *to,
		   int tolen);

int soft_rt_socket(int domain,
		   int type,
		   int protocol);

int soft_rt_socket_callback(int s,
			    int (*func)(int s, void *arg),
			    void *arg);

struct sock_t { 
	int sock, opnd; 
	int tosend, recvd; 
	struct sockaddr addr; 
	int addrlen; 
	int (*callback)(int sock, void *arg); 
	void *arg; 
	char msg[MAX_MSG_SIZE]; 
};

#ifdef COMPILE_ANYHOW

/* the hard RTNet external interface, used just to check netrpc compiles */

struct rtdm_dev_context;

struct rtnet_callback {
	void    (*func)(struct rtdm_dev_context *, void *);
	void    *arg;
};

#define RTIOC_TYPE_NETWORK    RTDM_CLASS_NETWORK

#define RTNET_RTIOC_CALLBACK  _IOW(RTIOC_TYPE_NETWORK, 0x12, struct rtnet_callback)

#endif /* COMPILE_ANYHOW */

#endif /* !__RTNET_RTNETP_H */
