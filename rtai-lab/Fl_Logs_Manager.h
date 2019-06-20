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

#ifndef _FL_LOGS_MANAGER_H_
#define _FL_LOGS_MANAGER_H_

#include <efltk/Fl.h>
#include <efltk/Fl_Window.h>
#include <efltk/Fl_Button.h>
#include <efltk/Fl_Light_Button.h>
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
#include <efltk/fl_ask.h>
#include <stdlib.h>
#include <icons/folder_small.xpm>
#include <xrtailab.h>

class Fl_Logs_Manager
{
	public:
		Fl_Logs_Manager(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name);
		int x();
		int y();
		int w();
		int h();
		void show();
		void hide();
		int visible();
		int points_time(int);
		void points_time(int, bool);
		int p_save(int);
		void p_save(int, int);
		float t_save(int);
		void t_save(int, float);
		const char *file_name(int);
		void file_name(int, const char *);
		int n_points_to_save(int);
		int start_saving(int);
		void stop_saving(int);
		FILE *save_file(int);
	private:
		Fl_MDI_Window *LWin;
		Fl_Browser *Logs_Tree;
		Fl_Tabs **Logs_Tabs;
		Fl_Check_Button **Save_Type;
		Fl_Int_Input **Save_Points;
		Fl_Float_Input **Save_Time;
		Fl_Input **Save_File;
		Fl_Light_Button **Save;
		int *Save_Flag;
		FILE **Save_File_Pointer;
		Fl_Button *Help, *Close;
		inline void select_log_i(Fl_Browser *, void *);
		static void select_log(Fl_Browser *, void *);
		inline void select_save_i(Fl_Check_Button *, void *);
		static void select_save(Fl_Check_Button *, void *);
		inline void enable_saving_i(Fl_Light_Button *, void *);
		static void enable_saving(Fl_Light_Button *, void *);
		inline void close_i(Fl_Button *, void *);
		static void close(Fl_Button *, void *);
};

#endif
