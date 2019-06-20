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

#include <Fl_Synch_Window.h>

int Fl_Synch_Window::x()
{
	return SWin->x();
}

int Fl_Synch_Window::y()
{
	return SWin->y();
}

int Fl_Synch_Window::w()
{
	return SWin->w() - SWin->box()->dw();
}

int Fl_Synch_Window::h()
{
	return SWin->h() - SWin->titlebar()->h() - SWin->box()->dh();
}

void Fl_Synch_Window::show()
{
	SWin->show();
}

void Fl_Synch_Window::hide()
{
	SWin->hide();
}

int Fl_Synch_Window::is_visible()
{
	return SWin->visible();
}

Fl_Synch_Window::Fl_Synch_Window(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name)
{
	Fl::lock();
	s->begin();
	Fl_MDI_Window *w = SWin = new Fl_MDI_Window(0, 0, width, height, name);
	w->resizable(w->view());
	w->view()->begin();
	Fl_Synch *o = Synch = new Fl_Synch(0, 0, width, height, name);
	o->mode(FL_DOUBLE|FL_RGB);
	w->view()->end();
	s->end();
	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);
	w->position(x, y);
	Fl::unlock();
}
