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

#ifndef _FL_METERS_MANAGER_H_
#define _FL_METERS_MANAGER_H_

#include <efltk/Fl.h>
#include <efltk/Fl_Window.h>
#include <efltk/Fl_Button.h>
#include <efltk/Fl_Check_Button.h>
#include <efltk/Fl_Browser.h>
#include <efltk/Fl_Menu_Bar.h>
#include <efltk/Fl_Main_Window.h>
#include <efltk/Fl_MDI_Window.h>
#include <efltk/Fl_Widget.h>
#include <efltk/Fl_Tabs.h>
#include <efltk/Fl_Float_Input.h>
#include <efltk/Fl_Color_Chooser.h>
#include <efltk/Fl_Image.h>
#include <Fl_Meter_Window.h>
#include <icons/meter_icon.xpm>
#include <xrtailab.h>

class Fl_Meters_Manager
{
	public:
		Fl_Meters_Manager(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name);
		int x();
		int y();
		int w();
		int h();
		void show();
		void hide();
		int show_hide(int);
		void show_hide(int, bool);
		int visible();
		float minv(int);
		float maxv(int);
		void minv(int, float);
		void maxv(int, float);
		void b_color(int, RGB_Color_T);
		void a_color(int, RGB_Color_T);
		void g_color(int, RGB_Color_T);
		float b_color(int, int);
		float a_color(int, int);
		float g_color(int, int);
		int mw_x(int);
		int mw_y(int);
		int mw_w(int);
		int mw_h(int);
		Fl_Meter_Window **Meter_Windows;
	private:
		Fl_MDI_Window *MWin;
		Fl_Browser *Meters_Tree;
		Fl_Tabs **Meters_Tabs;
		Fl_Check_Button **Meter_Show;
		Fl_Float_Input **Min_Val;
		Fl_Float_Input **Max_Val;
		Fl_Button **Bg_Color;
		Fl_Button **Arrow_Color;
		Fl_Button **Grid_Color;
		Fl_Choice **Meter_Style;
		Fl_Menu_Button **Meter_Options;

		Fl_Button *Help, *Close;
		inline void select_meter_i(Fl_Browser *, void *);
		static void select_meter(Fl_Browser *, void *);
		inline void show_meter_i(Fl_Check_Button *, void *);
		static void show_meter(Fl_Check_Button *, void *);
		inline void enter_minval_i(Fl_Float_Input *, void *);
		static void enter_minval(Fl_Float_Input *, void *);
		inline void enter_maxval_i(Fl_Float_Input *, void *);
		static void enter_maxval(Fl_Float_Input *, void *);
		inline void enter_options_i(Fl_Menu_Button *, void *);
		static void enter_options(Fl_Menu_Button *, void *);
		inline void enter_meter_style_i(Fl_Choice *, void *);
		static void enter_meter_style(Fl_Choice *, void *);
		inline void select_bg_color_i(Fl_Button *, void *);
		static void select_bg_color(Fl_Button *, void *);
		inline void select_grid_color_i(Fl_Button *, void *);
		static void select_grid_color(Fl_Button *, void *);
		inline void select_arrow_color_i(Fl_Button *, void *);
		static void select_arrow_color(Fl_Button *, void *);
		inline void close_i(Fl_Button *, void *);
		static void close(Fl_Button *, void *);
};

#endif
