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

#include <efltk/fl_math.h>
#include <efltk/Fl.h>
#include <Fl_Led.h>
#include <efltk/Fl_Group.h>
#include <efltk/fl_draw.h>
#include <stdlib.h>

void Fl_Led::draw()
{
	int X = 0, Y = 0, W = w(), H = h();
	if (!(type() == FILL && box() == FL_OVAL_BOX)) {
		if (damage()&FL_DAMAGE_ALL) draw_box();
        	box()->inset(X,Y,W,H);
	}
	Fl_Color fillcolor = selection_color();
	Fl_Color linecolor = highlight_color();
	if (!active_r()) {
        	fillcolor = fl_inactive(fillcolor);
        	linecolor = fl_inactive(linecolor);
	}
	float angle = (a2-a1)*float((value()-minimum())/(maximum()-minimum())) + a1;
       	if (damage()&FL_DAMAGE_EXPOSE && box() == FL_OVAL_BOX) {
       		fl_push_clip(0, 0, w(), h());
       		parent()->draw_group_box();
       		fl_pop_clip();
       	}
       	fl_color(color());
       	fl_pie(X, Y, W-1, H-1, float(270-a1), float(angle > a1 ? 360+270-angle : 270-360-angle));
       	fl_color(fillcolor);
       	fl_pie(X, Y, W-1, H-1, float(270-angle), float(270-a1));
       	if (box() == FL_OVAL_BOX) {
       	    fl_ellipse(X, Y, W-1, H-1);
       	    fl_color(linecolor); fl_stroke();
       	}
	if (focused() && focus_box()!=FL_NO_BOX) {
        	fl_ellipse(X+2, Y+2, W-5, H-5);
        	fl_color(linecolor);
        	fl_line_style(FL_DASH);
        	fl_stroke();
        	fl_line_style(0);
	}
}

static void revert(Fl_Style* s)
{
	s->focus_box = FL_NO_BOX;
	s->box = FL_ROUND_DOWN_BOX;
	s->selection_color = FL_DARK2;
	s->highlight_color = FL_BLACK;
}

static Fl_Named_Style style("Led", revert, &Fl_Led::default_style);
Fl_Named_Style* Fl_Led::default_style = &::style;

Fl_Led::Fl_Led(int x, int y, int w, int h, const char* l) : Fl_Valuator(x, y, w, h, l)
{
	style(default_style);
	a1 = 45;
	a2 = 315;
	index = 0;
}

Fl_Led::Fl_Led(const char* l,int layout_size,Fl_Align layout_al,int label_w) : Fl_Valuator(l,layout_size,layout_al,label_w)
{
	style(default_style);
	a1 = 45;
	a2 = 315;
	index = 0;
}
