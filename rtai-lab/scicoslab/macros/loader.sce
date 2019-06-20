// Copyright (C) 2005-2017 The RTAI project
// This [file] is free software; the RTAI project
// gives unlimited permission to copy and/or distribute it,
// with or without modifications, as long as this notice is preserved.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY, to the extent permitted by law; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.

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
