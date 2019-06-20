//
// Copyright (C) 1999-2017 Roberto Bucher
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//

mode(-1);
actDIR = pwd();

// specific part
libname='rtai' // name of scilab function library [CUSTOM]
DIR = get_absolute_file_path('loader.sce');
chdir(DIR)

// You should also modify the  ROUTINES/loader.sce with the new 
// Scilab primitive for the path

rtailib = lib(DIR+'/macros/')
exec('SCI/contrib/RTAI/routines/loader.sce');

chdir('macros');
exec('loadmacros.sce');
chdir(actDIR);

disp("Scicos-RTAI Ready");

