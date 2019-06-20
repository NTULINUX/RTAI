// Copyright (C) 2005-2017 The RTAI project
// This [file] is free software; the RTAI project
// gives unlimited permission to copy and/or distribute it,
// with or without modifications, as long as this notice is preserved.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY, to the extent permitted by law; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.

mode(-1);
// specific part
libname='rtai' // name of scilab function library [CUSTOM]

//** It is a better function to recover the absolute path information 
DIR = get_absolute_file_path('builder.sce')

if ~MSDOS then // Unix Linux
  if part(DIR,1)<>'/' then DIR=getcwd()+'/'+DIR,end
  MACROS=DIR+'macros/' // Path of the macros directory
  ROUTINES = DIR+'routines/' 
else  // windows- Visual C++
  if part(DIR,2)<>':' then DIR=getcwd()+'\'+DIR,end
  MACROS=DIR+'macros\' // Path of the macros directory
  ROUTINES = DIR+'routines\' 
end

//compile sci files if necessary and build lib file
genlib(libname,MACROS)

cd(ROUTINES)

names=['rtsinus';
       'rtsquare';
       'rt_step';
       'exit_on_error';
       'par_getstr']
files=['rtai_sinus.o';
       'rtai_square.o';
       'rtai_step.o';
       'exit_on_error.o';
       'getstr.o']

libn=ilib_for_link(names,files,[],"c","Makelib","loader.sce","rtinp","","-I.")

quit
