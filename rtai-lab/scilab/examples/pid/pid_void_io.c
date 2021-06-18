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
#include "/home/mante/scilab-2.6/routines/machine.h"
/*---------------------------------------- Actuators */ 
void 
pid_actuator(flag,nport,nevprt,t,u,nu)
     /*
      * To be customized for standalone execution
      * flag  : specifies the action to be done
      * nport : specifies the  index of the Super Bloc 
      *         regular input (The input ports are numbered 
      *         from the top to the bottom ) 
      * nevprt: indicates if an activation had been received
      *         0 = no activation
      *         1 = activation
      * t     : the current time value
      * u     : the vector inputs value
      * nu    : the input  vector size
      */
     integer *flag,*nevprt,*nport;
     integer *nu;

     double  *t, u[];
{
  int k;
  switch (*nport) {
  case 1 :/* Port number 1 ----------*/
    /* skeleton to be customized */
    switch (*flag) {
    case 2 : 
      if(*nevprt>0) {/* get the input value */
  	/*for (k=0;k<*nu;k++) {????=u[k];} */
      } 
      break;
    case 4 : /* actuator initialisation */
      /* do whatever you want to initialize the actuator */
      break;
    case 5 : /* actuator ending */
      /* do whatever you want to end the actuator */
      break;
    }
  break;
  case 2 :/* Port number 2 ----------*/
    /* skeleton to be customized */
    switch (*flag) {
    case 2 : 
      if(*nevprt>0) {/* get the input value */
  	/*for (k=0;k<*nu;k++) {????=u[k];} */
      } 
      break;
    case 4 : /* actuator initialisation */
      /* do whatever you want to initialize the actuator */
      break;
    case 5 : /* actuator ending */
      /* do whatever you want to end the actuator */
      break;
    }
  break;
  case 3 :/* Port number 3 ----------*/
    /* skeleton to be customized */
    switch (*flag) {
    case 2 : 
      if(*nevprt>0) {/* get the input value */
  	/*for (k=0;k<*nu;k++) {????=u[k];} */
      } 
      break;
    case 4 : /* actuator initialisation */
      /* do whatever you want to initialize the actuator */
      break;
    case 5 : /* actuator ending */
      /* do whatever you want to end the actuator */
      break;
    }
  break;
  case 4 :/* Port number 4 ----------*/
    /* skeleton to be customized */
    switch (*flag) {
    case 2 : 
      if(*nevprt>0) {/* get the input value */
  	/*for (k=0;k<*nu;k++) {????=u[k];} */
      } 
      break;
    case 4 : /* actuator initialisation */
      /* do whatever you want to initialize the actuator */
      break;
    case 5 : /* actuator ending */
      /* do whatever you want to end the actuator */
      break;
    }
  break;
  }
}
/*---------------------------------------- Sensor */ 
void 
pid_sensor(flag,nport,nevprt,t,y,ny)
     /*
      * To be customized for standalone execution
      * flag  : specifies the action to be done
      * nport : specifies the  index of the Super Bloc 
      *         regular input (The input ports are numbered 
      *         from the top to the bottom ) 
      * nevprt: indicates if an activation had been received
      *         0 = no activation
      *         1 = activation
      * t     : the current time value
      * y     : the vector outputs value
      * ny    : the output  vector size
      */
     integer *flag,*nevprt,*nport;
     integer *ny;

     double  *t, y[];
{
  int k;
  switch (*flag) {
  case 1 : /* set the ouput value */
    /* for (k=0;k<*ny;k++) {y[k]=????;}*/
    break;
  case 2 : /* Update internal discrete state if any */
    break;
  case 4 : /* sensor initialisation */
    /* do whatever you want to initialize the sensor */
    break;
  case 5 : /* sensor ending */
    /* do whatever you want to end the sensor */
    break;
  }
}
/*---------------------------------------- callback at user params updates */ 
void 
pid_upar_update(int index)
{
}

