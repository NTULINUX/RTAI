function RTAICodeGen_()
//Copyright (c) 1989-2010 Metalau project INRIA
//
// Last update : 19/07/10
//
// Input editor function of Scicos code generator
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

    k = [] ; //** index of the CodeGen source superbloc candidate

    xc = %pt(1); //** last valid click position
    yc = %pt(2);

    %pt   = []   ;
    Cmenu = [] ;

    k  = getobj(scs_m,[xc;yc]) ; //** look for a block
    needcompile = 4  ;// this have to be done before calling the generator to avoid error with modelica blocks
                      // updateC in compile_modelica must be true when linking the functions

    //@@ default global variable
    ALL = %f;      //@@ entire diagram generation
    with_gui = %t; //@@ entire diagram generation with gui

    //** check if we have clicked near an object
    if k==[] then
      ALL = %t;
      //return
    //** check if we have clicked near a block
    elseif typeof(scs_m.objs(k))<>'Block' then
      return
    end

    if ~ALL then
      //** If the clicked/selected block is really a superblock
      //**             <k>
      if scs_m.objs(k).model.sim(1)=='super' | scs_m.objs(k).gui =='DSUPER'  then

         //##adjust selection if necessary
         if Select==[] then
           Select=[k curwin]
         end

         //##
         global scs_m_top

         //## test to know if a simulation have not been finished
         if alreadyran then
            Scicos_commands=['%diagram_path_objective=[];%scicos_navig=1';
                             '[alreadyran,%cpr]=do_terminate();'+...
                             '%diagram_path_objective='+sci2exp(super_path)+';%scicos_navig=1';
                             '%pt='+sci2exp(%pt)+';Cmenu='"RTAI CodeGen"'';]

         //## test to know if the precompilation of that sblock have been done
         elseif ( isequal(scs_m_top,[]) | isequal(scs_m_top,list()) ) then
            Scicos_commands=['%diagram_path_objective=[];%scicos_navig=1';
                             'global scs_m_top; scs_m_top=adjust_all_scs_m(scs_m,'+sci2exp(k)+');'+...
                             '%diagram_path_objective='+sci2exp(super_path)+';%scicos_navig=1';
                             '%pt='+sci2exp(%pt)+';Cmenu='"RTAI CodeGen"'';]

         else
            // Got to target sblock.
            scs_m_top=goto_target_scs_m(scs_m_top)
            //## call do_compile_superblock
            [ok, XX, gui_path, flgcdgen, szclkINTemp, freof, c_atomic_code] = ...
                              do_compile_superblock42(scs_m_top, k);

            clearglobal scs_m_top;

            //**quick fix for sblock that contains scope
            gh_curwin=scf(curwin)
         end
      else
        //** the clicked/selected block is NOT a superblock
        message("Generation Code only work for a Superblock ! ")
      end

    //@@
    else
        x_message("Embedded Generation only work for a Superblock ! ")
    end

endfunction

// *******************************************************

function [txt]=call_block42(bk,pt,flag)
//Copyright (c) 1989-2010 Metalau project INRIA

//** call_block42 : generate C calling sequence
//                  of a scicos block
//
// Input : bk   : block index
//         pt   : evt activation number
//         flag : flag
//
// Output : txt  : the generated calling sequence
//                 of the scicos block
//

  txt=[]

  if flag==2 & (zptr(bk+1)-zptr(bk)+xptr(bk+1)-xptr(bk) == 0) then
    return
  end

  //**
  if flag==2 & ((zcptr(bk+1)-zcptr(bk))<>0) & pt<0 then

  else
    if flag==2 & ((zptr(bk+1)-zptr(bk))+..
                  (ozptr(bk+1)-ozptr(bk))+..
                  (xptr(bk+1)-xptr(bk)+..
                  with_work(bk))==0 |..
                  (pt<=0 & (with_work(bk))==0) ) & ~(stalone & or(bk==actt(:,1))) then
      return // block without state or continuously activated
    end
  end
  if (flag==0 | flag==10) & ((xptr(bk+1)-xptr(bk))==0 & with_work(bk)==0) then
    return // block without continuous state
  end
  if flag==7 & ((xptr(bk+1)-xptr(bk))==0) then
    return // block without continuous state
  end
  if flag==9 & ((zcptr(bk+1)-zcptr(bk))==0) then
    return // block without continuous state
  end
  if flag==3 & ((clkptr(bk+1)-clkptr(bk))==0) then
    return
  end

  //** adjust pt
  if ~(flag==3 & ((zcptr(bk+1)-zcptr(bk))<>0) |..
       flag==2 & ((zcptr(bk+1)-zcptr(bk))<>0)) then
    pt=abs(pt)
  end

  //## check and adjust function type
  ftyp=funtyp(bk)
  ftyp_tmp=modulo(funtyp(bk),10000)
  if ftyp_tmp>2000 then
    ftyp=ftyp-2000
  elseif ftyp_tmp>1000 then
    ftyp=ftyp-1000
  end

  //** change flag 7 to flag 0 for ftyp<10000
  flagi=flag
  if flag==7 & ftyp < 10000 then
    flag=0;
  end

  //** set nevprt and flag for called block
  txt_nf=['block_'+rdnom+'['+string(bk-1)+'].nevprt = '+string(pt)+';'
          'local_flag = '+string(flag)+';']

  //@@ add block number for standalone
  if stalone then
    txt_nf=[txt_nf;
            'set_block_number('+string(bk)+');']
  end

  //@@ init evout
  if flag==3 then
    txt_init_evout=['/* initialize evout */'
                    'for(kf=0;kf<block_'+rdnom+'['+string(bk-1)+'].nevout;kf++) {'
                    '  block_'+rdnom+'['+string(bk-1)+'].evout[kf]=-1.;'
                    '}']
  else
    txt_init_evout=[]
  end

  //** add comment
  txt=[get_comment('call_blk',list(funs(bk),funtyp(bk),bk,ztyp(bk)));]

  //@@ remove call to the end block for standalone
  if stalone & funs(bk)=='scicosexit' then
    txt=[];
    return
  end

  //** see if its bidon, actuator or sensor
  if funs(bk)=='bidon' then
    txt=[];
    return
  elseif funs(bk)=='bidon2' then
    txt=[];
    return
  //@@ agenda_blk
  elseif funs(bk)=='agenda_blk' then
    txt=[];
    return
  //## sensor
  elseif or(bk==capt(:,1)) then
    ind=find(bk==capt(:,1))
    yk=capt(ind,2);

    txt = [txt_init_evout;
           txt;
           txt_nf
           'nport = '+string(ind)+';']

    txt = [txt;
           rdnom+'_sensor(&local_flag, &nport, &block_'+rdnom+'['+string(bk-1)+'].nevprt, \'
           get_blank(rdnom+'_sensor')+' &t, ('+mat2scs_c_ptr(outtb(yk))+' *)block_'+rdnom+'['+string(bk-1)+'].outptr[0], \'
           get_blank(rdnom+'_sensor')+' &block_'+rdnom+'['+string(bk-1)+'].outsz[0], \'
           get_blank(rdnom+'_sensor')+' &block_'+rdnom+'['+string(bk-1)+'].outsz[1], \'
           get_blank(rdnom+'_sensor')+' &block_'+rdnom+'['+string(bk-1)+'].outsz[2], \'
           get_blank(rdnom+'_sensor')+' block_'+rdnom+'['+string(bk-1)+'].insz[0], \'
           get_blank(rdnom+'_sensor')+' block_'+rdnom+'['+string(bk-1)+'].inptr[0]);']
    //## errors management
//    txt = [txt;
//           '/* error handling */'
//           'if(local_flag < 0) {']
//    if stalone then
//      txt =[txt;
//            '  set_block_error(5 - local_flag);']
//      if flag<>5 & flag<>4 then
//        if ALL then
//          txt =[txt;
//                '  '+rdnom+'_cosend();'
//                '  return get_block_error();']
//        else
//          txt =[txt;
//                '  Cosend();'
//                '  return get_block_error();']
//        end
//      end
//    else
//      txt =[txt;
//            '  set_block_error(local_flag);']
//      if flag<>5 & flag<>4 then
//        txt = [txt;
//               '  return;']
//      end
//    end
//    txt = [txt;
//           '}']
    return
  //## actuator
  elseif or(bk==actt(:,1)) then
    ind=find(bk==actt(:,1))

    uk=actt(ind,2)
    nin=size(ind,2)

    for j=1:nin
      txt = [txt_init_evout;
             txt;
             txt_nf
             'nport = '+string(ind(j))+';']
      if strindex(funs(bk),"dummy")<>[] then
        txt = [txt;
               rdnom+'_dummy_actuator(&local_flag, &nport, &block_'+rdnom+'['+string(bk-1)+'].nevprt, \'
               get_blank(rdnom+'_actuator')+' &t, \'
               get_blank(rdnom+'_actuator')+' block_'+rdnom+'['+string(bk-1)+'].outsz['+string(j-1)+'], \'
               get_blank(rdnom+'_actuator')+' block_'+rdnom+'['+string(bk-1)+'].outptr['+string(j-1)+']);']
      else
        txt = [txt;
               rdnom+'_actuator(&local_flag, &nport, &block_'+rdnom+'['+string(bk-1)+'].nevprt, \'
               get_blank(rdnom+'_actuator')+' &t, ('+mat2scs_c_ptr(outtb(uk(j)))+' *)block_'+rdnom+'['+string(bk-1)+'].inptr['+string(j-1)+'], \'
               get_blank(rdnom+'_actuator')+' &block_'+rdnom+'['+string(bk-1)+'].insz['+string(j-1)+'], \'
               get_blank(rdnom+'_actuator')+' &block_'+rdnom+'['+string(bk-1)+'].insz['+string(nin+j-1)+'], \'
               get_blank(rdnom+'_actuator')+' &block_'+rdnom+'['+string(bk-1)+'].insz['+string(2*nin+j-1)+'], \'
               get_blank(rdnom+'_actuator')+' block_'+rdnom+'['+string(bk-1)+'].outsz['+string(j-1)+'], \'
               get_blank(rdnom+'_actuator')+' block_'+rdnom+'['+string(bk-1)+'].outptr['+string(j-1)+']);']
      end
      //## errors management
//      txt = [txt;
//             '/* error handling */'
//             'if(local_flag < 0) {']
//      if stalone then
//        txt =[txt;
//              '  set_block_error(5 - local_flag);']
//        if flag<>5 & flag<>4 then
//          if ALL then
//            txt =[txt;
//                  '  '+rdnom+'_cosend();'
//                  '  return get_block_error();']
//          else
//            txt =[txt;
//                  '  Cosend();'
//                  '  return get_block_error();']
//          end
//        end
//      else
//        txt =[txt;
//              '  set_block_error(local_flag);']
//        if flag<>5 & flag<>4 then
//          txt = [txt;
//                 '  return;']
//        end
//      end
//      txt = [txt;
//             '}']
    end

    return
  end

  //**
  nx=xptr(bk+1)-xptr(bk);
  nz=zptr(bk+1)-zptr(bk);
  nrpar=rpptr(bk+1)-rpptr(bk);
  nipar=ipptr(bk+1)-ipptr(bk);
  nin=inpptr(bk+1)-inpptr(bk);  //* number of input ports */
  nout=outptr(bk+1)-outptr(bk); //* number of output ports */

  //**
  //ipar start index ptr
  if nipar<>0 then ipar=ipptr(bk), else ipar=1;end
  //rpar start index ptr
  if nrpar<>0 then rpar=rpptr(bk), else rpar=1; end
  //z start index ptr (warning -1)
  if nz<>0 then z=zptr(bk)-1, else z=0;end
  //x start index ptr
  if nx<>0 then x=xptr(bk)-1, else x=0;end

  //** check function type
  if ftyp < 0 then //** ifthenelse eselect blocks
      txt = [];
      return;
  else
    if (ftyp<>0 & ftyp<>1 & ftyp<>2 & ftyp<>3 & ftyp<>4  & ftyp<>10004) then
      disp("Types other than 0,1,2,3 or 4/10004 are not supported.")
      txt = [];
      return;
    end
  end

  select ftyp

    case 0 then

      txt=[txt_init_evout;
           txt;
           txt_nf]

      //**** input/output addresses definition ****//
      //## concatenate input
      if nin>1 then
        for k=1:nin
          uk=inplnk(inpptr(bk)-1+k);
          nuk=size(outtb(uk),1);
          //## YAUNEERREURICIFAUTRECOPIERTOUTDANSRDOUTTB
//           txt=[txt;
//                'rdouttb['+string(k-1)+']=(double *)'+rdnom+'_block_outtbptr['+string(uk-1)+'];']

          txt=[txt;
               'rdouttb['+string(k-1)+']=(double *)block_'+rdnom+'['+string(bk-1)+'].inptr['+string(k-1)+'];']

        end
        txt=[txt;
             'args[0]=&(rdouttb[0]);']
      elseif nin==0
        uk=0;
        nuk=0;
        txt=[txt;
             'args[0]=NULL;']
      else
        uk=inplnk(inpptr(bk));
        nuk=size(outtb(uk),1);
        txt=[txt;
             'args[0]=(double *)block_'+rdnom+'['+string(bk-1)+'].inptr[0];']
      end

      //## concatenate outputs
      if nout>1 then
        for k=1:nout
          yk=outlnk(outptr(bk)-1+k);
          nyk=size(outtb(yk),1);
          //## YAUNEERREURICIFAUTRECOPIERTOUTDANSRDOUTTB
//           txt=[txt;
//                'rdouttb['+string(k+nin-1)+']=(double *)'+rdnom+'_block_outtbptr['+string(yk-1)+'];'];
          txt=[txt;
               'rdouttb['+string(k+nin-1)+']=(double *)block_'+rdnom+'['+string(bk-1)+'].outptr['+string(k-1)+'];']
        end
        txt=[txt;
             'args[1]=&(rdouttb['+string(nin)+']);'];
      elseif nout==0
        yk=0;
        nyk=0;
        txt=[txt;
             'args[1]=NULL;'];
      else
        yk=outlnk(outptr(bk));
        nyk=size(outtb(yk),1);
        txt=[txt;
             'args[1]=(double *)block_'+rdnom+'['+string(bk-1)+'].outptr[0];'];
      end
      //*******************************************//

      //@@ this is for compatibility, jroot is returned in g for old type
      if (zcptr(bk+1)-zcptr(bk))<>0 & pt<0 then
        txt=[txt;
             '/* Update g array */'
             'for(i=0;i<block_'+rdnom+'['+string(bk-1)+'].ng;i++) {'
             '  block_'+rdnom+'['+string(bk-1)+'].g[i]=(double)block_'+rdnom+'['+string(bk-1)+'].jroot[i];'
             '}']
      end

      //## adjust continuous state array before call
      if impl_blk & flag==0 then
        txt=[txt;
             '/* adjust continuous state array before call */'
             'block_'+rdnom+'['+string(bk-1)+'].res = &(res['+string(xptr(bk)-1)+']);'];

        //*********** call seq definition ***********//
        txtc=['(&local_flag,&block_'+rdnom+'['+string(bk-1)+'].nevprt,&t,block_'+rdnom+'['+string(bk-1)+'].res, \';
              'block_'+rdnom+'['+string(bk-1)+'].x,&block_'+rdnom+'['+string(bk-1)+'].nx, \';
              'block_'+rdnom+'['+string(bk-1)+'].z,&block_'+rdnom+'['+string(bk-1)+'].nz,block_'+rdnom+'['+string(bk-1)+'].evout, \';
              '&block_'+rdnom+'['+string(bk-1)+'].nevout,block_'+rdnom+'['+string(bk-1)+'].rpar,&block_'+rdnom+'['+string(bk-1)+'].nrpar, \';
              'block_'+rdnom+'['+string(bk-1)+'].ipar,&block_'+rdnom+'['+string(bk-1)+'].nipar, \';
              '(double *)args[0],(nrd_1='+string(nuk)+',&nrd_1),(double *)args[1],(nrd_2='+string(nyk)+',&nrd_2));'];
      else
        //*********** call seq definition ***********//
        txtc=['(&local_flag,&block_'+rdnom+'['+string(bk-1)+'].nevprt,&t,block_'+rdnom+'['+string(bk-1)+'].xd, \';
              'block_'+rdnom+'['+string(bk-1)+'].x,&block_'+rdnom+'['+string(bk-1)+'].nx, \';
              'block_'+rdnom+'['+string(bk-1)+'].z,&block_'+rdnom+'['+string(bk-1)+'].nz,block_'+rdnom+'['+string(bk-1)+'].evout, \';
              '&block_'+rdnom+'['+string(bk-1)+'].nevout,block_'+rdnom+'['+string(bk-1)+'].rpar,&block_'+rdnom+'['+string(bk-1)+'].nrpar, \';
             'block_'+rdnom+'['+string(bk-1)+'].ipar,&block_'+rdnom+'['+string(bk-1)+'].nipar, \';
             '(double *)args[0],(nrd_1='+string(nuk)+',&nrd_1),(double *)args[1],(nrd_2='+string(nyk)+',&nrd_2));'];
      end

      if (funtyp(bk)>2000 & funtyp(bk)<3000)
        blank = get_blank(funs(bk)+'( ');
        txtc(1) = funs(bk)+txtc(1);
      elseif (funtyp(bk)<2000)
        //@@ special case for andlog func : should be fixed in scicos
        if funs(bk) <> 'andlog' then
          txtc(1) = 'C2F('+funs(bk)+')'+txtc(1);
          blank = get_blank('C2F('+funs(bk)+') ');
        else
          blank = get_blank(funs(bk)+'( ');
          txtc(1) = funs(bk)+txtc(1);
        end 
      end
      txtc(2:$) = blank + txtc(2:$);
      txt = [txt;txtc];
      //*******************************************//

      //## adjust continuous state array after call
      if impl_blk & flag==0 then
        if flagi==7 then
          txt=[txt;
               '/* adjust continuous state array after call */'
               'for (i=0;i<block_'+rdnom+'['+string(bk-1)+'].nx;i++) {'
               '  block_'+rdnom+'['+string(bk-1)+'].xd[i] = block_'+rdnom+'['+string(bk-1)+'].res[i];'
               '}']
        else
          txt=[txt;
               '/* adjust continuous state array after call */'
               'for (i=0;i<block_'+rdnom+'['+string(bk-1)+'].nx;i++) {'
               '  block_'+rdnom+'['+string(bk-1)+'].res[i] = block_'+rdnom+'['+string(bk-1)+'].res[i] - '+...
                 'block_'+rdnom+'['+string(bk-1)+'].xd[i];'
               '}']
        end
      end
      //## errors management
//      txt = [txt;
//             '/* error handling */'
//             'if(local_flag < 0) {']
//      if stalone then
//        txt =[txt;
//              '  set_block_error(5 - local_flag);']
//        if flag<>5 & flag<>4 then
//          if ALL then
//            txt =[txt;
//                  '  '+rdnom+'_cosend();'
//                  '  return get_block_error();']
//          else
//            txt =[txt;
//                  '  Cosend();'
//                  '  return get_block_error();']
//          end
//        end
//      else
//        txt =[txt;
//              '  set_block_error(local_flag);']
//        if flag<>5 & flag<>4 then
//          txt = [txt;
//                 '  return;']
//        end
//      end
//      txt = [txt;
//             '}']

      if flag==3 then
        //@@ addevs function call
        if ALL & size(evs,'*')<>0 then
          ind=get_ind_clkptr(bk,clkptr,funtyp)
          txt = [txt;
                 '/* addevs function call */'
                 'for(kf=0;kf<block_'+rdnom+'['+string(bk-1)+'].nevout;kf++) {'
                 '  if (block_'+rdnom+'['+string(bk-1)+'].evout[kf]>=t) {'
                 '    '+rdnom+'_addevs(ptr, block_'+rdnom+'['+string(bk-1)+'].evout[kf], '+string(ind)+'+kf);'
                 '  }'
                 '}']
        else
          //@@ adjust values of output register
          //@@ TODO : multiple output event block
          txt = [txt;
                 '/* adjust values of output register */'
                 '/* TODO :  multiple output event block */'
                 'block_'+rdnom+'['+string(bk-1)+'].evout[0] = block_'+rdnom+'['+string(bk-1)+'].evout[0] - t;']
        end
      end
      return

    //**
    case 1 then

      txt=[txt_init_evout;
           txt;
           txt_nf]

      //@@ this is for compatibility, jroot is returned in g for old type
      if (zcptr(bk+1)-zcptr(bk))<>0 & pt<0 then
        txt=[txt;
             '/* Update g array */'
             'for(i=0;i<block_'+rdnom+'['+string(bk-1)+'].ng;i++) {'
             '  block_'+rdnom+'['+string(bk-1)+'].g[i]=(double)block_'+rdnom+'['+string(bk-1)+'].jroot[i];'
             '}']
      end

      //## adjust continuous state array before call
      if impl_blk & flag==0 then
        txt=[txt;
             '/* adjust continuous state array before call */'
             'block_'+rdnom+'['+string(bk-1)+'].res = &(res['+string(xptr(bk)-1)+']);'];

        //*********** call seq definition ***********//
        txtc=['(&local_flag,&block_'+rdnom+'['+string(bk-1)+'].nevprt,&t,block_'+rdnom+'['+string(bk-1)+'].res, \';
              'block_'+rdnom+'['+string(bk-1)+'].x,&block_'+rdnom+'['+string(bk-1)+'].nx, \';
              'block_'+rdnom+'['+string(bk-1)+'].z,&block_'+rdnom+'['+string(bk-1)+'].nz,block_'+rdnom+'['+string(bk-1)+'].evout, \';
              '&block_'+rdnom+'['+string(bk-1)+'].nevout,block_'+rdnom+'['+string(bk-1)+'].rpar,&block_'+rdnom+'['+string(bk-1)+'].nrpar, \';
              'block_'+rdnom+'['+string(bk-1)+'].ipar,&block_'+rdnom+'['+string(bk-1)+'].nipar , \'];
      else
        //*********** call seq definition ***********//
        txtc=['(&local_flag,&block_'+rdnom+'['+string(bk-1)+'].nevprt,&t,block_'+rdnom+'['+string(bk-1)+'].xd, \';
              'block_'+rdnom+'['+string(bk-1)+'].x,&block_'+rdnom+'['+string(bk-1)+'].nx, \';
              'block_'+rdnom+'['+string(bk-1)+'].z,&block_'+rdnom+'['+string(bk-1)+'].nz,block_'+rdnom+'['+string(bk-1)+'].evout, \';
              '&block_'+rdnom+'['+string(bk-1)+'].nevout,block_'+rdnom+'['+string(bk-1)+'].rpar,&block_'+rdnom+'['+string(bk-1)+'].nrpar, \';
              'block_'+rdnom+'['+string(bk-1)+'].ipar,&block_'+rdnom+'['+string(bk-1)+'].nipar , \'];
      end

      if (funtyp(bk)>2000 & funtyp(bk)<3000)
        blank = get_blank(funs(bk)+'( ');
        txtc(1) = funs(bk)+txtc(1);
      elseif (funtyp(bk)<2000)
        txtc(1) = 'C2F('+funs(bk)+')'+txtc(1);
        blank = get_blank('C2F('+funs(bk)+') ');
      end
      if nin>=1 | nout>=1 then
        if nin>=1 then
          for k=1:nin
            uk=inplnk(inpptr(bk)-1+k);
            txtc=[txtc;
                  '(double *)block_'+rdnom+'['+string(bk-1)+'].inptr['+string(k-1)+'],&block_'+rdnom+'['+string(bk-1)+'].insz['+string(k-1)+'], \';]
          end
        end
        if nout>=1 then
          for k=1:nout
            yk=outlnk(outptr(bk)-1+k);
            txtc=[txtc;
                  '(double *)block_'+rdnom+'['+string(bk-1)+'].outptr['+string(k-1)+'],&block_'+rdnom+'['+string(bk-1)+'].outsz['+string(k-1)+'], \';]
          end
        end
      end

      if ztyp(bk) then
        txtc=[txtc;
              'block_'+rdnom+'['+string(bk-1)+'].g,&block_'+rdnom+'['+string(bk-1)+'].ng);']
      else
        txtc($)=part(txtc($),1:length(txtc($))-3)+');';
      end

      txtc(2:$) = blank + txtc(2:$);
      txt = [txt;txtc];
      //*******************************************//

      //## adjust continuous state array after call
      if impl_blk & flag==0 then
        if flagi==7 then
          txt=[txt;
               '/* adjust continuous state array after call */'
               'for (i=0;i<block_'+rdnom+'['+string(bk-1)+'].nx;i++) {'
               '  block_'+rdnom+'['+string(bk-1)+'].xd[i] = block_'+rdnom+'['+string(bk-1)+'].res[i];'
               '}']
        else
          txt=[txt;
               '/* adjust continuous state array after call */'
               'for (i=0;i<block_'+rdnom+'['+string(bk-1)+'].nx;i++) {'
               '  block_'+rdnom+'['+string(bk-1)+'].res[i] = block_'+rdnom+'['+string(bk-1)+'].res[i] - '+...
                 'block_'+rdnom+'['+string(bk-1)+'].xd[i];'
               '}']
        end
      end
      //## errors management
//      txt = [txt;
//             '/* error handling */'
//             'if(local_flag < 0) {']
//      if stalone then
//        txt =[txt;
//              '  set_block_error(5 - local_flag);']
//        if flag<>5 & flag<>4 then
//          if ALL then
//            txt =[txt;
//                  '  '+rdnom+'_cosend();'
//                  '  return get_block_error();']
//          else
//            txt =[txt;
//                  '  Cosend();'
//                  '  return get_block_error();']
//          end
//        end
//      else
//        txt =[txt;
//              '  set_block_error(local_flag);']
//        if flag<>5 & flag<>4 then
//          txt = [txt;
//                 '  return;']
//        end
//      end
//      txt = [txt;
//             '}']

      if flag==3 then
        //@@ addevs function call
        if ALL & size(evs,'*')<>0 then
          ind=get_ind_clkptr(bk,clkptr,funtyp)
          txt = [txt;
                 '/* addevs function call */'
                 'for(kf=0;kf<block_'+rdnom+'['+string(bk-1)+'].nevout;kf++) {'
                 '  if (block_'+rdnom+'['+string(bk-1)+'].evout[kf]>=t) {'
                 '    '+rdnom+'_addevs(ptr, block_'+rdnom+'['+string(bk-1)+'].evout[kf], '+string(ind)+'+kf);'
                 '  }'
                 '}']
        else
          //@@ adjust values of output register
          //@@ TODO : multiple output event block
          txt = [txt;
                 '/* adjust values of output register */'
                 '/* TODO :  multiple output event block */'
                 'block_'+rdnom+'['+string(bk-1)+'].evout[0] = block_'+rdnom+'['+string(bk-1)+'].evout[0] - t;']
        end
      end
      return

    //**
    case 2 then

      txt=[txt_init_evout;
           txt;
           txt_nf]

      //@@ this is for compatibility, jroot is returned in g for old type
      if (zcptr(bk+1)-zcptr(bk))<>0 & pt<0 then
        txt=[txt;
             '/* Update g array */'
             'for(i=0;i<block_'+rdnom+'['+string(bk-1)+'].ng;i++) {'
             '  block_'+rdnom+'['+string(bk-1)+'].g[i]=(double)block_'+rdnom+'['+string(bk-1)+'].jroot[i];'
             '}']
      end

      //## adjust continuous state array before call
      if impl_blk & flag==0 then
        txt=[txt;
             '/* adjust continuous state array before call */'
             'block_'+rdnom+'['+string(bk-1)+'].res = &(res['+string(xptr(bk)-1)+']);'];

        //*********** call seq definition ***********//
        txtc=[funs(bk)+'(&local_flag,&block_'+rdnom+'['+string(bk-1)+'].nevprt,&t,block_'+rdnom+'['+string(bk-1)+'].res, \';
              'block_'+rdnom+'['+string(bk-1)+'].x,&block_'+rdnom+'['+string(bk-1)+'].nx, \';
              'block_'+rdnom+'['+string(bk-1)+'].z,&block_'+rdnom+'['+string(bk-1)+'].nz,block_'+rdnom+'['+string(bk-1)+'].evout, \';
              '&block_'+rdnom+'['+string(bk-1)+'].nevout,block_'+rdnom+'['+string(bk-1)+'].rpar,&block_'+rdnom+'['+string(bk-1)+'].nrpar, \';
              'block_'+rdnom+'['+string(bk-1)+'].ipar,&block_'+rdnom+'['+string(bk-1)+'].nipar, \';
              '(double **)block_'+rdnom+'['+string(bk-1)+'].inptr,block_'+rdnom+'['+string(bk-1)+'].insz,&block_'+rdnom+'['+string(bk-1)+'].nin, \';
              '(double **)block_'+rdnom+'['+string(bk-1)+'].outptr,block_'+rdnom+'['+string(bk-1)+'].outsz, &block_'+rdnom+'['+string(bk-1)+'].nout'];
      else
        //*********** call seq definition ***********//
        txtc=[funs(bk)+'(&local_flag,&block_'+rdnom+'['+string(bk-1)+'].nevprt,&t,block_'+rdnom+'['+string(bk-1)+'].xd, \';
              'block_'+rdnom+'['+string(bk-1)+'].x,&block_'+rdnom+'['+string(bk-1)+'].nx, \';
              'block_'+rdnom+'['+string(bk-1)+'].z,&block_'+rdnom+'['+string(bk-1)+'].nz,block_'+rdnom+'['+string(bk-1)+'].evout, \';
              '&block_'+rdnom+'['+string(bk-1)+'].nevout,block_'+rdnom+'['+string(bk-1)+'].rpar,&block_'+rdnom+'['+string(bk-1)+'].nrpar, \';
              'block_'+rdnom+'['+string(bk-1)+'].ipar,&block_'+rdnom+'['+string(bk-1)+'].nipar, \';
              '(double **)block_'+rdnom+'['+string(bk-1)+'].inptr,block_'+rdnom+'['+string(bk-1)+'].insz,&block_'+rdnom+'['+string(bk-1)+'].nin, \';
              '(double **)block_'+rdnom+'['+string(bk-1)+'].outptr,block_'+rdnom+'['+string(bk-1)+'].outsz, &block_'+rdnom+'['+string(bk-1)+'].nout'];
      end

      if ~ztyp(bk) then
        txtc($)=txtc($)+');';
      else
        txtc($)=txtc($)+', \';
        txtc=[txtc;
              'block_'+rdnom+'['+string(bk-1)+'].g,&block_'+rdnom+'['+string(bk-1)+'].ng);']
      end
      blank = get_blank(funs(bk)+'( ');
      txtc(2:$) = blank + txtc(2:$);
      txt = [txt;txtc];
      //*******************************************//

      //## adjust continuous state array after call
      if impl_blk & flag==0 then
        if flagi==7 then
          txt=[txt;
               '/* adjust continuous state array after call */'
               'for (i=0;i<block_'+rdnom+'['+string(bk-1)+'].nx;i++) {'
               '  block_'+rdnom+'['+string(bk-1)+'].xd[i] = block_'+rdnom+'['+string(bk-1)+'].res[i];'
               '}']
        else
          txt=[txt;
               '/* adjust continuous state array after call */'
               'for (i=0;i<block_'+rdnom+'['+string(bk-1)+'].nx;i++) {'
               '  block_'+rdnom+'['+string(bk-1)+'].res[i] = block_'+rdnom+'['+string(bk-1)+'].res[i] - '+...
                 'block_'+rdnom+'['+string(bk-1)+'].xd[i];'
               '}']
        end
      end
      //## errors management
//      txt = [txt;
//             '/* error handling */'
//             'if(local_flag < 0) {']
//      if stalone then
//        txt =[txt;
//              '  set_block_error(5 - local_flag);']
//        if flag<>5 & flag<>4 then
//          if ALL then
//            txt =[txt;
//                  '  '+rdnom+'_cosend();'
//                  '  return get_block_error();']
//          else
//            txt =[txt;
//                  '  Cosend();'
//                  '  return get_block_error();']
//          end
//        end
//      else
//        txt =[txt;
//              '  set_block_error(local_flag);']
//        if flag<>5 & flag<>4 then
//          txt = [txt;
//                 '  return;']
//        end
//      end
//      txt = [txt;
//             '}']

      if flag==3 then
        //@@ addevs function call
        if ALL & size(evs,'*')<>0 then
          ind=get_ind_clkptr(bk,clkptr,funtyp)
          txt = [txt;
                 '/* addevs function call */'
                 'for(kf=0;kf<block_'+rdnom+'['+string(bk-1)+'].nevout;kf++) {'
                 '  if (block_'+rdnom+'['+string(bk-1)+'].evout[kf]>=t) {'
                 '    '+rdnom+'_addevs(ptr, block_'+rdnom+'['+string(bk-1)+'].evout[kf], '+string(ind)+'+kf);'
                 '  }'
                 '}']
        else
          //@@ adjust values of output register
          //@@ TODO : multiple output event block
          txt = [txt;
                 '/* adjust values of output register */'
                 '/* TODO :  multiple output event block */'
                 'block_'+rdnom+'['+string(bk-1)+'].evout[0] = block_'+rdnom+'['+string(bk-1)+'].evout[0] - t;']
        end
      end
      return

    //**
    case 4 then

      txt=[txt_init_evout;
           txt;
           txt_nf]

      //## adjust continuous state array before call
      if impl_blk & flag==0 then
        txt=[txt;
             '/* adjust continuous state array before call */'
             'block_'+rdnom+'['+string(bk-1)+'].xd  = &(res['+string(xptr(bk)-1)+']);'
             'block_'+rdnom+'['+string(bk-1)+'].res = &(res['+string(xptr(bk)-1)+']);'];
      end

      txt=[txt;
           funs(bk)+'(&block_'+rdnom+'['+string(bk-1)+'],local_flag);'];

      //## adjust continuous state array after call
      if impl_blk & flag==0  then
        if flagi==7 then
          txt=[txt;
               '/* adjust continuous state array after call */'
               'block_'+rdnom+'['+string(bk-1)+'].xd = &(xd['+string(xptr(bk)-1)+']);'
               'for (i=0;i<block_'+rdnom+'['+string(bk-1)+'].nx;i++) {'
               '  block_'+rdnom+'['+string(bk-1)+'].xd[i] = block_'+rdnom+'['+string(bk-1)+'].res[i];'
               '}']
        else
          txt=[txt;
               '/* adjust continuous state array after call */'
               'block_'+rdnom+'['+string(bk-1)+'].xd = &(xd['+string(xptr(bk)-1)+']);'
               'for (i=0;i<block_'+rdnom+'['+string(bk-1)+'].nx;i++) {'
               '  block_'+rdnom+'['+string(bk-1)+'].res[i] = block_'+rdnom+'['+string(bk-1)+'].res[i] - '+...
                 'block_'+rdnom+'['+string(bk-1)+'].xd[i];'
               '}']
        end
      end

    //**
    case 10004 then

      txt=[txt_init_evout;
           txt;
           txt_nf]

      txt=[txt;
           funs(bk)+'(&block_'+rdnom+'['+string(bk-1)+'],local_flag);'];

  end

  //## errors management
//  if stalone then
//    txt =[txt;
//          '/* error handling */'
//          'if (get_block_error() < 0) {'
//          '  set_block_error(5 - get_block_error());']
//    if flag<>5 & flag<>4 then
//        if ALL then
//          txt =[txt;
//                '  '+rdnom+'_cosend();'
//                '  return get_block_error();']
//         else
//           txt = [txt;
//                  '  Cosend();'
//                  '  return get_block_error();']
//         end
//    end
//    txt =[txt;
//          '}']
//  else
//    if flag<>5 & flag<>4 then
//      txt =[txt;
//            '/* error handling */'
//            'if (get_block_error() < 0) {'
//            '  return;'
//            '}']
//    end
//  end

  //@@ addevs function call
  if flag==3 then
    if ALL & size(evs,'*')<>0 then
      ind=get_ind_clkptr(bk,clkptr,funtyp)
      txt = [txt;
             '/* addevs function call */'
             'for(kf=0;kf<block_'+rdnom+'['+string(bk-1)+'].nevout;kf++) {'
             '  if (block_'+rdnom+'['+string(bk-1)+'].evout[kf]>=0.) {'
             '    '+rdnom+'_addevs(ptr, block_'+rdnom+'['+string(bk-1)+'].evout[kf] + t, '+string(ind)+'+kf);'
             '  }'
             '}']
    else
      //@@ adjust values of output register
      //@@ TODO : multiple output event block
      txt=txt
    end
  end

endfunction


// *******************************************************

function [CCode,FCode]=gen_blocks()
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ gen_blocks : generates code for dynamically
//                linked Fortran and C blocks
//
// Output : CCode/FCode list, pair of name/routines contents
//

  //@@ initial empty lists
  CCode=list()
  FCode=list()

  //## look at for the modelica block
  kdyn=find(funtyp==10004);
  ind=[]
  if (size(kdyn,'*')>=1)
    for i=1:size(kdyn,'*')
      if type(corinv(kdyn(i)))==15 then
        ind=kdyn(i);
        break;
      end
    end
  end

  if ind<>[] then
    CCode($+1)=funs(ind)
    CCode($+1)=mgetl(TMPDIR+'/'+funs(ind)+'.c');
  end

  //## remove implicit number
  funtyp=modulo(funtyp,10000)

  kdyn=find(funtyp>1000) //dynamically linked blocs
                         //100X : Fortran blocks
                         //200X : C blocks

  //@@ many dynamical computational functions
  if (size(kdyn,'*') >1)
    kfuns=[];
    //get the block data structure in the initial scs_m structure
    if size(corinv(kdyn(1)),'*')==1 then
      O=scs_m.objs(corinv(kdyn(1)));
    else
      path=list('objs');
      for l=corinv(kdyn(1))(1:$-1)
        path($+1)=l;
        path($+1)='model';
        path($+1)='rpar';
        path($+1)='objs';
      end
      path($+1)=corinv(kdyn(1))($);
      O=scs_m(path);
    end
    if funtyp(kdyn(1))>2000 then
      //C block
      CCode($+1)=funs(kdyn(1))
      CCode($+1)=O.graphics.exprs(2)
    else
      //Fortran block
      FCode($+1)=funs(kdyn(1))
      FCode($+1)=O.graphics.exprs(2)
    end
    kfuns=funs(kdyn(1));
    for i=2:size(kdyn,'*')
      //get the block data structure in the initial scs_m structure
      if size(corinv(kdyn(i)),'*')==1 then
        O=scs_m.objs(corinv(kdyn(i)));
      else
        path=list('objs');
         for l=corinv(kdyn(i))(1:$-1)
           path($+1)=l;
           path($+1)='model';
           path($+1)='rpar';
           path($+1)='objs';
        end
        path($+1)=corinv(kdyn(i))($);
        O=scs_m(path);
      end
      if (find(kfuns==funs(kdyn(i))) == [])
        kfuns=[kfuns;funs(kdyn(i))];
        if funtyp(kdyn(i))>2000  then
          //C block
          CCode($+1)=funs(kdyn(i))
          CCode($+1)=O.graphics.exprs(2)
        else
          //Fortran block
          FCode($+1)=funs(kdyn(1))
          FCode($+1)=O.graphics.exprs(2)
        end
      end
    end
  //@@ one dynamical computational function
  elseif (size(kdyn,'*')==1)
    //get the block data structure in the initial scs_m structure
    if size(corinv(kdyn),'*')==1 then
      O=scs_m.objs(corinv(kdyn));
    else
      path=list('objs');
      for l=corinv(kdyn)(1:$-1)
        path($+1)=l;
        path($+1)='model';
        path($+1)='rpar';
        path($+1)='objs';
      end
      path($+1)=corinv(kdyn)($);
      O=scs_m(path);
    end
    if funtyp(kdyn)>2000 then
      //C block
      CCode($+1)=funs(kdyn)
      CCode($+1)=O.graphics.exprs(2)
    else
      //Fortran block
      FCode($+1)=funs(kdyn(1))
      FCode($+1)=O.graphics.exprs(2)
    end
  end

  //@@ A special case for block of type 4 or 10004
  //@@ that use get_scicos_time/do_cold_restart/get_phase_simulation
  //@@ is done here with the help of the function get_blk4_code
  //@@
  //@@ other routines of scicos blocks can also be included
  //@@
  //@@ warning we use cpr :
  if ALL then
    cod_include=[]
    for kf=1:size(cpr.sim.funtyp,1)
      if (cpr.sim.funtyp(kf)==4) | (cpr.sim.funtyp(kf)==10004) then
        if find(cpr.sim.funs(kf)==cod_include)==[] then
          cod=get_blk4_code(cpr.sim.funs(kf))
          if cod<>[] then
            for i=1:size(cod,1)
              if strindex(cod(i),'get_scicos_time')<>[] then
                cod_include=[cod_include;cpr.sim.funs(kf)]
                CCode($+1)=cpr.sim.funs(kf)
                CCode($+1)=cod
                break
              elseif strindex(cod(i),'do_cold_restart')<>[] then
                cod_include=[cod_include;cpr.sim.funs(kf)]
                CCode($+1)=cpr.sim.funs(kf)
                CCode($+1)=cod
                break
              elseif strindex(cod(i),'get_phase_simulation')<>[] then
                cod_include=[cod_include;cpr.sim.funs(kf)]
                CCode($+1)=cpr.sim.funs(kf)
                CCode($+1)=cod
                break
              end
            end
          end
        end
      end
    end
    //@@ extract dgelsy1
    for kf=1:lstsize(cpr.sim.funs)
      if cpr.sim.funs(kf)=='mat_bksl' | cpr.sim.funs(kf)=='mat_div' then
        cod=get_blk4_code('dgelsy1')
        CCode($+1)='dgelsy1'
        CCode($+1)=cod
        break
      end
    end
  end

  if CCode==list()
//     CCode($+1)=['void no_ccode()'
//                 '{'
//                 '  return;'
//                 '}']
  end
endfunction


// *******************************************************

function [ok,XX,gui_path,flgcdgen,szclkINTemp,freof,c_atomic_code,cpr]=do_compile_superblock42(all_scs_m,numk,atomicflag)
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ do_compile_superblock42 : transforms a given Scicos discrete and continuous
//                             SuperBlock into a C defined Block
//
// Input  : all_scs_m     :
//          numk          :
//          atomicflag    :
//
// Output : ok            :
//          XX            :
//          gui_path      :
//          flgcdgen      :
//          szclkINTemp   :
//          freof         :
//          c_atomic_code :
//          cpr           :
//

  act = [];
  cap = [];

  //******************* atomic blk **********
  [lhs,rhs] = argn(0)
  if rhs<3 then atomicflag=%f; end
  c_atomic_code=[];
  freof=[];
  flgcdgen=[];
  szclkINTemp=[];cpr=list();
  //*****************************************
  //## set void value for gui_path
  gui_path=[];

  if numk<>-1 then
    //## get the model of the sblock
    XX=all_scs_m.objs(numk);

    //## get the diagram inside the sblock
    scs_m=XX.model.rpar
  else
    XX=[]
    scs_m=all_scs_m;
  end

  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  //@@ check and set global variables
  if ~exists('ALL') then ALL=%f, end
  if ALL<>%f & ALL<>%t then ALL=%f, end

  //@@ list_of_scopes
  if ~exists('list_of_scopes') then
    list_of_scopes=[]
  end
  list_of_scopes=[list_of_scopes;
                  'bouncexy'  'ipar(1)';
                  'canimxy'   'ipar(1)';
                  'canimxy3d' 'ipar(1)';
                  'cevscpe'   'ipar(1)';
                  'cfscope'   'ipar(1)';
                  'cmat3d'    ''
                  'cmatview'  ''
                  'cmscope'   'ipar(1)';
                  'cscope'    'ipar(1)';
                  'cscopxy'   'ipar(1)';
                  'cscopxy3d' 'ipar(1)'
                  scs_m.codegen.scopes]
  //@@ sim_to_be_removed
  if ~exists('sim_to_be_removed') then
    sim_to_be_removed=[]
  end
  sim_to_be_removed=[sim_to_be_removed;
                     'cfscope' 'Floating Scope';
                     scs_m.codegen.remove]

  //@@ debug_cdgen
  debug_cdgen=scs_m.codegen.enable_debug

  //@@ silent_mode
  silent_mode=scs_m.codegen.silent

  //@@ option for standalone
  sta__#=scs_m.codegen.opt

  //@@ with_gui
  with_gui=scs_m.codegen.cblock

  //@@ cdgen_libs
  if ~exists('%scicos_libs') then
    %scicos_libs=[]
  end
  cdgen_libs=[%scicos_libs(:)',scs_m.codegen.libs(:)']

  //@@ set rdnom
  rdnom=scs_m.codegen.rdnom

  //@@ set rpath
  rpat=scs_m.codegen.rpat
  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  //@@ get parameter and name of diagram
  par=scs_m.props;
  hname=scs_m.props.title(1)

  //***********************************************************
  //Check blocks properties and adapt them if necessary
  //***********************************************************


  //**  These blocks are not ALLOWED for Emb code generation 
  vorbidden_items=["CLKOUT_f","activation (events) output ports";
                   "IN_f","input ports";
                   "OUT_f","output ports";
                   "CLKOUTV_f","activation outputs";
                   "CLOCK_c","clocks";
                   "CLOCK_f","clocks";
                   "SampleCLK","clocks";
                   "rtai_ext_clock","clocks";
                   "RFILE_f","Read block";
                   "READC_f","Read_block";
                   "WFILE_f","Write block";
                   "WRITEC_f","Write block"]

  clkIN = [];
  
  //** scan Problem with incapsulated Superblocks
  for i=1:size(scs_m.objs)

    //** BLOCKS 
    if typeof(scs_m.objs(i))=="Block" then
      ind=find(vorbidden_items==scs_m.objs(i).gui);
      if(ind~=[]) then
        ok = %f ;
        %cpr = list();
        message(vorbidden_items(ind(1),2)+" not allowed in Superblock");
        return; // EXIT point
      
      elseif scs_m.objs(i).gui=="CLKINV_f" then //** input clock from external diagram 
        //** replace event input ports by  fictious block
        scs_m.objs(i).gui="EVTGEN_f";
        scs_m.objs(i).model.sim(1)="bidon"
        if clkIN==[] then
          clkIN = 1;
        else
          ok = %f;
          %cpr = list();
          message("Only one activation block allowed!");
          return; // EXIT point
        end
      end
    end
  end
 
  szclkIN = size(clkIN,2);

  flgcdgen=szclkIN

  //## overload some functions used
  //## in modelica block compilation
  //## disable it for codegeneration
  //   %mprt=funcprot()
  //   funcprot(0)
  //   deff('[ok] =  buildnewblock(blknam,files,filestan,filesint,libs,rpat,ldflags,cflags)','ok=%t')
  //   funcprot(%mprt)

  //## first pass of compilation
  if ~ALL then
    [bllst,connectmat,clkconnect,cor,corinv,ok,flgcdgen,freof]=c_pass1(scs_m,flgcdgen);
  else
    [bllst,connectmat,clkconnect,cor,corinv,ok,flgcdgen,freof]=c_pass1(scs_m);
    szclkINTemp=flgcdgen;
  end

  //## restore buildnewblock
  clear buildnewblock

  if ~ok then
    %cpr=list()
    error('Sorry: problem in the pre-compilation step.')
  end

  //@@ adjust scope win number if needed
  [bllst,ok]=adjust_id_scopes(list_of_scopes,bllst)
  if ~ok then
    %cpr=list()
    error('Problem adjusting scope id number.')
  end

  //###########################//
  //## Detect implicit block ##//

  //## Force here generation of implicit block
  if scs_m.props.tol(6)==100 & ALL then
    impl_blk = %t;
    %scicos_solver=100;
  else
    impl_blk = %f;
    for blki=bllst
      if type(blki.sim)==15 then
        if blki.sim(2)>10000 then
          impl_blk = %t;
          %scicos_solver=100;
          break;
        end
      end
    end
  end
  //###########################//

  //@@ inform flgcdgen
  if ~ALL then
    if flgcdgen<> szclkIN
      clkIN=[clkIN flgcdgen]
    end
    szclkINTemp=szclkIN;
    szclkIN=flgcdgen;
  end

  for i=1:size(bllst)
    if (bllst(i).sim(1)=="bidon") then //** clock input 
      howclk = i;
    end
  end

  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  //@@ remove problematic scopes
  for i=1:lstsize(bllst)
    ind = find(bllst(i).sim(1)==sim_to_be_removed(:,1))
    if ind<>[] then
      mess=[sim_to_be_removed(ind,2)+' block is not allowed.' ;
            'It will be not called.';];
      if ~silent_mode then
        okk=message(mess,['Ok';'Go Back'])
      else
        disp(mess);
        okk=1
      end
      if okk==1 then
        bllst(i).sim(1)='bidon'
        if type(bllst(i).sim(1))==13 then
          bllst(i).sim(2)=0
        end
      else
        ok=%f
        %cpr=list()
        return
      end
    end
  end
  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

//  FIRING=[]
//  for i=1:size(allhowclk2,1)
//    j = find(clkconnect(:,3)==allhowclk2(i))
//    if j<>[] then
//      FIRING=[FIRING;bllst(clkconnect(j,1)).firing(clkconnect(j,2))]
//    end
//  end

//  Code_gene_run=[];
  //******************** To avoid asking for size or type more than one time in incidence_mat*******
  //************************** when creating atomicity *********************************************
  //************************** in other cases it can be done in adjust_all_scs_m *******************
  c_pass2=c_pass2;
  if ok then
    [ok,bllst]=adjust_inout(bllst,connectmat);
  end

  if ok then
    [ok,bllst]=adjust_typ(bllst,connectmat);
  end
  //*************************************************************************************************
  //## second pass of compilation
  cpr=c_pass2(bllst,connectmat,clkconnect,cor,corinv,"silent")

  if cpr==list() then
    ok=%f,
    error("Problem compiling; perhaps an algebraic loop.");
  end

  // computing the incidence matrix to derive actual block's depu
  [depu_mat,ok]=incidence_mat(bllst,connectmat,clkconnect,cor,corinv)
  if ~ok then
    //rmk : incidence_mat seems to return allways ok=%t
    return
  else
    depu_vec=depu_mat*ones(size(depu_mat,2),1)>0
  end

  //## Detect synchro block and type1
  funs_save   = cpr.sim.funs;
  funtyp_save = cpr.sim.funtyp;
  with_work   = zeros(cpr.sim.nb,1)
  with_synchro = %f
  with_nrd     = %f
  with_type1   = %f

  //## loop on blocks
  for i=1:lstsize(cpr.sim.funs)
    //## look at for funs of type string
    if type(cpr.sim.funs(i))==10 then
      if part(cpr.sim.funs(i),1:10)=='actionneur' then
        cpr.sim.funs(i) ='bidon'
        cpr.sim.funtyp(i) = 0
      elseif part(cpr.sim.funs(i),1:7)=='capteur' then
        cpr.sim.funs(i) ='bidon'
        cpr.sim.funtyp(i) = 0
      elseif cpr.sim.funs(i)=='bidon2' then
        cpr.sim.funs(i) ='bidon'
        cpr.sim.funtyp(i) = 0
      elseif cpr.sim.funs(i)=='agenda_blk' then
        cpr.sim.funs(i) ='bidon'
        cpr.sim.funtyp(i) = 0
      elseif cpr.sim.funs(i)=='affich' then
        cpr.sim.funs(i) ='bidon'
        cpr.sim.funtyp(i) = 0
      elseif cpr.sim.funs(i)=='affich2' then
        cpr.sim.funs(i) ='bidon'
        cpr.sim.funtyp(i) = 0
      end
    end
    //## look at for type of block
    if cpr.sim.funtyp(i) < 0 then
       with_synchro = %t //## with_synchro flag comes global
    elseif cpr.sim.funtyp(i) == 0 then
       with_nrd = %t //## with_nrd flag comes global
    elseif cpr.sim.funtyp(i) == 1 then
       if cpr.sim.funs(i) ~= 'bidon' then
         with_type1 = %t //## with_type1 flag comes global
       end
    end
  end //## end of for

  //**** solve which blocks use work ****//
  BeforeCG_WinList = winsid();

  //  ierr=execstr('[state,t]=scicosim(cpr.state,0,0,cpr.sim,'+..
  //               '''start'',scs_m.props.tol)','errcatch')
  ierr = 1
  //@@ save initial outtb
  if ierr==0
    outtb_init = state.outtb;
  else
    with_work = ones(cpr.sim.nb,1)
    outtb_init = cpr.state.outtb
    state = cpr.state
    ierr = 0;
  end


  //** retrieve all open ScicosLab windows with winsid()
  AfterCG_WinList = winsid();
  if ierr==0 then
    for i=1:cpr.sim.nb
       if state.iz(i)<>0 then
          with_work(i)=%t
       end
    end

  //  ierr=execstr('[state,t]=scicosim(state,0,0,cpr.sim,'+..
  //               '''finish'',scs_m.props.tol)','errcatch')
  end

  //@@ remove windows opened by simulation
  xdel(setdiff(AfterCG_WinList,BeforeCG_WinList))

  //*************************************//

  //@@ retrieve original funs name
  cpr.sim.funs=funs_save;
  cpr.sim.funtyp=funtyp_save;

  //@@ add a work ptr for agenda blk
  for i=cpr.sim.nb:-1:1
    if cpr.sim.funs(i)=='agenda_blk' then
      with_work(i)=%t
      break
    end
  end

  //@@ cpr ptrs declaration
  x=cpr.state.x;
  z=cpr.state.z;
  //outtb=cpr.state.outtb;
  outtb=outtb_init;
  zcptr=cpr.sim.zcptr;
  ozptr=cpr.sim.ozptr;
  rpptr=cpr.sim.rpptr;
  ipptr=cpr.sim.ipptr;
  opptr=cpr.sim.opptr;
  funs=cpr.sim.funs;
  xptr=cpr.sim.xptr;
  zptr=cpr.sim.zptr;
  inpptr=cpr.sim.inpptr;
  inplnk=cpr.sim.inplnk;
  outptr=cpr.sim.outptr;
  outlnk=cpr.sim.outlnk;
  ordclk=cpr.sim.ordclk;
  funtyp=cpr.sim.funtyp;
  cord=cpr.sim.cord;
  ncord=size(cord,1);
  nblk=cpr.sim.nb;
  ztyp=cpr.sim.ztyp;
  clkptr=cpr.sim.clkptr
  //taille totale de z : nztotal
  nztotal=size(z,1);
  //taille totale de x : nxtotal
  nxtotal=size(x,'*');

  //*******************************
  //Checking if superblock is valid
  //*******************************
  msg=[]
  for i=1:length(funs)-1
    if funtyp(i)==3 | funtyp(i)==5 then
      msg=[msg;'Scilab block''s not allowed.']
    elseif (clkptr(i+1)-clkptr(i))<>0 &funtyp(i)>-1 &funs(i)~='bidon' then
//      //msg=[msg;'Regular block generating activation not allowed yet']
    end
    if msg<>[] then
      ok=%f
      error(msg)
    end
  end

  //********************************************************
  // Change logical units for readf and writef blocks if any
  //********************************************************
  lunit=0
  for d=1:length(funs)
    if funs(d)=='readf'  then
      z(zptr(d)+2)=lunit
      lunit=lunit+1;
    elseif funs(d)=='writef'
      z(zptr(d)+1)=lunit
      lunit=lunit+1;
    end
  end

  //** Find the clock connected to the SuperBlock and retreive
  //** the sampling time
  //** Modified for use with external clock by Henrik Slotholt
  
  useInternTimer = 1;
  extClockCode = ['void rtextclk(void) { }']

  if XX.graphics.pein==[] | XX.graphics.pein(1)==0 then
    sTsamp="0.001"; //** default value is ONE millisecond 
  else
    o_ev = XX.graphics.pein(1);
    o_ev=all_scs_m.objs(o_ev).from(1);

    while (all_scs_m.objs(o_ev).gui~='CLOCK_c' & ...
           all_scs_m.objs(o_ev).gui~='CLOCK_f' & ...
           all_scs_m.objs(o_ev).gui~='SampleCLK' & ...
           all_scs_m.objs(o_ev).gui~='rtai_ext_clock')

               o_ev = all_scs_m.objs(o_ev).graphics.pein(1);
               o_ev = all_scs_m.objs(o_ev).from(1);

    end

    if all_scs_m.objs(o_ev).gui=='SampleCLK' then
      sTsamp=all_scs_m.objs(o_ev).model.rpar(1);
      sTsamp=sci2exp(sTsamp);
      Tsamp_delay=all_scs_m.objs(o_ev).model.rpar(2);
      Tsamp_delay=sci2exp(Tsamp_delay);
    elseif all_scs_m.objs(o_ev).gui=='rtai_ext_clock' then
      sTsamp=all_scs_m.objs(o_ev).graphics.exprs(1);
      sTsamp=sci2exp(eval(sTsamp));
      Tsamp_delay="0.0";
      useInternTimer = 0;
      extClockCode = all_scs_m.objs(o_ev).graphics.exprs(2);
    else
      sTsamp=all_scs_m.objs(o_ev).model.rpar.objs(2).graphics.exprs(1);
      sTsamp=sci2exp(eval(sTsamp));
      Tsamp_delay=all_scs_m.objs(o_ev).model.rpar.objs(2).graphics.exprs(2);
      Tsamp_delay=sci2exp(eval(Tsamp_delay));
    end
  end

  //***********************************
  // Get the name of the file
  //***********************************
  okk = %f; 
  rdnom=hname; 
  rpat = getcwd(); 
  archname=''; 
  Tsamp = sci2exp(eval(sTsamp));
  can_flag = %f;
  
  if XX.model.rpar.props.void3 == [] then
	target = 'rtai'; //** default compilation chain 
        template = ''; //** default values for this version 
	odefun = 'ode4';  //** default solver 
	odestep = '10';   //** default continous step size 
  elseif size(XX.model.rpar.props.void3,2)==3 then
	target = 'rtai'; //** default compilation chain 
        template = ''; //** default target board
	odefun = 'ode4';  //** default solver 
	odestep = '10';   //** default continous step size 
  else  
	target   = XX.model.rpar.props.void3(1); //** user defined parameters
	template = XX.model.rpar.props.void3(2);
	odefun   = XX.model.rpar.props.void3(3);
	odestep  = XX.model.rpar.props.void3(4);
  end

  ode_x=['ode1';'ode2';'ode4']; //** available continous solver 
  libs='';

  while %t do
    ok=%t  // to avoid infinite loop

    //@@ check name of block for space,'-','_'
    if grep(rdnom," ")<>[] | grep(rdnom,"-")<>[]  | grep(rdnom,".")<>[] then
      rdnom = strsubst(rdnom,' ','_');
      rdnom = strsubst(rdnom,'-','_');
      rdnom = strsubst(rdnom,'.','_');
      rdnom = strsubst(rdnom,"",'_');
    end

    //** dialog box default variables 
    label1=[rdnom;getcwd()+'/'+rdnom+"_scig";target;template];
    label2=[rdnom;getcwd()+'/'+rdnom+"_scig";target;template;odefun;odestep];
  
    if x==[] then
      //** Pure discrete system NO CONTINOUS blocks 
     
      [okk, rdnom, rpat,target,template,label1] = getvalue(..
	            'Embedded Code Generation',..
		        ['New block''s name :';
		         'Created files Path:';
			 'Toolchain: ';
			 'Target Board: '],..
		         list('str',1,'str',1,'str',1,'str',1),label1);
    else
      //** continous blocks are presents
      [okk,rdnom,rpat,target,template,odefun,odestep,label2] = getvalue(..
	            "Embedded Code Generation",..
		        ["New block''s name: "  ;
		         "Created files Path: " ;
			 "Toolchain: "          ;
			 "Target Board: "       ;
		         "ODE solver type: "       ;
		         "ODE solver steps betw. samples: "],..
		         list('str',1,'str',1,'str',1,'str',1,'str',1,'str',1),label2);
    end

    if okk==%f then
      ok = %f
      return ; //** EXIT point 
    end
    rpat = stripblanks(rpat);


    //** I put a warning here in order to inform the user
    //** that the name of the superblock will change
    //** because the space char in name isn't allowed.
    if grep(rdnom," ")<>[] then
      message(['Superblock name cannot contains space characters.';
               'space chars will be automatically substituted by ""_"" '])
    end
    rdnom = strsubst(rdnom,' ','_');

    //** Put a warning here in order to inform the user
    //** that the name of the superblock will change
    //** because the "-" char could generate GCC problems
    //** (the C functions contains the name of the superblock).
    if grep(rdnom,"-")<>[] then
      message(['For full C compiler compatibility ';
               'Superblock name cannot contains ""-"" characters';
               '""-"" chars will be automatically substituted by ""_"" '])
    end

    rdnom = strsubst(rdnom,'-','_'); 

    dirinfo = fileinfo(rpat)
    
    if dirinfo==[] then
      [pathrp, fnamerp, extensionrp] = fileparts(rpat); 
      ok = mkdir(pathrp, fnamerp+extensionrp) ; 
      if ~ok then 
        message("Directory '+rpat+' cannot be created");
      end
    elseif filetype(dirinfo(2))<>'Directory' then
      ok = %f;
      message(rpat+" is not a directory");
    end

    if stripblanks(rdnom)==emptystr() then 
      ok = %f;
      message("Sorry: C file name not defined");
    end


    //** This comments will be moved in the documentation 

    //** /contrib/RT_templates/pippo.gen

    //** 1: pippo.mak 
    //** 2: pippo.cmd

    //** pippo.mak : scheletro del Makefile 
    //**             - GNU/Linux : Makefile template
    //**             - Windows/Erika : conf.oil
    //**                               erika.cmd
 
    //** pippo.cmd : sequenza di comandi Scilab 


    TARGETDIR = SCI+"/contrib/RT_templates";


    [fd,ierr] = mopen(TARGETDIR+'/'+target+'.gen','r');

    if ierr==0 then
      mclose(fd);
    else
      ok = %f;
      message("Target not valid " + target + ".gen");
    end
    
    if ok then
      target_t = mgetl(TARGETDIR+'/'+target+'.gen');
      makfil = target_t(1);
      cmdfil = target_t(2);

      [fd,ierr]=mopen(TARGETDIR+'/'+makfil,'r');
      if ierr==0 then
        mclose(fd);
      else
        ok = %f ;
        message("Makefile not valid " + makfil);
      end
    end

    if x ~= [] then
      if grep(odefun,ode_x) == [] then
         message("Ode function not valid");
         ok = %f;
      end
    end

    if ok then break,end
  end

//------------------ The real code generation is here ------------------------------------

  //************************************************************************
  //generate the call to the blocks and blocs simulation function prototypes
  //************************************************************************
  wfunclist = list();
  nbcap = 0;
  nbact = 0;
  capt  = [];
  actt  = [];
  Protostalone = [];
  Protos       = [];
  dfuns        = [] ;
  


  //** scan the data structure and call the generating functions 
  //** Substitute previous code!!!!
  
  for i=1:length(funs)
    ki= find(funs(i) == dfuns) ; //** 
    dfuns = [dfuns; funs(i)] ; 
      
    if ki==[] then
      Protostalone=[Protostalone;'';BlockProto(i)];
    end
  end


  //***********************************
  // Scilab and C files generation
  //***********************************

  cmdseq = mgetl(TARGETDIR+'/' + cmdfil);
  n_cmd = size(cmdseq,1);

  for i=1:n_cmd
  
    if (cmdseq(i)~="") then
         disp("Executing " + """" +cmdseq(i)+ """" + '...'); 
    end;
    
    execstr(cmdseq(i));
  
  end

  disp("----> Target generation terminated!");

endfunction

// *******************************************************

function [txt]=BlockProto(bk)
//Copyright (c) 1989-2010 Metalau project INRIA

//BlockProto : generate prototype for a calling C sequence
//             of a scicos computational function
//
// Input :  bk   : block index in cpr
//
// Output : txt  : the generated prototype
//

  nin=inpptr(bk+1)-inpptr(bk);  //* number of input ports */
  nout=outptr(bk+1)-outptr(bk); //* number of output ports */
  funs_bk=funs(bk); //* name of the computational function */
  funtyp_bk=funtyp(bk); //* type of the computational function */
  ztyp_bk=ztyp(bk); //* zero crossing type */

  //&& call make_BlockProto
  txt=make_BlockProto(nin,nout,funs_bk,funtyp_bk,ztyp_bk,bk)

endfunction


// *******************************************************

function [txt]=make_BlockProto(nin,nout,funs_bk,funtyp_bk,ztyp_bk,bk)
//Copyright (c) 1989-2010 Metalau project INRIA

//make_BlockProto : generate prototype for a calling C sequence
//                  of a scicos computational function
//
// Input :  nin       : number of input ports
//          nout      : number of output ports
//          funs_bk   : name of the computational function
//          funtyp_bk : type of the computational function
//          ztyp_bk   : say if block has zero crossings
//          bk        : an index number
//
// Output : txt  : the generated prototype
//

  //## check and adjust function type
  ftyp=funtyp_bk
  ftyp_tmp=modulo(funtyp_bk,10000)
  if ftyp_tmp>2000 then
    ftyp=ftyp-2000
  elseif ftyp_tmp>1000 then
    ftyp=ftyp-1000
  end

  //** check function type
  if ftyp < 0 then //** ifthenelse eselect blocks
      txt = [];
      return;
  else
    if (ftyp<>0 & ftyp<>1 & ftyp<>2 & ftyp<>3 & ftyp<>4  & ftyp<>10004) then
      disp("types other than 0,1,2,3 or 4/10004 are not yet supported.")
      txt = [];
      return;
    end
  end

  //@@ agenda_blk
  if funs_bk=='agenda_blk' then
    txt=[]
    return;
  end

  //** add comment
  txt=[get_comment('proto_blk',list(funs_bk,funtyp_bk,bk,ztyp_bk));]

  select ftyp
    //** zero funtyp
    case 0 then

      //*********** prototype definition ***********//
      txtp=['(int *, int *, double *, double *, double *, int *, double *, \';
            ' int *, double *, int *, double *, int *,int *, int *, \';
            ' double *, int *, double *, int *);'];
      if (funtyp_bk>2000 & funtyp_bk<3000)
        blank = get_blank('void '+funs_bk+'(');
        txtp(1) = 'void '+funs_bk+txtp(1);
      elseif (funtyp_bk<2000)
        //@@ special case for andlog func : should be fixed in scicos
        if funs_bk <> 'andlog' then
          txtp(1) = 'void C2F('+funs_bk+')'+txtp(1);
          blank = get_blank('void C2F('+funs_bk+')');
        else
          blank = get_blank('void '+funs_bk+'(');
          txtp(1) = 'void '+funs_bk+txtp(1);
        end
      end
      txtp(2:$) = blank + txtp(2:$);
      txt = [txt;txtp];
      //*******************************************//


    //**
    case 1 then

      //*********** prototype definition ***********//
      txtp=['(int *, int *, double *, double *, double *, int *, double *, \';
            ' int *, double *, int *, double *, int *,int *, int *';]
      if (funtyp_bk>2000 & funtyp_bk<3000)
        blank = get_blank('void '+funs_bk+'(');
        txtp(1) = 'void '+funs_bk+txtp(1);
      elseif (funtyp_bk<2000)
        txtp(1) = 'void C2F('+funs_bk+')'+txtp(1);
        blank = get_blank('void C2F('+funs_bk+')');
      end
      if nin>=1 | nout>=1 then
        txtp($)=txtp($)+', \'
        txtp=[txtp;'']
        if nin>=1 then
          for k=1:nin
            txtp($)=txtp($)+' double *, int * ,'
          end
          txtp($)=part(txtp($),1:length(txtp($))-1); //remove last ,
        end
        if nout>=1 then
          if nin>=1 then
            txtp($)=txtp($)+', \'
            txtp=[txtp;'']
          end
          for k=1:nout
            txtp($)=txtp($)+' double *, int * ,'
          end
          txtp($)=part(txtp($),1:length(txtp($))-1); //remove last ,
        end
      end

      if ztyp_bk then
        txtp($)=txtp($)+', \'
        txtp=[txtp;' double *,int *);'];
      else
        txtp($)=txtp($)+');';
      end

      txtp(2:$) = blank + txtp(2:$);
      txt = [txt;txtp];
      //*******************************************//

    //**
    case 2 then

      //*********** prototype definition ***********//

      txtp=['void '+funs_bk+...
            '(int *, int *, double *, double *, double *, int *, double *, \';
            ' int *, double *, int *, double *, int *, int *, int *, \'
            ' double **, int *, int *, double **,int *, int *'];
      if ~ztyp_bk then
        txtp($)=txtp($)+');';
      else
        txtp($)=txtp($)+', \';
        txtp=[txtp;
              ' double *,int *);']
      end
      blank = get_blank('void '+funs_bk);
      txtp(2:$) = blank + txtp(2:$);
      txt = [txt;txtp];
      //********************************************//

    //**
    case 4 then
      txt=[txt;
           'void '+funs_bk+'(scicos_block *, int );'];

    //**
    case 10004 then
      txt=[txt;
           'void '+funs_bk+'(scicos_block *, int );'];

  end

endfunction


// *******************************************************

function txt=make_rtai_params()

  txt=[''];

  //*** Real parameters ***//
  nbrpa=0;strRCode='';lenRCode=[];ntot_r=0;
  if size(rpar,1) <> 0 then
    txt=[txt;
	 '/* def real parameters */'
         'double *RPAR[ ] = {'];

    for i=1:(length(rpptr)-1)
      if rpptr(i+1)-rpptr(i)>0  then

        idPrefix=''
        //## Modelica block
        if type(corinv(i))==15 then
          //## we can extract here all informations
          //## from original scicos blocks with corinv : TODO
          Code($+1)='  /* MODELICA BLK RPAR COMMENTS : TODO */';
        else
          if size(corinv(i),'*')==1 then
            OO=scs_m.objs(corinv(i));
          else
            path=list('objs');
            for l=cpr.corinv(i)(1:$-1)
              path($+1)=l;
              OO=scs_m(path);
              if stripblanks(OO.graphics.id)~=emptystr() then
                idPrefix = idPrefix + string(OO.graphics.id) + '/';
              end
              path($+1)='model';
              path($+1)='rpar';
              path($+1)='objs';
            end
            path($+1)=cpr.corinv(i)($);
            OO=scs_m(path);
          end
        end
     
        //** Add comments **//
        nbrpa=nbrpa+1;
        ntot_r = ntot_r + (rpptr(i+1)-rpptr(i));

        if stripblanks(OO.graphics.id)~=emptystr() then
          str_id = idPrefix + string(OO.graphics.id);
        else
          str_id = idPrefix + 'RPARAM[' + string(nbrpa) +']';
        end

        txt=[txt;
             '  rpar_'+string(i)+','
            ]
	      strRCode = strRCode + '""' + str_id + '"",';
	      lenRCode = lenRCode + string(rpptr(i+1)-rpptr(i)) + ',';
      end
    end
    txt=[txt;
           '};']
  else
    txt($+1)='double *RPAR[1]={NULL};';
  end

  txt = [txt;
         '';
         '#ifdef linux';
        ]
  txt($+1) = 'int NRPAR = '+string(nbrpa)+';';
  txt($+1) = 'int NTOTRPAR = '+string(ntot_r)+';';
    
  strRCode = 'char * strRPAR[' + string(nbrpa) + '] = {' + ..
             part(strRCode,[1:length(strRCode)-1]) + '};';

  if nbrpa <> 0 then
    txt($+1) = strRCode;
    lenRCode = 'int lenRPAR[' + string(nbrpa) + '] = {' + ..
               part(lenRCode,[1:length(lenRCode)-1]) + '};';
  else
     txt($+1) = 'char * strRPAR;'
     lenRCode = 'int lenRPAR[1] = {0};'
  end
  txt($+1) = lenRCode;
  txt = [txt;
         '#endif';
         '';
        ]

  //***********************//

  //*** Integer parameters ***//
  nbipa=0;strICode='';lenICode=[];ntot_i=0;
  if size(ipar,1) <> 0 then
    txt=[txt;
           '/* def integer parameters */'
           'int *IPAR[ ] = {'];

    for i=1:(length(ipptr)-1)
      if ipptr(i+1)-ipptr(i)>0  then
        idPrefix='';
        //## Modelica block
        if type(corinv(i))==15 then
          //## we can extract here all informations
          //## from original scicos blocks with corinv : TODO
          Code($+1)='  /* MODELICA BLK IPAR COMMENTS : TODO */';
        else
          if size(corinv(i),'*')==1 then
            OO=scs_m.objs(corinv(i));
          else
            path=list('objs');
            for l=cpr.corinv(i)(1:$-1)
              path($+1)=l
              OO=scs_m(path);
              if stripblanks(OO.graphics.id)~=emptystr() then
                idPrefix = idPrefix + string(OO.graphics.id) + '/';
              end
              path($+1)='model'
              path($+1)='rpar'
              path($+1)='objs'
            end
            path($+1)=cpr.corinv(i)($);
            OO=scs_m(path);
          end
        end

        //** Add comments **//
        nbipa=nbipa+1;
        ntot_i = ntot_i + (ipptr(i+1)-ipptr(i));

        if stripblanks(OO.graphics.id)~=emptystr() then
          str_id = idPrefix + string(OO.graphics.id);
        else
          str_id = idPrefix + 'IPARAM[' + string(nbipa) +']';
        end

        txt=[txt;
             '  ipar_'+string(i)+','
            ]
        strICode = strICode + '""' + str_id + '"",';
        lenICode = lenICode + string(ipptr(i+1)-ipptr(i)) + ',';
      end
    end
    txt=[txt;
         '};']
  else
    txt($+1)='int *IPAR[1]={NULL};';
  end

  txt = [txt;
         '';
         '#ifdef linux';
        ]
  txt($+1) = 'int NIPAR = '+string(nbipa)+';';
  txt($+1) = 'int NTOTIPAR = '+string(ntot_i)+';';

  strICode = 'char * strIPAR[' + string(nbipa) + '] = {' + ..
             part(strICode,[1:length(strICode)-1]) + '};';

  if nbipa <> 0 then
     txt($+1) = strICode;
     lenICode = 'int lenIPAR[' + string(nbipa) + '] = {' + ..
                part(lenICode,[1:length(lenICode)-1]) + '};';
  else
     txt($+1) = 'char * strIPAR;'
     lenICode = 'int lenIPAR[1] = {0};'
  end
  txt($+1) = lenICode;
  txt = [txt;
         '#endif';
         '';
        ]
endfunction

// *******************************************************

function [Code, Code_common]=make_standalonert()
//Copyright (c) 1989-2010 Metalau project INRIA

// Modified for RTAI by Roberto Bucher roberto.bucher@supsi.ch

//@@ make_standalone42() : generates code of the standalone simulation procedure
//
// Output : Code : text of the generated routines
//
// rmk : zdoit is not used
//
Code_common= []

  x=cpr.state.x;
  modptr=cpr.sim.modptr;
  rpptr=cpr.sim.rpptr;
  ipptr=cpr.sim.ipptr;
  opptr=cpr.sim.opptr;
  rpar=cpr.sim.rpar;
  ipar=cpr.sim.ipar;
  opar=cpr.sim.opar;
  oz=cpr.state.oz;
  ordptr=cpr.sim.ordptr;
  oord=cpr.sim.oord;
  zord=cpr.sim.zord;
  iord=cpr.sim.iord;
  tevts=cpr.state.tevts;
  evtspt=cpr.state.evtspt;
  zptr=cpr.sim.zptr;
  ozptr=cpr.sim.ozptr;
  clkptr=cpr.sim.clkptr;
  ordptr=cpr.sim.ordptr;
  pointi=cpr.state.pointi;
  funs=cpr.sim.funs;
  funtyp=cpr.sim.funtyp;
  noord=size(cpr.sim.oord,1);
  nzord=size(cpr.sim.zord,1);
  niord=size(cpr.sim.iord,1);

  Indent='  ';
  Indent2=Indent+Indent;
  BigIndent='          ';

  nX=size(x,'*');

  stalone = %t;

  //** evs : find source activation number
  //## with_nrd2 : find blk type 0 (wihtout)
  blks=find(funtyp>-1);
  evs=[];

  with_nrd2=%f;
  if ~ALL then
    for blk=blks
      for ev=clkptr(blk):clkptr(blk+1)-1
        if funs(blk)=='bidon' then
          if ev > clkptr(howclk) -1
           evs=[evs,ev];
          end
        end
      end

      //## all blocks without sensor/actuator
      if (part(funs(blk),1:7) ~= 'capteur' &...
          part(funs(blk),1:10) ~= 'actionneur' &...
          funs(blk) ~= 'bidon') then
        //## with_nrd2 ##//
        if funtyp(blk)==0 then
          with_nrd2=%t;
        end
      end
    end
  else
    for blk=blks
      for ev=clkptr(blk):clkptr(blk+1)-1
        if funs(blk)=='agenda_blk' then
          nb_agenda_blk=blk
          //if ev > clkptr(howclk) -1
          evs=[evs,ev];
         //end
        end
      end
      //## all blocks without sensor/actuator
      if (part(funs(blk),1:7) ~= 'capteur' &...
          part(funs(blk),1:10) ~= 'actionneur' &...
          funs(blk) ~= 'bidon') then
        //## with_nrd2 ##//
        if funtyp(blk)==0 then
          with_nrd2=%t;
        end
      end
    end
  end

  Code=['/* Code prototype for standalone use  */'
        '/*     Generated by Code_Generation toolbox of Scicos with '+ ..
        get_scicos_version()+' */'
        '/*     date : '+date()+' */'
        ''
        '/* Copyright (c) 1989-2010 Metalau project INRIA */'
	'/* Code generation modified by Roberto Bucher */'
        ''
        '/* ---- Headers ---- */'
        '#include <stdio.h>'
        '#include <stdlib.h>'
        '#include <math.h>'
        '#include <string.h>'
        '#include <memory.h>'
        '#include <scicos_block4.h>'
        '#include <machine.h>'
        ''
	'#ifdef linux'
	'#define __CONST__ static'
	'#else'
	'#define __CONST__ static const'
	'#endif'
	''
	'double '+rdnom+'_get_tsamp()'
	'{'
	'  return(' + string(Tsamp) + ');'
	'}'
	''
	'double '+rdnom+'_get_tsamp_delay()'
        '{'
	'  return(' + string(Tsamp_delay) + ');'
	'}'
	''
        ]

// Modified for use with external clocks by Henrik Slotholt
  Code=[Code
        '/* ---- Clock code ---- */'
        'int '+rdnom+'_useInternTimer(void) {'
        '	return '+string(useInternTimer)+';'
        '}'
        ''
        extClockCode
        ''
        ]

  Code=[Code
        '/* ---- Internals functions declaration ---- */'
        'int '+rdnom+'_init(void);'
        'int '+rdnom+'_isr(double);'
        'int '+rdnom+'_end(void);'
        Protostalone
        '']

  if x<>[] then
    if impl_blk then
      Code=[Code
            '/* ---- Solver functions prototype for standalone use ---- */'
            'int '+rdnom+'simblk_imp(double , double *, double *, double *);'
            'int dae1();'
            'int dae2();'
            'int dae4();'
            '']
    else
      Code=[Code
            '/* ---- Solver functions prototype for standalone use ---- */'
            'int '+rdnom+'simblk(double , double *, double *);'
            'int ode1();'
            'int ode2();'
            'int ode4();'
            '']
    end
  end

  //*** Continuous state ***//
  if x <> [] then
    //## implicit block
    if impl_blk then
      Code=[Code;
            '/* def number of continuous state */'
            '#define NEQ '+string(nX/2)
            '']
    //## explicit block
    else
      Code=[Code;
            '/* def number of continuous state */'
            '#define NEQ '+string(nX)
            '']
    end
  end

  Code=[Code;
        '/* def phase sim variable */'
	'extern int phase;'
        '/* block_error must be pass in argument of _sim function */'
        'extern int *block_error;'
        '/* block_number */'
        'extern int block_number;'
        '']

  if impl_blk then
    Code=[Code;
          '/* Jacobian parameters */'
          'int Jacobian_Flag;'
          'double CI, CJ, CJJ;'
          'double SQuround;'
          '']
  end

  //## rmk: we can remove a 'bidon' structure
  //## sometimes at the end
  if funs(nblk)=='bidon' then nblk=nblk-1, end;
  
   Code=[Code
        '/* declaration of scicos block structures */'
        'scicos_block block_'+rdnom+'['+string(nblk)+'];'
        '']

  //### Real parameters ###//
  if size(rpar,1) <> 0 then
    Code=[Code;
          '  /* Real parameters declaration */']
          //'static double RPAR1[ ] = {'];

    for i=1:(length(rpptr)-1)
      if rpptr(i+1)-rpptr(i)>0  then

        //** Add comments **//

        //## Modelica block
        if type(corinv(i))==15 then
          //## we can extract here all informations
          //## from original scicos blocks with corinv : TODO
          Code($+1)='  /* MODELICA BLK RPAR COMMENTS : TODO */';
        else
          if size(corinv(i),'*')==1 then
            OO=scs_m.objs(corinv(i));
          else
            path=list('objs');
            for l=cpr.corinv(i)(1:$-1)
              path($+1)=l;
              path($+1)='model';
              path($+1)='rpar';
              path($+1)='objs';
            end
            path($+1)=cpr.corinv(i)($);
            OO=scs_m(path);
          end

          Code($+1)='  /* Routine name of block: '+strcat(string(cpr.sim.funs(i)));
          Code($+1)='   * Gui name of block: '+strcat(string(OO.gui));
          Code($+1)='   * Compiled structure index: '+strcat(string(i));

          if stripblanks(OO.model.label)~=emptystr() then
            Code=[Code;
                  '  '+cformatline(' * Label: '+strcat(string(OO.model.label)),70)];
          end
          if stripblanks(OO.graphics.exprs(1))~=emptystr() then
            Code=[Code;
                  '  '+cformatline(' * Exprs: '+strcat(OO.graphics.exprs(1),","),70)];
          end
          if stripblanks(OO.graphics.id)~=emptystr() then
            Code=[Code;
                  '  '+cformatline(' * Identification: '+strcat(string(OO.graphics.id)),70)];
          end
          //txt=[txt;' * rpar='];
          Code($+1)='   */';
        end
        //******************//

        txt=cformatline(strcat(msprintf('%.16g,\n',rpar(rpptr(i):rpptr(i+1)-1))),70);

        txt(1)='static double rpar_'+string(i)+'[]={'+txt(1);
        for j=2:size(txt,1)
          txt(j)=get_blank('static double rpar_'+string(i)+'[]')+txt(j);
        end
        txt($)=part(txt($),1:length(txt($))-1)+'};'
        Code=[Code;
              '  '+txt
              '']
//        Code=[Code;
//              '  double rpar_'+string(i)+'['+...
//                string(size(rpptr(i):rpptr(i+1),'*')-1)+'];'
//              '']
      end
    end
  end
  //#######################//

  //### Integer parameters ###//
  if size(ipar,1) <> 0 then
    Code=[Code;
          '  /* Integers parameters declaration */']

    for i=1:(length(ipptr)-1)
      if ipptr(i+1)-ipptr(i)>0  then

        //** Add comments **//

        //## Modelica block
        if type(corinv(i))==15 then
          //## we can extract here all informations
          //## from original scicos blocks with corinv : TODO
          Code($+1)='  /* MODELICA BLK IPAR COMMENTS : TODO */';
        else
          if size(corinv(i),'*')==1 then
            OO=scs_m.objs(corinv(i));
          else
            path=list('objs');
            for l=cpr.corinv(i)(1:$-1)
              path($+1)=l
              path($+1)='model'
              path($+1)='rpar'
              path($+1)='objs'
            end
            path($+1)=cpr.corinv(i)($);
            OO=scs_m(path);
          end

          Code($+1)='  /* Routine name of block: '+strcat(string(cpr.sim.funs(i)));
          Code($+1)='   * Gui name of block: '+strcat(string(OO.gui));
          Code($+1)='   * Compiled structure index: '+strcat(string(i));
          if stripblanks(OO.model.label)~=emptystr() then
            Code=[Code;
                  '  '+cformatline(' * Label: '+strcat(string(OO.model.label)),70)];
          end

          if stripblanks(OO.graphics.exprs(1))~=emptystr() then
            Code=[Code;
                  '  '+cformatline(' * Exprs: '+strcat(OO.graphics.exprs(1),","),70)];
          end
          if stripblanks(OO.graphics.id)~=emptystr() then
            Code=[Code;
                  '  '+cformatline(' * Identification: '+strcat(string(OO.graphics.id)),70)];
          end
          Code=[Code;
                '  '+cformatline(' * ipar= {'+strcat(string(ipar(ipptr(i):ipptr(i+1)-1)),",")+'};',70)];
          Code($+1)='   */';
        end
        //******************//

        txt=cformatline(strcat(string(ipar(ipptr(i):ipptr(i+1)-1))+','),70);

        txt(1)='static int ipar_'+string(i)+'[]={'+txt(1);
        for j=2:size(txt,1)
          txt(j)=get_blank('static int ipar_'+string(i)+'[]')+txt(j);
        end
        txt($)=part(txt($),1:length(txt($))-1)+'};'
        Code=[Code;
              '  '+txt
              '']
      end
    end
  end

Code = [Code;
        ''
	make_rtai_params();
	]
  //##########################//

  //### Object parameters ###//

  //** declaration of opar
  Code_opar = [];
  Code_ooparsz=[];
  Code_oopartyp=[];
  Code_oparptr=[];

  for i=1:(length(opptr)-1)
    nopar = opptr(i+1)-opptr(i)
    if nopar>0  then
      //** Add comments **//

      //## Modelica block
      if type(corinv(i))==15 then
        //## we can extract here all informations
        //## from original scicos blocks with corinv : TODO
        Code_opar($+1)='  /* MODELICA BLK OPAR COMMENTS : TODO */';
      else
        //@@ 04/11/08, disable generation of comment for opar
        //@@ for m_frequ because of sample clock
        if funs(i)=='m_frequ' then
        else
          if size(corinv(i),'*')==1 then
            OO=scs_m.objs(corinv(i));
          else
            path=list('objs');
            for l=cpr.corinv(i)(1:$-1)
              path($+1)=l;
              path($+1)='model';
              path($+1)='rpar';
              path($+1)='objs';
            end
            path($+1)=cpr.corinv(i)($);
            OO=scs_m(path);
          end

          Code_opar($+1)='  /* Routine name of block: '+strcat(string(cpr.sim.funs(i)));
          Code_opar($+1)='   * Gui name of block: '+strcat(string(OO.gui));
          Code_opar($+1)='   * Compiled structure index: '+strcat(string(i));
          if stripblanks(OO.model.label)~=emptystr() then
            Code_opar=[Code_opar;
                  '  '+cformatline(' * Label: '+strcat(string(OO.model.label)),70)];
          end
          if stripblanks(OO.graphics.id)~=emptystr() then
            Code_opar=[Code_opar;
                  '  '+cformatline(' * Identification: '+strcat(string(OO.graphics.id)),70)];
          end
          Code_opar($+1)='   */';
        end
      end
      //******************//

      for j=1:nopar
        if mat2scs_c_nb(opar(opptr(i)+j-1)) <> 11 then

	// Thanks to Matteo Morelli for the correction of this bug

          Code_tmp = cformatline(mat2c_typ(opar(opptr(i)+j-1)) +...
                                 ' opar_'+string(opptr(i)+j-1) + '[]={'+...
                                 strcat(string(opar(opptr(i)+j-1)),',')+'};',70);
          Code_tmp(1) = '  __CONST__ '+Code_tmp(1); 
          Code_opar =[Code_opar; Code_tmp]         





       else //** cmplx test
          Code_tmp = cformatline(mat2c_typ(opar(opptr(i)+j-1)) +...
                         ' opar_'+string(opptr(i)+j-1) + '[]={'+...
                             strcat(string([real(opar(opptr(i)+j-1)(:));
                                            imag(opar(opptr(i)+j-1)(:))]),',')+'};',70);
          Code_tmp(1) = '  __CONST__ '+Code_tmp(1); 
          Code_opar =[Code_opar; Code_tmp]         
        end
      end

      //## size
      Code_oparsz   = []
      //** 1st dim **//
      for j=1:nopar
        Code_oparsz=[Code_oparsz
                     string(size(opar(opptr(i)+j-1),1))]
      end
      //** 2dn dim **//
      for j=1:nopar
        Code_oparsz=[Code_oparsz
                     string(size(opar(opptr(i)+j-1),2))]
      end
      Code_tooparsz=cformatline(strcat(Code_oparsz,','),70);
      Code_tooparsz(1)='__CONST__ int oparsz_'+string(i)+'[]={'+Code_tooparsz(1);
      for j=2:size(Code_tooparsz,1)
        Code_tooparsz(j)=get_blank('__CONST__ int oparsz_'+string(i)+'[]')+Code_tooparsz(j);
      end
      Code_tooparsz($)=Code_tooparsz($)+'};'
      Code_ooparsz=[Code_ooparsz;
                    Code_tooparsz];

      //## typ
      Code_opartyp   = []
      for j=1:nopar
        Code_opartyp=[Code_opartyp
                      mat2scs_c_typ(opar(opptr(i)+j-1))]
      end
      Code_toopartyp=cformatline(strcat(Code_opartyp,','),70);
      Code_toopartyp(1)='__CONST__ int opartyp_'+string(i)+'[]={'+Code_toopartyp(1);
      for j=2:size(Code_toopartyp,1)
        Code_toopartyp(j)=get_blank('__CONST__ int opartyp_'+string(i)+'[]')+Code_toopartyp(j);
      end
      Code_toopartyp($)=Code_toopartyp($)+'};'
      Code_oopartyp=[Code_oopartyp;
                     Code_toopartyp];

      //## ptr
      Code_tooparptr=cformatline(strcat(string(zeros(1,nopar)),','),70);
      Code_tooparptr(1)='static void *oparptr_'+string(i)+'[]={'+Code_tooparptr(1);
      for j=2:size(Code_tooparptr,1)
        Code_tooparptr(j)=get_blank('static void *oparptr_'+string(i)+'[]')+Code_tooparptr(j);
      end
      Code_tooparptr($)=Code_tooparptr($)+'};'
      Code_oparptr=[Code_oparptr
                    Code_tooparptr]

    end
  end

  if Code_opar <> [] then
    Code=[Code;
          '  /* Object parameters declaration */'
          Code_opar
          ''
          '  '+Code_ooparsz
          ''
          '  '+Code_oopartyp
          ''
          '  '+Code_oparptr
          '']
  end

  //##########################//

  //*** Continuous state ***//
  if x <> [] then
   //## implicit block
   if impl_blk then
     Code=[Code;
           '  /* Continuous states declaration */'
           cformatline('  static double x[]={'+strcat(string(x(1:nX)),',')+'};',70)
           cformatline('  static double xd[]={'+strcat(string(zeros(nX/2+1:nX)),',')+'};',70)
           cformatline('  static double res[]={'+strcat(string(zeros(1,nX/2)),',')+'};',70)
           ''
           '/* def xproperty */'
           cformatline('     static int xprop[]={'+strcat(string(ones(1:nX/2)),',')+'};',70)
           '']

   //## explicit block
   else
     Code=[Code;
           '  /* Continuous states declaration */'
           cformatline('  static double x[]={'+strcat(string(x),',')+'};',70)
           cformatline('  static double xd[]={'+strcat(string(zeros(1,nX)),',')+'};',70)
           '']
   end
  end
  //************************//

  //### discrete states ###//
  if size(z,1) <> 0 then
    Code=[Code;
          '  /* Discrete states declaration */']
    for i=1:(length(zptr)-1)
      if zptr(i+1)-zptr(i)>0 then

        //** Add comments **//

        //## Modelica block
        if type(corinv(i))==15 then
          //## we can extract here all informations
          //## from original scicos blocks with corinv : TODO
          Code($+1)='  /* MODELICA BLK Z COMMENTS : TODO ';
        else
          if size(corinv(i),'*')==1 then
            OO=scs_m.objs(corinv(i))
          else
            path=list('objs')
            for l=cpr.corinv(i)(1:$-1)
              path($+1)=l;path($+1)='model'
              path($+1)='rpar'
              path($+1)='objs'
            end
            path($+1)=cpr.corinv(i)($)
            OO=scs_m(path)
          end
          aaa=OO.gui
          bbb=emptystr(3,1);
          if and(aaa+bbb~=['INPUTPORTEVTS';'OUTPUTPORTEVTS';'EVTGEN_f']) then
            Code($+1)='  /* Routine name of block: '+strcat(string(cpr.sim.funs(i)));
            Code($+1)='     Gui name of block: '+strcat(string(OO.gui));
            //Code($+1)='/* Name block: '+strcat(string(cpr.sim.funs(i)));
            //Code($+1)='Object number in diagram: '+strcat(string(cpr.corinv(i)));
            Code($+1)='     Compiled structure index: '+strcat(string(i));
            if stripblanks(OO.model.label)~=emptystr() then
              Code=[Code;
                    cformatline('     Label: '+strcat(string(OO.model.label)),70)]
            end
            if stripblanks(OO.graphics.exprs(1))~=emptystr() then
              Code=[Code;
                    cformatline('     Exprs: '+strcat(OO.graphics.exprs(1),","),70)]
            end
            if stripblanks(OO.graphics.id)~=emptystr() then
              Code=[Code;
                    cformatline('     Identification: '+..
                       strcat(string(OO.graphics.id)),70)]
            end
          end
        end
        Code($+1)='  */';
        Code=[Code;
              cformatline('  static double z_'+string(i)+'[]={'+...
              strcat(string(z(zptr(i):zptr(i+1)-1)),",")+'};',70)]
        Code($+1)='';
      end
      //******************//
    end
  end
  //#######################//

  //** declaration of work
  Code_work=[]
  for i=1:size(with_work,1)
    if with_work(i)==1 then
       Code_work=[Code_work
                  '  static void *work_'+string(i)+'[]={0};']
    end
  end

  if Code_work<>[] then
    Code=[Code
          '  /* Work array declaration */'
          Code_work
          '']
  end

  //### Object state ###//
  //** declaration of oz
  Code_oz = [];
  Code_oozsz=[];
  Code_ooztyp=[];
  Code_ozptr=[];

  for i=1:(length(ozptr)-1)
    noz = ozptr(i+1)-ozptr(i)
    if noz>0 then

      for j=1:noz
        if mat2scs_c_nb(oz(ozptr(i)+j-1)) <> 11 then
          Code_oz=[Code_oz;
                   cformatline('  static '+mat2c_typ(oz(ozptr(i)+j-1))+...
                               ' oz_'+string(ozptr(i)+j-1)+'[]={'+...
                               strcat(string(oz(ozptr(i)+j-1)(:)),',')+'};',70)]
        else //** cmplx test
          Code_oz=[Code_oz;
                   cformatline('  static '+mat2c_typ(oz(ozptr(i)+j-1))+...
                               ' oz_'+string(ozptr(i)+j-1)+'[]={'+...
                               strcat(string([real(oz(ozptr(i)+j-1)(:));
                                              imag(oz(ozptr(i)+j-1)(:))]),',')+'};',70)]
        end
      end

      //## size
      Code_ozsz   = []
      //** 1st dim **//
      for j=1:noz
        Code_ozsz=[Code_ozsz
                     string(size(oz(ozptr(i)+j-1),1))]
      end
      //** 2dn dim **//
      for j=1:noz
        Code_ozsz=[Code_ozsz
                     string(size(oz(ozptr(i)+j-1),2))]
      end
      Code_toozsz=cformatline(strcat(Code_ozsz,','),70);
      Code_toozsz(1)='static int ozsz_'+string(i)+'[]={'+Code_toozsz(1);
      for j=2:size(Code_toozsz,1)
        Code_toozsz(j)=get_blank('static int ozsz_'+string(i)+'[]')+Code_toozsz(j);
      end
      Code_toozsz($)=Code_toozsz($)+'};'
      Code_oozsz=[Code_oozsz;
                    Code_toozsz];

      //## typ
      Code_oztyp   = []
      for j=1:noz
        Code_oztyp=[Code_oztyp
                      mat2scs_c_typ(oz(ozptr(i)+j-1))]
      end
      Code_tooztyp=cformatline(strcat(Code_oztyp,','),70);
      Code_tooztyp(1)='static int oztyp_'+string(i)+'[]={'+Code_tooztyp(1);
      for j=2:size(Code_tooztyp,1)
        Code_tooztyp(j)=get_blank('static int oztyp_'+string(i)+'[]')+Code_tooztyp(j);
      end
      Code_tooztyp($)=Code_tooztyp($)+'};'
      Code_ooztyp=[Code_ooztyp;
                     Code_tooztyp];

      //## ptr
      Code_toozptr=cformatline(strcat(string(zeros(1,noz)),','),70);
      Code_toozptr(1)='static void *ozptr_'+string(i)+'[]={'+Code_toozptr(1);
      for j=2:size(Code_toozptr,1)
        Code_toozptr(j)=get_blank('static void *ozptr_'+string(i)+'[]')+Code_toozptr(j);
      end
      Code_toozptr($)=Code_toozptr($)+'};'
      Code_ozptr=[Code_ozptr
                    Code_toozptr]

    end
  end

  if Code_oz <> [] then
    Code=[Code;
          '  /* Object discrete states declaration */'
          Code_oz
          ''
          '  '+Code_oozsz
          ''
          '  '+Code_ooztyp
          ''
          '  '+Code_ozptr
          '']
  end
  //#######################//

  //** declaration of outtb
  Code_outtb = [];
  for i=1:lstsize(outtb)
    if mat2scs_c_nb(outtb(i)) <> 11 then
      Code_outtb=[Code_outtb;
                  cformatline('  static '+mat2c_typ(outtb(i))+...
                              ' outtb_'+string(i)+'[]={'+...
                              strcat(string_to_c_string(outtb(i)(:)),',')+'};',70)]
    else //** cmplx test
      Code_outtb=[Code_outtb;
                  cformatline('  static '+mat2c_typ(outtb(i))+...
                              ' outtb_'+string(i)+'[]={'+...
                              strcat(string_to_c_string([real(outtb(i)(:));
                                             imag(outtb(i)(:))]),',')+'};',70)]
    end
  end

  if Code_outtb<>[] then
    Code=[Code
          '  /* Output declaration */'
          Code_outtb
          '']
  end

  Code_outtbptr=[];
  for i=1:lstsize(outtb)
    Code_outtbptr=[Code_outtbptr;
                   '  '+rdnom+'_block_outtbptr['+...
                    string(i-1)+'] = (void *) outtb_'+string(i)+';'];
  end

  //##### insz/outsz #####//
  Code_iinsz=[];
  Code_inptr=[];
  Code_ooutsz=[];
  Code_outptr=[];
  for kf=1:nblk
    nin=inpptr(kf+1)-inpptr(kf);  //** number of input ports
    Code_insz=[];

    //########
    //## insz
    //########

    //## case sensor ##//
    if or(kf==capt(:,1)) then
      ind=find(kf==capt(:,1))
      //Code_insz = 'typin['+string(ind-1)+']'
    //## other blocks ##//
    elseif nin<>0 then
      //** 1st dim **//
      for kk=1:nin
         lprt=inplnk(inpptr(kf)-1+kk);
         Code_insz=[Code_insz
                    string(size(outtb(lprt),1))]
      end
      //** 2dn dim **//
      for kk=1:nin
         lprt=inplnk(inpptr(kf)-1+kk);
         Code_insz=[Code_insz
                    string(size(outtb(lprt),2))]
      end
      //** typ **//
      for kk=1:nin
         lprt=inplnk(inpptr(kf)-1+kk);
         Code_insz=[Code_insz
                    mat2scs_c_typ(outtb(lprt))]
      end
    end
    if Code_insz<>[] then
      Code_toinsz=cformatline(strcat(Code_insz,','),70);
      Code_toinsz(1)='static int insz_'+string(kf)+'[]={'+Code_toinsz(1);
      for j=2:size(Code_toinsz,1)
        Code_toinsz(j)=get_blank('static int insz_'+string(kf)+'[]')+Code_toinsz(j);
      end
      Code_toinsz($)=Code_toinsz($)+'};'
      Code_iinsz=[Code_iinsz
                  Code_toinsz]
    end

    //########
    //## inptr
    //########

    //## case sensor ##//
    if or(kf==capt(:,1)) then
      Code_inptr=[Code_inptr;
                  'static void *inptr_'+string(kf)+'[]={0};';]
    //## other blocks ##//
    elseif nin<>0 then
      Code_toinptr=cformatline(strcat(string(zeros(1,nin)),','),70);
      Code_toinptr(1)='static void *inptr_'+string(kf)+'[]={'+Code_toinptr(1);
      for j=2:size(Code_toinptr,1)
        Code_toinptr(j)=get_blank('static void *inptr_'+string(kf)+'[]')+Code_toinptr(j);
      end
      Code_toinptr($)=Code_toinptr($)+'};'
      Code_inptr=[Code_inptr
                  Code_toinptr]
    end

    nout=outptr(kf+1)-outptr(kf); //** number of output ports
    Code_outsz=[];

    //########
    //## outsz
    //########

    //## case actuators ##//
    if or(kf==actt(:,1)) then
      ind=find(kf==actt(:,1))
      //Code_outsz = 'typout['+string(ind-1)+']'
    //## other blocks ##//
    elseif nout<>0 then
      //** 1st dim **//
      for kk=1:nout
         lprt=outlnk(outptr(kf)-1+kk);
         Code_outsz=[Code_outsz
                     string(size(outtb(lprt),1))]
      end
      //** 2dn dim **//
      for kk=1:nout
         lprt=outlnk(outptr(kf)-1+kk);
         Code_outsz=[Code_outsz
                     string(size(outtb(lprt),2))]
      end
      //** typ **//
      for kk=1:nout
         lprt=outlnk(outptr(kf)-1+kk);
         Code_outsz=[Code_outsz
                     mat2scs_c_typ(outtb(lprt))]
      end
    end
    if Code_outsz<>[] then
      Code_tooustz=cformatline(strcat(Code_outsz,','),70);
      Code_tooustz(1)='static int outsz_'+string(kf)+'[]={'+Code_tooustz(1);
      for j=2:size(Code_tooustz,1)
        Code_tooustz(j)=get_blank('static int outsz_'+string(kf)+'[]')+Code_tooustz(j);
      end
      Code_tooustz($)=Code_tooustz($)+'};'
      Code_ooutsz=[Code_ooutsz
                   Code_tooustz]
    end

    //#########
    //## outptr
    //#########

    //## case actuators ##//
    if or(kf==actt(:,1)) then
      Code_outptr=[Code_outptr;
                   'static void *outptr_'+string(kf)+'[]={0};';]
    //## other blocks ##//
    elseif nout<>0 then
      Code_tooutptr=cformatline(strcat(string(zeros(1,nout)),','),70);
      Code_tooutptr(1)='static void *outptr_'+string(kf)+'[]={'+Code_tooutptr(1);
      for j=2:size(Code_tooutptr,1)
        Code_tooutptr(j)=get_blank('static void *outptr_'+string(kf)+'[]')+Code_tooutptr(j);
      end
      Code_tooutptr($)=Code_tooutptr($)+'};'
      Code_outptr=[Code_outptr
                   Code_tooutptr]
    end
  end

  if Code_iinsz<>[] then
     Code=[Code;
          '  /* Inputs */'
          '  '+Code_iinsz
          ''];
  end
  if Code_inptr<>[] then
     Code=[Code;
          '  '+Code_inptr
          ''];
  end
  if Code_ooutsz<>[] then
     Code=[Code;
          '  /* Outputs */'
          '  '+Code_ooutsz
          ''];
  end
  if Code_outptr<>[] then
     Code=[Code;
          '  '+Code_outptr
          ''];
  end
  //######################//

  //##### out events #####//
  Code_evout=[];
  for kf=1:nblk
    if funs(kf)<>'bidon' then
      nevout=clkptr(kf+1)-clkptr(kf);
      if nevout <> 0 then
        Code_toevout=cformatline(strcat(string(cpr.state.evtspt((clkptr(kf):clkptr(kf+1)-1))),','),70);
        Code_toevout(1)='static double evout_'+string(kf)+'[]={'+Code_toevout(1);
        for j=2:size(Code_toevout,1)
          Code_toevout(j)=get_blank('static double evout_'+string(kf)+'[]')+Code_toevout(j);
        end
        Code_toevout($)=Code_toevout($)+'};';
        Code_evout=[Code_evout
                    Code_toevout];
      end
      //## with_nrd2 ##//
      if funtyp(kf)==0 then
        with_nrd2=%t;
      end
    end
  end
  if Code_evout<>[] then
     Code=[Code;
          '  /* Outputs event declaration */'
          '  '+Code_evout
          ];
  end
  //################//

  Code=[Code;
        ''
        '/*'+part('-',ones(1,40))+'  Initialisation function */'
        'int '+rdnom+'_init()'
        '{'
	'  double t;'
        '  int local_flag;'
//	#ifdef linux'
//      '  double *args[2];'
//	#endif'
        '']

   if impl_blk then
    Code=[Code;
          '  double h,dt;'
	  '  int i;'
          '']
  end
 
  if with_nrd then
    if with_nrd2 then
      Code=[Code;
            '  /* Variables for constant values */'
            '  int nrd_1, nrd_2;'
            ''
            '  double *args[2];'
            '']
    end
   end


  //## input connection to outtb
  Code_inptr=[]
  for kf=1:nblk
    nin=inpptr(kf+1)-inpptr(kf);  //** number of input ports
    //## case sensor ##//
    if or(kf==capt(:,1)) then
      ind=find(kf==capt(:,1))
      Code_inptr=[Code_inptr;
                  '  inptr_'+string(kf)+'[0] = inptr['+string(ind-1)+'];';]
    //## other blocks ##//
    elseif nin<>0 then
      for k=1:nin
        lprt=inplnk(inpptr(kf)-1+k);
        Code_inptr=[Code_inptr
                    '  inptr_'+string(kf)+'['+string(k-1)+'] = (void *) outtb_'+string(lprt)+';']
      end
    end
  end
  if Code_inptr<>[] then
    Code=[Code;
          '  /* Affectation of inptr */';
          Code_inptr;
          ''];
  end

  //## output connection to outtb
  Code_outptr=[]
  for kf=1:nblk
    nout=outptr(kf+1)-outptr(kf); //** number of output ports
    //## case actuators ##//
    if or(kf==actt(:,1)) then
    ind=find(kf==actt(:,1))
    Code_outptr=[Code_outptr;
                 '  outptr_'+string(kf)+'[0] = outptr['+string(ind-1)+'];';]
    //## other blocks ##//
    elseif nout<>0 then
      for k=1:nout
        lprt=outlnk(outptr(kf)-1+k);
        Code_outptr=[Code_outptr
                    '  outptr_'+string(kf)+'['+string(k-1)+'] = (void *) outtb_'+string(lprt)+';']
      end
    end
  end
  if Code_outptr<>[] then
    Code=[Code;
          '  /* Affectation of outptr */';
          Code_outptr;
          ''];
  end

  //## affectation of oparptr
  Code_oparptr=[]
  for kf=1:nblk
    nopar=opptr(kf+1)-opptr(kf); //** number of object parameters
    if nopar<>0 then
      for k=1:nopar
        Code_oparptr=[Code_oparptr
                    '  oparptr_'+string(kf)+'['+string(k-1)+'] = (void *) opar_'+string(opptr(kf)+k-1)+';']
      end
    end
  end
  if Code_oparptr<>[] then
    Code=[Code;
          '  /* Affectation of oparptr */';
          Code_oparptr;
          ''];
  end

  //## affectation of ozptr
  Code_ozptr=[]
  for kf=1:nblk
    noz=ozptr(kf+1)-ozptr(kf); //** number of object states
    if noz<>0 then
      for k=1:noz
        Code_ozptr=[Code_ozptr
                    '  ozptr_'+string(kf)+'['+string(k-1)+'] = (void *) oz_'+string(ozptr(kf)+k-1)+';']
      end
    end
  end
  if Code_ozptr<>[] then
    Code=[Code;
          '  /* Affectation of ozptr */';
          Code_ozptr;
          ''];
  end

  //## fields of each scicos structure
  for kf=1:nblk
    if funs(kf)<>'bidon' then
      nx=xptr(kf+1)-xptr(kf);         //** number of continuous state
      nz=zptr(kf+1)-zptr(kf);         //** number of discrete state
      nin=inpptr(kf+1)-inpptr(kf);    //** number of input ports
      nout=outptr(kf+1)-outptr(kf);   //** number of output ports
      nevout=clkptr(kf+1)-clkptr(kf); //** number of event output ports

      //** add comment
      txt=[get_comment('set_blk',list(funs(kf),funtyp(kf),kf));]

      Code=[Code;
            '  '+txt];

      Code=[Code;
            '  block_'+rdnom+'['+string(kf-1)+'].type    = '+string(funtyp(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].ztyp    = '+string(ztyp(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].ng      = '+string(zcptr(kf+1)-zcptr(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].nz      = '+string(zptr(kf+1)-zptr(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].nx      = '+string(nx)+';';
            '  block_'+rdnom+'['+string(kf-1)+'].noz     = '+string(ozptr(kf+1)-ozptr(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].nrpar   = '+string(rpptr(kf+1)-rpptr(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].nopar   = '+string(opptr(kf+1)-opptr(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].nipar   = '+string(ipptr(kf+1)-ipptr(kf))+';'
            '  block_'+rdnom+'['+string(kf-1)+'].nin     = '+string(inpptr(kf+1)-inpptr(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].nout    = '+string(outptr(kf+1)-outptr(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].nevout  = '+string(clkptr(kf+1)-clkptr(kf))+';';
            '  block_'+rdnom+'['+string(kf-1)+'].nmode   = '+string(modptr(kf+1)-modptr(kf))+';';]

      if nx <> 0 then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].x       = &(x['+string(xptr(kf)-1)+']);'
              '  block_'+rdnom+'['+string(kf-1)+'].xd      = &(xd['+string(xptr(kf)-1)+']);']
        if impl_blk then
          Code=[Code;
                '  block_'+rdnom+'['+string(kf-1)+'].res     = &(res['+string(xptr(kf)-1)+']);'
                '  block_'+rdnom+'['+string(kf-1)+'].xprop   = &(xprop['+string(xptr(kf)-1)+']);'];
        end
      end

      //** evout **//
      if nevout<>0 then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].evout   = evout_'+string(kf)+';']
      end

      //***************************** input port *****************************//
      //## case sensor ##//
      if or(kf==capt(:,1)) then
        ind=find(kf==capt(:,1))
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].inptr   = inptr_'+string(kf)+';'
              '  block_'+rdnom+'['+string(kf-1)+'].insz    = &typin['+string(ind-1)+'];']
      //## other blocks ##//
      elseif nin<>0 then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].inptr   = inptr_'+string(kf)+';'
              '  block_'+rdnom+'['+string(kf-1)+'].insz    = insz_'+string(kf)+';']
      end
      //**********************************************************************//

      //***************************** output port *****************************//
      //## case actuators ##//
      if or(kf==actt(:,1)) then
        ind=find(kf==actt(:,1))
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].outptr  = outptr_'+string(kf)+';'
              '  block_'+rdnom+'['+string(kf-1)+'].outsz   = &typout['+string(ind-1)+'];']
      //## other blocks ##//
      elseif nout<>0 then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].outptr  = outptr_'+string(kf)+';'
              '  block_'+rdnom+'['+string(kf-1)+'].outsz   = outsz_'+string(kf)+';']
      end
      //**********************************************************************//

      //## discrete states ##//
      if (nz>0) then
        Code=[Code
              '  block_'+rdnom+'['+string(kf-1)+...
              '].z       = z_'+string(kf)+';']
      end

      //** rpar **//
      if (rpptr(kf+1)-rpptr(kf)>0) then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+...
              '].rpar    = rpar_'+string(kf)+';']
      end

      //** ipar **//
      if (ipptr(kf+1)-ipptr(kf)>0) then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+...
              '].ipar    = ipar_'+string(kf)+';']
      end

      //** opar **//
      if (opptr(kf+1)-opptr(kf)>0) then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].oparptr = oparptr_'+string(kf)+';'
              '  block_'+rdnom+'['+string(kf-1)+'].oparsz  = oparsz_'+string(kf)+';'
              '  block_'+rdnom+'['+string(kf-1)+'].opartyp = opartyp_'+string(kf)+';'
              ]
      end

      //** oz **//
      if (ozptr(kf+1)-ozptr(kf)>0) then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].ozptr   = ozptr_'+string(kf)+';'
              '  block_'+rdnom+'['+string(kf-1)+'].ozsz    = ozsz_'+string(kf)+';'
              '  block_'+rdnom+'['+string(kf-1)+'].oztyp   = oztyp_'+string(kf)+';'
              ]
      end

      //** work **/
      if with_work(kf)==1 then
        Code=[Code;
              '  block_'+rdnom+'['+string(kf-1)+'].work    = work_'+string(kf)+';'
              ]
      end

      //** TODO label **//

      Code=[Code;
            '']
    end
  end

  Code=[Code;
        '  /* set initial phase simulation */'
        '  phase = 1;'
        '']

  if impl_blk then
    Code=[Code;
          '  /* default jacob_param */'
	  '  dt='+rdnom+'_get_tsamp();'
          '  h=dt/'+odestep+';' 
          '  CJJ=1/h;'
          '']
  end


  //** init
  Code=[Code;
        '  '+get_comment('flag',list(4))]

  for kf=1:nblk
    if funs(kf)=='agenda_blk' then
      if size(evs,'*')<>0 then
        new_pointi=adjust_pointi(cpr.state.pointi,clkptr,funtyp)
        Code=[Code;
              '';
              '  /* Init of agenda_blk (blk nb '+string(kf)+') */'
              '  *(block_'+rdnom+'['+string(kf-1)+'].work) = '+...
                '(agenda_struct*) scicos_malloc(sizeof(agenda_struct));'
              '  ptr = *(block_'+rdnom+'['+string(kf-1)+'].work);'
              '  ptr->pointi     = '+string(new_pointi)+';'
              '  ptr->fromflag3  = 0;'
              '  ptr->old_pointi = 0;'
             ]
        new_evtspt=adjust_agenda(cpr.state.evtspt,clkptr,funtyp)
        for i=1:size(new_evtspt,1)
          if new_evtspt(i)>0 then
            new_evtspt(i)=adjust_pointi(new_evtspt(i),clkptr,funtyp)
          end
        end
        for i=1:size(evs,'*')
          Code=[Code;
                '  ptr->evtspt['+string(i-1)+'] = '+string(new_evtspt(i))+';'
               ]
        end
        new_tevts=adjust_agenda(cpr.state.tevts,clkptr,funtyp)
        for i=1:size(evs,'*')
          Code=[Code;
                '  ptr->tevts['+string(i-1)+']  = '+string_to_c_string(new_tevts(i))+';'
               ]
        end
      end
    else
      txt = call_block42(kf,0,4);
      if txt <> [] then
        Code=[Code;
              '';
              '  '+txt]
      end
    end
  end


  //** cst blocks and it's dep
  txt=write_code_idoit()

  if txt<>[] then
    Code=[Code;
          ''
          '  /* Initial blocks must be called with flag 1 */'
          txt];
  end

  //## reinidoit
  if x <> [] then
    //## implicit block
    if impl_blk then
      txt=[write_code_reinitdoit(1) //** first pass
           write_code_reinitdoit(7) //** second pass
          ]

      if txt<>[] then
        Code=[Code;
              '  /* Initial derivative computation */'
              txt];
      end
    end
  end

  Code=[Code;
	'  return(local_flag);'
	'}'];

  Code=[Code;
        ''
        '/*'+part('-',ones(1,40))+'  ISR function */'
        'int '+rdnom+'_isr(double t)'
        '{'
        '  int local_flag;'
	'  int i;'
       ]

  if (x <> []) then
    Code=[Code
          '  double tout, dt, he, h;'
          '']
  end

  if with_nrd then
    if with_nrd2 then
      Code=[Code;
            '  /* Variables for constant values */'
            '  int nrd_1, nrd_2;'
            ''
            '  double *args[2];'
            '']
    end
  end  

  //** flag 1,2,3
  for flag=[1,3,2]

    txt3=[]

    //** continuous time blocks must be activated
    //** for flag 1
    if flag==1 then
      txt = write_code_cdoit(flag);

      if txt <> [] then
        txt3=[''
              '    '+get_comment('ev',list(0))
              txt;
             ];
      end
    end

    //** blocks with input discrete event must be activated
    //** for flag 1, 2 and 3
    if size(evs,2)>=1 then
      txt4=[]
      //**
      for ev=evs
        txt2=write_code_doit(ev,flag);
        if txt2<>[] then
          //** adjust event number because of bidon block
          if ~ALL then
            new_ev=ev-(clkptr(howclk)-1)
          else
            new_ev=ev-min(evs)+1
          end
          //**
          txt4=[txt4;
//                Indent2+['  case '+string(new_ev)+' : '+...
//                get_comment('ev',list(new_ev))
                   txt2;
//                '      break;';
               '']
        end
      end

      //**
      if txt4 <> [] then
        if ~ALL then
          txt3=[txt3;
                Indent+'  /* Discrete activations */'
//                Indent+'  switch (nevprt) {'
                txt4
//                '    }'
		            ''];
        else
          txt33=[]
          if flag==3 then
            txt33=[Indent+'  ptr->pointi = ptr->evtspt[kever-1];'
                   Indent+'  ptr->evtspt[kever-1] = -1;']
          end
          txt3=[txt3;
                txt33;
//                Indent+'  switch (kever) {'
                txt4
//                '    }'
                ''];
        end
      end
    end

    //**
    if txt3<>[] then
      Code=[Code;
            '    '+get_comment('flag',list(flag))
            txt3];
    end

  end

  if impl_blk then
    intgfun=strsubst(odefun,'ode','dae');
  else
    intgfun=odefun
  end

  if x<>[] then
    Code=[Code
          ''
          '    tout=t;'
	  '    dt='+rdnom+'_get_tsamp();'
          '    h=dt/'+odestep+';' 
          '    while (tout+h<t+dt){'
	  ]
    if impl_blk then
      Code=[Code
            '    '+intgfun+'('+rdnom+'simblk_imp,x,xd,res,tout,h);'
	   ]
    else
      Code=[Code
            '    '+intgfun+'('+rdnom+'simblk,x,xd,tout,h);'
	   ]
    end

    Code=[Code
          '     tout=tout+h;'
          '  }'
          ''
          '  he=t+dt-tout;'
	  ]
    if impl_blk then
      Code=[Code
            '  '+intgfun+'('+rdnom+'simblk_imp,x,xd,res,tout,he);'
	   ]

    else
      Code=[Code
            '  '+intgfun+'('+rdnom+'simblk,x,xd,tout,he);'
            '']
    end
  end

  //** 13/10/07, fix bug provided by Roberto Bucher
  if nX <> 0 then
    Code=[Code;
          ''
          '    /* update ptrs of continuous array */']
    for kf=1:nblk
      nx=xptr(kf+1)-xptr(kf);  //** number of continuous state
      if nx<>0 then
        Code=[Code;
              '    block_'+rdnom+'['+string(kf-1)+'].nx = '+...
                string(nx)+';';
              '    block_'+rdnom+'['+string(kf-1)+'].x  = '+...
               '&(x['+string(xptr(kf)-1)+']);'
              '    block_'+rdnom+'['+string(kf-1)+'].xd = '+...
               '&(xd['+string(xptr(kf)-1)+']);']
        if impl_blk then
          Code=[Code;
                '    block_'+rdnom+'['+string(kf-1)+'].res = '+...
                 '&(res['+string(xptr(kf)-1)+']);']
        end
      end
    end
  end

  Code=[Code
	''
	'  return 0;'
        '}']

  //** flag 5

  Code=[Code
        '/*'+part('-',ones(1,40))+'  Termination function */'
        'int '+rdnom+'_end()'
        '{'
	'  double t;'
        '  int local_flag;'
//	'#ifdef linux'
//        '  double *args[2];'
//	'#endif'
        '']

  if with_nrd then
    if with_nrd2 then
      Code=[Code;
            '  /* Variables for constant values */'
            '  int nrd_1, nrd_2;'
            ''
            '  double *args[2];'
            '']
    end
  end
  
  Code=[Code;
        '  '+get_comment('flag',list(5))]

  for kf=1:nblk
//    if or(kf==act) | or(kf==cap) then
//        txt = call_block42(kf,0,5);
//        if txt <> [] then
//          Code=[Code;
//                '';
//                '  '+txt];
//        end
//    else
      txt = call_block42(kf,0,5);
      if txt <> [] then
        Code=[Code;
              '';
              '  '+txt];
      end
//    end
  end

  Code=[Code
        '  return 0;'
        '}'
	'']

  Code_common=['/* Code prototype for common use  */'
               '/*     Generated by Code_Generation toolbox of Scicos with '+ ..
                getversion()+' */'
               '/*     date : '+date()+' */'
               ''
               '/* ---- Headers ---- */'
               '#include <memory.h>'
               '#include <machine.h>'
	       ''
	       'int phase;'
	       'int err = 0;'
	       'int * block_error = &err;'
	       ''
	       ]

  Code_common=[Code_common
               '/* block_number */'
               'int block_number;'
               '']

	       if(isempty(grep(SCI,'5.1.1'))) then
	       Code_common=[Code_common
               '/*'+part('-',ones(1,40))+'  Lapack messag function */';
               'void C2F(xerbla)(SRNAME,INFO,L)'
               '     char *SRNAME;'
               '     int *INFO;'
               '     long int L;'
               '{}'
	       '']
	       end

               Code_common=[Code_common
               'void set_block_error(int err)'
               '{'
 	       '*block_error = err;'
               '  return;'
               '}'
               '']

               Code_common=[Code_common
               'int get_block_error(int err)'
               '{'
 	       '  return *block_error;'
               '}'
               '']

	       Code_common=[Code_common
               'void set_block_number(int kfun)'
               '{'
               '  block_number = kfun;'
               '  return;'
               '}'
               '']

	       Code_common=[Code_common
               'int get_block_number()'
               '{'
               '  return block_number;'
               '}'
               '']
 
               Code_common=[Code_common
               'int get_phase_simulation()'
               '{'
               '  return phase;'
               '}'
               '']

               Code_common=[Code_common
               'void * scicos_malloc(size_t size)'
               '{'
               '  return malloc(size);'
               '}'
               '']

               Code_common=[Code_common
               'void scicos_free(void *p)'
               '{'
               '  free(p);'
               '}'
               '']

               Code_common=[Code_common
               'void do_cold_restart()'
               '{'
               '  return;'
               '}'
               '']

               Code_common=[Code_common
               'void sciprint (char *fmt)'
               '{'
               '  return;'
               '}'
               '']

  if impl_blk then
    Code=[Code;
          'void Set_Jacobian_flag(int flag)'
          '{'
          '  Jacobian_Flag=flag;'
          '  return;'
          '}'
          ''
          'double Get_Jacobian_ci()'
          '{'
          '  return CI;'
          '}'
          ''
          'double Get_Jacobian_cj()'
          '{'
          '  return CJ;'
          '}'
          ''
          'double Get_Jacobian_parameter(void)'
          '{'
          '  return CJJ;'
          '}'
          ''
          'double Get_Scicos_SQUR(void)'
          '{'
          '  return  SQuround;'
          '}'
         ]
  end

  if (x <> []) then

    //## implicit case
    if impl_blk then
      Code=[Code;
            'int '+rdnom+'simblk_imp(t, x, xd, res)'
            ''
            '   double t, *x, *xd, *res;'
            ''
            '     /*'
            '      *  !purpose'
            '      *  compute state derivative of the continuous part'
            '      *  !calling sequence'
            '      *  NEQ   : a defined integer : the size of the  continuous state'
            '      *  t     : current time'
            '      *  x     : double precision vector whose contains the continuous state'
            '      *  xd    : double precision vector whose contains the computed derivative'
            '      *          of the state'
            '      *  res   : double precision vector whose contains the computed residual'
            '      *          of the state'
            '      */'
            '{'
            '  /* local variables used to call block */'
            '  int local_flag;']

      if act<>[] | cap<>[] then
        Code=[Code;
              '  int nport;']
      end

      Code=[Code;
            ''
            '  /* counter local variable */'
            '  int i;'
            '']

      if with_nrd then
        //## look at for block of type 0 (no captor)
        ind=find(funtyp==0)
        if ind<>[] then
          with_nrd2=%t
        else
          with_nrd2=%f
        end
        if with_nrd2 then
          Code=[Code;
                '  /* Variables for constant values */'
                '  int nrd_1, nrd_2;'
                ''
                '  double *args[2];'
                '']
        end
      end

      Code=[Code;
            '  /* set phase simulation */'
            '  phase=2;'
            ''
            '  /* initialization of residual */'
            '  for(i=0;i<NEQ;i++) res[i]=xd[i];'
            '']

      Code=[Code;
            '  '+get_comment('update_xd',list())]

      for kf=1:nblk
        if (xptr(kf+1)-xptr(kf)) > 0 then
          Code=[Code;
                '  block_'+rdnom+'['+string(kf-1)+'].x='+...
                  '&(x['+string(xptr(kf)-1)+']);'
                '  block_'+rdnom+'['+string(kf-1)+'].xd='+...
                  '&(xd['+string(xptr(kf)-1)+']);'
                '  block_'+rdnom+'['+string(kf-1)+'].res='+...
                  '&(res['+string(xptr(kf)-1)+']);']
        end
      end

      Code=[Code;
            ''
            write_code_odoit(1)
            write_code_odoit(0)
           ]

      Code=[Code
            ''
            '  return 0;'
            '}'
            ''
            '/* DAE Method */'
	    '/* Euler''s Method */'
            'int dae1(f,x,xd,res,t,h)'
            '  int (*f) ();'
            '  double *x, *xd, *res;'
            '  double t, h;'
            '{'
            '  int i;'
            '  int ierr;'
            ''
            '  /**/'
            '  ierr=(*f)(t,x, xd, res);'
            '  if (ierr!=0) return ierr;'
            ''
            '  for (i=0;i<NEQ;i++) {'
            '   x[i]=x[i]+h*res[i];'
            '  }'
            ''
            '  return 0;'
	    '}'
             ''
            '/* Heun''s Method */'
            'int dae2(f,x,xd,res,t,h)'
            '  int (*f) ();'
            '  double *x, *xd, *res;'
            '  double t, h;'
            '{'
            '  int i;'
            '  int ierr;'
            '  double y['+string(nX)+'],yh['+string(nX)+'],temp,f0['+string(nX)+'],th;'
            ''
            '  /**/'
            '  memcpy(y,x,NEQ*sizeof(double));'
            '  memcpy(f0,res,NEQ*sizeof(double));'
            ''
            '  /**/'
            '  ierr=(*f)(t,y, f0, res);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+h*f0[i];'
            '  }'
            '  th=t+h;'
            '  for (i=0;i<NEQ;i++) {'
            '    yh[i]=y[i]+h*f0[i];'
            '  }'
            '  ierr=(*f)(th,yh, xd, res);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  temp=0.5*h;'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+temp*(f0[i]+res[i]);'
            '  }'
            ''
            '  return 0;'
            '}'
            ''
            '/* Fourth-Order Runge-Kutta (RK4) Formula */'
            'int dae4(f,x,xd,res,t,h)'
            '  int (*f) ();'
            '  double *x, *xd, *res;'
            '  double t, h;'
            '{'
            '  int i;'
            '  int ierr;'
            '  double y['+string(nX)+'],yh['+string(nX)+'],'+...
              'temp,f0['+string(nX)+'],th,th2,'+...
              'f1['+string(nX)+'],f2['+string(nX)+'];'
            ''
            '  /**/'
            '  memcpy(y,x,NEQ*sizeof(double));'
            '  memcpy(f0,res,NEQ*sizeof(double));'
            ''
            '  /**/'
            '  ierr=(*f)(t,y, xd, f0);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+h*f0[i];'
            '  }'
            '  th2=t+h/2;'
            '  for (i=0;i<NEQ;i++) {'
            '    yh[i]=y[i]+(h/2)*f0[i];'
            '  }'
            '  ierr=(*f)(th2,yh, xd, f1);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  temp=0.5*h;'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+temp*f1[i];'
            '  }'
            '  for (i=0;i<NEQ;i++) {'
            '    yh[i]=y[i]+(h/2)*f1[i];'
            '  }'
            '  ierr=(*f)(th2,yh, xd, f2);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+h*f2[i];'
            '  }'
            '  th=t+h;'
            '  for (i=0;i<NEQ;i++) {'
            '    yh[i]=y[i]+h*f2[i];'
            '  }'
            '  ierr=(*f)(th2,yh, xd, res);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  temp=h/6;'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+temp*(f0[i]+2.0*f1[i]+2.0*f2[i]+res[i]);'
            '  }'
            ''
            '  return 0;'
           '}']
    //## explicit case
    else
      Code=[Code;
            'int '+rdnom+'simblk(t, x, xd)'
            ''
            '   double t, *x, *xd;'
            ''
            '     /*'
            '      *  !purpose'
            '      *  compute state derivative of the continuous part'
            '      *  !calling sequence'
            '      *  NEQ   : a defined integer : the size of the  continuous state'
            '      *  t     : current time'
            '      *  x     : double precision vector whose contains the continuous state'
            '      *  xd    : double precision vector whose contains the computed derivative'
            '      *          of the state'
            '      */'
            '{'
            '  /* local variables used to call block */'
            '  int local_flag;']

      if act<>[] | cap<>[] then
        Code=[Code;
              '  int nport;']
      end

      Code=[Code;
            ''
            '  /* counter local variable */'
            '  int i;'
            '']

      if with_nrd then
        //## look at for block of type 0 (no captor)
        ind=find(funtyp==0)
        if ind<>[] then
          with_nrd2=%t
        else
          with_nrd2=%f
        end
        if with_nrd2 then
          Code=[Code;
                '  /* Variables for constant values */'
                '  int nrd_1, nrd_2;'
                ''
                '  double *args[2];'
                '']
        end
      end

      Code=[Code;
            '  /* set phase simulation */'
            '  phase=2;'
            ''
            '  /* initialization of derivatives */'
            '  for(i=0;i<NEQ;i++) xd[i]=0.;'
            '']

      Code=[Code;
            '  '+get_comment('update_xd',list())]

      for kf=1:nblk
        if (xptr(kf+1)-xptr(kf)) > 0 then
          Code=[Code;
                '  block_'+rdnom+'['+string(kf-1)+'].x='+...
                  '&(x['+string(xptr(kf)-1)+']);'
                '  block_'+rdnom+'['+string(kf-1)+'].xd='+...
                  '&(xd['+string(xptr(kf)-1)+']);']
        end
      end

      Code=[Code;
            ''
            write_code_odoit(1)
            write_code_odoit(0)
           ]

      Code=[Code
            ''
            '  return 0;'
            '}'
            ''
            '/* Euler''s Method */'
            'int ode1(f,x,xd,t,h)'
            '  int (*f) ();'
            '  double *x,*xd;'
            '  double t, h;'
            '{'
            '  int i;'
            '  int ierr;'
            ''
            '  /**/'
            '  ierr=(*f)(t,x, xd);'
            '  if (ierr!=0) return ierr;'
            ''
            '  for (i=0;i<NEQ;i++) {'
            '   x[i]=x[i]+h*xd[i];'
            '  }'
            ''
            '  return 0;'
            '}'
            ''
            '/* Heun''s Method */'
            'int ode2(f,x,xd,t,h)'
            '  int (*f) ();'
            '  double *x,*xd;'
            '  double t, h;'
            '{'
            '  int i;'
            '  int ierr;'
            '  double y['+string(nX)+'],yh['+string(nX)+'],temp,f0['+string(nX)+'],th;'
            ''
            '  /**/'
            '  memcpy(y,x,NEQ*sizeof(double));'
            '  memcpy(f0,xd,NEQ*sizeof(double));'
            ''
            '  /**/'
            '  ierr=(*f)(t,y, f0);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+h*f0[i];'
            '  }'
            '  th=t+h;'
            '  for (i=0;i<NEQ;i++) {'
            '    yh[i]=y[i]+h*f0[i];'
            '  }'
            '  ierr=(*f)(th,yh, xd);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  temp=0.5*h;'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+temp*(f0[i]+xd[i]);'
            '  }'
            ''
            '  return 0;'
            '}'
            ''
            '/* Fourth-Order Runge-Kutta (RK4) Formula */'
            'int ode4(f,x,xd,t,h)'
            '  int (*f) ();'
            '  double *x,*xd;'
            '  double t, h;'
            '{'
            '  int i;'
            '  int ierr;'
            '  double y['+string(nX)+'],yh['+string(nX)+'],'+...
              'temp,f0['+string(nX)+'],th,th2,'+...
              'f1['+string(nX)+'],f2['+string(nX)+'];'
            ''
            '  /**/'
            '  memcpy(y,x,NEQ*sizeof(double));'
            '  memcpy(f0,xd,NEQ*sizeof(double));'
            ''
            '  /**/'
            '  ierr=(*f)(t,y, f0);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+h*f0[i];'
            '  }'
            '  th2=t+h/2;'
            '  for (i=0;i<NEQ;i++) {'
            '    yh[i]=y[i]+(h/2)*f0[i];'
            '  }'
            '  ierr=(*f)(th2,yh, f1);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  temp=0.5*h;'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+temp*f1[i];'
            '  }'
            '  for (i=0;i<NEQ;i++) {'
            '    yh[i]=y[i]+(h/2)*f1[i];'
            '  }'
            '  ierr=(*f)(th2,yh, f2);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+h*f2[i];'
            '  }'
            '  th=t+h;'
            '  for (i=0;i<NEQ;i++) {'
            '    yh[i]=y[i]+h*f2[i];'
            '  }'
            '  ierr=(*f)(th2,yh, xd);'
            '  if (ierr!=0) return ierr;'
            ''
            '  /**/'
            '  temp=h/6;'
            '  for (i=0;i<NEQ;i++) {'
            '    x[i]=y[i]+temp*(f0[i]+2.0*f1[i]+2.0*f2[i]+xd[i]);'
            '  }'
            ''
            '  return 0;'
            '}']
    end
  end

endfunction


// *******************************************************

function [ccmat]=adj_clkconnect_dep(blklst,ccmat)
//Copyright (c) 1989-2010 Metalau project INRIA

//this part was taken from c_pass2 and put in c_pass1;!!
nbl=size(blklst)
fff=ones(nbl,1)==1
clkptr=zeros(nbl+1,1);clkptr(1)=1; typ_l=fff;typ_t=fff;
for i=1:nbl
  ll=blklst(i);
  clkptr(i+1)=clkptr(i)+size(ll.evtout,'*');
  //tblock(i)=ll.dep_ut($);
  typ_l(i)=ll.blocktype=='l';
  typ_t(i)=ll.dep_ut($)
end
all_out=[]
for k=1:size(clkptr,1)-1
  if ~typ_l(k) then
    kk=[1:(clkptr(k+1)-clkptr(k))]'
    all_out=[all_out;[k*ones(kk),kk]]
  end
end
all_out=[all_out;[0,0]]
ind=find(typ_t==%t)
ind=ind(:);
for k=ind'
  ccmat=[ccmat;[all_out,ones(all_out)*[k,0;0,0]]]
end
endfunction



// *******************************************************

function [new_agenda]=adjust_agenda(evts,clkptr,funtyp)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ adjust_agenda : remove secondary activation sources
//                   from the scicos cpr agenda
//
// Input : evts   :   evts vector of scicos agenda
//         clkptr : compiled clkptr vector
//         funtyp : compiled funtyp vector
//
// Output : new_agenda : new evts vector without
//                       secondary activation sources
//

  //@@ initial var
  k=1
  new_agenda=[]

  //@@ loop on number of computational functions
  for i=1:size(clkptr,1)
    if i<>size(clkptr,1)
      j = clkptr(i+1) - clkptr(i)
      if j<>0 then
        //@@ check type of activation source
        if funtyp(i)<>-1 & funtyp(i)<>-2 then
          new_agenda = [new_agenda;evts(k:k+j-1)]
        end
        k=k+j
      end
    end
  end

endfunction


// *******************************************************

function [clkptr]=adjust_clkptr(clkptr,funtyp)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ adjust_clkptr : remove secondary activation sources
//                   from the compiled clkptr
//
// Input :  clkptr : compiled clkptr vector
//          funtyp : compiled funtyp vector
//
// Output : clkptr : new clkptr vector without
//                   secondary activation sources
//

  //@@ initial var
  j=0

  //@@ loop on number of computational functions
  for i=1:size(clkptr,1)
    clkptr(i)=clkptr(i)-j
    if i<>size(clkptr,1)
      //@@ check type of activation source
      if funtyp(i)==-1 | funtyp(i)==-2 then
        j = clkptr(i+1) - clkptr(i)
      end
    end
  end

endfunction


// *******************************************************

function [bllst,ok]=adjust_id_scopes(list_of_scopes,bllst)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ adjust_adjust_id_scopes : function to adjust negative
//                             and positive id of scicos scopes
//
// Input :  list_of_scopes : list of scicos scopes
//          bllst : list of models of scicos blocks
//
// Output : bllst : output bllst with adjusted models for scopes
//          ok    : output flag
//

  //@@ initial var
  ok=%t
  pos_win=[]
  pos_i=[]
  pos_ind=[]

  //@@ loop on number of scicos models
  for i=1:lstsize(bllst)
    ind = find(bllst(i).sim(1)==list_of_scopes(:,1))
    if ind<>[] then
      ierr=execstr('win=bllst(i).'+list_of_scopes(ind,2),'errcatch');
      if ierr<>0 then
        ok=%f, return;
      end
      if win<0 then
        win = 30000 + i
        execstr('bllst(i).'+list_of_scopes(ind,2)+'='+string(win))
      else
        pos_win=[pos_win;win]
        pos_i=[pos_i;i]
        pos_ind=[pos_ind;ind]
      end
    end
  end

  if pos_win<>[] & size(pos_win,1)<>1 then
    ko=%t;
    while ko
      for j=1:size(pos_win,1)
        if find(pos_win(j)==pos_win(j+1:$))<>[] then
          pos_win(j)=pos_win(j)+1
          ko=%t
          break
        end
        ko=%f
      end
    end
    for i=1:size(pos_ind,1)
      execstr('bllst(pos_i(i)).'+list_of_scopes(pos_ind(i),2)+'='+string(pos_win(i)))
    end
  end

endfunction
// remove synchro block from clkptr

// *******************************************************

function new_pointi=adjust_pointi(pointi,clkptr,funtyp)
j=0
new_pointi=pointi
for i=1:size(clkptr,1)
  if clkptr(i)>pointi then break, end;
  if i<>size(clkptr,1)
    if funtyp(i)==-1 | funtyp(i)==-2 then
       j = clkptr(i+1) - clkptr(i)
       new_pointi=new_pointi-j
    end
  end
end
endfunction


// *******************************************************

function [t1]=cformatline(t ,l)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ cformatline : utilitary fonction used to format long C instruction
//
// Input : t  : a string containing a C instruction
//         l  : max line length allowed
//
// Output : t1 : formatted output text
//

  sep=[',','+']
  l1=l-2
  t1=[]
  kw=strindex(t,' ')
  nw=0

  if kw<>[] then
    if kw(1)==1 then //there is leading blanks
      k1=find(kw(2:$)-kw(1:$-1)<>1)
      if k1==[] then //there is a single blank
        nw=1
      else
        nw=kw(k1(1))
      end
    end
  end

  t=part(t,nw+1:length(t))
  bl=part(' ',ones(1,nw))
  l1=l-nw
  first=%t

  while %t
    if length(t)<=l then
      t1=[t1;bl+t]
      return
    end

    k=strindex(t,sep)
    if k==[] then t1=[t1;bl+t]
      return
    end

    k($+1)=length(t)+1 //positions of the commas
    i=find(k(1:$-1)<=l&k(2:$)>l) //nearest left comma (reltively to l)

    if i==[] then i=1,end

    t1=[t1;bl+part(t,1:k(i))]
    t=part(t,k(i)+1:length(t))

    if first then
       l1=l1-2;bl=bl+'  '
       first=%f
    end
  end

endfunction


// *******************************************************

function [txt]=code_to_read_params(varname,var,fpp,typ_str)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ code_to_read_params :  function that write binary data in a scilab file
//                          descriptor fpp and returns the C code to read
//                          binary data from the file descriptor fpp
//                          To have a test concerning reading use the wrapper
//                          get_code_to_read_params
//
// Input :  varname : the C ptr (buffer)
//          var     : the ScicosLab variable
//          fpp     : the ScicosLab file descriptor to write the var
//          typ_str : the C type of the data
//
// Output : txt : the C cmd to read binary data in the file descriptor fpp
//
// nb : data is written in little-endian format
//

 //** check rhs paramaters
 [lhs,rhs]=argn(0);
 if rhs == 3 then typ_str=[], end

 txt=[]

 select type(var)
   //real matrix
   case 1 then
      if isreal(var) then
        mput(var,"dl",fpp)
        if typ_str==[] then typ_str='SCSREAL_COP', end
        txt='fread('+varname+', sizeof('+typ_str+'), '+string(size(var,'*'))+', fpp)'
      else
        mput(real(var),"dl",fpp)
        mput(imag(var),"dl",fpp)
        if typ_str==[] then typ_str='SCSCOMPLEX_COP', end
        txt='fread('+varname+', 2*sizeof('+typ_str+'), '+string(size(var,'*'))+', fpp)'
      end
   //integer matrix
   case 8 then
      select typeof(var)
         case 'int32' then
           mput(var,"ll",fpp)
           if typ_str==[] then typ_str='SCSINT32_COP', end
           txt='fread('+varname+', sizeof('+typ_str+'), '+string(size(var,'*'))+', fpp)'
         case 'int16' then
           mput(var,"sl",fpp)
           if typ_str==[] then typ_str='SCSINT16_COP', end
           txt='fread('+varname+', sizeof('+typ_str+'), '+string(size(var,'*'))+', fpp)'
         case 'int8' then
           mput(var,"cl",fpp)
           if typ_str==[] then typ_str='SCSINT8_COP', end
           txt='fread('+varname+', sizeof('+typ_str+'), '+string(size(var,'*'))+', fpp)'
         case 'uint32' then
           mput(var,"ull",fpp)
           if typ_str==[] then typ_str='SCSUINT32_COP', end
           txt='fread('+varname+', sizeof('+typ_str+'), '+string(size(var,'*'))+', fpp)'
         case 'uint16' then
           mput(var,"usl",fpp)
           if typ_str==[] then typ_str='SCSUINT16_COP', end
           txt='fread('+varname+', sizeof('+typ_str+'), '+string(size(var,'*'))+', fpp)'
         case 'uint8' then
           mput(var,"ucl",fpp)
           if typ_str==[] then typ_str='SCSUINT8_COP', end
           txt='fread('+varname+', sizeof('+typ_str+'), '+string(size(var,'*'))+', fpp)'
      end
   else
     break;
 end

endfunction


// *******************************************************

function [t]=filetype(m)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ filetype : return type of file using fileinfo
//              output 2
//
// Input :  m : fileinfo(2)
//
// Output : t : file types
//

  m=int32(m)

  filetypes=['Directory','Character device','Block device',...
             'Regular file','FIFO','Symbolic link','Socket']

  bits=[16384,8192,24576,32768,4096,40960,49152]

  m=int32(m)&int32(61440)

  t=filetypes(find(m==int32(bits)))

endfunction


// *******************************************************

function [txt]=get_blank(str)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ get_blank : return blanks with a length
//               of the given input string
//
// Input : str : a string
//
// Output : txt : blanks
//

 txt=''

 for i=1:length(str)
     txt=txt+' '
 end

endfunction


// *******************************************************

function [txt]=get_code_to_read_params(varname,var,fpp,typ_str)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ get_code_to_read_params : function that write binary data in a scilab file
//                             descriptor fpp and returns the C code to read
//                             binary data from the file descriptor fpp
//
// Input :  varname : the C ptr (buffer)
//          var     : the ScicosLab variable
//          fpp     : the ScicosLab file descriptor to write the var
//          typ_str : the C type of the data
//
// Output : txt : the C cmd to read binary data in the file descriptor fpp
//
// nb : data is written in little-endian format
//

 //** check rhs paramaters
 [lhs,rhs]=argn(0);
 if rhs == 3 then typ_str=[], end

 //@@ call code_to_read_params
 txt=[]
 txt=code_to_read_params(varname,var,fpp,typ_str)

 //@@ add a C check
 if txt<>[] then
  txt=['if (('+txt+') != '+string(size(var,'*'))+') {'
       '  fclose(fpp);'
       '  return(1001);'
       '}']
 end

endfunction


// *******************************************************

function [txt]=get_comment(typ,param)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ get_comment : return a C comment
//                 for generated code
//
// Input : typ : a string
//         param : a list
//
// Output : txt : a C comment
//

  txt = [];

  select typ
    //** main flag
    case 'flag' then
        select param(1)
          case 0 then
             txt = '/* Continuous state computation */'
          case 1 then
             txt = '/* Output computation */'
          case 2 then
             txt = '/* Discrete state computation */'
          case 3 then
             txt = '/* Output Event computation */'
          case 4 then
             txt = '/* Initialization */'
          case 5 then
             txt = '/* Ending */'
          case 9 then
             txt = '/* Update zero crossing surfaces */'
        end
    //** blocks activated on event number
    case 'ev' then
       txt = '/* Blocks activated on the event number '+string(param(1))+' */'

    //** blk calling sequence
    case 'call_blk' then
        txt = ['/* Call of '''+param(1) + ...
               ''' (type '+string(param(2))+' - blk nb '+...
                    string(param(3))];
        if param(4) then
          txt=txt+' - with zcross) */';
        else
          txt=txt+') */';
        end
    //** proto calling sequence
    case 'proto_blk' then
        txt = ['/* prototype of '''+param(1) + ...
               ''' (type '+string(param(2))];
        if param(4) then
          txt=txt+' - with zcross) */';
        else
          txt=txt+') */';
        end
    //** ifthenelse calling sequence
    case 'ifthenelse_blk' then
        txt = ['/* Call of ''if-then-else'' blk (blk nb '+...
                    string(param(1))+') */']
    //** eventselect calling sequence
    case 'evtselect_blk' then
        txt = ['/* Call of ''event-select'' blk (blk nb '+...
                    string(param(1))+') */']
    //** set block structure
    case 'set_blk' then
        txt = ['/* set blk struc. of '''+param(1) + ...
               ''' (type '+string(param(2))+' - blk nb '+...
                    string(param(3))+') */'];
    //** Update xd vector ptr
    case 'update_xd' then
        txt = ['/* Update xd vector ptr */'];
    //** Update g vector ptr
    case 'update_g' then
        txt = ['/* Update g vector ptr */'];
    //@@ Prototype sensor
    case 'proto_sensor' then
        txt = ['/* prototype of ''sensor'' */'];
    //@@ Prototype actuator
    case 'proto_actuator' then
        txt = ['/* prototype of ''actuator'' */'];
    //@@ update scicos_time variable
    case 'update scicos_time' then
        txt = ['/* update scicos_time variable */'];
    else
      break;
  end
endfunction


// *******************************************************

function [ind]=get_ind_clkptr(bk,clkptr,funtyp)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ get_ind_clkptr : get event index of adjusted
//                    compiled clkptr vector
//
// Input :  bk     : block number
//          clkptr : compiled clkptr vector
//          funtyp : compiled funtyp vector
//
// Output : ind : indices of adjusted event
//

  //@@ call adjust_clkptr
  clkptr=adjust_clkptr(clkptr,funtyp)

  //@@ return indices
  ind=clkptr(bk)

endfunction



// *******************************************************

function [scs_m]=goto_target_scs_m(scs_m)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ goto_target_scs_m : look if we want generate a sblock
//                       contained in a sblock
//
// scs_m : a scicos diagram structure
//

  //@@ get the super path
  kk=super_path

  //## scs_temp becomes the scs_m of the upper-level sblock
  if size(kk,'*')>1 then
    while size(kk,'*')>1 do
      scs_m=scs_m.objs(kk(1)).model.rpar
      kk(1)=[];
    end
    scs_m=scs_m.objs(kk).model.rpar
  elseif size(kk,'*')>0 then
    scs_m=scs_m.objs(kk).model.rpar
  end

endfunction


// *******************************************************

function [depu_mat,ok]=incidence_mat(bllst,connectmat,clkconnect,cor,corinv)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ incidence_mat : compute the incidence matrix
//
// Input :  bllst      : list of scicos blocks models
//          connectmat : regular link connection matrix
//          clkconnect : event link connection matrix
//          cor        : scs_m to cpr list
//          corinv     : cpr to scs_m list
//
// Output : depu_mat   : incidence matrix
//          ok         : output flag
//

  //@@ initial variables
  ok         = %t
  In_blocks  = []
  OUt_blocks = []
  depu_mat   = []

  //@@ get vector of blocks sensor/actuators
  for i=1:lstsize(bllst)
    sim=bllst(i).sim;sim=sim(1);
    if type(sim)==10 then
      if part(sim,1:10)=='actionneur' then
        OUt_blocks(bllst(i).ipar)=i
      elseif part(sim,1:7)=='capteur' then
        In_blocks(bllst(i).ipar)=i
      end
    end
  end

  //@@ disable function protection
  //   to overload message function
  %mprt=funcprot()
  funcprot(0) 
  deff('message(txt)',' ')
  funcprot(%mprt)

  in = 0

  for i=In_blocks'
    in  = in+1
    out = 0
    for j=OUt_blocks'
      out = out+1
      if is_dep(i,j,bllst,connectmat,clkconnect,cor,corinv) then
        depu_mat(in,out)=1
      else
        depu_mat(in,out)=0
      end
    end
  end

endfunction


// *******************************************************

function [dep]=is_dep(i,j,bllst,connectmat,clkconnect,cor,corinv)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ is_dep : return the dep_u dependance concerning block i and j
//
// Input :  i,j        : block indices
//          bllst      : list of scicos blocks models
//          connectmat : regular link connection matrix
//          clkconnect : event link connection matrix
//          cor        : scs_m to cpr list
//          corinv     : cpr to scs_m list
//
// Output : dep        : the dep_u dependance
//

  bllst(i).dep_ut=[%t,%t]
  bllst(i).in=1;
  bllst(i).in2=1;
  bllst(i).intyp=1

  bllst(j).dep_ut=[%t,%t]
  bllst(j).out=1;
  bllst(j).out2=1;
  bllst(j).outtyp=1

  connectmat=[connectmat;j 1 i 1];
  clkconnect=adj_clkconnect_dep(bllst,clkconnect)
  cpr=c_pass2(bllst,connectmat,clkconnect,cor,corinv,"silent")

  dep=(cpr==list())

endfunction


// *******************************************************

function [txt]=mat2c_typ(outtb)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ mat2c_typ : returns the C type of a given ScicosLab
//               variable
//
// Input :  outtb : a ScicosLab variable
//
// Output : txt : string that give the C type
//

 select type(outtb)
   //real matrix
   case 1 then
      if isreal(outtb) then
        txt = "double"
      else
        txt = "double"
      end
   //integer matrix
   case 8 then
      select typeof(outtb)
         case 'int32' then
           txt = "long"
         case 'int16' then
           txt = "short"
         case 'int8' then
           txt = "char"
         case 'uint32' then
           txt = "unsigned long"
         case 'uint16' then
           txt = "unsigned short"
         case 'uint8' then
           txt = "unsigned char"
      end
   else
     break;
 end

endfunction


// *******************************************************

function [c_nb]=mat2scs_c_nb(outtb)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ mat2scs_c_nb : returns the scicos C type of a
//                  given ScicosLab variable
//
// Input :  outtb : a ScicosLab variable
//
// Output : c_nb : scalar that give the scicos C type
//

 select type(outtb)
   //real matrix
   case 1 then
      if isreal(outtb) then
        c_nb = 10
      else
        c_nb = 11
      end
   //integer matrix
   case 8 then
      select typeof(outtb)
         case 'int32' then
           c_nb = 84
         case 'int16' then
           c_nb = 82
         case 'int8' then
           c_nb = 81
         case 'uint32' then
           c_nb = 814
         case 'uint16' then
           c_nb = 812
         case 'uint8' then
           c_nb = 811
      end
   else
     break;
 end

endfunction


// *******************************************************

function [txt]=mat2scs_c_ptr(outtb)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ mat2scs_c_ptr : returns the scicos C ptr of a
//                  given ScicosLab variable
//
// Input : outtb : a ScicosLab variable
//
// Output : txt  : string of the C scicos ptr
//                 of the data of outtb
//

 select type(outtb)
   //real matrix
   case 1 then
      if isreal(outtb) then
        txt = "SCSREAL_COP"
      else
        txt = "SCSCOMPLEX_COP"
      end
   //integer matrix
   case 8 then
      select typeof(outtb)
         case 'int32' then
           txt = "SCSINT32_COP"
         case 'int16' then
           txt = "SCSINT16_COP"
         case 'int8' then
           txt = "SCSINT8_COP"
         case 'uint32' then
           txt = "SCSUINT32_COP"
         case 'uint16' then
           txt = "SCSUINT16_COP"
         case 'uint8' then
           txt = "SCSUINT8_COP"
      end
   else
     break;
 end

endfunction


// *******************************************************

function [txt]=mat2scs_c_typ(outtb)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ mat2scs_c_typ matrix to scicos C type
//
// Input : outtb : a ScicosLab variable
//
// Output : txt  : string of the C scicos typ
//                 of the data of outtb
//

 select type(outtb)
   //real matrix
   case 1 then
      if isreal(outtb) then
        txt = "SCSREAL_N"
      else
        txt = "SCSCOMPLEX_N"
      end
   //integer matrix
   case 8 then
      select typeof(outtb)
         case 'int32' then
           txt = "SCSINT32_N"
         case 'int16' then
           txt = "SCSINT16_N"
         case 'int8' then
           txt = "SCSINT8_N"
         case 'uint32' then
           txt = "SCSUINT32_N"
         case 'uint16' then
           txt = "SCSUINT16_N"
         case 'uint8' then
           txt = "SCSUINT8_N"
      end
   else
     break;
 end

endfunction


// *******************************************************

function [str]=string_to_c_string(a)
//Copyright (c) 1989-2010 Metalau project INRIA
//
//@@ string_to_c_string : ScicosLab string to C string
//
// Input :  a   : ScicosLab variable
//
// Output : str : the C string
//

  //@@ converter ScicosLab variable
  //   in a ScicosLab string
  str=string(a)

  //@@ look at for D-/D+
  for i=1:size(str,1)
    if strindex(str(i),"D-")<>[] then
      str(i)=strsubst(str(i),"D-","e-");
    elseif strindex(str(i),"D+")<>[] then
      str(i)=strsubst(str(i),"D+","e");
    end
  end

endfunction


// *******************************************************

function [txt]=write_code_cdoit(flag)
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ write_code_cdoit : generate body of the code for
//                      for all time dependant blocks
//
// Input : flag : flag number for block's call
//
// Output : txt : text for cord blocks
//

  txt=[];

  for j=1:ncord
    bk=cord(j,1);
    pt=cord(j,2);
    //** blk
    if funtyp(bk)>-1 then
      if or(bk==act) | or(bk==cap) then
        if stalone then
          txt2=call_block42(bk,pt,flag);
          if txt2<>[] then
            txt=[txt;
                 '    '+txt2
                 ''];
          end
        end
      else
        txt2=call_block42(bk,pt,flag);
        if txt2<>[] then
          txt=[txt;
               '    '+txt2
               ''];
        end
      end
    //** ifthenelse blk
    elseif funtyp(bk)==-1 then
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      thentxt=write_code_doit(clkptr(bk),flag);
      elsetxt=write_code_doit(clkptr(bk)+1,flag);
      if thentxt<>[] | elsetxt<>[] then
        txt=[txt;
             '    '+get_comment('ifthenelse_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '    if('+tmp_+'>0) {']
        //*******//
        txt=[txt;
             Indent+thentxt];
        if elsetxt<>[] then
          //** C **//
          txt=[txt;
               '    }';
               '    else {';]
          //*******//
          txt=[txt;
               Indent+elsetxt];
        end
        //** C **//
        txt=[txt;
             '    }']
        //*******//
      end
    //** eventselect blk
    elseif funtyp(bk)==-2 then
      Noutport=clkptr(bk+1)-clkptr(bk);
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      II=[];
      switchtxt=list()
      for i=1: Noutport
        switchtxt(i)=write_code_doit(clkptr(bk)+i-1,flag);
        if switchtxt(i)<>[] then II=[II i];end
      end
      if II<>[] then
        txt=[txt;
             '    '+get_comment('evtselect_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '    i=max(min((int) '+...
              tmp_+',block_'+rdnom+'['+string(bk-1)+'].nevout),1);'
             '    switch(i)'
             '    {']
        //*******//
        for i=II
         //** C **//
         txt=[txt;
              '     case '+string(i)+' :';]
         //*******//
         txt=[txt;
              BigIndent+write_code_doit(clkptr(bk)+i-1,flag);]
         //** C **//
         txt=[txt;
              BigIndent+'break;']
         //*******//
        end
        //** C **//
        txt=[txt;
             '    }'];
        //*******//
      end
    //** Unknown block
    else
      error('Unknown block type '+string(bk));
    end
  end

endfunction


// *******************************************************

function [txt]=write_code_doit(ev,flag,vvvv)
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ write_code_doit : generate body of the code for
//                     ordering calls of blocks during
//                     flag 1,2 & flag 3
//
// Input : ev   : evt number for block's call
//         flag : flag number for block's call
//
// Output : txt : text for flag 1 or 2, or flag 3
//

  txt=[];
  zeroflag=(argn(2)==3);

  for j=ordptr(ev):ordptr(ev+1)-1
    bk=ordclk(j,1);
    if zeroflag then
      pt=0;
    else
      pt=ordclk(j,2);
    end
    //** blk
    if funtyp(bk)>-1 then
      if or(bk==act) | or(bk==cap) then
        if stalone then
          txt2=call_block42(bk,pt,flag);
          if txt2<>[] then
            txt=[txt;
                 '    '+txt2
                 ''];
          end
        end
      else
        if flag==1 | pt>0  then
          txt2=call_block42(bk,pt,flag);
        elseif  (flag==2 & pt<=0 & with_work(bk)==1) then
          pt=0
          txt2=call_block42(bk,pt,flag)
        else
          txt2=[]
        end
        if txt2<>[] then
          txt=[txt;
               '    '+txt2
               ''];
        end
      end
    //** ifthenelse blk
    elseif funtyp(bk)==-1 then
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      thentxt=write_code_doit(clkptr(bk),flag);
      elsetxt=write_code_doit(clkptr(bk)+1,flag);
      if thentxt<>[] | elsetxt<>[] then
        txt=[txt;
             '    '+get_comment('ifthenelse_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_ = '*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '    if('+tmp_+'>0) {']
        //*******//
        txt=[txt;
             Indent+thentxt]
        //@@
        if ALL then
          if cpr.sim.critev(clkptr(bk)) == 1 then
            if stalone then
              if nX<>0 then
                txt=[txt;
                     Indent+'    /* critical event */'
                     Indent+'    hot = 0;']
              end
            else
              txt=[txt;
                   Indent+'    /* critical event */'
                   Indent+'    do_cold_restart();']
            end
          end
        end
        if elsetxt<>[] then
          //** C **//
          txt=[txt;
               '    }';
               '    else {';]
          //*******//
          txt=[txt;
               Indent+elsetxt]
          //@@
          if ALL then
            if cpr.sim.critev(clkptr(bk)+1) == 1 then
              if stalone then
                if nX<>0 then
                  txt=[txt;
                       Indent+'    /* critical event */'
                       Indent+'    hot = 0;']
                end
              else
                txt=[txt;
                     Indent+'    /* critical event */'
                     Indent+'    do_cold_restart();']
              end
            end
          end
        end
        //** C **//
        txt=[txt;
             '    }']
        //*******//
      end
    //** eventselect blk
    elseif funtyp(bk)==-2 then
      Noutport=clkptr(bk+1)-clkptr(bk);
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      II=[];
      switchtxt=list()
      for i=1: Noutport
        switchtxt(i)=write_code_doit(clkptr(bk)+i-1,flag);
        if switchtxt(i)<>[] then II=[II i];end
      end
      if II<>[] then
        txt=[txt;
             '    '+get_comment('evtselect_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '    i=max(min((int) '+...
              tmp_+',block_'+rdnom+'['+string(bk-1)+'].nevout),1);'
             '    switch(i)'
             '    {']
        //*******//
        for i=II
          //** C **//
          txt=[txt;
               '     case '+string(i)+' :';]
          //*******//
          txt=[txt;
               BigIndent+write_code_doit(clkptr(bk)+i-1,flag);]
          //@@
          if ALL then
            if cpr.sim.critev(clkptr(bk)+i-1) == 1 then
              if stalone then
                if nX<>0 then
                  txt=[txt;
                       BigIndent+'    /* critical event */'
                       BigIndent+'    hot = 0;']
                end
              else
                txt=[txt;
                     BigIndent+'    /* critical event */'
                     BigIndent+'do_cold_restart();']
              end
            end
          end
          //** C **//
          txt=[txt;
               BigIndent+'break;']
          //*******//
        end
        //** C **//
        txt=[txt;
             '    }']
        //*******//
      end
    //** Unknown block
    else
      error('Unknown block type '+string(bk));
    end
  end

endfunction


// *******************************************************

function [txt]=write_code_idoit()
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ write_code_idoit : generate body of the code for
//                   ordering calls of initial
//                   called blocks
//
// Input : nothing (blocks are called with flag 1)
//
// Output : txt : text for iord
//

  txt=[];

  for j=1:niord
    bk=iord(j,1);
    pt=iord(j,2);
    //** blk
    if funtyp(bk)>-1 then
      if or(bk==act) | or(bk==cap) then
        if stalone then
          txt2=call_block42(bk,pt,1);
          if txt2<>[] then
            txt=[txt;
                 '  '+txt2
                 ''];
          end
        end
      else
        txt2=call_block42(bk,pt,1);
        if txt2<>[] then
          txt=[txt;
               '  '+txt2
               ''];
        end
      end
    //** ifthenelse blk
    elseif funtyp(bk)==-1 then
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      thentxt=write_code_doit(clkptr(bk),1);
      elsetxt=write_code_doit(clkptr(bk)+1,1);
      if thentxt<>[] | elsetxt<>[] then
        txt=[txt;
             '  '+get_comment('ifthenelse_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_ = '*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '  if('+tmp_+'>0) {']
        //*******//
        txt=[txt;
             Indent+thentxt];
        if elsetxt<>[] then
           //** C **//
           txt=[txt;
                '  }';
                '  else {';]
           //*******//
           txt=[txt;
                Indent+elsetxt];
        end
        //** C **//
        txt=[txt;
             '  }']
        //*******//
      end
    //** eventselect blk
    elseif funtyp(bk)==-2 then
      Noutport=clkptr(bk+1)-clkptr(bk);
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      II=[];
      switchtxt=list()
      for i=1: Noutport
        switchtxt(i)=write_code_doit(clkptr(bk)+i-1,1);
        if switchtxt(i)<>[] then II=[II i];end
      end
      if II<>[] then
        txt=[txt;
             '  '+get_comment('evtselect_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '  i=max(min((int) '+...
              tmp_+',block_'+rdnom+'['+string(bk-1)+'].nevout),1);']
        txt=[txt;
             '  switch(i)'
             '  {']
        //*******//
        for i=II
          //** C **//
          txt=[txt;
               '   case '+string(i)+' :';]
          //*******//
          txt=[txt;
               BigIndent+write_code_doit(clkptr(bk)+i-1,1);]
          //** C **//
          txt=[txt;
               BigIndent+'break;']
          //*******//
        end
        //** C **//
        txt=[txt;
             '  }'];
        //*******//
      end
    //** Unknown block
    else
      error('Unknown block type '+string(bk));
    end
  end

endfunction


// *******************************************************

function [txt]=write_code_initdoit(ev,flag)
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ write_code_initdoit : generate body of the code for
//                         ordering calls of blocks during
//                         implicit solver initialization
//
// Input : ev   : evt number for block's call
//         flag : flag number for block's call
//
// Output : txt : text to call block
//

  txt=[];

  for j=ordptr(ev):ordptr(ev+1)-1
    bk=ordclk(j,1);
    pt=ordclk(j,2);
    //** blk
    if funtyp(bk)>-1 then
      if or(bk==act) | or(bk==cap) then
        if stalone then
          txt2=call_block42(bk,pt,flag);
          if txt2<>[] then
            txt=[txt;
                 '  '+txt2
                 ''];
          end
        end
      else
        txt2=call_block42(bk,pt,flag);
        if txt2<>[] then
          txt=[txt;
               '  '+txt2
               ''];
        end
      end
    //** ifthenelse blk
    elseif funtyp(bk)==-1 then
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      thentxt=write_code_initdoit(clkptr(bk),flag);
      elsetxt=write_code_initdoit(clkptr(bk)+1,flag);
      if thentxt<>[] | elsetxt<>[] then
        txt=[txt;
             '  '+get_comment('ifthenelse_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_ = '*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '  if('+tmp_+'>0) {']
        //*******//
        txt=[txt;
             Indent+thentxt]
        if elsetxt<>[] then
           //** C **//
           txt=[txt;
                '  }';
                '  else {';]
           //*******//
           txt=[txt;
                Indent+elsetxt];
        end
        //** C **//
        txt=[txt;
             '  }']
        //*******//
      end
    //** eventselect blk
    elseif funtyp(bk)==-2 then
      Noutport=clkptr(bk+1)-clkptr(bk);
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      II=[];
      switchtxt=list()
      for i=1: Noutport
        switchtxt(i)=write_code_doit(clkptr(bk)+i-1,flag);
        if switchtxt(i)<>[] then II=[II i];end
      end
      if II<>[] then
        txt=[txt;
             '  '+get_comment('evtselect_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '  i=max(min((int) '+...
              tmp_+',block_'+rdnom+'['+string(bk-1)+'].nevout),1);'
             '  switch(i)'
             '  {']
        //*******//
        for i=II
          //** C **//
          txt=[txt;
               '   case '+string(i)+' :';]
          //*******//
          txt=[txt;
               BigIndent+write_code_doit(clkptr(bk)+i-1,flag);]
          //** C **//
          txt=[txt;
               BigIndent+'break;']
          //*******//
        end
        //** C **//
        txt=[txt;
             '  }']
        //*******//
      end
    //** Unknown block
    else
      error('Unknown block type '+string(bk));
    end
  end

endfunction


// *******************************************************

function [txt]=write_code_odoit(flag)
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ write_code_odoit : generate body of the code for
//                      ordering calls of blocks before
//                      continuous time integration
//
// Input : flag : flag number for block's call
//
// Output : txt : text for flag 0
//

  txt=[];

  for j=1:noord
    bk=oord(j,1);
    if flag==2 then
      pt=0;
    else
      pt=oord(j,2);
    end
    //** blk
    if funtyp(bk)>-1 then
      if or(bk==cap) then
        if stalone then
          txt2=call_block42(bk,pt,flag);
          if txt2<>[] then
            txt=[txt;
                 '  '+txt2
                 ''];
          end
        end
      else
        txt2=call_block42(bk,pt,flag);
        if txt2<>[] then
          if flag==10 & stalone then
            txt2=['if (AJacobian_block=='+string(bk)+') {'
                  '  '+txt2
                  '}']
          end
          txt=[txt;
               '  '+txt2
               ''];
        end
      end
    //** ifthenelse blk
    elseif funtyp(bk)==-1 then
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      thentxt=write_code_ozdoit(clkptr(bk),flag);
      elsetxt=write_code_ozdoit(clkptr(bk)+1,flag);
      if thentxt<>[] | elsetxt<>[] then
        txt=[txt;
             '  '+get_comment('ifthenelse_blk',list(bk));]
        //** C **//
        if stalone then
          for i=1:lstsize(funs)-1
            for j=1:outptr(i+1)-outptr(i)
              if outlnk(outptr(i)+j-1)-1 == ix then
                tmp_ = '*(('+TYPE+' *)block_'+rdnom+'['+string(i-1)+'].outptr['+string(j-1)+'])'
                break
              end
            end
          end
          //tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '  if ((block_'+rdnom+'['+string(bk-1)+'].nmode<0'+...
              ' && '+tmp_+'>0)'+...
              ' || \'
             '      (block_'+rdnom+'['+string(bk-1)+'].nmode>0'+...
              ' && block_'+rdnom+'['+string(bk-1)+'].mode[0]==1)) {']
        //*******//
        txt=[txt;
             Indent+thentxt]
        //** C **//
        txt=[txt;
             '  }'];
        //*******//
        if elsetxt<>[] then
          //** C **//
          txt=[txt;
               '  else if  ((block_'+rdnom+'['+string(bk-1)+'].nmode<0'+...
                ' && '+tmp_+'<=0)'+...
                ' || \'
               '            (block_'+rdnom+'['+string(bk-1)+'].nmode>0'+...
                ' && block_'+rdnom+'['+string(bk-1)+'].mode[0]==2)) {';]
          //*******//
          txt=[txt;
               Indent+elsetxt]
          //** C **//
          txt=[txt;
               '  }'];
          //*******//
        end
      end
    //** eventselect blk
    elseif funtyp(bk)==-2 then
      Noutport=clkptr(bk+1)-clkptr(bk);
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      II=[];
      switchtxt=list()
      for i=1: Noutport
        switchtxt(i)=write_code_ozdoit(clkptr(bk)+i-1,flag);
        if switchtxt(i)<>[] then II=[II i];end
      end
      if II<>[] then
        txt=[txt;
             '  '+get_comment('evtselect_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '  if (block_'+rdnom+'['+string(bk-1)+'].nmode<0) {';
             '    i=max(min((int) '+...
                tmp_+',block_'+rdnom+'['+string(bk-1)+'].nevout),1);'
             '  }'
             '  else {'
             '    i=block_'+rdnom+'['+string(bk-1)+'].mode[0];'
             '  }']
        txt=[txt;
             '  switch(i)'
             '  {'];
        //*******//
        for i=II
          //** C **//
          txt=[txt;
               '   case '+string(i)+' :';]
          //*******//
          txt=[txt;
               BigIndent+write_code_ozdoit(clkptr(bk)+i-1,flag);]
          //** C **//
          txt=[txt;
               BigIndent+'break;']
          //*******//
        end
        //** C **//
        txt=[txt;
             '  }'];
        //*******//
      end
    //** Unknown block
    else
      error('Unknown block type '+string(bk));
    end
  end

endfunction


// *******************************************************

function [txt]=write_code_ozdoit(ev,flag)
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ write_code_ozdoit : generate body of the code for both
//                       flag 0 & flag 9
//
// Input:  ev   : evt number for block's call
//         flag : flag number for block's call
//
// Output : txt : text for flag 0 or flag 9
//

  txt=[];

  for j=ordptr(ev):ordptr(ev+1)-1
    bk=ordclk(j,1);
    pt=ordclk(j,2);
    //** blk
    if funtyp(bk)>-1 then
      if (or(bk==act) | or(bk==cap)) & (flag==1) then
        if stalone then
          txt=[txt;
               '    '+call_block42(bk,pt,flag)
               ''];
        end
      else
        txt2=call_block42(bk,pt,flag);
        if txt2<>[] then
          txt=[txt;
               '    '+txt2
               ''];
        end
      end
    //** ifthenelse blk
    elseif funtyp(bk)==-1 then
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      thentxt=write_code_ozdoit(clkptr(bk),flag);
      elsetxt=write_code_ozdoit(clkptr(bk)+1,flag);
      if thentxt<>[] | elsetxt<>[] then
        txt=[txt;
             '    '+get_comment('ifthenelse_blk',list(bk));]
        //** C **//
        if stalone then
          for i=1:lstsize(funs)-1
            for j=1:outptr(i+1)-outptr(i)
              if outlnk(outptr(i)+j-1)-1 == ix then
                tmp_ = '*(('+TYPE+' *)'+rdnom+'_block['+string(i-1)+'].outptr['+string(j-1)+'])'
                break
              end
            end
          end
          //tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_ = '*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '    if (((phase==1'+...
              ' || block_'+rdnom+'['+string(bk-1)+'].nmode==0)'+...
              ' && '+tmp_+'>0)'+...
              ' || \'
             '        ((phase!=1'+...
              ' && block_'+rdnom+'['+string(bk-1)+'].nmode!=0)'+...
              ' && block_'+rdnom+'['+string(bk-1)+'].mode[0]==1)) {']
        //*******//
        txt=[txt;
             Indent+thentxt]
        //** C **//
        txt=[txt;
             '    }'];
        //*******//
        if elsetxt<>[] then
           //** C **//
           txt=[txt;
                '    else if (((phase==1'+...
                 ' || block_'+rdnom+'['+string(bk-1)+'].nmode==0)'+...
                 ' && '+tmp_+'<=0)'+...
                 ' || \'
                '               ((phase!=1'+...
                 ' && block_'+rdnom+'['+string(bk-1)+'].nmode!=0)'+...
                 ' && block_'+rdnom+'['+string(bk-1)+'].mode[0]==2)) {';]
          //*******//
          txt=[txt;
               Indent+elsetxt]
          //** C **//
          txt=[txt;
                '    }'];
          //*******//
        end
      end
    //** eventselect blk
    elseif funtyp(bk)==-2 then
      Noutport=clkptr(bk+1)-clkptr(bk);
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      II=[];
      switchtxt=list()
      for i=1: Noutport
        switchtxt(i)=write_code_ozdoit(clkptr(bk)+i-1,flag);
        if switchtxt(i)<>[] then II=[II i];end
      end
      if II<>[] then
        txt=[txt;
             '    '+get_comment('evtselect_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '    if (phase==1 || block_'+rdnom+'['+string(bk-1)+'].nmode==0) {';
             '      i=max(min((int) '+...
              tmp_+',block_'+rdnom+'['+string(bk-1)+'].nevout),1);'
             '    }'
             '    else {'
             '      i=block_'+rdnom+'['+string(bk-1)+'].mode[0];'
             '    }']
        txt=[txt;
             '    switch(i)'
             '    {'];
        //*******//
        for i=II
          //** C **//
          txt=[txt;
               '     case '+string(i)+' :';]
          //*******//
          txt=[txt;
               BigIndent+write_code_ozdoit(clkptr(bk)+i-1,flag);]
          //** C **//
          txt=[txt;
               BigIndent+'break;']
          //*******//
        end
        //** C **//
        txt=[txt;
             '    }'];
        //*******//
      end
    //** Unknown block
    else
      error('Unknown block type '+string(bk));
    end
  end

endfunction


// *******************************************************

function [txt]=write_code_reinitdoit(flag)
//Copyright (c) 1989-2010 Metalau project INRIA

//@@ write_code_reinitdoit : generate body of the code for
//                           implicit solver reinitialization
//
// Input  : flag : flag number for block's call
//
// Output : txt : text for xproperties
//

  txt=[];

  for j=1:noord
    bk=oord(j,1);
    pt=oord(j,2);
    //** blk
    if funtyp(bk)>-1 then
      if or(bk==cap) then
        if stalone then
          txt2=call_block42(bk,pt,flag);
          if txt2<>[] then
            txt=[txt;
                 '  '+txt2
                 ''];
          end
        end
      else
        txt2=call_block42(bk,pt,flag);
        if txt2<>[] then
          txt=[txt;
               '  '+txt2
               ''];
        end
      end
    //** ifthenelse blk
    elseif funtyp(bk)==-1 then
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      thentxt=write_code_initdoit(clkptr(bk),flag);
      elsetxt=write_code_initdoit(clkptr(bk)+1,flag);
      if thentxt<>[] | elsetxt<>[] then
        txt=[txt;
             '  '+get_comment('ifthenelse_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '  if ((block_'+rdnom+'['+string(bk-1)+'].nmode<0'+...
              ' && '+tmp_+'>0)'+...
              ' || \'
             '      (block_'+rdnom+'['+string(bk-1)+'].nmode>0'+...
              ' && block_'+rdnom+'['+string(bk-1)+'].mode[0]==1)) {']
        //*******//
        txt=[txt;
             Indent+thentxt]
        //** C **//
        txt=[txt;
             '  }'];
        //*******//
        if elsetxt<>[] then
          //** C **//
          txt=[txt;
               '  else if  ((block_'+rdnom+'['+string(bk-1)+'].nmode<0'+...
                ' && '+tmp_+'<=0)'+...
                ' || \'
               '            (block_'+rdnom+'['+string(bk-1)+'].nmode>0'+...
                ' && block_'+rdnom+'['+string(bk-1)+'].mode[0]==2)) {';]
          //*******//
          txt=[txt;
               Indent+elsetxt]
          //** C **//
          txt=[txt;
               '  }'];
          //*******//
        end
      end
    //** eventselect blk
    elseif funtyp(bk)==-2 then
      Noutport=clkptr(bk+1)-clkptr(bk);
      ix=-1+inplnk(inpptr(bk));
      TYPE=mat2c_typ(outtb(ix+1)); //** scilab index start from 1
      II=[];
      switchtxt=list()
      for i=1: Noutport
        switchtxt(i)=write_code_ozdoit(clkptr(bk)+i-1,flag);
        if switchtxt(i)<>[] then II=[II i];end
      end
      if II<>[] then
        txt=[txt;
             '  '+get_comment('evtselect_blk',list(bk));]
        //** C **//
        if stalone then
          tmp_='*(('+TYPE+' *)outtb_'+string(ix+1)+')'
        else
          tmp_='*(('+TYPE+' *)'+rdnom+'_block_outtbptr['+string(ix)+'])'
        end
        txt=[txt;
             '  if (block_'+rdnom+'['+string(bk-1)+'].nmode<0) {';
             '    i=max(min((int) '+...
                tmp_+',block_'+rdnom+'['+string(bk-1)+'].nevout),1);'
             '  }'
             '  else {'
             '    i=block_'+rdnom+'['+string(bk-1)+'].mode[0];'
             '  }']
        txt=[txt;
             '  switch(i)'
             '  {'];
        //*******//
        for i=II
          //** C **//
          txt=[txt;
               '   case '+string(i)+' :';]
          //*******//
          txt=[txt;
               BigIndent+write_code_initdoit(clkptr(bk)+i-1,flag);]
          //** C **//
          txt=[txt;
               BigIndent+'break;']
          //*******//
        end
        //** C **//
        txt=[txt;
             '  }'];
        //*******//
      end
    //** Unknown block
    else
      error('Unknown block type '+string(bk));
    end
  end

endfunction


// *******************************************************

function Makename=rt_gen_make(name,files,libs)

  Makename=rpat+'/Makefile';

  T=mgetl(TARGETDIR+'/'+makfil);
  T=strsubst(T,'$$MODEL$$',name);
  T=strsubst(T,'$$OBJ$$',strcat(files+'.o',' '));
  T=strsubst(T,'$$SCILAB_DIR$$',SCI);
  mputl(T,Makename)

endfunction


// *******************************************************

function [files]=write_code(Code,CCode,FCode,Code_common)

// Original file from Project Metalau - INRIA
// Modified for RT purposes by Roberto Bucher - RTAI Team
// roberto.bucher@supsi.ch

 ierr=execstr('mputl(Code,rpat+''/''+rdnom+''.c'')','errcatch')
  if ierr<>0 then
    message(lasterror())
    ok=%f
    return
  end

 ierr=execstr('mputl(Code_common,rpat+''/common.c'')','errcatch')
  if ierr<>0 then
    message(lasterror())
    ok=%f
    return
  end

  if FCode<>list() then
    fcod=[]
    for i=1:2:lstsize(FCode)
      fcod=[fcod;FCode(i+1);'']
    end
    ierr=execstr('mputl(fcod,rpat+''/''+rdnom+''f.f'')','errcatch')
    if ierr<>0 then
      message(lasterror())
      ok=%f
      return
    end
  end

  ccod = [
          '#include <math.h>';
          '#include <stdlib.h>';
          '#include <scicos_block4.h>';
          ''];

  if CCode<>list() then
    for i=1:2:lstsize(CCode)
      ccod = [ccod;CCode(i+1);'']
    end
  end

  ierr=execstr('mputl(ccod,rpat+''/''+rdnom+''_Cblocks.c'')','errcatch')
  if ierr<>0 then
    message(lasterror())
    ok=%f
    return
  end
  
  files=[]
  [fd,ierr]=mopen(rpat+'/'+rdnom+'f.f','r')
  if ierr==0 then mclose(fd),files=[files,rdnom+'f'],end
  [fd,ierr]=mopen(rpat+'/'+rdnom+'_Cblocks.c','r')
  if ierr==0 then mclose(fd),files=[files,rdnom+'_Cblocks'],end

endfunction

//==========================================================================

// *******************************************************

function ok = compile_standalone()
//compile rt standalone executable for standalone
// 22.01.2004
//Author : Roberto Bucher (roberto.bucher@die.supsi.ch)


  xinfo('Compiling standalone');
  wd = pwd();
  chdir(rpat);

  if getenv('WIN32','NO')=='OK' then
     unix_w('nmake -f Makefile.mak');
  else
     unix_w('make')
  end
  chdir(wd);
  ok = %t;
endfunction


