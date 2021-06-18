/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2003  RobertoBucher (roberto.bucher@supsi.ch)
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

#ifndef _FL_SCOPES_MANAGER_H_
#define _FL_SCOPES_MANAGER_H_

#include <efltk/Fl.h>
#include <efltk/Fl_Window.h>
#include <efltk/Fl_Button.h>
#include <efltk/Fl_Light_Button.h>
#include <efltk/Fl_Check_Button.h>
#include <efltk/Fl_Input.h>
#include <efltk/Fl_Int_Input.h>
#include <efltk/Fl_Float_Input.h>
#include <efltk/Fl_Browser.h>
#include <efltk/Fl_Input_Browser.h>
#include <efltk/Fl_Dial.h>
#include <efltk/Fl_Menu_Bar.h>
#include <efltk/Fl_Main_Window.h>
#include <efltk/Fl_MDI_Window.h>
#include <efltk/Fl_Widget.h>
#include <efltk/Fl_Tabs.h>
#include <efltk/Fl_Image.h>
#include <efltk/Fl_Color_Chooser.h>
#include <efltk/fl_ask.h>
#include <stdlib.h>
#include <Fl_Scope_Window.h>
#include <icons/scope_icon.xpm>
#include <xrtailab.h>

class Fl_Scopes_Manager
{
	public:
		Fl_Scopes_Manager(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name);
		int x();
		int y();
		int w();
		int h();
		void show();
		void hide();
		int show_hide(int);
		void show_hide(int, bool);
		int trace_show_hide(int, int);
		void trace_show_hide(int, int, bool);
		int grid_on_off(int);
		void grid_on_off(int, bool);
		int points_time(int);
		void points_time(int, bool);
		int sw_x(int);
		int sw_y(int);
		int sw_w(int);
		int sw_h(int);
		void b_color(int, RGB_Color_T);
		void g_color(int, RGB_Color_T);
		void t_color(int, int, RGB_Color_T);
		float b_color(int, int);
		float g_color(int, int);
		float t_color(int, int, int);
		int visible();
		float sec_div(int);
		void sec_div(int, float);
		float trace_unit_div(int, int);
		void trace_unit_div(int, int, float);
		float trace_offset(int, int);
		void trace_offset(int, int, float);
		float trace_width(int, int);
		void trace_width(int, int, float);
		void trace_flags(int, int, int);
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
		Fl_Scope_Window **Scope_Windows;
	private:
		Fl_MDI_Window *SWin;
		Fl_Browser *Scopes_Tree;
		Fl_Tabs **Scopes_Tabs;
		Fl_Check_Button **Scope_Show;
		Fl_Button **Scope_Pause;
		Fl_Check_Button **Scope_OneShot;
		Fl_Menu_Button **Scope_Options;
		Fl_Button **Grid_Color, **Bg_Color;
		Fl_Input_Browser **Sec_Div;
		Fl_Check_Button **Save_Type;
		Fl_Int_Input **Save_Points;
		Fl_Float_Input **Save_Time;
		Fl_Input **Save_File;
		Fl_Light_Button **Save;
		int *Save_Flag;
		FILE **Save_File_Pointer;
		Fl_Choice **Trigger_Mode;
		Fl_Group ***Trace_Page;
		Fl_Check_Button ***Trace_Show;
		Fl_Input_Browser ***Units_Div;
		Fl_Button ***Trace_Color;
		Fl_Dial ***Trace_Pos;
		Fl_Dial ***Trace_Width;
		Fl_Menu_Button ***Trace_Options;

		Fl_Button *Help, *Close;
		inline void select_scope_i(Fl_Browser *, void *);
		static void select_scope(Fl_Browser *, void *);
		inline void show_scope_i(Fl_Check_Button *, void *);
		static void show_scope(Fl_Check_Button *, void *);
		inline void pause_scope_i(Fl_Button *, void *);
		static void pause_scope(Fl_Button *, void *);
		inline void oneshot_scope_i(Fl_Check_Button *, void *);
		static void oneshot_scope(Fl_Check_Button *, void *);
		inline void select_grid_color_i(Fl_Button *, void *);
		static void select_grid_color(Fl_Button *, void *);
		inline void select_bg_color_i(Fl_Button *, void *);
		static void select_bg_color(Fl_Button *, void *);
		inline void enter_secdiv_i(Fl_Input_Browser *, void *);
		static void enter_secdiv(Fl_Input_Browser *, void *);
		inline void select_save_i(Fl_Check_Button *, void *);
		static void select_save(Fl_Check_Button *, void *);
		inline void enable_saving_i(Fl_Light_Button *, void *);
		static void enable_saving(Fl_Light_Button *, void *);
		inline void show_trace_i(Fl_Check_Button *, void *);
		static void show_trace(Fl_Check_Button *, void *);
		inline void enter_unitsdiv_i(Fl_Input_Browser *, void *);
		static void enter_unitsdiv(Fl_Input_Browser *, void *);
		inline void enter_trigger_mode_i(Fl_Choice *, void *);
		static void enter_trigger_mode(Fl_Choice *, void *);
		inline void select_trace_color_i(Fl_Button *, void *);
		inline void enter_options_i(Fl_Menu_Button *, void *);
		static void enter_options(Fl_Menu_Button *, void *);
		static void select_trace_color(Fl_Button *, void *);
		inline void change_trace_pos_i(Fl_Dial *, void *);
		static void change_trace_pos(Fl_Dial *, void *);
		inline void change_trace_width_i(Fl_Dial *, void *);
		static void change_trace_width(Fl_Dial *, void *);
		inline void close_i(Fl_Button *, void *);
		static void close(Fl_Button *, void *);
};

#endif
