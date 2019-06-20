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

//exec file used to load the "compiled" block into Scilab
rdnom='pid'
// get the absolute path of this loader file
[units,typs,nams]=file();clear units typs;
for k=size(nams,'*'):-1:1
   l=strindex(nams(k),rdnom+'_loader.sce');
   if l<>[] then
     DIR=part(nams(k),1:l($)-1);
     break
   end
end
archname=emptystr()
Makename = DIR+rdnom+'_Makefile';
select getenv('COMPILER','NO');
case 'VC++'   then 
  Makename = strsubst(Makename,'/','\')+'.mak';
case 'ABSOFT' then 
  Makename = strsubst(Makename,'/','\')+'.amk';
end
libn=ilib_compile('libpid',Makename)
//unlink if necessary
[a,b]=c_link(rdnom); while a ;ulink(b);[a,b]=c_link(rdnom);end
link(libn,rdnom,'c')
//load the gui function
getf(DIR+'/'+rdnom+'_c.sci');
