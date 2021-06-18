/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)

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

#ifndef _FL_SYNCH_H_
#define _FL_SYNCH_H_

#include <efltk/Fl.h>
#include <efltk/Fl_Gl_Window.h>
#include <efltk/Fl_Color.h>
#include <efltk/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <math.h>

class Fl_Synch : public Fl_Gl_Window
{
	public:
		Fl_Synch(int x, int y, int w, int h, const char *title="rtSynchro");
		void value(float);
		float value();
	private:
		int Start_Deg, End_Deg;
		float Start_Rad, End_Rad;
		float Radius;
		float Arrow_Length;
		int N_Ticks;
		float Big_Ticks_Angle;
		float Minimum_Value;
		float Maximum_Value;
		float Value;
		float Bg_rgb[3];
		Fl_Color Bg_Color;
		float Grid_rgb[3];
		Fl_Color Grid_Color;
		float Arrow_rgb[3];
		Fl_Color Arrow_Color;
	protected:
		void initgl();
		void drawgrid();
		void draw();
};

#endif
