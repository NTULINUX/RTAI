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

#include <efltk/Fl.h>
#include <Fl_Led_Window.h>
#include <efltk/Fl_Group.h>
#include <efltk/fl_draw.h>
#include <efltk/fl_math.h>
#include <stdlib.h>

int Fl_Led_Window::x()
{
	return LWin->x();
}

int Fl_Led_Window::y()
{
	return LWin->y();
}

int Fl_Led_Window::w()
{
	return LWin->w()- LWin->box()->dw();
}

int Fl_Led_Window::h()
{
	return LWin->h()- LWin->titlebar()->h() - LWin->box()->dh();
}

void Fl_Led_Window::resize(int x, int y, int w, int h)
{
	LWin->resize(x,y,w,h);
}

void Fl_Led_Window::show()
{
	LWin->show();
}

void Fl_Led_Window::hide()
{
	LWin->hide();
}

int Fl_Led_Window::is_visible()
{
	return LWin->visible();
}

void Fl_Led_Window::update()
{
	LWin->redraw();
}

void Fl_Led_Window::activate_led(int n)
{
	Leds[n]->color(color_index);
	Leds[n]->selection_color(color_index);
}

void Fl_Led_Window::deactivate_led(int n)
{
	Leds[n]->color(fl_darker(fl_darker(color_index)));
	Leds[n]->selection_color(fl_darker(fl_darker(color_index)));
}

void Fl_Led_Window::led_mask(unsigned int mask)
{
	Led_Mask = mask;
}

void Fl_Led_Window::led_on_off()
{
	for (int i = 0; i < num_leds; i++) {
		if (Led_Mask & (1 << i)) {
			activate_led(i);
		} else {
			deactivate_led(i);
		}
	}
}

void Fl_Led_Window::led_color(Fl_String col)
{
	const char *color = strdup(col.c_str());

	if (strcmp(color,"green") == 0) {
		color_index = FL_GREEN;
		return;
	} 
	if (strcmp(color,"red") == 0) {
		color_index = FL_RED;
		return;
	}
	if (strcmp(color,"yellow") == 0) {
		color_index = FL_YELLOW;
		return;
	}
}

void Fl_Led_Window::layout(int n_cols)
{
}

Fl_Led_Window::Fl_Led_Window(int x, int y, int width, int height, Fl_MDI_Viewport *s, const char *name, int n_leds)
{
	int N_Columns = 7;
	int N_Rows;
	int W, H;
	int xpos, ypos;

	num_leds = n_leds;
	color_index = FL_GREEN;
	Led_Mask = 0;

	if (num_leds <= N_Columns) {
		W = 10 + num_leds*(LED_WIDTH + 10) + 10;
		H = 5 + 10 + LED_HEIGHT + 20 + 5;
	} else {
		num_leds % N_Columns ? N_Rows = (int)(num_leds / N_Columns) + 1 : N_Rows = (int)(num_leds / N_Columns);
		W = 10 + N_Columns*(LED_WIDTH + 10) + 10;
		H = 5 + 10 + N_Rows*(LED_HEIGHT + 20) + 5;
	}

	Fl::lock();

	s->begin();

	Fl_MDI_Window *w = LWin = new Fl_MDI_Window(0, 0, W, H, name);

	w->view()->begin();

	Leds = new Fl_Led*[num_leds];
	{ Fl_Group *g = Led_Group = new Fl_Group(5, 5, W - 10, H - 10);
	  g->box(FL_ENGRAVED_BOX);
	  xpos = 0;
	  ypos = 0;
	  for (int i = 1; i <= num_leds; i++) {
		char led_label[10];
		sprintf(led_label, "%d\n", i);
		{ Fl_Led *o = Leds[i-1] = new Fl_Led(10 + (LED_WIDTH + 10)*xpos, 10 + (LED_HEIGHT + 20)*ypos, LED_WIDTH, LED_HEIGHT, strdup(led_label));
		  o->number(i-1);
		  o->color(fl_darker(fl_darker(color_index)));
		  o->selection_color(fl_darker(fl_darker(color_index)));
		  xpos++;
		  if (i >= N_Columns) {
		  	if ((N_Columns*(ypos + 1) % i) == 0) {
				xpos = 0;
				ypos++;
		  	}
		  }
		}
	  }  
	  g->end();
	}

	w->view()->end();

	s->end();

	w->titlebar()->h(15);
	w->titlebar()->color(FL_BLACK);
	w->position(x, y);

	Fl::unlock();
}
