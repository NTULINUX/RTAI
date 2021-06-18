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

#include <Fl_Synchs_Manager.h>

extern int Num_Synchs;
extern Target_Synchs_T *Synchs;
extern Fl_Tool_Button *RLG_Synchs_Mgr_Button;
extern Fl_Menu_Bar *RLG_Main_Menu;
extern Fl_Main_Window *RLG_Main_Window;
extern Fl_Menu_Item RLG_Main_Menu_Table[];

int Fl_Synchs_Manager::x()
{
	return SWin->x();
}

int Fl_Synchs_Manager::y()
{
	return SWin->y();
}

int Fl_Synchs_Manager::w()
{
	return SWin->w();
}

int Fl_Synchs_Manager::h()
{
	return SWin->h();
}

void Fl_Synchs_Manager::show()
{
        SWin->show();
}

void Fl_Synchs_Manager::hide()
{
        SWin->hide();
}

int Fl_Synchs_Manager::show_hide(int n)
{
	return (int)(Synch_Show[n]->value());
}

void Fl_Synchs_Manager::show_hide(int n, bool v)
{
	if (v) {
		Synch_Show[n]->set();
		Synchs[n].visible = true;
	} else {
		Synch_Show[n]->clear();
		Synchs[n].visible = false;
	}
}

int Fl_Synchs_Manager::visible()
{
        return SWin->visible();
}

int Fl_Synchs_Manager::sw_x(int n)
{
	return Synch_Windows[n]->x();
}

int Fl_Synchs_Manager::sw_y(int n)
{
	return Synch_Windows[n]->y();
}

int Fl_Synchs_Manager::sw_w(int n)
{
	return Synch_Windows[n]->w();
}

int Fl_Synchs_Manager::sw_h(int n)
{
	return Synch_Windows[n]->h();
}

inline void Fl_Synchs_Manager::select_synch_i(Fl_Browser *b, void *v)
{
	int n = b->value();
	for (int i = 0; i < Num_Synchs; i++) {
		Synchs_Tabs[i]->hide();
	}
	Synchs_Tabs[n]->show();
}

void Fl_Synchs_Manager::select_synch(Fl_Browser *b, void *v)
{
	((Fl_Synchs_Manager *)(b->parent()->parent()->user_data()))->select_synch_i(b,v);
}

inline void Fl_Synchs_Manager::show_synch_i(Fl_Check_Button *b, void *v)
{
	long n = (long)v;
	if (b->value()) {
		Synchs[n].visible = true;
	} else {
		Synchs[n].visible = false;
	}
}

void Fl_Synchs_Manager::show_synch(Fl_Check_Button *b, void *v)
{
	((Fl_Synchs_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->show_synch_i(b,v);
}

inline void Fl_Synchs_Manager::close_i(Fl_Button *b, void *v)
{
	SWin->hide();
	RLG_Synchs_Mgr_Button->clear();
	RLG_Main_Menu_Table[13].clear();
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void Fl_Synchs_Manager::close(Fl_Button *b, void *v)
{
	((Fl_Synchs_Manager *)(b->parent()->parent()->user_data()))->close_i(b,v);
}

Fl_Synchs_Manager::Fl_Synchs_Manager(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name)
{
	Fl::lock();

	s->begin();
	Fl_MDI_Window *w = SWin = new Fl_MDI_Window(0, 0, width, height, name);
	w->user_data((void *)this);
	w->resizable(w->view());

	w->titlebar()->close_button()->hide();

	w->view()->begin();

	Synchs_Tabs = new Fl_Tabs*[Num_Synchs];
	Synch_Show = new Fl_Check_Button*[Num_Synchs];
	Synch_Windows = new Fl_Synch_Window*[Num_Synchs];

	for (int i = 0; i < Num_Synchs; i++) {
		{ Fl_Tabs *o = Synchs_Tabs[i] = new Fl_Tabs(160, 5, width-165, height-40);
		  o->new_page("Synch");
		  { Fl_Check_Button *o = Synch_Show[i] = new Fl_Check_Button(10, 10, 100, 20, "Show/Hide");
		    o->value(0);
		    o->callback((Fl_Callback *)show_synch, (void *)i);
		  }
		  o->end();
		  Fl_Group::current()->resizable(w);
		}
	}
	for (int i = 1; i < Num_Synchs; i++) {
		Synchs_Tabs[i]->hide();
	}
	Synchs_Tabs[0]->show();

	Help = new Fl_Button(width-150, height-30, 70, 25, "Help");
	Close = new Fl_Button(width-75, height-30, 70, 25, "Close");
	Close->callback((Fl_Callback *)close);

	Fl_Browser *o = Synchs_Tree = new Fl_Browser(5, 5, 150, height-10);
	o->indented(1);
	o->callback((Fl_Callback *)select_synch);
	for (int i = 0; i < Num_Synchs; i++) {
		add_paper(Synchs_Tree, Synchs[i].name, Fl_Image::read_xpm(0, synchronoscope_icon));
	}

	w->view()->end();

	s->end();

	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);
	w->position(x, y);

	Fl::unlock();
}
