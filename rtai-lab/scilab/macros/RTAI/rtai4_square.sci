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

function [x,y,typ] = rtai4_square(job,arg1,arg2)
  x=[];y=[];typ=[];
  select job
  case 'plot' then
    exprs=arg1.graphics.exprs;    
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
    exprs=graphics.exprs;
    while %t do
      [ok,A,prd,pulse,bias,delay,exprs]=..
      getvalue('Set RTAI-square block parameters',..
      ['Amplitude:';
       'Period:';
       'Impulse width:';
       'Bias:';
       'Delay:'],..
      list('vec',-1,'vec',-1,'vec',-1,'vec',-1,'vec',-1),exprs)
      if ~ok then break,end
      if exists('outport') then out=ones(outport,1), in=[], else out=1, in=[], end
      [model,graphics,ok]=check_io(model,graphics,in,out,1,[])
      if ok then
        graphics.exprs=exprs;
        model.rpar=[A;prd;pulse;bias;delay];
        model.ipar=[];
        model.dstate=[];
        x.graphics=graphics;x.model=model
        break
      end
    end
  case 'define' then
    A=1
    prd=4
    pulse=2
    bias=0
    delay=0
    model=scicos_model()
    model.sim=list('rtsquare',4)
    if exists('outport') then model.out=ones(outport,1), model.in=[], else model.out=1, model.in=[], end
    model.evtin=1
    model.rpar=[A;prd;pulse;bias;delay]
    model.ipar=[]
    model.dstate=[];
    model.blocktype='d'
    model.dep_ut=[%t %f]
    exprs=[sci2exp(A),sci2exp(prd),sci2exp(pulse),sci2exp(bias),sci2exp(delay)]
    gr_i=['xstringb(orig(1),orig(2),[''Square''],sz(1),sz(2),''fill'');']
    x=standard_define([3 2],model,exprs,gr_i)
  end
endfunction
