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

#ifndef _FL_LEDS_MANAGER_H_
#define _FL_LEDS_MANAGER_H_

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
#include <efltk/Fl_String.h>
#include <efltk/Fl_Button_Group.h>
#include <efltk/Fl_Radio_Buttons.h>
#include <Fl_Led_Window.h>
#include <icons/led_icon.xpm>
#include <xrtailab.h>

class Fl_Leds_Manager
{
	public:
		Fl_Leds_Manager(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name);
		int x();
		int y();
		int w();
		int h();
		void show();
		int show_hide(int);
		void show_hide(int, bool);
		void hide();
		int visible();
		Fl_String color(int);
		void color(int, Fl_String);
		int lw_x(int);
		int lw_y(int);
		int lw_w(int);
		int lw_h(int);
		Fl_Led_Window **Led_Windows;
	private:
		Fl_MDI_Window *LWin;
		Fl_Browser *Leds_Tree;
		Fl_Tabs **Leds_Tabs;
		Fl_Check_Button **Led_Show;
		Fl_Radio_Buttons **Led_Colors;
		Fl_Button *Help, *Close;
		inline void select_led_i(Fl_Browser *, void *);
		static void select_led(Fl_Browser *, void *);
		inline void show_led_i(Fl_Check_Button *, void *);
		static void show_led(Fl_Check_Button *, void *);
		inline void select_led_color_i(Fl_Button_Group *, void *);
		static void select_led_color(Fl_Button_Group *, void *);
		inline void led_layout_i(Fl_Value_Input *, void *);
		static void led_layout(Fl_Value_Input *, void *);
		inline void close_i(Fl_Button *, void *);
		static void close(Fl_Button *, void *);
};

#endif
