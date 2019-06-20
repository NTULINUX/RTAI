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

#include <Fl_Meters_Manager.h>

extern int Num_Meters;
extern Target_Meters_T *Meters;
extern Fl_Tool_Button *RLG_Meters_Mgr_Button;
extern Fl_Menu_Bar *RLG_Main_Menu;
extern Fl_Main_Window *RLG_Main_Window;
extern Fl_Menu_Item RLG_Main_Menu_Table[];

Fl_Menu_Item Meter_Opts[] = {
  	  {"Show Value",       0, 0, (void*)2, FL_MENU_TOGGLE},
  	  {"Show ticmarks",   0, 0, (void*)32, FL_MENU_TOGGLE},
	  {0}
	};



int Fl_Meters_Manager::x()
{
	return MWin->x();
}

int Fl_Meters_Manager::y()
{
	return MWin->y();
}

int Fl_Meters_Manager::w()
{
	return MWin->w();
}

int Fl_Meters_Manager::h()
{
	return MWin->h();
}

void Fl_Meters_Manager::show()
{
        MWin->show();
}

void Fl_Meters_Manager::hide()
{
        MWin->hide();
}

int Fl_Meters_Manager::show_hide(int n)
{
	return (int)(Meter_Show[n]->value());
}

void Fl_Meters_Manager::show_hide(int n, bool v)
{
	if (v) {
		Meter_Show[n]->set();
		Meters[n].visible = true;
	} else {
		Meter_Show[n]->clear();
		Meters[n].visible = false;
	}
}

int Fl_Meters_Manager::visible()
{
        return MWin->visible();
}

float Fl_Meters_Manager::minv(int n)
{
	float val = (float)atof(Min_Val[n]->value());
	return val;
}

float Fl_Meters_Manager::maxv(int n)
{
	float val = (float)atof(Max_Val[n]->value());
	return val;
}

void Fl_Meters_Manager::minv(int n, float val)
{
	char buf[20];
	sprintf(buf, "%.2f", val);
	Min_Val[n]->value(buf);
	Meter_Windows[n]->Meter->minimum_value(val);
}

void Fl_Meters_Manager::maxv(int n, float val)
{
	char buf[20];
	sprintf(buf, "%.2f", val);
	Max_Val[n]->value(buf);
	Meter_Windows[n]->Meter->maximum_value(val);
}

void Fl_Meters_Manager::b_color(int n, RGB_Color_T c_rgb)
{
	Meter_Windows[n]->Meter->bg_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(c_rgb.r*255.), (unsigned char)(c_rgb.g*255.), (unsigned char)(c_rgb.b*255.)));
	Meter_Windows[n]->Meter->bg_color(c_rgb.r, c_rgb.g, c_rgb.b);
}

void Fl_Meters_Manager::a_color(int n, RGB_Color_T c_rgb)
{
	Meter_Windows[n]->Meter->arrow_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(c_rgb.r*255.), (unsigned char)(c_rgb.g*255.), (unsigned char)(c_rgb.b*255.)));
	Meter_Windows[n]->Meter->arrow_color(c_rgb.r, c_rgb.g, c_rgb.b);
}

void Fl_Meters_Manager::g_color(int n, RGB_Color_T c_rgb)
{
	Meter_Windows[n]->Meter->grid_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(c_rgb.r*255.), (unsigned char)(c_rgb.g*255.), (unsigned char)(c_rgb.b*255.)));
	Meter_Windows[n]->Meter->grid_color(c_rgb.r, c_rgb.g, c_rgb.b);
}

float Fl_Meters_Manager::b_color(int n, int type)
{
	switch (type) {
		case R_COLOR:
			return Meter_Windows[n]->Meter->bg_r();
		case G_COLOR:
			return Meter_Windows[n]->Meter->bg_g();
		case B_COLOR:
			return Meter_Windows[n]->Meter->bg_b();
		default:
			return 0.0;
	}
}

float Fl_Meters_Manager::a_color(int n, int type)
{
	switch (type) {
		case R_COLOR:
			return Meter_Windows[n]->Meter->arrow_r();
		case G_COLOR:
			return Meter_Windows[n]->Meter->arrow_g();
		case B_COLOR:
			return Meter_Windows[n]->Meter->arrow_b();
		default:
			return 0.0;
	}
}

float Fl_Meters_Manager::g_color(int n, int type)
{
	switch (type) {
		case R_COLOR:
			return Meter_Windows[n]->Meter->grid_r();
		case G_COLOR:
			return Meter_Windows[n]->Meter->grid_g();
		case B_COLOR:
			return Meter_Windows[n]->Meter->grid_b();
		default:
			return 0.0;
	}
}

int Fl_Meters_Manager::mw_x(int n)
{
	return Meter_Windows[n]->x();
}

int Fl_Meters_Manager::mw_y(int n)
{
	return Meter_Windows[n]->y();
}

int Fl_Meters_Manager::mw_w(int n)
{
	return Meter_Windows[n]->w();
}

int Fl_Meters_Manager::mw_h(int n)
{
	return Meter_Windows[n]->h();
}

inline void Fl_Meters_Manager::select_meter_i(Fl_Browser *b, void *v)
{
	int n = b->value();
	for (int i = 0; i < Num_Meters; i++) {
		Meters_Tabs[i]->hide();
	}
	Meters_Tabs[n]->show();
}

void Fl_Meters_Manager::select_meter(Fl_Browser *b, void *v)
{
	((Fl_Meters_Manager *)(b->parent()->parent()->user_data()))->select_meter_i(b,v);
}

inline void Fl_Meters_Manager::show_meter_i(Fl_Check_Button *b, void *v)
{
	long n = (long)v;
	if (b->value()) {
		Meters[n].visible = true;
	} else {
		Meters[n].visible = false;
	}
}

void Fl_Meters_Manager::show_meter(Fl_Check_Button *b, void *v)
{
	((Fl_Meters_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->show_meter_i(b,v);
}

inline void Fl_Meters_Manager::enter_minval_i(Fl_Float_Input *b, void *v)
{
	long n = (long)v;
	float val = (float)atof(b->value());
	Meter_Windows[n]->Meter->minimum_value(val);
}

void Fl_Meters_Manager::enter_minval(Fl_Float_Input *b, void *v)
{
	((Fl_Meters_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enter_minval_i(b,v);
}

inline void Fl_Meters_Manager::enter_maxval_i(Fl_Float_Input *b, void *v)
{
	long n = (long)v;
	float val = (float)atof(b->value());
	Meter_Windows[n]->Meter->maximum_value(val);
}

void Fl_Meters_Manager::enter_maxval(Fl_Float_Input *b, void *v)
{
	((Fl_Meters_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enter_maxval_i(b,v);
}

inline void Fl_Meters_Manager::select_bg_color_i(Fl_Button *bb, void *v)
{
	long n = (long)v;
	uchar r,g,b;
	Fl_Color c;

	c = Meter_Windows[n]->Meter->bg_color();
	fl_get_color(c,r,g,b);
	if (!fl_color_chooser("New color:",r,g,b)) return;
	c = FL_FREE_COLOR;
	Meter_Windows[n]->Meter->bg_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb(r,g,b));
	Meter_Windows[n]->Meter->bg_color(r/255.,g/255.,b/255.);
}

void Fl_Meters_Manager::select_bg_color(Fl_Button *bb, void *v)
{
	((Fl_Meters_Manager *)(bb->parent()->parent()->parent()->parent()->user_data()))->select_bg_color_i(bb,v);
}

inline void Fl_Meters_Manager::select_grid_color_i(Fl_Button *bb, void *v)
{
	long n = (long)v;
	uchar r,g,b;
	Fl_Color c;

	c = Meter_Windows[n]->Meter->grid_color();
	fl_get_color(c,r,g,b);
	if (!fl_color_chooser("New color:",r,g,b)) return;
	c = FL_FREE_COLOR;
	Meter_Windows[n]->Meter->grid_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb(r,g,b));
	Meter_Windows[n]->Meter->grid_color(r/255.,g/255.,b/255.);
}

void Fl_Meters_Manager::select_grid_color(Fl_Button *bb, void *v)
{
	((Fl_Meters_Manager *)(bb->parent()->parent()->parent()->parent()->user_data()))->select_grid_color_i(bb,v);
}

inline void Fl_Meters_Manager::select_arrow_color_i(Fl_Button *bb, void *v)
{
	long n = (long)v;
	uchar r,g,b;
	Fl_Color c;

	c = Meter_Windows[n]->Meter->arrow_color();
	fl_get_color(c,r,g,b);
	if (!fl_color_chooser("New color:",r,g,b)) return;
	c = FL_FREE_COLOR;
	Meter_Windows[n]->Meter->arrow_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb(r,g,b));
	Meter_Windows[n]->Meter->arrow_color(r/255.,g/255.,b/255.);
}

void Fl_Meters_Manager::select_arrow_color(Fl_Button *bb, void *v)
{
	((Fl_Meters_Manager *)(bb->parent()->parent()->parent()->parent()->user_data()))->select_arrow_color_i(bb,v);
}

inline void Fl_Meters_Manager::enter_meter_style_i(Fl_Choice *b, void *v)
{
	int styles[] = {MS_DIAL, MS_BAR, MS_BARCENTER, MS_ANGLE, MS_VALUE};
	long n = (long)v;
	int val = b->value();
	Meter_Windows[n]->Meter->meter_style(styles[val]);
	
}

void Fl_Meters_Manager::enter_meter_style(Fl_Choice *b, void *v)
{
	((Fl_Meters_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enter_meter_style_i(b,v);
}

inline void Fl_Meters_Manager::enter_options_i(Fl_Menu_Button *b, void *v)
{
	long n = (long)v;
	int val = 0;
        int i;
 	
        for(i=0;i<b->children();i++) { // loop through all menu items, and add checked items to the value
      if( b->child(i)->value() ) val |= (int)(long)b->child(i)->user_data();		} 
	
	Meter_Windows[n]->Meter->meter_style( 
	Meter_Windows[n]->Meter->meter_style() | val); 
}

void Fl_Meters_Manager::enter_options(Fl_Menu_Button *b, void *v)
{
	((Fl_Meters_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enter_options_i(b,v);
}


inline void Fl_Meters_Manager::close_i(Fl_Button *b, void *v)
{
	MWin->hide();
	RLG_Meters_Mgr_Button->clear();
	RLG_Main_Menu_Table[13].clear();
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void Fl_Meters_Manager::close(Fl_Button *b, void *v)
{
	((Fl_Meters_Manager *)(b->parent()->parent()->user_data()))->close_i(b,v);
}

Fl_Meters_Manager::Fl_Meters_Manager(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name)
{
	Fl::lock();

	s->begin();
	Fl_MDI_Window *w = MWin = new Fl_MDI_Window(0, 0, width, height, name);
	w->user_data((void *)this);
	w->resizable(w->view());

	w->titlebar()->close_button()->hide();

	w->view()->begin();

	Meters_Tabs = new Fl_Tabs*[Num_Meters];
	Meter_Show = new Fl_Check_Button*[Num_Meters];
	Min_Val = new Fl_Float_Input*[Num_Meters];
	Max_Val = new Fl_Float_Input*[Num_Meters];
	Bg_Color = new Fl_Button*[Num_Meters];
	Arrow_Color = new Fl_Button*[Num_Meters];
	Grid_Color = new Fl_Button*[Num_Meters];
	Meter_Windows = new Fl_Meter_Window*[Num_Meters];
	Meter_Style = new Fl_Choice*[Num_Meters];
	Meter_Options = new  Fl_Menu_Button*[Num_Meters];

	for (int i = 0; i < Num_Meters; i++) {
		{ Fl_Tabs *o = Meters_Tabs[i] = new Fl_Tabs(160, 5, width-165, height-40);
		  o->new_page("Meter");
		  { Fl_Check_Button *o = Meter_Show[i] = new Fl_Check_Button(10, 10, 100, 20, "Show/Hide");
		    o->value(0);
		    o->callback((Fl_Callback *)show_meter, (void *)i);
		  }
		  { Fl_Float_Input *o = Min_Val[i] = new Fl_Float_Input(77, 35, 60, 20, "Minimum:   ");
		    o->align(FL_ALIGN_LEFT);
		    o->value("-1.2");
		    o->when(FL_WHEN_ENTER_KEY);
		    o->callback((Fl_Callback *)enter_minval, (void *)i);
		  }
		  { Fl_Float_Input *o = Max_Val[i] = new Fl_Float_Input(77, 60, 60, 20, "Maximum:  ");
		    o->align(FL_ALIGN_LEFT);
		    o->value("1.2");
		    o->when(FL_WHEN_ENTER_KEY);
		    o->callback((Fl_Callback *)enter_maxval, (void *)i);
		  }
  		  {  Fl_Choice *o = Meter_Style[i] = new Fl_Choice(10, 90, 90, 25, "");
                    o->add("Dial|Bar|+- Bar|Rotameter|Value");
                    o->align(FL_ALIGN_LEFT);
                    o->value(0);
                    o->when(FL_WHEN_ENTER_KEY);
                    o->callback((Fl_Callback *)enter_meter_style, (void *)i);
                  }
		  { Fl_Button *o = Bg_Color[i] = new Fl_Button(10, 120, 90, 25, "Bg Color");
		    o->callback((Fl_Callback *)select_bg_color, (void *)i);
		  }
		  { Fl_Button *o = Arrow_Color[i] = new Fl_Button(10, 150, 90, 25, "Arrow Color");
		    o->callback((Fl_Callback *)select_arrow_color, (void *)i);
		  }
		  { Fl_Button *o = Grid_Color[i] = new Fl_Button(10, 180, 90, 25, "Grid Color");
		    o->callback((Fl_Callback *)select_grid_color, (void *)i);
		  }
		  {
		    Fl_Menu_Button *o = Meter_Options[i] = new Fl_Menu_Button(10, 210, 90, 25, "Options ");
	 	    o->menu(Meter_Opts);
	            o->when(FL_WHEN_ENTER_KEY);
		    o->callback((Fl_Callback *)enter_options, (void *)i);
	           }
		  o->end();
		  Fl_Group::current()->resizable(w);
		}
	}
	for (int i = 1; i < Num_Meters; i++) {
		Meters_Tabs[i]->hide();
	}
	Meters_Tabs[0]->show();

	Help = new Fl_Button(width-150, height-30, 70, 25, "Help");
	Close = new Fl_Button(width-75, height-30, 70, 25, "Close");
	Close->callback((Fl_Callback *)close);

	Fl_Browser *o = Meters_Tree = new Fl_Browser(5, 5, 150, height-10);
	o->indented(1);
	o->callback((Fl_Callback *)select_meter);
	for (int i = 0; i < Num_Meters; i++) {
		add_paper(Meters_Tree, Meters[i].name, Fl_Image::read_xpm(0, meter_icon));
	}

	w->view()->end();

	s->end();

	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);
	w->position(x, y);

	Fl::unlock();
}
