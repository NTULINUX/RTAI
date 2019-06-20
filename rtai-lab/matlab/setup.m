% *******
% setup.m
% *******
%
% Don't forget to copy ptinfo.tlc to [matlabroot]\rtw\c\tlc\mw if you're
% using Matlab 7.0 or newer.
%
% Copyright (C) 1999-2017 Lorenzo Dozio
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
%


if ispc, devpath='\rtw\c\rtai\devices';
else devpath='/rtw/c/rtai/devices';
end

devices = [matlabroot, devpath];
addpath(devices);
savepath;

cdir=cd;
cd(devices);

sfuns = dir('*.c')
for cnt = 1:length(sfuns)
    eval(['mex ' sfuns(cnt).name])
end

cd(cdir);

% ****************
