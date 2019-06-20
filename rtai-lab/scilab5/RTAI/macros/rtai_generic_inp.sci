//
// Copyright (C) 1999-2017 Roberto Bucher
// Copyright (C) 2010-2011 Holger  Nahrstaedt
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
function [x,y,typ]=rtai_generic_inp(job,arg1,arg2)
//
// Copyright roberto.bucher@supsi.ch
x=[];y=[];typ=[];
select job
case 'plot' then
  graphics=arg1.graphics; exprs=graphics.exprs;
  name=exprs(1)(2);
  standard_draw(arg1)
case 'getinputs' then
  [x,y,typ]=standard_inputs(arg1)
case 'getoutputs' then
  [x,y,typ]=standard_outputs(arg1)
case 'getorigin' then
  [x,y]=standard_origin(arg1)
case 'set' then
  x=arg1
  model=arg1.model;graphics=arg1.graphics;
  label=graphics.exprs;
  while %t do
    [ok,op,name,lab]=..
        scicos_getvalue('Set RTAI generic input block parameters',..
        ['output ports';
	'Identifier'],..
         list('vec',-1,'str',1),label(1)')

    if ~ok then break,end
    label(1)=lab
    funam='i_generic_' + name;
    xx=[];ng=[];z=0;
    nx=0;nz=0;
    o=[];
    i=[];
    for nn = 1 : op
      o=[o,1];
    end
    o=int(o(:));nout=size(o,1);
    ci=1;nevin=1;
    co=[];nevout=0;
    funtyp=2004;
    depu=%t;
    dept=%f;
    dep_ut=[depu dept];

    tt=label(2);
    [ok,tt]=getCode_generic_inp(funam,tt)
    if ~ok then break,end
    [model,graphics,ok]=check_io(model,graphics,i,o,ci,co)
    if ok then
      model.sim=list(funam,funtyp)
      model.in=[]
      model.out=o
      model.evtin=ci
      model.evtout=[]
      model.state=[]
      model.dstate=[]
      model.rpar=[]
      model.ipar=[]
      model.firing=[]
      model.dep_ut=dep_ut
      model.nzcross=0
      label(2)=tt
      x.model=model
      graphics.exprs=label
      x.graphics=graphics
      break
    end
  end
case 'define' then
  out=1
  outsz = 1
  name = 'SENS'

  model=scicos_model()
  model.sim=list(' ',2004)
  model.in=[]
  model.out=outsz
  model.evtin=1
  model.evtout=[]
  model.state=[]
  model.dstate=[]
  model.rpar=[]
  model.ipar=[]
  model.blocktype='c'
  model.firing=[]
  model.dep_ut=[%t %f]
  model.nzcross=0

  label=list([sci2exp(out),name],[])

  gr_i=['xstringb(orig(1),orig(2),[''SENSOR'';name],sz(1),sz(2),''fill'');']
  x=standard_define([3 2],model,label,gr_i)

end
endfunction

function [ok,tt]=getCode_generic_inp(funam,tt)
if tt==[] then
  
   textmp=[
          '#ifndef MODEL'
          '#include <math.h>';
          '#include <stdlib.h>';
          '#include <scicos/scicos_block.h>';
          '#endif'
          '';
          'void '+funam+'(scicos_block *block,int flag)';
         ];
  textmp($+1)='{'
  textmp($+1)='#ifdef MODEL'
  textmp($+1)='int i;'
  textmp($+1)='double y[' + string(nout) + '];'
  textmp($+1)='double t = get_scicos_time();' 
  textmp($+1)='  switch(flag) {'
  textmp($+1)='  case 4:'
  textmp($+1)='    /* Initialisation */'
  textmp($+1)='    break;'; 
  textmp($+1)='  case 1:'
  textmp($+1)='    /* input(y,t); */'
  textmp($+1)='    for (i=0;i<' + string(nout) + ';i++) block->outptr[i][0] = y[i];'
  textmp($+1)='    break;'
  textmp($+1)='  case 5:'
  textmp($+1)='    /* end */'
  textmp($+1)='    break;'
  textmp($+1)='  }'
  textmp($+1)='#endif'
  textmp($+1)='}'

else
  textmp=tt;
end

while 1==1
  [txt]=x_dialog(['Function definition in C';
		  'Here is a skeleton of the functions which you shoud edit'],..
		 textmp);
  
  if txt<>[] then
    tt=txt
    [ok]=scicos_block_link(funam,tt,'c')
    if ok then
      textmp=txt;
    end
    break;
  else
    ok=%f;break;
  end  
end

endfunction
