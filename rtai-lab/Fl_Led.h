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

#ifndef _FL_LED_H_
#define _FL_LED_H_

#include <efltk/Fl.h>
#include <efltk/Fl_Valuator.h>

class FL_API Fl_Led : public Fl_Valuator
{
	public:
		static Fl_Named_Style* default_style;
		Fl_Led(int x,int y,int w,int h, const char *l = 0);
		Fl_Led(const char* l = 0,int layout_size=30,Fl_Align layout_al=FL_ALIGN_TOP,int label_w=100);
		enum {
			NORMAL = 0,
			LINE,
			FILL
		};
		void draw();
		void number(int idx) {index = idx;}
		int number() const {return index;}

	private:
		short a1,a2;
		int index;
};

#endif
