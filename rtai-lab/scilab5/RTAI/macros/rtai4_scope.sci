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
function [x,y,typ] = rtai4_scope(job,arg1,arg2)
  x=[];y=[];typ=[];

  select job
  case 'plot' then

    name=arg1.graphics.exprs(1)
    standard_draw(arg1)
    orig=arg1.graphics.orig
    sz=arg1.graphics.sz
    flip=arg1.graphics.flip
    traceNames=arg1.graphics.exprs(2)
    index=strindex(traceNames,';')
    if index<>[] then
      tnames=strsplit(traceNames,index)
      tnames=strsubst(tnames,';','')
    else
      tnames=traceNames
    end
    if size(tnames,1)==size(arg1.model.in,1) then
      if size(tnames,1)>1 then
        [xin,yin,typin]=standard_inputs(arg1)
        dy=yin(2)-yin(1)
        for i=1:size(tnames,1)
          if flip then
            xstringb(orig(1),yin(i)-dy/2,tnames(i),sz(1),dy,'fill')
          else
            xstringb(orig(1)-sz(1),yin(i)-dy/2,tnames(i),sz(1),dy,'fill')
          end
        end
      else
        xstringb(orig(1),orig(2),tnames,sz(1),sz(2),'fill')
      end
    end
  case 'getinputs' then
    [x,y,typ]=standard_inputs(arg1)
  case 'getoutputs' then
    [x,y,typ]=standard_outputs(arg1)
  case 'getorigin' then
    [x,y]=standard_origin(arg1)
  case 'set' then
    if size(arg1.model.ipar,1) > 0 then
      //Convert from old to new scope
      name=arg1.graphics.exprs(2)
      arg1.model.opar=list(iconvert([ascii(stripblanks(name,1)),0],11))
      traceNames=''
      for i=1:size(arg1.model.in,1)
        if i~=1
          traceNames=traceNames+';'
        end
        traceNames=traceNames+sci2exp(i)
        arg1.model.opar(i+1)=iconvert([ascii(sci2exp(i)),0],11)
      end
      arg1.graphics.exprs=[name,traceNames]
      arg1.model.ipar=[]
      arg1.model.rpar=[]
      arg1.graphics.gr_i=['xstringb(orig(1),orig(2)-25,[name],sz(1),25,''fill'');']
    end

    x=arg1
    model=arg1.model;graphics=arg1.graphics;
    exprs=graphics.exprs;
    while %t do
      [ok,name,traceNames,exprs]=..
      scicos_getvalue('Set RTAI-scope block parameters',..
      ['Scope name';
			 'Name of traces (seperator=;)'],..
      list('str',1,'str',1),exprs)
      if ~ok then break,end
      in=[]
      out=[]
      if exists('traceNames') then 
		    index=strindex(traceNames,';')
				if index<>[] then
		      in=ones(size(index,2)+1,1)
				else
					in=1
				end
			end
			
      [model,graphics,ok]=check_io(model,graphics,in,out,1,[])
      if ok then
        graphics.exprs=exprs
        model.rpar=[]
        model.ipar=[]
        if index<>[] then
          tnames=strsplit(traceNames,index)
          tnames=strsubst(tnames,';','')
        else
          tnames=traceNames
        end
        opar=list(iconvert([ascii(stripblanks(name,%t)),0],11))
        for i=1:size(tnames, 1)
          opar(i+1)=iconvert([ascii(stripblanks(tnames(i),%t)),0],11)
        end
        model.opar=opar
        model.dstate=[]
        x.graphics=graphics;x.model=model
        break
      end
    end
  case 'define' then
    traceNames='1'
    name='SCOPE'
    model=scicos_model()
    model.sim=list('rtscope',4)
    model.in=1
    model.out=[]
    model.evtin=1
    model.rpar=[]
    model.ipar=[]
    model.opar=list(iconvert([ascii(name),0],11),iconvert([ascii(traceNames),0],11))
    model.dstate=[0]
    model.blocktype='d'
    model.dep_ut=[%t %f]
    exprs=[name;traceNames]
    gr_i=['xstringb(orig(1),orig(2)-25,[name],sz(1),25,''fill'');']
    x=standard_define([3 2],model,exprs,gr_i)
  end
endfunction
