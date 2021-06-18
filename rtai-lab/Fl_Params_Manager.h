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

#ifndef _FL_PARAMS_MANAGER_H_
#define _FL_PARAMS_MANAGER_H_

#include <efltk/Fl.h>
#include <efltk/Fl_Window.h>
#include <efltk/Fl_Button.h>
#include <efltk/Fl_Check_Button.h>
#include <efltk/Fl_Input.h>
#include <efltk/Fl_Int_Input.h>
#include <efltk/Fl_Float_Input.h>
#include <efltk/Fl_Browser.h>
#include <efltk/Fl_Menu_Bar.h>
#include <efltk/Fl_Main_Window.h>
#include <efltk/Fl_MDI_Window.h>
#include <efltk/Fl_Widget.h>
#include <efltk/Fl_Tabs.h>
#include <efltk/Fl_Image.h>
#include <icons/block_icon.xpm>
#include <xrtailab.h>

class Fl_Parameters_Manager
{
	public:
		Fl_Parameters_Manager(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name);
		int x();
		int y();
		int w();
		int h();
		void show();
		void hide();
		int visible();
		int batch_download();
		void batch_download(bool);
		int batch_counter();
		void batch_counter(int);
		int increment_batch_counter();
		int update_parameter(int, int, double);
		void reload_params();
	private:
		Fl_MDI_Window *PWin;
		Fl_Browser *Parameters_Tree;
		Fl_Tabs **Parameters_Tabs;
		int b_counter;
		Fl_Check_Button *Batch_Download;
		Fl_Button *Download;
		Fl_Button *Upload;
		Fl_Button *Help, *Close;
		inline void select_block_i(Fl_Browser *, void *);
		static void select_block(Fl_Browser *, void *);
		inline void batch_download_i(Fl_Check_Button *, void *);
		static void batch_download_cb(Fl_Check_Button *, void *);
		inline void close_i(Fl_Button *, void *);
		static void close(Fl_Button *, void *);
};

#endif
