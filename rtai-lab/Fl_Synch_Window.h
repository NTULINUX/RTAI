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

#ifndef _FL_SYNCHRONOSCOPE_WINDOW_H_
#define _FL_SYNCHRONOSCOPE_WINDOW_H_

#include <Fl_Synch.h>
#include <efltk/Fl_MDI_Window.h>

class Fl_Synch_Window
{
	public:
		Fl_Synch_Window(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name);
		Fl_Synch *Synch;
		int x();
		int y();
		int w();
		int h();
		void show();
		void hide();
		int is_visible();
	private:
		Fl_MDI_Window *SWin;
};

#endif
