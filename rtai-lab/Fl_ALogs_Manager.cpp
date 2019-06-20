/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2006  Alberto Sechi (albertosechi@libero.it)	
COPYRIGHT (C) 2006  Roberto Bucher (roberto.bucher@supsi.ch)	

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

#include <Fl_ALogs_Manager.h>

extern int Num_ALogs;
extern Target_ALogs_T *ALogs;
extern Fl_Tool_Button *RLG_ALogs_Mgr_Button;
extern Fl_Menu_Bar *RLG_Main_Menu;
extern Fl_Main_Window *RLG_Main_Window;
extern Fl_Menu_Item RLG_Main_Menu_Table[];

int Fl_ALogs_Manager::x()
{
	return ALWin->x();
}

int Fl_ALogs_Manager::y()
{
	return ALWin->y();
}

int Fl_ALogs_Manager::w()
{
	return ALWin->w() - ALWin->box()->dw();
}

int Fl_ALogs_Manager::h()
{
	return ALWin->h() - ALWin->titlebar()->h() - ALWin->box()->dh();
}

void Fl_ALogs_Manager::show()
{
        ALWin->show();
}

void Fl_ALogs_Manager::hide()
{
        ALWin->hide();
}

int Fl_ALogs_Manager::visible()
{
        return ALWin->visible();
}

int Fl_ALogs_Manager::start_saving(int n)
{
	return Save_Flag[n];
}

/*int Fl_Logs_Manager::points_time(int n)
{
	return (int)(Save_Type[n]->value());
}

void Fl_Logs_Manager::points_time(int n, bool v)
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
	Logs_Tabs[n]->redraw();
}

int Fl_Logs_Manager::p_save(int n)
{
	int val = (int)atoi(Save_Points[n]->value());
	return val;
}

void Fl_Logs_Manager::p_save(int n, int val)
{
	char buf[20];

	if (val > 0) {
		sprintf(buf, "%d", val);
		Save_Points[n]->value(buf);
	}
}

float Fl_Logs_Manager::t_save(int n)
{
	float val = (float)atof(Save_Time[n]->value());
	return val;
}

void Fl_Logs_Manager::t_save(int n, float val)
{
	char buf[20];

	if (val > 0.) {
		sprintf(buf, "%.4f", val);
		Save_Time[n]->value(buf);
	}
}*/

const char *Fl_ALogs_Manager::file_name(int n)
{
	return (strdup(Save_File[n]->value()));
}

void Fl_ALogs_Manager::file_name(int n, const char *fn)
{
	Save_File[n]->value(strdup(fn));
	ALogs_Tabs[n]->redraw();
}

/*int Fl_ALogs_Manager::n_points_to_save(int n)
{
	int n_points;

	if (Save_Type[n]->value()) {
		n_points = (int)atoi(Save_Points[n]->value());
		if (n_points < 0) return 0;
	} else {
		n_points = (int)(atof(Save_Time[n]->value())/Logs[n].dt);
		if (n_points < 0) return 0;
	}
	return n_points;
}*/

void Fl_ALogs_Manager::stop_saving(int n)
{
	fclose(Save_File_Pointer[n]);
	/*Save_Type[n]->activate();
	if (Save_Type[n]->value()) {
		Save_Points[n]->activate();
	} else {
		Save_Time[n]->activate();
	}
	Save_File[n]->activate();
	Save[n]->activate();
	Save[n]->value(0);
	Save[n]->label("Save");*/
	ALogs_Tabs[n]->redraw();
	Save_Flag[n] = false;
}

FILE *Fl_ALogs_Manager::save_file(int n)
{
	return Save_File_Pointer[n];
}

inline void Fl_ALogs_Manager::select_log_i(Fl_Browser *b, void *v)
{
	int n = b->value();
	for (int i = 0; i < Num_ALogs; i++) {
		ALogs_Tabs[i]->hide();
	}
	ALogs_Tabs[n]->show();
}

void Fl_ALogs_Manager::select_log(Fl_Browser *b, void *v)
{
	((Fl_ALogs_Manager *)(b->parent()->parent()->user_data()))->select_log_i(b,v);
}

inline void Fl_ALogs_Manager::select_save_i(Fl_Check_Button *b, void *v)
{
        long n = (long)v;
	if (b->value()) {
		Save_Points[n]->activate();
		Save_Time[n]->deactivate();
	} else {
		Save_Points[n]->deactivate();
		Save_Time[n]->activate();
	}
	ALogs_Tabs[n]->redraw();
}

void Fl_ALogs_Manager::select_save(Fl_Check_Button *b, void *v)
{
	((Fl_ALogs_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->select_save_i(b,v);
}

inline void Fl_ALogs_Manager::enable_saving_i(Fl_Light_Button *b, void *v)
{
	long n = (long)v;
	if (b->value()) {
		if ((Save_File_Pointer[n] = fopen(Save_File[n]->value(), "a+")) == NULL) {
			fl_alert("Error in opening file %s", Save_File[n]->value());
			return;
		}
		b->deactivate();
		/*Save_Type[n]->deactivate();
		Save_Time[n]->deactivate();
		Save_Points[n]->deactivate();
		Save_File[n]->deactivate();
		Save[n]->label("Saving...");*/
		ALogs_Tabs[n]->redraw();
		Save_Flag[n] = true;
	}
}

void Fl_ALogs_Manager::enable_saving(Fl_Light_Button *b, void *v)
{
	((Fl_ALogs_Manager *)(b->parent()->parent()->parent()->parent()->user_data()))->enable_saving_i(b,v);
}

inline void Fl_ALogs_Manager::close_i(Fl_Button *b, void *v)
{
	ALWin->hide();
	RLG_ALogs_Mgr_Button->clear();
	RLG_Main_Menu_Table[12].clear();
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void Fl_ALogs_Manager::close(Fl_Button *b, void *v)
{
	((Fl_ALogs_Manager *)(b->parent()->parent()->user_data()))->close_i(b,v);
}

Fl_ALogs_Manager::Fl_ALogs_Manager(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name)
{
	Fl::lock();

	s->begin();
	Fl_MDI_Window *w = ALWin = new Fl_MDI_Window(0, 0, width, height, name);
	w->user_data((void *)this);
	w->resizable(w->view());

	w->titlebar()->close_button()->hide();

	w->view()->begin();

	ALogs_Tabs = new Fl_Tabs*[Num_ALogs];
	/*Save_Type = new Fl_Check_Button*[Num_Logs];
	Save_Points = new Fl_Int_Input*[Num_Logs];
	Save_Time = new Fl_Float_Input*[Num_Logs];*/
	Save_File = new Fl_Input*[Num_ALogs];
	Save = new Fl_Light_Button*[Num_ALogs];
	Save_Flag = new int[Num_ALogs];
	Save_File_Pointer = new FILE*[Num_ALogs];

	for (int i = 0; i < Num_ALogs; i++) {
		Save_Flag[i] = false;
		{ Fl_Tabs *o = ALogs_Tabs[i] = new Fl_Tabs(160, 5, width-165, height-40);
		  o->new_page("Saving");
		  /*{ Fl_Check_Button *o = Save_Type[i] = new Fl_Check_Button(10, 25, 100, 20, "Points/Time");
		    o->value(1);
		    o->callback((Fl_Callback *)select_save, (void *)i);
		  }
		  { Fl_Int_Input *o = Save_Points[i] = new Fl_Int_Input(70, 50, 60, 20, "N Points: ");
		    o->align(FL_ALIGN_LEFT);
		    o->value("1000");
		  }
		  { Fl_Float_Input *o = Save_Time[i] = new Fl_Float_Input(70, 75, 60, 20, "Time [s]:  ");
		    o->align(FL_ALIGN_LEFT);
		    o->value("1.0");
		    o->deactivate();
		  }*/
		  { Fl_Input *o = Save_File[i] = new Fl_Input(70, 100, 0, 0, " ");
		    char buf[100];
		    o->align(FL_ALIGN_LEFT);
		    sprintf(buf, "%s", ALogs[i].name);
		    o->value(buf);
		  }
		  /*{ Fl_Light_Button *o = Save[i] = new Fl_Light_Button(10, 130, 90, 25, "Save");
		    o->selection_color(FL_BLACK);
		    o->callback((Fl_Callback *)enable_saving, (void *)i);
		  }*/
		  o->end();
		  Fl_Group::current()->resizable(w);
		}
	}
	for (int i = 1; i < Num_ALogs; i++) {
		ALogs_Tabs[i]->hide();
	}
	ALogs_Tabs[0]->show();

	Help = new Fl_Button(width-150, height-30, 70, 25, "Help");
	Close = new Fl_Button(width-75, height-30, 70, 25, "Close");
	Close->callback((Fl_Callback *)close);

	Fl_Browser *o = ALogs_Tree = new Fl_Browser(5, 5, 150, height-10);
	o->indented(1);
	o->callback((Fl_Callback *)select_log);
	for (int i = 0; i < Num_ALogs; i++) {
		add_paper(ALogs_Tree, ALogs[i].name, Fl_Image::read_xpm(0, folder_asmall));
	}

	w->view()->end();

	s->end();

	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);
	w->position(x, y);

	Fl::unlock();
}
