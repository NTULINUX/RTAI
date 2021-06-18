/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2003  Roberto Bucher (roberto.bucher@supsi.ch)
COPYRIGHT (C) 2003  Peter Brier (pbrier@dds.nl)

COPYRIGHT (C) 2009  Henrik Slotholt (rtai@slotholt.net)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty ofSh
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <http://www.gnu.org/licenses/>.

*/

#include <Fl_Scopes_Manager.h>

extern int Num_Scopes;
extern Target_Scopes_T *Scopes;
extern Fl_Tool_Button *RLG_Scopes_Mgr_Button;
extern Fl_Menu_Bar *RLG_Main_Menu;
extern Fl_Main_Window *RLG_Main_Window;
extern Fl_Menu_Item RLG_Main_Menu_Table[];

Fl_Menu_Item Trace_Opts[] = {
  	  {"Draw Zero axis",  0, 0, (void*)tfDrawZero, FL_MENU_TOGGLE},
  	  {"Draw Axis label", 0, 0, (void*)tfDrawLabel, FL_MENU_TOGGLE|FL_MENU_DIVIDER},
  	  {"Show Units/div",  0, 0, (void*)tfStatUd, FL_MENU_TOGGLE},
  	  {"Show Min",        0, 0, (void*)tfStatMin, FL_MENU_TOGGLE},
  	  {"Show Max",        0, 0, (void*)tfStatMax, FL_MENU_TOGGLE},
  	  {"Show Peak-Peak",  0, 0, (void*)tfStatPpk, FL_MENU_TOGGLE},
  	  {"Show Average",    0, 0, (void*)tfStatAvg, FL_MENU_TOGGLE},
  	  {"Show RMS",        0, 0, (void*)tfStatRms, FL_MENU_TOGGLE|FL_MENU_DIVIDER},
  	  {"Show value Y1",   0, 0, (void*)tfStatY1, FL_MENU_TOGGLE},
  	  {"Show value Y2",   0, 0, (void*)tfStatY2, FL_MENU_TOGGLE},
  	  {"Show value dy",   0, 0, (void*)tfStatDy, FL_MENU_TOGGLE},
 	  {"Show value dy/dt",0, 0, (void*)tfStatDyDt, FL_MENU_TOGGLE|FL_MENU_DIVIDER},
  	  {"AC coupling",     0, 0, (void*)tfAC, FL_MENU_TOGGLE},
	  {0}
	};

Fl_Menu_Item Scope_Opts[] = {
  	  {"Show grid",       0, 0, (void*)sfDrawGrid, FL_MENU_TOGGLE},
  	  {"Show ticmarks",   0, 0, (void*)sfDrawTics, FL_MENU_TOGGLE},
  	  {"Show cursors",    0, 0, (void*)sfCursors, FL_MENU_TOGGLE},
  	  {"Horizontal bars", 0, 0, (void*)sfHorBar, FL_MENU_TOGGLE},
  	  {"Vertical bars",   0, 0, (void*)sfVerBar, FL_MENU_TOGGLE},
	  {0}
	};


int Fl_Scopes_Manager::x()
{
	return SWin->x();
}

int Fl_Scopes_Manager::y()
{
	return SWin->y();
}

int Fl_Scopes_Manager::w()
{
	return SWin->w();
}

int Fl_Scopes_Manager::h()
{
	return SWin->h();
}

void Fl_Scopes_Manager::show()
{
        SWin->show();
}

void Fl_Scopes_Manager::hide()
{
        SWin->hide();
}

int Fl_Scopes_Manager::show_hide(int n)
{
	return (int)(Scope_Show[n]->value());
}

void Fl_Scopes_Manager::show_hide(int n, bool v)
{
	if (v) {
		Scope_Show[n]->set();
		Scopes[n].visible = true;
	} else {
		Scope_Show[n]->clear();
		Scopes[n].visible = false;
	}
	Scopes_Tabs[n]->redraw();
}

int Fl_Scopes_Manager::trace_show_hide(int n, int t)
{
	return (int)(Trace_Show[n][t]->value());
}

void Fl_Scopes_Manager::trace_show_hide(int n, int t, bool v)
{
	if (v) {
		Trace_Show[n][t]->set();
		Scope_Windows[n]->Plot->show_trace(t, true);
	} else {
		Trace_Show[n][t]->clear();
		Scope_Windows[n]->Plot->show_trace(t, false);
	}
	Scopes_Tabs[n]->redraw();
}

int Fl_Scopes_Manager::grid_on_off(int n)
{
	return Scope_Windows[n]->Plot->scope_flags() & sfDrawGrid;
}

void Fl_Scopes_Manager::grid_on_off(int n, bool v)
{
	if (v) {
		// Grid_On[n]->set();
		Scope_Windows[n]->Plot->grid_visible(true);
	} else {
		// Grid_On[n]->clear();
		Scope_Windows[n]->Plot->grid_visible(false);
	}
	Scopes_Tabs[n]->redraw();
}

int Fl_Scopes_Manager::points_time(int n)
{
	return (int)(Save_Type[n]->value());
}

void Fl_Scopes_Manager::points_time(int n, bool v)
{
	if (v) {
		Save_Type[n]->set();
		Save_Points[n]->activate();
		Save_Time[n]->deactivate();
	} else {
		Save_Type[n]->clear();
		Save_Points[n]->deactivate();
		Save_Time[n]->activate();
	}
	Scopes_Tabs[n]->redraw();
}

int Fl_Scopes_Manager::sw_x(int n)
{
	return Scope_Windows[n]->x();
}

int Fl_Scopes_Manager::sw_y(int n)
{
	return Scope_Windows[n]->y();
}

int Fl_Scopes_Manager::sw_w(int n)
{
	return Scope_Windows[n]->w();
}

int Fl_Scopes_Manager::sw_h(int n)
{
	return Scope_Windows[n]->h();
}

int Fl_Scopes_Manager::visible()
{
        return SWin->visible();
}

void Fl_Scopes_Manager::b_color(int n, RGB_Color_T c_rgb)
{
	Scope_Windows[n]->Plot->bg_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(c_rgb.r*255.), (unsigned char)(c_rgb.g*255.), (unsigned char)(c_rgb.b*255.)));
	Scope_Windows[n]->Plot->bg_color(c_rgb.r, c_rgb.g, c_rgb.b);
}

void Fl_Scopes_Manager::g_color(int n, RGB_Color_T c_rgb)
{
	Scope_Windows[n]->Plot->grid_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(c_rgb.r*255.), (unsigned char)(c_rgb.g*255.), (unsigned char)(c_rgb.b*255.)));
	Scope_Windows[n]->Plot->grid_color(c_rgb.r, c_rgb.g, c_rgb.b);
}

void Fl_Scopes_Manager::t_color(int n, int t, RGB_Color_T c_rgb)
{
	Scope_Windows[n]->Plot->trace_free_color(t);
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(c_rgb.r*255.), (unsigned char)(c_rgb.g*255.), (unsigned char)(c_rgb.b*255.)));
	Scope_Windows[n]->Plot->trace_color(t, c_rgb.r, c_rgb.g, c_rgb.b);
	Trace_Page[n][t]->label_color(fl_rgb((unsigned char)(c_rgb.r*255.), (unsigned char)(c_rgb.g*255.), (unsigned char)(c_rgb.b*255.)));
	Trace_Page[n][t]->redraw();
}

float Fl_Scopes_Manager::b_color(int n, int type)
{
	switch (type) {
		case R_COLOR:
			return Scope_Windows[n]->Plot->bg_r();
		case G_COLOR:
			return Scope_Windows[n]->Plot->bg_g();
		case B_COLOR:
			return Scope_Windows[n]->Plot->bg_b();
		default:
			return 0.0;
	}
}

float Fl_Scopes_Manager::g_color(int n, int type)
{
	switch (type) {
		case R_COLOR:
			return Scope_Windows[n]->Plot->grid_r();
		case G_COLOR:
			return Scope_Windows[n]->Plot->grid_g();
		case B_COLOR:
			return Scope_Windows[n]->Plot->grid_b();
		default:
			return 0.0;
	}
}

float Fl_Scopes_Manager::t_color(int n, int t, int type)
{
	switch (type) {
		case R_COLOR:
			return Scope_Windows[n]->Plot->tr_r(t);
		case G_COLOR:
			return Scope_Windows[n]->Plot->tr_g(t);
		case B_COLOR:
			return Scope_Windows[n]->Plot->tr_b(t);
		default:
			return 0.0;
	}
}

float Fl_Scopes_Manager::sec_div(int n)
{
	float val = (float)atof(Sec_Div[n]->value());
	return val;
}

void Fl_Scopes_Manager::sec_div(int n, float val)
{
	char buf[20];

	if (val > 0.) {
		sprintf(buf, "%.4f", val);
		Sec_Div[n]->value(buf);
		Scope_Windows[n]->Plot->time_range(val*NDIV_GRID_X);
	}
}

float Fl_Scopes_Manager::trace_unit_div(int n, int t)
{
	float val = (float)atof(Units_Div[n][t]->value());
	return val;
}

void Fl_Scopes_Manager::trace_unit_div(int n, int t, float val)
{
	char buf[20];

	if (val > 0.) {
		sprintf(buf, "%.4f", val);
		Units_Div[n][t]->value(buf);
		Scope_Windows[n]->Plot->y_range_inf(t, -val*NDIV_GRID_Y/2.);
		Scope_Windows[n]->Plot->y_range_sup(t, val*NDIV_GRID_Y/2.);
	}
}

float Fl_Scopes_Manager::trace_offset(int n, int t)
{
	float val = (float)(Trace_Pos[n][t]->value());
	return val;
}

void Fl_Scopes_Manager::trace_offset(int n, int t, float val)
{
	Scope_Windows[n]->Plot->trace_offset(t, val);
	Trace_Pos[n][t]->value((float)val);
}


void Fl_Scopes_Manager::trace_width(int n, int t, float val)
{
	Scope_Windows[n]->Plot->trace_width(t, val);
	Trace_Width[n][t]->value((float)val);
}

float Fl_Scopes_Manager::trace_width(int n, int t)
{
	float val = (float)(Trace_Width[n][t]->value());
	return val;
}

void Fl_Scopes_Manager::trace_flags(int n, int t, int flags)
{
	Scope_Windows[n]->Plot->trace_flags(t, flags);
}



inline void Fl_Scopes_Manager::enter_trigger_mode_i(Fl_Choice *b, void *v)
{
	int modes[] = {tmRoll, tmOverwrite, tmTriggerCh1Pos, tmTriggerCh1Neg, tmHold};
	long n = (long)v;
	int val = b->value();
	Scope_Windows[n]->Plot->trigger_mode(modes[val]);
	
}

void Fl_Scopes_Manager::enter_trigger_mode(Fl_Choice *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enter_trigger_mode_i(b,v);
}

// same callback is used for trace and scope options
inline void Fl_Scopes_Manager::enter_options_i(Fl_Menu_Button *b, void *v)
{
	s_idx_T *idx = (s_idx_T *)v;
	int val = 0;
        int i;
 	
        for(i=0;i<b->children();i++) { // loop through all menu items, and add checked items to the value
	  if( b->child(i)->value() ) val |= (int)(long)b->child(i)->user_data();
	} 
	if ( b->label() == "Options " ) { // callback is used for trace and scope flags, if there is a space after "Options" set trace flags
	  Scope_Windows[idx->scope_idx]->Plot->trace_flags(idx->trace_idx, val); 
	} else {
	  Scope_Windows[(int)(long)v]->Plot->scope_flags(val);
	}
}

void Fl_Scopes_Manager::enter_options(Fl_Menu_Button *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enter_options_i(b,v);
}


int Fl_Scopes_Manager::p_save(int n)
{
	int val = (int)atoi(Save_Points[n]->value());
	return val;
}

void Fl_Scopes_Manager::p_save(int n, int val)
{
	char buf[20];

	if (val > 0) {
		sprintf(buf, "%d", val);
		Save_Points[n]->value(buf);
	}
}

float Fl_Scopes_Manager::t_save(int n)
{
	float val = (float)atof(Save_Time[n]->value());
	return val;
}

void Fl_Scopes_Manager::t_save(int n, float val)
{
	char buf[20];

	if (val > 0.) {
		sprintf(buf, "%.4f", val);
		Save_Time[n]->value(buf);
	}
}

const char *Fl_Scopes_Manager::file_name(int n)
{
	return (strdup(Save_File[n]->value()));
}

void Fl_Scopes_Manager::file_name(int n, const char *fn)
{
	Save_File[n]->value(strdup(fn));
	Scopes_Tabs[n]->redraw();
}

int Fl_Scopes_Manager::start_saving(int n)
{
	return Save_Flag[n];
}

int Fl_Scopes_Manager::n_points_to_save(int n)
{
	int n_points;

	if (Save_Type[n]->value()) {
		n_points = (int)atoi(Save_Points[n]->value());
		if (n_points < 0) return 0;
	} else {
		n_points = (int)(atof(Save_Time[n]->value())*Scope_Windows[n]->Plot->sampling_frequency());
		if (n_points < 0) return 0;
	}
	return n_points;
}

void Fl_Scopes_Manager::stop_saving(int n)
{
	fclose(Save_File_Pointer[n]);
	Save_Type[n]->activate();
	if (Save_Type[n]->value()) {
		Save_Points[n]->activate();
	} else {
		Save_Time[n]->activate();
	}
	Save_File[n]->activate();
	Save[n]->activate();
	Save[n]->value(0);
	Save[n]->label("Save");
	Scopes_Tabs[n]->redraw();
	Save_Flag[n] = false;
}

FILE *Fl_Scopes_Manager::save_file(int n)
{
	return Save_File_Pointer[n];
}

inline void Fl_Scopes_Manager::select_scope_i(Fl_Browser *b, void *v)
{
	int n = b->value();
	for (int i = 0; i < Num_Scopes; i++) {
		Scopes_Tabs[i]->hide();
	}
	Scopes_Tabs[n]->show();
}

void Fl_Scopes_Manager::select_scope(Fl_Browser *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->user_data()))->select_scope_i(b,v);
}

inline void Fl_Scopes_Manager::show_scope_i(Fl_Check_Button *b, void *v)
{
	long n = (long)v;
	if (b->value()) {
		Scopes[n].visible = true;
		Scope_Pause[n]->activate();
		Scope_OneShot[n]->activate();
	} else {
		Scopes[n].visible = false;
		Scope_Pause[n]->deactivate();
		Scope_OneShot[n]->deactivate();
	}
}

void Fl_Scopes_Manager::show_scope(Fl_Check_Button *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->show_scope_i(b,v);
}

inline void Fl_Scopes_Manager::pause_scope_i(Fl_Button *b, void *v)
{

	long n = (long)v;
	if (b->value()) {
		Scope_Windows[n]->Plot->pause(false);
	} else {
		Scope_Windows[n]->Plot->pause(true);
	}
}

void Fl_Scopes_Manager::pause_scope(Fl_Button *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->pause_scope_i(b,v);
}

inline void Fl_Scopes_Manager::oneshot_scope_i(Fl_Check_Button *b, void *v)
{
	long n = (long)v;
	Scope_Windows[n]->Plot->oneshot(b->value() != 0);
        if ( b->value() ) 
  	  Scope_Pause[n]->activate();
	else 
	  Scope_Pause[n]->deactivate();
}

void Fl_Scopes_Manager::oneshot_scope(Fl_Check_Button *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->oneshot_scope_i(b,v);
}


inline void Fl_Scopes_Manager::select_grid_color_i(Fl_Button *bb, void *v)
{
	long n = (long)v;
	uchar r,g,b;
	Fl_Color c;

	c = Scope_Windows[n]->Plot->grid_color();
	fl_get_color(c,r,g,b);
	if (!fl_color_chooser("New color:",r,g,b)) return;
	c = FL_FREE_COLOR;
	Scope_Windows[n]->Plot->grid_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb(r,g,b));
	Scope_Windows[n]->Plot->grid_color(r/255.,g/255.,b/255.);
}

void Fl_Scopes_Manager::select_grid_color(Fl_Button *bb, void *v)
{
	((Fl_Scopes_Manager *)(bb->parent()->parent()->parent()->parent()->user_data()))->select_grid_color_i(bb,v);
}

inline void Fl_Scopes_Manager::select_bg_color_i(Fl_Button *bb, void *v)
{
	long n = (long)v;
	uchar r,g,b;
	Fl_Color c;

	c = Scope_Windows[n]->Plot->bg_color();
	fl_get_color(c,r,g,b);
	if (!fl_color_chooser("New color:",r,g,b)) return;
	c = FL_FREE_COLOR;
	Scope_Windows[n]->Plot->bg_free_color();
	fl_set_color(FL_FREE_COLOR, fl_rgb(r,g,b));
	Scope_Windows[n]->Plot->bg_color(r/255.,g/255.,b/255.);
}

void Fl_Scopes_Manager::select_bg_color(Fl_Button *bb, void *v)
{
	((Fl_Scopes_Manager *)(bb->parent()->parent()->parent()->parent()->user_data()))->select_bg_color_i(bb,v);
}

inline void Fl_Scopes_Manager::enter_secdiv_i(Fl_Input_Browser *b, void *v)
{
	long n = (long)v;
	float val = (float)atof(b->value());

	if (val > 0.) {
		Scope_Windows[n]->Plot->time_range(val*NDIV_GRID_X);
	}
}

void Fl_Scopes_Manager::enter_secdiv(Fl_Input_Browser *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enter_secdiv_i(b,v);
}

inline void Fl_Scopes_Manager::select_save_i(Fl_Check_Button *b, void *v)
{
	long n = (long)v;
	if (b->value()) {
		Save_Points[n]->activate();
		Save_Time[n]->deactivate();
	} else {
		Save_Points[n]->deactivate();
		Save_Time[n]->activate();
	}
	Scopes_Tabs[n]->redraw();
}

void Fl_Scopes_Manager::select_save(Fl_Check_Button *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->select_save_i(b,v);
}

inline void Fl_Scopes_Manager::enable_saving_i(Fl_Light_Button *b, void *v)
{
	long n = (long)v;
	if (b->value()) {
		if ((Save_File_Pointer[n] = fopen(Save_File[n]->value(), "a+")) == NULL) {
			fl_alert("Error in opening file %s", Save_File[n]->value());
			return;
		}
		b->deactivate();
		Save_Type[n]->deactivate();
		Save_Time[n]->deactivate();
		Save_Points[n]->deactivate();
		Save_File[n]->deactivate();
		Save[n]->label("Saving...");
		Scopes_Tabs[n]->redraw();
		Save_Flag[n] = true;
	}
}

void Fl_Scopes_Manager::enable_saving(Fl_Light_Button *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enable_saving_i(b,v);
}

inline void Fl_Scopes_Manager::show_trace_i(Fl_Check_Button *b, void *v)
{
	s_idx_T *idx = (s_idx_T *)v;
	int scope = idx->scope_idx;
	int trace = idx->trace_idx;
	if (b->value()) {
		Scope_Windows[scope]->Plot->show_trace(trace, true);
	} else {
		Scope_Windows[scope]->Plot->show_trace(trace, false);
	}
}

void Fl_Scopes_Manager::show_trace(Fl_Check_Button *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->show_trace_i(b,v);
}

inline void Fl_Scopes_Manager::enter_unitsdiv_i(Fl_Input_Browser *b, void *v)
{
	s_idx_T *idx = (s_idx_T *)v;
	int scope = idx->scope_idx;
	int trace = idx->trace_idx;
	float val = (float)atof(b->value());

	if (val > 0.) {
		Scope_Windows[scope]->Plot->y_range_inf(trace, -val*NDIV_GRID_Y/2.);
		Scope_Windows[scope]->Plot->y_range_sup(trace, val*NDIV_GRID_Y/2.);
	}
}

void Fl_Scopes_Manager::enter_unitsdiv(Fl_Input_Browser *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enter_unitsdiv_i(b,v);
}

inline void Fl_Scopes_Manager::select_trace_color_i(Fl_Button *bb, void *v)
{
	s_idx_T *idx = (s_idx_T *)v;
	int scope = idx->scope_idx;
	int trace = idx->trace_idx;
	uchar r,g,b;
	Fl_Color c;

	c = Scope_Windows[scope]->Plot->trace_color(trace);
	fl_get_color(c,r,g,b);
	if (!fl_color_chooser("New color:",r,g,b)) return;
	c = FL_FREE_COLOR;
	Scope_Windows[scope]->Plot->trace_free_color(trace);
	fl_set_color(FL_FREE_COLOR, fl_rgb(r,g,b));
	Scope_Windows[scope]->Plot->trace_color(trace, r/255.,g/255.,b/255.);
	Trace_Page[scope][trace]->label_color(fl_rgb(r,g,b));
	Trace_Page[scope][trace]->redraw();
}

void Fl_Scopes_Manager::select_trace_color(Fl_Button *bb, void *v)
{
	((Fl_Scopes_Manager *)(bb->parent()->parent()->parent()->parent()->user_data()))->select_trace_color_i(bb,v);
}

void Fl_Scopes_Manager::change_trace_pos(Fl_Dial *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->change_trace_pos_i(b,v);
}

inline void Fl_Scopes_Manager::change_trace_pos_i(Fl_Dial *b, void *v)
{
	s_idx_T *idx = (s_idx_T *)v;
	int scope = idx->scope_idx;
	int trace = idx->trace_idx;

	Scope_Windows[scope]->Plot->trace_offset(trace, (float)b->value());
}

void Fl_Scopes_Manager::change_trace_width(Fl_Dial *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->change_trace_width_i(b,v);
}

inline void Fl_Scopes_Manager::change_trace_width_i(Fl_Dial *b, void *v)
{
	s_idx_T *idx = (s_idx_T *)v;
	int scope = idx->scope_idx;
	int trace = idx->trace_idx;

	Scope_Windows[scope]->Plot->trace_width(trace, (float)b->value());
}


inline void Fl_Scopes_Manager::close_i(Fl_Button *b, void *v)
{
	SWin->hide();
	RLG_Main_Menu_Table[10].clear();
	RLG_Scopes_Mgr_Button->clear();
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void Fl_Scopes_Manager::close(Fl_Button *b, void *v)
{
	((Fl_Scopes_Manager *)(b->parent()->parent()->user_data()))->close_i(b,v);
}

Fl_Scopes_Manager::Fl_Scopes_Manager(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name)
{
	Fl::lock();

	s->begin();
	Fl_MDI_Window *w = SWin = new Fl_MDI_Window(0, 0, width, height, name);
	w->user_data((void *)this);
	w->resizable(w->view());

	w->titlebar()->close_button()->hide();

	w->view()->begin();

	Scopes_Tabs = new Fl_Tabs*[Num_Scopes];
	Scope_Show = new Fl_Check_Button*[Num_Scopes];
	Scope_Pause = new Fl_Button*[Num_Scopes];
	Scope_OneShot = new Fl_Check_Button*[Num_Scopes];
	Scope_Options = new Fl_Menu_Button*[Num_Scopes];
	Grid_Color = new Fl_Button*[Num_Scopes];
	Bg_Color = new Fl_Button*[Num_Scopes];
	Sec_Div = new Fl_Input_Browser*[Num_Scopes];
	Save_Type = new Fl_Check_Button*[Num_Scopes];
	Save_Points = new Fl_Int_Input*[Num_Scopes];
	Save_Time = new Fl_Float_Input*[Num_Scopes];
	Save_File = new Fl_Input*[Num_Scopes];
	Save = new Fl_Light_Button*[Num_Scopes];
	Save_Flag = new int[Num_Scopes];
	Save_File_Pointer = new FILE*[Num_Scopes];

	Trace_Page = new Fl_Group**[Num_Scopes];
	Trace_Show = new Fl_Check_Button**[Num_Scopes];
	Units_Div = new Fl_Input_Browser**[Num_Scopes];
	Trace_Color = new Fl_Button**[Num_Scopes];
	Trace_Pos = new Fl_Dial**[Num_Scopes];
	Trace_Width = new Fl_Dial**[Num_Scopes];
 	Trigger_Mode = new Fl_Choice*[Num_Scopes];

        Trace_Options = new Fl_Menu_Button**[Num_Scopes];

	Scope_Windows = new Fl_Scope_Window*[Num_Scopes];

	for (int i = 0; i < Num_Scopes; i++) {
		Save_Flag[i] = false;
		{ Fl_Tabs *o = Scopes_Tabs[i] = new Fl_Tabs(160, 5, width-165, height-40);
		  o->new_page("General");
		  { Fl_Check_Button *o = Scope_Show[i] = new Fl_Check_Button(10, 25, 100, 20, "Show/Hide");
		    o->callback((Fl_Callback *)show_scope, (void *)i);
		  }
		  { Fl_Button *o = Scope_Pause[i] = new Fl_Button(10, 75, 90, 25, "Trigger");
		    o->value(0);
		    o->deactivate();
		    o->when(FL_WHEN_CHANGED);
		    o->callback((Fl_Callback *)pause_scope, (void *)i);
		  }
		  { Fl_Check_Button *o = Scope_OneShot[i] = new Fl_Check_Button(10, 50, 100, 20, "OneShot/Run");
		    o->deactivate();
		    o->callback((Fl_Callback *)oneshot_scope, (void *)i);
		  }
		  { Fl_Menu_Button *o = Scope_Options[i] = new Fl_Menu_Button(10, 105, 90, 25, "Options");
			o->menu(Scope_Opts);
			o->when(FL_WHEN_ENTER_KEY);
			o->child(0)->set_value();
		    	o->callback((Fl_Callback *)enter_options, (void *)i);
		  }
		  { Fl_Button *o = Grid_Color[i] = new Fl_Button(10, 135, 90, 25, "Grid Color");
		    o->callback((Fl_Callback *)select_grid_color, (void *)i);
		  }
		  { Fl_Button *o = Bg_Color[i] = new Fl_Button(10, 165, 90, 25, "Bg Color");
		    o->callback((Fl_Callback *)select_bg_color, (void *)i);
		  }
		  { Fl_Input_Browser *o = Sec_Div[i] = new Fl_Input_Browser(200, 25, 60, 20, "Sec/Div:  ");
		    o->add("0.001|0.005|0.01|0.05|0.1|0.5|1");
		    o->align(FL_ALIGN_LEFT);
		    o->value("0.1");
		    o->when(FL_WHEN_ENTER_KEY);
		    o->callback((Fl_Callback *)enter_secdiv, (void *)i);
		  }
		  { Fl_Check_Button *o = Save_Type[i] = new Fl_Check_Button(140, 50, 100, 20, "Points/Time");
		    o->value(1);
		    o->callback((Fl_Callback *)select_save, (void *)i);
		  }
		  { Fl_Int_Input *o = Save_Points[i] = new Fl_Int_Input(200, 75, 60, 20, "N Points: ");
		    o->align(FL_ALIGN_LEFT);
		    o->value("1000");
		  }
		  { Fl_Float_Input *o = Save_Time[i] = new Fl_Float_Input(200, 105, 60, 20, "Time [s]:  ");
		    o->align(FL_ALIGN_LEFT);
		    o->value("1.0");
		    o->deactivate();
		  }
		  { Fl_Input *o = Save_File[i] = new Fl_Input(200, 135, 100, 20, "Filename:");
		    char buf[100];
		    o->align(FL_ALIGN_LEFT);
		    sprintf(buf, "%s", Scopes[i].name);
		    o->value(buf);
		  }
		  { Fl_Light_Button *o = Save[i] = new Fl_Light_Button(140, 165, 90, 25, "Save");
		    o->selection_color(FL_BLACK);
		    o->callback((Fl_Callback *)enable_saving, (void *)i);
		  }
   		  {  Fl_Choice *o = Trigger_Mode[i] = new Fl_Choice(60, 200, 170, 25, "Trigger:");
                    o->add("Continuous Roling|Continuous Overwrite|Rising (-to+) CH1|Falling (+to-) CH1|Hold");
                    o->align(FL_ALIGN_LEFT);
                    o->value(0);
                    o->when(FL_WHEN_ENTER_KEY);
                    o->callback((Fl_Callback *)enter_trigger_mode, (void *)i);
                  }


		  Trace_Page[i] = new Fl_Group*[Scopes[i].ntraces];
		  Trace_Show[i] = new Fl_Check_Button*[Scopes[i].ntraces];
		  Units_Div[i] = new Fl_Input_Browser*[Scopes[i].ntraces];
		  Trace_Color[i] = new Fl_Button*[Scopes[i].ntraces];
		  Trace_Pos[i] = new Fl_Dial*[Scopes[i].ntraces];
  		  Trace_Width[i] = new Fl_Dial*[Scopes[i].ntraces];
		  Trace_Options[i] = new Fl_Menu_Button*[Scopes[i].ntraces];
		  

		  for (int j = 0; j < Scopes[i].ntraces; j++) {
			s_idx_T *idx = new s_idx_T;
			idx->scope_idx = i;
			idx->trace_idx = j;
		  	Trace_Page[i][j] = o->new_page(Scopes[i].traceName[j]);
			Trace_Page[i][j]->label_color(FL_WHITE);
			{ Fl_Check_Button *o = Trace_Show[i][j] = new Fl_Check_Button(10, 25, 100, 20, "Show/Hide");
			  o->value(1);
		    	  o->callback((Fl_Callback *)show_trace, (void *)idx);
		  	}
		  	{ Fl_Input_Browser *o = Units_Div[i][j] = new Fl_Input_Browser(77, 55, 60, 20, "Units/Div:  ");
		    	  o->align(FL_ALIGN_LEFT);
		    	  o->value("2.5");
			  o->add("0.001|0.002|0.005|0.01|0.02|0.05|0.1|0.2|0.5|1|2|5|10|50|100|1000");
		    	  o->when(FL_WHEN_ENTER_KEY);
		    	  o->callback((Fl_Callback *)enter_unitsdiv, (void *)idx);
		  	}
		  	{ Fl_Button *o = Trace_Color[i][j] = new Fl_Button(10, 90, 90, 25, "Trace Color");
		    	  o->callback((Fl_Callback *)select_trace_color, (void *)idx);
		  	}
			{ Fl_Dial *o = Trace_Pos[i][j] = new Fl_Dial(170, 40, 50, 50, "Trace Offset");
			  o->type(Fl_Dial::LINE);
			  o->minimum(0.0);
			  o->maximum(2.0);
			  o->value(1);
		    	  o->callback((Fl_Callback *)change_trace_pos, (void *)idx);
			}
			{ Fl_Dial *o = Trace_Width[i][j] = new Fl_Dial(250, 40, 50, 50, "Trace Width");
			  o->type(Fl_Dial::LINE);
			  o->minimum(0.1);
			  o->maximum(40.0);
			  o->value(0.1);
		    	  o->callback((Fl_Callback *)change_trace_width, (void *)idx);
			}
			{ Fl_Menu_Button *o = Trace_Options[i][j] = new Fl_Menu_Button(10, 130, 90, 25, "Options ");
			  int i;
			  o->menu(Trace_Opts);
			  o->when(FL_WHEN_ENTER_KEY);
		    	  o->callback((Fl_Callback *)enter_options, (void *)idx);
			 // for(i=0;i<o->children();i++) { // loop through all menu items, and add checked items to the value
	  		 //   o->child(i)->set_value(); 
			 // } 
		  	}
		  }
		  o->end();
		  Fl_Group::current()->resizable(w);
		}
	}
	for (int i = 1; i < Num_Scopes; i++) {
		Scopes_Tabs[i]->hide();
	}
	Scopes_Tabs[0]->show();
	Help = new Fl_Button(width-150, height-30, 70, 25, "Help");
	Close = new Fl_Button(width-75, height-30, 70, 25, "Close");
	Close->callback((Fl_Callback *)close);
	Fl_Browser *o = Scopes_Tree = new Fl_Browser(5, 5, 150, height-10);
	o->indented(1);
	o->callback((Fl_Callback *)select_scope);
	for (int i = 0; i < Num_Scopes; i++) {
		add_paper(Scopes_Tree, Scopes[i].name, Fl_Image::read_xpm(0, scope_icon));
	}

	w->view()->end();

	s->end();

	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);

	w->position(x, y);

	Fl::unlock();
}
