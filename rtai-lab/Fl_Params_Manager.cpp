/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2003  Roberto Bucher (roberto.bucher@supsi.ch)

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

#include <Fl_Params_Manager.h>

extern int Num_Tunable_Parameters;
extern int Num_Tunable_Blocks;
extern Target_Parameters_T *Tunable_Parameters;
extern Target_Blocks_T *Tunable_Blocks;
extern Fl_Tool_Button *RLG_Params_Mgr_Button;
extern Fl_Menu_Bar *RLG_Main_Menu;
extern Fl_Main_Window *RLG_Main_Window;
extern Fl_Menu_Item RLG_Main_Menu_Table[];

Param_Input_Block_T * Input_Blocks;

int Fl_Parameters_Manager::x()
{
	return PWin->x();
}

int Fl_Parameters_Manager::y()
{
	return PWin->y();
}

int Fl_Parameters_Manager::w()
{
	return PWin->w()- PWin->box()->dw();                          //see Fl_MDI_Window.cpp line 374 for more information;
}

int Fl_Parameters_Manager::h()
{
	return PWin->h()- PWin->titlebar()->h() - PWin->box()->dh();  //see Fl_MDI_Window.cpp line 374 for more information;
}

void Fl_Parameters_Manager::show()
{
        PWin->show();
}

void Fl_Parameters_Manager::hide()
{
        PWin->hide();
}

int Fl_Parameters_Manager::visible()
{
        return PWin->visible();
}

int Fl_Parameters_Manager::batch_download()
{
	return (int)(Batch_Download->value());
}

void Fl_Parameters_Manager::batch_download(bool v)
{
	if (v) {
		Batch_Download->set();
		Download->activate();
	} else {
		Batch_Download->clear();
		Download->deactivate();
	}
}

int Fl_Parameters_Manager::batch_counter()
{
	return b_counter;
}

void Fl_Parameters_Manager::batch_counter(int cnt)
{
	b_counter = cnt;
}

int Fl_Parameters_Manager::increment_batch_counter()
{
	return b_counter++;
}

int Fl_Parameters_Manager::update_parameter(int idx, int mat_idx, double val)
{
	Tunable_Parameters[idx].data_value[mat_idx] = val;
	return 1;
}

inline void Fl_Parameters_Manager::select_block_i(Fl_Browser *b, void *v)
{
	int n = b->value();
	for (int i = 0; i < Num_Tunable_Blocks; i++) {
		Parameters_Tabs[i]->hide();
	}
	Parameters_Tabs[n]->show();
}

void Fl_Parameters_Manager::select_block(Fl_Browser *b, void *v)
{
	((Fl_Parameters_Manager *)(b->parent()->parent()->user_data()))->select_block_i(b,v);
}

void Fl_Parameters_Manager::reload_params()
{
	int val_idx;
	Fl_Float_Input *o;
	unsigned int ncols, nrows;
	char scalar_val[20];

	for (int i = 0; i < Num_Tunable_Parameters; i++) {
		ncols = Tunable_Parameters[i].n_cols;
		nrows = Tunable_Parameters[i].n_rows;
		for (unsigned int nr = 0; nr < nrows; nr++) {
			for (unsigned int nc = 0; nc < ncols; nc++) {
				o = Input_Blocks[i].inputW[nr*ncols+nc];
				sprintf(scalar_val, "%G", get_parameter(Tunable_Parameters[i], nr, nc, &val_idx));
				o->value(strdup(scalar_val));
			}
		}
	}
}

inline void Fl_Parameters_Manager::batch_download_i(Fl_Check_Button *b, void *v)
{
	if (b->value()) {
		Download->activate();
	} else {
		Download->deactivate();
	}
	b_counter = 0;
}

void Fl_Parameters_Manager::batch_download_cb(Fl_Check_Button *b, void *v)
{
	((Fl_Parameters_Manager *)(b->parent()->parent()->user_data()))->batch_download_i(b,v);
}

inline void Fl_Parameters_Manager::close_i(Fl_Button *b, void *v)
{
	PWin->hide();
	RLG_Params_Mgr_Button->clear();
	RLG_Main_Menu_Table[9].clear();
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void Fl_Parameters_Manager::close(Fl_Button *b, void *v)
{
	((Fl_Parameters_Manager *)(b->parent()->parent()->user_data()))->close_i(b,v);
}

Fl_Parameters_Manager::Fl_Parameters_Manager(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name)
{
	Fl::lock();

	s->begin();
	Fl_MDI_Window *w = PWin = new Fl_MDI_Window(0, 0, width, height, name);
	w->user_data((void *)this);
	w->resizable(w->view());

	w->titlebar()->close_button()->hide();

	w->view()->begin();

	Parameters_Tabs = new Fl_Tabs*[Num_Tunable_Blocks];
	Input_Blocks = new Param_Input_Block_T [Num_Tunable_Parameters];

	for (int i = 0; i < Num_Tunable_Blocks; i++) {
		{ Fl_Tabs *o = Parameters_Tabs[i] = new Fl_Tabs(160, 5, width-165, height-70);
		  o->new_page("Block Parameters");
		  int tot_rows = 0;
		  if (i == Num_Tunable_Blocks - 1) {
		  	for (int j = 0; j < Num_Tunable_Parameters - Tunable_Blocks[i].offset; j++) {
				char scalar_val[20];
				char param_label[MAX_NAMES_SIZE + 10];
				int val_idx;
				unsigned int ncols = Tunable_Parameters[Tunable_Blocks[i].offset+j].n_cols;
				unsigned int nrows = Tunable_Parameters[Tunable_Blocks[i].offset+j].n_rows;
				Input_Blocks[Tunable_Blocks[i].offset+j].inputW = new Fl_Float_Input*[nrows*ncols];
				
				sprintf(param_label, "%s", Tunable_Parameters[Tunable_Blocks[i].offset+j].param_name);
				for (unsigned int nr = 0; nr < nrows; nr++) {
					for (unsigned int nc = 0; nc < ncols; nc++) {
		  				{ Fl_Float_Input *o = new Fl_Float_Input(10 + nc*110, 30 + (j+nr+tot_rows)*40, 100, 20, strdup(param_label));
						  param_label[0] = '\0';
			    	  		  o->align(FL_ALIGN_LEFT|FL_ALIGN_TOP);
				  		  o->when(FL_WHEN_ENTER_KEY);
						  sprintf(scalar_val, "%G", get_parameter(Tunable_Parameters[Tunable_Blocks[i].offset+j], nr, nc, &val_idx));
			    	  		  o->value(strdup(scalar_val));
						  p_idx_T *idx = new p_idx_T;
						  idx->block_idx = i;
						  idx->param_idx = j;
						  idx->val_idx = val_idx;
				  		  o->callback((Fl_Callback *)rlg_update_parameters_cb, (void *)idx);
						  Input_Blocks[Tunable_Blocks[i].offset+j].inputW[val_idx]=o;
						}
					}
				}
				tot_rows = tot_rows + nrows - 1;
			}
		  } else {
		  	for (int j = 0; j < Tunable_Blocks[i+1].offset-Tunable_Blocks[i].offset; j++) {
				char scalar_val[20];
				char param_label[MAX_NAMES_SIZE + 10];
				int val_idx;
				unsigned int ncols = Tunable_Parameters[Tunable_Blocks[i].offset+j].n_cols;
				unsigned int nrows = Tunable_Parameters[Tunable_Blocks[i].offset+j].n_rows;
				Input_Blocks[Tunable_Blocks[i].offset+j].inputW = new Fl_Float_Input*[nrows*ncols];
				sprintf(param_label, "%s", Tunable_Parameters[Tunable_Blocks[i].offset+j].param_name);
				for (unsigned int nr = 0; nr < nrows; nr++) {
					for (unsigned int nc = 0; nc < ncols; nc++) {
		  				{ Fl_Float_Input *o = new Fl_Float_Input(10 + nc*110, 30 + (j+nr+tot_rows)*40, 100, 20, strdup(param_label));
						  param_label[0] = '\0';
			    	  		  o->align(FL_ALIGN_LEFT|FL_ALIGN_TOP);
				  		  o->when(FL_WHEN_ENTER_KEY);
						  sprintf(scalar_val, "%G", get_parameter(Tunable_Parameters[Tunable_Blocks[i].offset+j], nr, nc, &val_idx));
			    	  		  o->value(strdup(scalar_val));
						  p_idx_T *idx = new p_idx_T;
						  idx->block_idx = i;
						  idx->param_idx = j;
						  idx->val_idx = val_idx;
						  o->callback((Fl_Callback *)rlg_update_parameters_cb, (void *)idx);
						  Input_Blocks[Tunable_Blocks[i].offset+j].inputW[val_idx]=o;
						}
					}
				}
				tot_rows = tot_rows + nrows - 1;
			}
		  }
		  o->end();
		  Fl_Group::current()->resizable(w);
		}
	}
	for (int i = 1; i < Num_Tunable_Blocks; i++) {
		Parameters_Tabs[i]->hide();
	}
	Parameters_Tabs[0]->show();

	Batch_Download = new Fl_Check_Button(width-270, height-60, 120, 25, "Batch Download");
	Batch_Download->callback((Fl_Callback *)batch_download_cb);
	Download = new Fl_Button(width-150, height-60, 70, 25, "Download");
	Download->callback((Fl_Callback *)rlg_batch_update_parameters_cb);
	Download->deactivate();
	Upload = new Fl_Button(width-75, height-60, 70, 25, "Upload"); 
	Upload->callback((Fl_Callback *)rlg_upload_parameters_cb);
//	Upload->deactivate();
	Help = new Fl_Button(width-150, height-30, 70, 25, "Help");
	Close = new Fl_Button(width-75, height-30, 70, 25, "Close");
	Close->callback((Fl_Callback *)close);

	Fl_Browser *o = Parameters_Tree = new Fl_Browser(5, 5, 150, height-10);
	o->indented(1);
	o->callback((Fl_Callback *)select_block);
	for (int i = 0; i < Num_Tunable_Blocks; i++) {
		add_paper(Parameters_Tree, Tunable_Blocks[i].name, Fl_Image::read_xpm(0, block_icon));
	}

	w->view()->end();

	s->end();

	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);
	w->position(x, y);

	Fl::unlock();
}
