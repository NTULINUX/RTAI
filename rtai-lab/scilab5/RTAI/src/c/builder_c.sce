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
// Created by toolbox_creator
// cfiles and ofiles may be also be manually defined
// see SCI/contrib/toolbox_skeleton/src/c/builder_c.sce
//files=dir('*.c');
names=['rtsinus','rtsquare','rt_step']
files=[ 'rtai_sinus.c', 'rtai_square.c', 'rtai_step.c'];
src_c_path = get_absolute_file_path("builder_c.sce");

cflags    = '';
//source version
if isdir(SCI+"/modules/core/includes/") then
 cflags = cflags + " -I" + SCI + "/modules/scicos/includes/" + " -I" + SCI + "/modules/scicos_blocks/includes/";
end

// Binary version
if isdir(SCI+"/../../include/scilab/") then
 cflags = cflags + " -I" + SCI + "/../../include/scilab/scicos/" + " -I" + SCI + "/../../include/scilab/scicos_blocks/";
end

//ofiles=strsubst(files.name,'.c','.o');
tbx_build_src(names, files, 'c',src_c_path,"","",cflags);
//tbx_build_src(names, files, 'c', src_c_path ,libs,ldflags,cflags,fflags,cc,libname);
clear tbx_build_src, files, names, cflags, src_c_path;

