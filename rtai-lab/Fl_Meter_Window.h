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

#ifndef _FL_METER_WINDOW_H_
#define _FL_METER_WINDOW_H_

#include <Fl_Meter.h>
#include <efltk/Fl_MDI_Window.h>

class Fl_Meter_Window
{
	public:
		Fl_Meter_Window(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name);
		Fl_Meter *Meter;
		int x();
		int y();
		int w();
		int h();
		void resize(int, int, int, int);
		void show();
		void hide();
		int is_visible();
	private:
		Fl_MDI_Window *MWin;
};

#endif
