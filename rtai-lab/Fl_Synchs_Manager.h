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

#ifndef _FL_SYNCHS_MANAGER_H_
#define _FL_SYNCHS_MANAGER_H_

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
#include <Fl_Synch_Window.h>
#include <icons/synchronoscope_icon.xpm>
#include <xrtailab.h>

class Fl_Synchs_Manager
{
	public:
		Fl_Synchs_Manager(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name);
		int x();
		int y();
		int w();
		int h();
		void show();
		void hide();
		int show_hide(int);
		void show_hide(int, bool);
		int visible();
		int sw_x(int);
		int sw_y(int);
		int sw_w(int);
		int sw_h(int);
		Fl_Synch_Window **Synch_Windows;
	private:
		Fl_MDI_Window *SWin;
		Fl_Browser *Synchs_Tree;
		Fl_Tabs **Synchs_Tabs;
		Fl_Check_Button **Synch_Show;
		Fl_Button *Help, *Close;
		inline void select_synch_i(Fl_Browser *, void *);
		static void select_synch(Fl_Browser *, void *);
		inline void show_synch_i(Fl_Check_Button *, void *);
		static void show_synch(Fl_Check_Button *, void *);
		inline void close_i(Fl_Button *, void *);
		static void close(Fl_Button *, void *);
};

#endif
