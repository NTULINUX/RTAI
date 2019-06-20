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

#ifndef _FL_LED_WINDOW_H_
#define _FL_LED_WINDOW_H_

#include <Fl_Led.h>
#include <efltk/Fl_Group.h>
#include <efltk/Fl_MDI_Window.h>

#define LED_WIDTH       20
#define LED_HEIGHT      20

class Fl_Led_Window
{
        public:
		Fl_Led_Window(int x, int y, int w, int h, Fl_MDI_Viewport *s, const char *name, int n_leds);
		Fl_Led **Leds;
		Fl_Group *Led_Group;
		int x();
		int y();
		int w();
		int h();
		void resize(int, int, int, int);
		void show();
		void hide();
		void update();
		int is_visible();
		void layout(int);
		void activate_led(int);
		void deactivate_led(int);
		void led_color(Fl_String);
		void led_mask(unsigned int);
		void led_on_off();
	private:
		Fl_MDI_Window *LWin;
		int num_leds;
		int color_index;
		unsigned int Led_Mask;
};

#endif
