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

#include <Fl_Synch.h>

void Fl_Synch::value(float v)
{
	if (v < Minimum_Value) {
		Value = Minimum_Value;
		return;
	}
	if (v > Maximum_Value) {
		Value = Maximum_Value;
		return;
	}
	Value = v;
}

float Fl_Synch::value()
{
	return Value;
}
void Fl_Synch::initgl()
{
	glViewport(0, 0, w(), h());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w()/2., w()/2., -h()/2., h()/2., -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	gl_font(FL_HELVETICA_BOLD, 12);
	glClearColor(Bg_rgb[0], Bg_rgb[1], Bg_rgb[2], 0.0);
	glClear(GL_COLOR_BUFFER_BIT);	
}

void Fl_Synch::drawgrid()
{
	char tick_label[20];
	float tick_value;

	glLineWidth(0.1);
	glColor3f(Grid_rgb[0], Grid_rgb[1], Grid_rgb[2]);
	glLineWidth(2.0);
	for (int i = 0; i < N_Ticks; i++) {
		glBegin(GL_LINES);
		glVertex2f((Radius)*cos(Start_Rad + i*Big_Ticks_Angle), (Radius)*sin(Start_Rad + i*Big_Ticks_Angle));
		glVertex2f((Radius - Radius/10.)*cos(Start_Rad + i*Big_Ticks_Angle), (Radius - Radius/10.)*sin(Start_Rad + i*Big_Ticks_Angle));
		glEnd();
	}
	glDisable(GL_DEPTH_TEST);
	gl_color(fl_darker(FL_GRAY));
	gl_font(FL_TIMES, (int)(w()/25.));
	tick_value = Maximum_Value - ((N_Ticks - 1)/2.)*(Maximum_Value - Minimum_Value)/(N_Ticks - 1);
	sprintf(tick_label, "%1.1f", tick_value);
	gl_draw(tick_label, -(float(w())/50.), (Radius - Radius/4.));
	glEnable(GL_DEPTH_TEST);
}

void Fl_Synch::draw()
{
	float degree = End_Deg + (Start_Deg - End_Deg)*((Value-Minimum_Value)/(Maximum_Value-Minimum_Value));
	float rad = degree*(M_PI/180.);

	if (!valid()) {
		initgl();
		Radius = (float)(w()/2. - (w()/2.)/4.);
		Arrow_Length = Radius;
	}
	glClearColor(Bg_rgb[0], Bg_rgb[1], Bg_rgb[2], 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	drawgrid();
	glLineWidth(3.0);
	glColor3f(Arrow_rgb[0], Arrow_rgb[1], Arrow_rgb[2]);
	glBegin(GL_LINES);
	glVertex2f(0., 0.);
	glVertex2f(Arrow_Length*cos(rad), Arrow_Length*sin(rad));
	glEnd();
}

Fl_Synch::Fl_Synch(int x, int y, int w, int h, const char *title):Fl_Gl_Window(x,y,w,h,title)
{
	Start_Deg = -90;
	End_Deg   = 270;
	Start_Rad = (float)Start_Deg*(M_PI/180.);
	End_Rad   = (float)End_Deg*(M_PI/180.);
	Radius = (float)(w/2. - (w/2.)/4.);
	Arrow_Length = Radius;
	N_Ticks = 13;
	Big_Ticks_Angle = ((End_Deg - Start_Deg)*(M_PI/180.))/(N_Ticks - 1);
	Minimum_Value = -1.0;
	Maximum_Value = 1.0;
	Value = 1.0;
	Bg_rgb[0] = 1.0;
	Bg_rgb[1] = 1.0;
	Bg_rgb[2] = 1.0;
	Bg_Color = FL_GRAY;
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(Bg_rgb[0]*255.), (unsigned char)(Bg_rgb[1]*255.), (unsigned char)(Bg_rgb[2]*255.)));
	Grid_Color = FL_FREE_COLOR;
	Grid_rgb[0] = 0.650;
	Grid_rgb[1] = 0.650;
	Grid_rgb[2] = 0.650;
	Grid_Color = FL_GRAY;
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(Grid_rgb[0]*255.), (unsigned char)(Grid_rgb[1]*255.), (unsigned char)(Grid_rgb[2]*255.)));
	Grid_Color = FL_FREE_COLOR;
	Arrow_rgb[0] = 0.;
	Arrow_rgb[1] = 0.;
	Arrow_rgb[2] = 0.;
	Arrow_Color = FL_GRAY;
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(Arrow_rgb[0]*255.), (unsigned char)(Arrow_rgb[1]*255.), (unsigned char)(Arrow_rgb[2]*255.)));
	Arrow_Color = FL_FREE_COLOR;
}
