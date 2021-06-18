/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2003  Paolo Mantegazza (mantegazza@aero.polimi.it)
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

#include <Fl_Scope.h>
#include <math.h>

void Fl_Scope::show_trace(int n, int value)
{
	if (n < num_of_traces) {
		value ? Trace_Visible[n] = true : Trace_Visible[n] = false;
	}
}

void Fl_Scope::pause(int value)
{
	value ? Pause_Flag = true : Pause_Flag = false;
}

int Fl_Scope::pause()
{
	return Pause_Flag;
}

void Fl_Scope::oneshot(int value)
{
	value ? OneShot_Flag = true : OneShot_Flag = false;
}

int Fl_Scope::oneshot()
{
	return OneShot_Flag;
}

void Fl_Scope::grid_visible(int value)
{
	Scope_Flags &= ~sfDrawGrid;
	if ( value ) Scope_Flags |= sfDrawGrid;
}

void Fl_Scope::grid_color(float r, float g, float b)
{
	Grid_rgb[0] = r;
	Grid_rgb[1] = g;
	Grid_rgb[2] = b;
}

Fl_Color Fl_Scope::grid_color()
{
	return Grid_Color;
}

void Fl_Scope::grid_free_color()
{
	Grid_Color = FL_FREE_COLOR;
}

float Fl_Scope::grid_r()
{
	return Grid_rgb[0];
}

float Fl_Scope::grid_g()
{
	return Grid_rgb[1];
}

float Fl_Scope::grid_b()
{
	return Grid_rgb[2];
}

void Fl_Scope::bg_color(float r, float g, float b)
{
	Bg_rgb[0] = r;
	Bg_rgb[1] = g;
	Bg_rgb[2] = b;
}

Fl_Color Fl_Scope::bg_color()
{
	return Bg_Color;
}

void Fl_Scope::bg_free_color()
{
	Bg_Color = FL_FREE_COLOR;
}

float Fl_Scope::bg_r()
{
	return Bg_rgb[0];
}

float Fl_Scope::bg_g()
{
	return Bg_rgb[1];
}

float Fl_Scope::bg_b()
{
	return Bg_rgb[2];
}

void Fl_Scope::trace_color(int n, float r, float g, float b)
{
	if (n < num_of_traces) {
		Trace_rgb[n][0] = r;
		Trace_rgb[n][1] = g;
		Trace_rgb[n][2] = b;
	}
}

Fl_Color Fl_Scope::trace_color(int n)
{
	if (n < num_of_traces) {
		return Trace_Color[n];
	} else return 0;
}

void Fl_Scope::trace_free_color(int n)
{
	if (n < num_of_traces) {
		Trace_Color[n] = FL_FREE_COLOR;
	}
}

float Fl_Scope::tr_r(int n)
{
	return Trace_rgb[n][0];
}

float Fl_Scope::tr_g(int n)
{
	return Trace_rgb[n][1];
}

float Fl_Scope::tr_b(int n)
{
	return Trace_rgb[n][2];
}

void Fl_Scope::sampling_frequency(float freq)
{
	Sampling_Frequency = freq;
}

float Fl_Scope::sampling_frequency()
{
	return Sampling_Frequency;
}

void Fl_Scope::trace_offset(int n, float value)
{
	if (n < num_of_traces) {
		Trace_Offset_Value[n] = value;
		Trace_Offset[n] = (h()/2.)*value;
	}
}

void Fl_Scope::trace_width(int n, float value)
{
	if (n < num_of_traces) {
		Trace_Width[n] = value;
	}
}

void Fl_Scope::trace_flags(int n, int value)
{
	if (n>=0 && n < num_of_traces ) {
		Trace_Flags[n] = value;
	}
}
int Fl_Scope::trace_flags(int n)
{
	if (n>=0 && n < num_of_traces ) {
		return Trace_Flags[n];
	}
}

void Fl_Scope::scope_flags(int value)
{
  Scope_Flags = value;
}
int Fl_Scope::scope_flags(void)
{
  return Scope_Flags;
}


void Fl_Scope::time_range(float range)
{
	Time_Range = range;
}

float Fl_Scope::time_range()
{
	return Time_Range;
}

void Fl_Scope::trigger_mode(int m)
{
       Trigger_Mode = m;
}
int Fl_Scope::trigger_mode(void)
{
	return Trigger_Mode;
}



void Fl_Scope::y_range_inf(int n, float range)
{
	if (n < num_of_traces) {
		Y_Range_Inf[n] = range;
	}
}

void Fl_Scope::y_range_sup(int n, float range)
{
	if (n < num_of_traces) {
		Y_Range_Sup[n] = range;
	}
}

void Fl_Scope::setdx()
{
	dx = w()/(Sampling_Frequency*Time_Range);
}

float Fl_Scope::getdx()
{
	return dx;
}

void Fl_Scope::trace_length(int width)
{
	Trace_Len = (int)(width/dx);
}

int Fl_Scope::trace_length()
{
	return Trace_Len;
}

void Fl_Scope::trace_pointer(int pos)
{
	Trace_Ptr = pos;
}

int Fl_Scope::trace_pointer()
{
	return Trace_Ptr;
}

int Fl_Scope::increment_trace_pointer()
{
	return Trace_Ptr++;
}

void Fl_Scope::write_to_trace(int n, float val)
{
	if (Write_Ptr[n] < 0 || Write_Ptr[n]>Trace_Len) Write_Ptr[n] = Trace_Len;	
	Trace[n][Write_Ptr[n]--] = val;
}


void Fl_Scope::add_to_trace(int n, float val)
{
  // if AC coupled, subtract the DC part from the signal, DC part has a fixed tau
  if ( Trace_Flags[n] & tfAC ) {
    DC_Val[n] = 0.9999 * DC_Val[n] + 0.0001*val;
    val -= DC_Val[n];
  }

  if ( n == 0 ) { // trigger logic is only evaluated for first channel
    if ( (Data_Ptr[0] >= 0) && (Data_Ptr[0] < Trace_Len) ) {  // Aquiring
      Data_Ptr[0]--;
    }
    else { // Waiting for trigger 
      switch ( Trigger_Mode) {
      case tmRoll: 
	Trigger = 1;
        break;
      case tmOverwrite: 
        Trigger = 1;
        break;
      case tmTriggerCh1Pos: 
        Trigger = (Prev_Val <= 0) && (val > 0 );
        Prev_Val = val;
        break;
      case tmTriggerCh1Neg: 
        Trigger = (Prev_Val >= 0) && (val < 0 );
        Prev_Val = val;
        break;
      case tmHold:
        Trigger = 0;
        break;
      default:
        Trigger =  !Pause_Flag;
        break;
      } // case
      if ( (Trigger && !OneShot_Flag) || // Not a oneshot, just start again when triggered
	   (Trigger && OneShot_Flag && !Pause_Flag) ) // oneshot: continue if manually retriggered
        Data_Ptr[0] = Trace_Len-1; 
    } // if
  } // trigger logic


  // Check index and Add data to the trace, if mode == 0 (roll) move all elements in array:
  if ( Trigger_Mode == tmRoll ) {
    memmove(Trace[n] + 1, Trace[n], (Trace_Len - 1)*sizeof(float));
    Trace[n][0] = val;
  } else {
    if ((Data_Ptr[0] >= 0) && (Data_Ptr[0] < Trace_Len && Trigger_Mode != tmHold)) Trace[n][Data_Ptr[0]] = val;
  }
		  
} // add_to_trace

void Fl_Scope::add_to_trace(int pos, int n, float val)
{
	Trace[n][pos] = val;
}


void Fl_Scope::initgl()
{
	glViewport(0, 0, w(), h());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w(), 0, h(), -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glClearColor(Bg_rgb[0], Bg_rgb[1], Bg_rgb[2], 0.0);
	glClear(GL_COLOR_BUFFER_BIT);	
}

void Fl_Scope::drawticks()
{
	float lx = (w()/NDIV_GRID_X)/10., ly = (h()/NDIV_GRID_Y)/10.;
	float x = 0., y = 0.;

	glLineWidth(0.1);
	glColor3f(Grid_rgb[0], Grid_rgb[1], Grid_rgb[2]);
	glBegin(GL_LINES);
	for (;;) {
		y += dyGrid;
		if (y >= h()) break;
		glVertex2f(0., y);
		glVertex2f(lx, y);
		glVertex2f(w()-lx, y);
		glVertex2f(w(), y);
	}
	for (;;) {
		x += dxGrid;
		if (x >= w()) break;
		glVertex2f(x, 0.);
		glVertex2f(x, ly);
		glVertex2f(x, h()-ly);
		glVertex2f(x, h());
	}
	glEnd();
}

void Fl_Scope::drawgrid()
{
	float x = 0., y = 0.;

	glLineWidth(0.1);
	glLineStipple(1, 0xAAAA);
	glEnable(GL_LINE_STIPPLE);
	glColor3f(Grid_rgb[0], Grid_rgb[1], Grid_rgb[2]);
	glBegin(GL_LINES);
	for (;;) {
		y += dyGrid;
		if (y >= h()) break;
		glVertex2f(0., y);
		glVertex2f(w(), y);
	}
	for (;;) {
		x += dxGrid;
		if (x >= w()) break;
		glVertex2f(x, 0.);
		glVertex2f(x, h());
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

/* Draw the statistics and legend on top of the graph */
void Fl_Scope::drawstats()
{
  char str[200];
  int i,j;
  float xo,yo,yh,ydiv;
  float t = (Time_Range / w()), ystep;
  trStat s;

 
  glDisable(GL_DEPTH_TEST);
  yh = 2 + (int)w()/35.0; if (yh>15.0) yh=15.0;
  gl_font(FL_SCREEN, yh-0.8);


  for (int i = j = 0; (i < num_of_traces) && (j < 4); i++) {

    if (Trace_Flags[i] & tfStats) {
      s.sum=0; s.sum2=0; s.min=1E99; s.max=-1E99;
      for (int n = Trace_Len - 1; n >= 0; n--) {
        s.min = (Trace[i][n] < s.min) ? Trace[i][n] : s.min;
        s.max = (Trace[i][n] > s.max) ? Trace[i][n] : s.max;
        s.sum += Trace[i][n];
      }

      s.n = Trace_Len;
      s.avg = s.sum / s.n;
      s.pkpk = s.max - s.min;
      for (int n = Trace_Len - 1; n >= 0; n--) 
        s.sum2 += (Trace[i][n]-s.avg)*(Trace[i][n]-s.avg);
      if (s.sum2 != 0.0) s.rms = sqrt(s.sum2/s.n); else s.rms = 0.0;
     
      s.t1 = t*cursors[0].x;  
      s.t2 = t*cursors[1].x;  
      ydiv = (Y_Range_Sup[i]-Y_Range_Inf[i])/NDIV_GRID_Y;
      ystep = (Y_Range_Inf[i]-Y_Range_Sup[i])/h(); 
      s.y1 = ystep*(cursors[0].y-(h()-Trace_Offset[i]));
      s.y2 = ystep*(cursors[1].y-(h()-Trace_Offset[i]));
      s.dy = s.y2-s.y1;
      s.dt = s.t2 - s.t1;
      if ( s.dt != 0 ) s.dydt = s.dy / s.dt; else s.dydt=0;

      j++; // If there is something to show, increment column and calculate start position
      xo = 10 + (j-1)*(w()/3.0); 
      yo = h()-yh;

      gl_color(FL_GRAY);
      if ( Trace_Flags[i] & tfDrawLabel) { 
        sprintf(str, " Trace %d", i+1); gl_draw(str, xo,yo); yo -= yh; 
        glLineWidth(Trace_Width[i]);
        glColor3f(Trace_rgb[i][0], Trace_rgb[i][1], Trace_rgb[i][2]);
        glBegin(GL_LINE_STRIP);
        glVertex2f(xo+5*yh, h()-0.7*yh);
        glVertex2f(xo+8*yh, h()-0.7*yh);
        glEnd();
      }
      gl_color(FL_GRAY);
      if ( Trace_Flags[i] & tfStatUd ) { sprintf(str, "U/d: %10.4f",ydiv); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatMin) { sprintf(str, "Min: %10.4f",s.min); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatMax) { sprintf(str, "Max: %10.4f",s.max); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatPpk) { sprintf(str, "Ppk: %10.4f",s.pkpk); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatAvg) { sprintf(str, "Avg: %10.4f",s.avg); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatRms) { sprintf(str, "RMS: %10.4f",s.rms); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatY1 ) { sprintf(str, " y1: %10.4f",s.y1); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatY2 ) { sprintf(str, " y2: %10.4f",s.y2); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatDy ) { sprintf(str, " dy: %10.4f",s.dy); gl_draw(str, xo,yo); yo -= yh; }
      if ( Trace_Flags[i] & tfStatDyDt ) { sprintf(str, "y/t: %10.4f",s.dydt); gl_draw(str, xo,yo); yo -= yh; }
    } // Anything to display?
  } // loop
  glEnable(GL_DEPTH_TEST);
}

void Fl_Scope::drawhline(float y, float c[], float lw, GLushort p)
{
	  glLineWidth(lw);
	  glLineStipple(1, p);
	  glEnable(GL_LINE_STIPPLE);
  	  glBegin(GL_LINES);
	  glColor3f(c[0], c[1], c[2]);
	  glVertex2f(0., y);
	  glVertex2f(w(),y);
          glEnd();
	  glDisable(GL_LINE_STIPPLE);
}

void Fl_Scope::drawvline(float x, float c[], float lw, GLushort p)
{
	  glLineWidth(lw);
	  glLineStipple(1, p);
	  glEnable(GL_LINE_STIPPLE);
  	  glBegin(GL_LINES);
	  glColor3f(c[0], c[1], c[2]);
	  glVertex2f(x, 0);
	  glVertex2f(x, h());
          glEnd();
	  glDisable(GL_LINE_STIPPLE);
}

void Fl_Scope::draw()
{
	char secdiv[100], buf[100], *tmode;
	int triggered = 0;

	if (!valid()) {
		initgl();
		dxGrid = w()/(float)((int)(w()/(w()/((float)NDIV_GRID_X))));
		dyGrid = (h()/2.)/(float)((int)((h()/2.)/(h()/((float)NDIV_GRID_Y))));
		for (int n = 0; n < num_of_traces; n++) {
			Trace_Offset[n] = (h()/2.)*Trace_Offset_Value[n];
		}
	}

	dx = w()/(Sampling_Frequency*Time_Range);
	Trace_Len = ceil((w()/dx));
	glClearColor(Bg_rgb[0], Bg_rgb[1], Bg_rgb[2], 0.0);
	glClear(GL_COLOR_BUFFER_BIT);




	if (Scope_Flags & sfDrawTics) drawticks();
	if (Scope_Flags & sfDrawGrid) drawgrid();
	
// Draw all trace lines
	for (int nn = 0; nn < num_of_traces; nn++) {
	        glLineWidth(Trace_Width[nn]);
		glColor3f(Trace_rgb[nn][0], Trace_rgb[nn][1], Trace_rgb[nn][2]);
		if (Trace_Visible[nn]) {
			glBegin(GL_LINE_STRIP);
			rtPoint p;
			p.x = p.y = 0;
			for (int n = Trace_Len - 1; n >= 0; n--) {
				p.y = Trace[nn][n]*(((float)(h()))/(float)(Y_Range_Sup[nn]-Y_Range_Inf[nn])) + Trace_Offset[nn];
				glVertex2f(p.x, p.y);
				p.x += dx;
			}
			glEnd();
		}
	}

// Draw zero line for all traces, include trace identifier
  
	glLineWidth(0.1);
	glLineStipple(1, 0x1A1A);
	glEnable(GL_LINE_STIPPLE);
  	for (int nn = 0; nn < num_of_traces; nn++) {
	  if ( Trace_Flags[nn] & tfDrawZero ) {
	    glBegin(GL_LINES);
	    glColor3f(Trace_rgb[nn][0], Trace_rgb[nn][1], Trace_rgb[nn][2]);
	    glVertex2f(0., Trace_Offset[nn]);
	    glVertex2f(w(), Trace_Offset[nn]);
	    glEnd();
	    sprintf(secdiv, "%d%s", nn+1, (Trace_Flags[nn] & tfAC) ? " AC" : "" );
	    gl_draw(secdiv, 4.0, 4.0+Trace_Offset[nn]);
	  }
	}
	glDisable(GL_LINE_STIPPLE);


/** draw bottom line + cursors */
	{
	  float c[] = {0.8,0.8,0.8};
	  float t = (Time_Range / w());
	  float dt =  t * (cursors[1].x - cursors[0].x);
	  float idt = (dt!=0.0) ? 1/dt : 0;
 	  triggered = (Data_Ptr[0] >= 0) && (Data_Ptr[0] < Trace_Len);
          if (Trigger_Mode == tmRoll) tmode="roll";
          else if (Trigger_Mode == tmHold) tmode="hold";
          else if (triggered) tmode = "trig";
          else tmode = "wait";
  	  gl_color(FL_GRAY);
	  glDisable(GL_DEPTH_TEST);
	  sprintf(secdiv,"x:%.5Gs/div %s ", Time_Range/NDIV_GRID_X, tmode);
          sprintf(buf, "t1:%.4Gsec t2:%.4Gsec dt:%.4Gsec 1/dt:%.4GHz", t*cursors[0].x, t*cursors[1].x,  dt, idt);
          if ( Scope_Flags & sfCursors )  strcat(secdiv, buf);

	  gl_draw(secdiv, 10.0f, 5.0f);
	  if ( Scope_Flags & sfCursors ) {
	    if ( Scope_Flags & sfHorBar ) { 
	      drawhline(h()-cursors[0].y, c, 2., 0xAAAA);
	      drawhline(h()-cursors[1].y, c, 2, 0xAAAA);
            }
            if ( Scope_Flags & sfVerBar ) {
	      drawvline(cursors[0].x, c, 2, 0xAAAA);
	      drawvline(cursors[1].x, c, 2, 0xAAAA);
	    }
            glRectf(cursors[0].x-2, h()-cursors[0].y-2, cursors[0].x+2, h()-cursors[0].y+2); 
	    gl_draw("1", cursors[0].x-10, h()-cursors[0].y-5);
            glRectf(cursors[1].x-2, h()-cursors[1].y-2, cursors[1].x+2, h()-cursors[1].y+2); 
	    gl_draw("2", cursors[1].x-10., h()-cursors[1].y-5);
	  }
	}

	glEnable(GL_DEPTH_TEST);      

	drawstats();

}


int Fl_Scope::handle(int e)
{
	static int active_cursor=-1;
	if ( Scope_Flags & sfCursors ) {
	  switch (e) {
	  case FL_PUSH:
		if ( abs(cursors[0].x - Fl::event_x()) < abs(cursors[1].x - Fl::event_x()) )
		  active_cursor = 0;
   		else
		  active_cursor = 1; 
		cursors[active_cursor].x = Fl::event_x();
		cursors[active_cursor].y = Fl::event_y();
		return(1); 
		break;
	  case FL_DRAG:
		if ( active_cursor < 0 || active_cursor > 1) return (1);
		cursors[active_cursor].x = Fl::event_x();
		cursors[active_cursor].y = Fl::event_y();
		return(1); 
		break;

/*	  case FL_RELEASE:
		Event = Fl::event_state();
		return (1); */
	  }
        }
	return Fl_Gl_Window::handle(e);
}



Fl_Scope::Fl_Scope(int x, int y, int w, int h, int ntr, const char *title):Fl_Gl_Window(x,y,w,h,title)
{
	int i;

	num_of_traces = ntr;
	dxGrid = 0.;
	dyGrid = 0.;
	Time_Range = 1.0;
 	Trigger_Mode = tmRoll;
    

	Trace_Visible = new int[num_of_traces];
	Y_Div = new float[num_of_traces];
	Y_Range_Inf = new float[num_of_traces];
	Y_Range_Sup = new float[num_of_traces];
	Trace_rgb = new float*[num_of_traces];
	Trace_Color = new Fl_Color[num_of_traces];
	Trace_Offset = new float[num_of_traces];
	Trace_Offset_Value = new float[num_of_traces];
	Trace_Width = new float[num_of_traces];

	Trace = new float*[num_of_traces];
	Trace_Flags = new int[num_of_traces];
	Write_Ptr = new int[num_of_traces];
	Data_Ptr = new int[num_of_traces];
	Stats = new trStat[num_of_traces];
	DC_Val = new float[num_of_traces];


	for (i = 0; i < num_of_traces; i++) {
		Trace_Visible[i] = true;
		Y_Div[i] = 2.5;
		Y_Range_Inf[i] = -Y_Div[i]*((float)NDIV_GRID_Y/2.);
		Y_Range_Sup[i] = Y_Div[i]*((float)NDIV_GRID_Y/2.);
		Trace_rgb[i] = new float[3];
		Trace_Offset[i] = h/2;
		Trace_Offset_Value[i] = 1.0;
		Trace_Width[i] = 0.1;
		Trace[i] = new float[MAX_TRACE_LENGTH];
		Write_Ptr[i] =0 ;
		Trace_Flags[i]=tfNone;
	}
	Pause_Flag = true;
	OneShot_Flag = false;
	Scope_Flags = sfDrawGrid;
	Grid_rgb[0] = 0.650;
	Grid_rgb[1] = 0.650;
	Grid_rgb[2] = 0.650;
	Grid_Color = FL_GRAY;
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(Grid_rgb[0]*255.), (unsigned char)(Grid_rgb[1]*255.), (unsigned char)(Grid_rgb[2]*255.)));
	Grid_Color = FL_FREE_COLOR;
	Bg_rgb[0] = 0.388;
	Bg_rgb[1] = 0.451;
	Bg_rgb[2] = 0.604;
	Bg_Color = FL_GRAY;
	fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(Bg_rgb[0]*255.), (unsigned char)(Bg_rgb[1]*255.), (unsigned char)(Bg_rgb[2]*255.)));
	Bg_Color = FL_FREE_COLOR;
	for (i = 0; i < num_of_traces; i++) {
		Trace_rgb[i][0] = 1.0;
		Trace_rgb[i][1] = 1.0;
		Trace_rgb[i][2] = 1.0;
		Trace_Color[i] = FL_GRAY;
		fl_set_color(FL_FREE_COLOR, fl_rgb((unsigned char)(Trace_rgb[i][0]*255.), (unsigned char)(Trace_rgb[i][1]*255.), (unsigned char)(Trace_rgb[i][2]*255.)));
	}
}
