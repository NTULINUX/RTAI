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

#include <Fl_Leds_Manager.h>

extern int Num_Leds;
extern Target_Leds_T *Leds;
extern Fl_Tool_Button *RLG_Leds_Mgr_Button;
extern Fl_Menu_Bar *RLG_Main_Menu;
extern Fl_Main_Window *RLG_Main_Window;
extern Fl_Menu_Item RLG_Main_Menu_Table[];

int Fl_Leds_Manager::x()
{
	return LWin->x();
}

int Fl_Leds_Manager::y()
{
	return LWin->y();
}

int Fl_Leds_Manager::w()
{
	return LWin->w();
}

int Fl_Leds_Manager::h()
{
	return LWin->h();
}

void Fl_Leds_Manager::show()
{
        LWin->show();
}

int Fl_Leds_Manager::show_hide(int n)
{
	return (int)(Led_Show[n]->value());
}

void Fl_Leds_Manager::show_hide(int n, bool v)
{
	if (v) {
		Led_Show[n]->set();
		Leds[n].visible = true;
	} else {
		Led_Show[n]->clear();
		Leds[n].visible = false;
	}
}

void Fl_Leds_Manager::hide()
{
        LWin->hide();
}

int Fl_Leds_Manager::visible()
{
        return LWin->visible();
}

Fl_String Fl_Leds_Manager::color(int n)
{
	Fl_String s = Led_Colors[n]->value();
	return s;
}

void Fl_Leds_Manager::color(int n, Fl_String c)
{
	Led_Colors[n]->value(c);
	Led_Windows[n]->led_color(c);
}

int Fl_Leds_Manager::lw_x(int n)
{
	return Led_Windows[n]->x();
}

int Fl_Leds_Manager::lw_y(int n)
{
	return Led_Windows[n]->y();
}

int Fl_Leds_Manager::lw_w(int n)
{
	return Led_Windows[n]->w();
}

int Fl_Leds_Manager::lw_h(int n)
{
	return Led_Windows[n]->h();
}

inline void Fl_Leds_Manager::select_led_i(Fl_Browser *b, void *v)
{
	int n = b->value();
	for (int i = 0; i < Num_Leds; i++) {
		Leds_Tabs[i]->hide();
	}
	Leds_Tabs[n]->show();
}

void Fl_Leds_Manager::select_led(Fl_Browser *b, void *v)
{
	((Fl_Leds_Manager *)(b->parent()->parent()->user_data()))->select_led_i(b,v);
}

inline void Fl_Leds_Manager::show_led_i(Fl_Check_Button *b, void *v)
{
	long n = (long)v;
	if (b->value()) {
		Leds[n].visible = true;
	} else {
		Leds[n].visible = false;
	}
}

void Fl_Leds_Manager::show_led(Fl_Check_Button *b, void *v)
{
	((Fl_Leds_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->show_led_i(b,v);
}

inline void Fl_Leds_Manager::select_led_color_i(Fl_Button_Group *bg, void *v)
{
	long n = (long)v;
	Fl_String s = bg->value();

	Led_Windows[n]->led_color(s);
}

void Fl_Leds_Manager::select_led_color(Fl_Button_Group *bg, void *v)
{
	((Fl_Leds_Manager *)(bg->parent()->parent()->parent()->parent()->user_data()))->select_led_color_i(bg,v);
}

inline void Fl_Leds_Manager::led_layout_i(Fl_Value_Input *b, void *v)
{
	long n = (long)v;
	int value = (int)(b->value());

	Led_Windows[n]->layout(value);
}

void Fl_Leds_Manager::led_layout(Fl_Value_Input *b, void *v)
{
	((Fl_Leds_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->led_layout_i(b,v);
}

inline void Fl_Leds_Manager::close_i(Fl_Button *b, void *v)
{
	LWin->hide();
	RLG_Leds_Mgr_Button->clear();
	RLG_Main_Menu_Table[12].clear();
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void Fl_Leds_Manager::close(Fl_Button *b, void *v)
{
	((Fl_Leds_Manager *)(b->parent()->parent()->user_data()))->close_i(b,v);
}

Fl_Leds_Manager::Fl_Leds_Manager(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name)
{
	Fl::lock();

	s->begin();
	Fl_MDI_Window *w = LWin = new Fl_MDI_Window(0, 0, width, height, name);
	w->user_data((void *)this);
	w->resizable(w->view());

	w->titlebar()->close_button()->hide();

	w->view()->begin();

	Leds_Tabs = new Fl_Tabs*[Num_Leds];
	Led_Show = new Fl_Check_Button*[Num_Leds];
	Led_Colors = new Fl_Radio_Buttons*[Num_Leds];
	Led_Windows = new Fl_Led_Window*[Num_Leds];

	for (int i = 0; i < Num_Leds; i++) {
		{ Fl_Tabs *o = Leds_Tabs[i] = new Fl_Tabs(160, 5, width-165, height-40);
		  o->new_page("Leds");
		  { Fl_Check_Button *o = Led_Show[i] = new Fl_Check_Button(10, 25, 100, 20, "Show/Hide");
		    o->callback((Fl_Callback *)show_led, (void *)i);
		  }
		  { Fl_Radio_Buttons *o = Led_Colors[i] = new Fl_Radio_Buttons(10, 80, 90, 75, "Led Colors");
		    Fl_String_List colors("green,red,yellow", ",");
		    o->buttons(colors);
		    o->align(FL_ALIGN_TOP|FL_ALIGN_LEFT);
		    o->value("green");
		    o->callback((Fl_Callback *)select_led_color, (void *)i);
		  }
		  o->end();
		  Fl_Group::current()->resizable(w);
		}
	}
	for (int i = 1; i < Num_Leds; i++) {
		Leds_Tabs[i]->hide();
	}
	Leds_Tabs[0]->show();

	Help = new Fl_Button(width-150, height-30, 70, 25, "Help");
	Close = new Fl_Button(width-75, height-30, 70, 25, "Close");
	Close->callback((Fl_Callback *)close);

	Fl_Browser *o = Leds_Tree = new Fl_Browser(5, 5, 150, height-10);
	o->indented(1);
	o->callback((Fl_Callback *)select_led);
	for (int i = 0; i < Num_Leds; i++) {
		add_paper(Leds_Tree, Leds[i].name, Fl_Image::read_xpm(0, led_icon));
	}

	w->view()->end();

	s->end();

	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);
	w->position(x, y);

	Fl::unlock();
}
