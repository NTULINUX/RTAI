%
% Copyright (C) 1999-2017 Roberto Bucher
%
% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License as
% published by the Free Software Foundation; either version 2 of the
% License, or (at your option) any later version.
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program.  If not, see <http://www.gnu.org/licenses/>.
%

pathL=get_absolute_file_path('loadmacros.sce');

%scicos_menu($+1)=['RTAI','RTAI CodeGen','Set Target'];
scicos_pal($+1,:)=['RTAI-Lib','SCI/contrib/RTAI/macros/RTAI-Lib.cosf'];
%CmenuTypeOneVector($+1,:)=['RTAI CodeGen','Click on a Superblock (without activation output) to obtain a coded block!'];
%CmenuTypeOneVector($+1,:)=['Set Target','Click on a Superblock (without activation output) to set the Target!'];
%scicos_gif($+1)=[ pathL + 'gif_icons/' ];
