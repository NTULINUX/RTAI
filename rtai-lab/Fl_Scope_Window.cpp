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

#include <Fl_Scope_Window.h>

int Fl_Scope_Window::x()
{
	return SWin->x();
}

int Fl_Scope_Window::y()
{
	return SWin->y();
}

int Fl_Scope_Window::w()
{
	return SWin->w() - SWin->box()->dw();                          //see Fl_MDI_Window.cpp line 374 for more information
}

int Fl_Scope_Window::h()
{
	return SWin->h() - SWin->titlebar()->h() - SWin->box()->dh(); //see Fl_MDI_Window.cpp line 374 for more information
}

void Fl_Scope_Window::resize(int x, int y, int w, int h)
{
	SWin->resize(x,y,w,h);
}

void Fl_Scope_Window::show()
{
	SWin->show();
}

void Fl_Scope_Window::hide()
{
	SWin->hide();
}

int Fl_Scope_Window::is_visible()
{
	return SWin->visible();
}

Fl_Scope_Window::Fl_Scope_Window(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name, int n_traces, float dt)
{
	Fl::lock();
	s->begin();
	Fl_MDI_Window *w = SWin = new Fl_MDI_Window(0, 0, width, height, name);
	w->resizable(w->view());
	w->view()->begin();
	Fl_Scope *o = Plot = new Fl_Scope(0, 0, width, height, n_traces, name);
	o->sampling_frequency(1./dt);
	o->setdx();
	o->trace_length(width);
	o->mode(FL_DOUBLE|FL_RGB);
	w->view()->end();
	s->end();
	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);
	w->position(x, y);
	Fl::unlock();
}
