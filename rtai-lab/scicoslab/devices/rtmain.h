/*
COPYRIGHT (C) 2003  Roberto Bucher (roberto.bucher@die.supsi.ch)

COPYRIGHT (C) 2009  Henrik Slotholt (rtai@slotholt.net)

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

void *get_a_name(const char *root, char *name);
int rtRegisterScope(char *name, char **traceNames, int n);
int rtRegisterLed(const char *name, int n);
int rtRegisterMeter(const char *name, int n);
void exit_on_error(void);


