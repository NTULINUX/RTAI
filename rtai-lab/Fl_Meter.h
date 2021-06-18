/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2003  Roberto Bucher (roberto.bucher@supsi.ch)
COPYRIGHT (C) 2003  Peter Brier (pbrier@dds.nl)

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

#ifndef _FL_METER_H_
#define _FL_METER_H_

#include <efltk/Fl.h>
#include <efltk/Fl_Gl_Window.h>
#include <efltk/Fl_Color.h>
#include <efltk/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <math.h>

/* Meter styles */
enum { MS_DIAL=1, MS_VALUE=2, MS_BAR=4, MS_BARCENTER=8, MS_ANGLE=16, MS_GRID=32 };

class Fl_Meter : public Fl_Gl_Window
{
	public:
		Fl_Meter(int x, int y, int w, int h, const char *title="rtMeter");
		void value(float);
		float value();
		void minimum_value(float);
		void maximum_value(float);
		void bg_color(float r, float g, float b);
		void bg_color(Fl_Color c);
		float bg_r();
		float bg_g();
		float bg_b();
		Fl_Color bg_color();
		void bg_free_color();
		void grid_color(float r, float g, float b);
		void grid_color(Fl_Color c);
		float grid_r();
		float grid_g();
		float grid_b();
		Fl_Color grid_color();
		void grid_free_color();
		void arrow_color(float r, float g, float b);
		void arrow_color(Fl_Color c);
		float arrow_r();
		float arrow_g();
		float arrow_b();
		Fl_Color arrow_color();
		void arrow_free_color();
		int meter_style(void);
		void meter_style(int);
	private:
		int Start_Deg, End_Deg;
		float Start_Rad, End_Rad;
		float Radius;
		float Arrow_Length;
		int N_Ticks;
		int N_Ticks_Per_Div;
		float Big_Ticks_Angle;
		float Small_Ticks_Angle;
		float Minimum_Value;
		float Maximum_Value;
		float Value, Unclipped_Value;
		int overload;
		float Bg_rgb[3];
		Fl_Color Bg_Color;
		float Grid_rgb[3];
		Fl_Color Grid_Color;
		float Arrow_rgb[3];
		Fl_Color Arrow_Color;
		int Meter_Style;
	protected:
		void initgl();
		void drawgrid();
		void draw();
};

#endif
