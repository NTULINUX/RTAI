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
// ffiles and ofiles may be also be manually defined
// see SCI/contrib/toolbox_skeleton/src/fortran/builder_fortran.sce
files=dir('*.f');
ffiles=strsubst(files.name,'.f','');
ofiles=strsubst(files.name,'.f','.o');
tbx_build_src(ffiles, ofiles, 'f',get_absolute_file_path('builder_fortran.sce'));
clear tbx_build_src, files, ffiles, ofiles;

