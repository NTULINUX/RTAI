dnl ########################### -*- Mode: M4 -*- #######################
dnl Copyright (C) 98, 1999 Matthew D. Langston <langston@SLAC.Stanford.EDU>
dnl
dnl This macro is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This file is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this file; if not, see <http://www.gnu.org/licenses/>.
dnl
dnl ####################################################################
dnl @synopsis MDL_HAVE_OPENGL
dnl 
dnl Search for OpenGL.  We search first for Mesa (a GPL'ed version of
dnl OpenGL) before a vendor's version of OpenGL, unless we were
dnl specifically asked not to with `--with-Mesa=no' or `--without-Mesa'.
dnl
dnl The four "standard" OpenGL libraries are searched for: "-lGL",
dnl "-lGLU", "-lGLX" (or "-lMesaGL", "-lMesaGLU" as the case may be) and
dnl "-lglut".
dnl
dnl All of the libraries that are found (since "-lglut" or "-lGLX" might
dnl be missing) are added to the shell output variable "GL_LIBS", along
dnl with any other libraries that are necessary to successfully link an
dnl OpenGL application (e.g. the X11 libraries).  Care has been taken to
dnl make sure that all of the libraries in "GL_LIBS" are listed in the
dnl proper order.
dnl
dnl Additionally, the shell output variable "GL_CFLAGS" is set to any
dnl flags (e.g. "-I" flags) that are necessary to successfully compile
dnl an OpenGL application.
dnl
dnl The following shell variable (which are not output variables) are
dnl also set to either "yes" or "no" (depending on which libraries were
dnl found) to help you determine exactly what was found.
dnl
dnl   have_GL
dnl   have_GLU
dnl   have_GLX
dnl   have_glut
dnl
dnl A complete little toy "Automake `make distcheck'" package of how to
dnl use this macro is available at:
dnl
dnl   ftp://ftp.slac.stanford.edu/users/langston/autoconf/ac_opengl-0.01.tar.gz
dnl
dnl Please note that as the ac_opengl macro and the toy example evolves,
dnl the version number increases, so you may have to adjust the above
dnl URL accordingly.
dnl
dnl @version 0.01 $Id: acinclude.m4,v 1.7 2013/10/22 14:54:15 ando Exp $
dnl @author Matthew D. Langston <langston@SLAC.Stanford.EDU>
dnl
dnl Patched by <rpm@xenomai.org> to suit RTAI's requirements.

AC_DEFUN([MDL_HAVE_OPENGL],
[
  AC_REQUIRE([AC_PATH_X])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_CACHE_CHECK([for OpenGL], mdl_cv_have_OpenGL,
  [
dnl Check for Mesa first, unless we were asked not to.
    AC_ARG_ENABLE(Mesa, [], use_Mesa=$enableval, use_Mesa=yes)

    if test x"$use_Mesa" = xyes; then
       GL_search_list="MesaGL   GL"
      GLU_search_list="MesaGLU GLU"
      GLX_search_list="MesaGLX GLX"
    else
       GL_search_list="GL  MesaGL"
      GLU_search_list="GLU MesaGLU"
      GLX_search_list="GLX MesaGLX"
    fi      

    AC_LANG_SAVE
    AC_LANG_C

dnl If we are running under X11 then add in the appropriate libraries.
    if ! test x"$no_x" = xyes; then
dnl Add everything we need to compile and link X programs to GL_CFLAGS
dnl and GL_X_LIBS.
      GL_CFLAGS="$X_CFLAGS"
      GL_X_LIBS="$X_PRE_LIBS $X_LIBS -lX11 -lXext -lXmu -lXt -lXi $X_EXTRA_LIBS -lm"
    fi
    GL_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$GL_CFLAGS"

    GL_save_LIBS="$LIBS"
    LIBS="$GL_X_LIBS"

    # Save the "AC_MSG_RESULT file descriptor" to FD 8.
    exec 8>&AC_FD_MSG

    # Temporarily turn off AC_MSG_RESULT so that the user gets pretty
    # messages.
    exec AC_FD_MSG>/dev/null

    AC_SEARCH_LIBS(glAccum,          $GL_search_list, have_GL=yes,   have_GL=no)
    AC_SEARCH_LIBS(gluBeginCurve,   $GLU_search_list, have_GLU=yes,  have_GLU=no)
    AC_SEARCH_LIBS(glXChooseVisual, $GLX_search_list, have_GLX=yes,  have_GLX=no)
    AC_SEARCH_LIBS(glutInit,        glut,             have_glut=yes, have_glut=no)

    # Restore pretty messages.
    exec AC_FD_MSG>&8

    if test -n "$LIBS"; then
      mdl_cv_have_OpenGL=yes
      GL_LIBS="$LIBS"
      AC_SUBST(GL_CFLAGS)
      AC_SUBST(GL_LIBS)
    else
      mdl_cv_have_OpenGL=no
      GL_CFLAGS=
    fi

dnl Reset GL_X_LIBS regardless, since it was just a temporary variable
dnl and we don't want to be global namespace polluters.
    GL_X_LIBS=

    LIBS="$GL_save_LIBS"
    CPPFLAGS="$GL_save_CPPFLAGS"

    AC_LANG_RESTORE
  ])
])

AC_DEFUN([SC_PATH_EFLTK], [

dnl EFLTK dir passed to the command line overrides the Kconfig setting

AC_ARG_WITH(efltk, [  --with-efltk              directory containing EFLTK], with_efltk=${withval})

dnl Defaults to the Kconfig setting (CONFIG_RTAI_EFLTK_DIR) if unset

test x$with_efltk = x && with_efltk=$1

AC_MSG_CHECKING([for EFLTK])

AC_CACHE_VAL(ac_cv_efltk,[

    # First check to see if --with-efltk was specified.
    if test x"${with_efltk}" != x ; then
	if test -f "${with_efltk}/bin/efltk-config" ; then
	    ac_cv_efltk=`(cd ${with_efltk}; pwd)`
	else
	    AC_MSG_ERROR([${with_efltk}/bin directory doesn't contain efltk-config])
	fi
    fi

    # then check for a private EFLTK installation
    if test x"${ac_cv_efltk}" = x ; then
	for i in \
		../efltk \
		`ls -dr ../efltk* 2>/dev/null` \
		../../efltk \
		`ls -dr ../../efltk* 2>/dev/null` \
		../../../efltk \
		`ls -dr ../../../efltk* 2>/dev/null` ; do
	    if test -f "$i/bin/efltk-config" ; then
		ac_cv_efltk=`(cd $i; pwd)`
		break
	    fi
	done
    fi

    # check in a few common install locations
    if test x"${ac_cv_efltk}" = x ; then
	for i in ${prefix}/efltk* /usr/local/efltk* /usr/pkg/efltk* /usr \
		`ls -dr /usr/lib/efltk* 2>/dev/null` ; do
	    if test -f "$i/bin/efltk-config" ; then
		ac_cv_efltk=`(cd $i; pwd)`
		break
	    fi
	done
    fi

    # check in a few other private locations
    if test x"${ac_cv_efltk}" = x ; then
	for i in \
		${srcdir}/../efltk \
		`ls -dr ${srcdir}/../efltk* 2>/dev/null` ; do
	    if test -f "$i/bin/efltk-config" ; then
	    ac_cv_efltk=`(cd $i; pwd)`
	    break
	fi
	done
    fi
    ])

if test x"${ac_cv_efltk}" = x ; then
    EFLTK_DIR="# no EFLTK installation found"
    AC_MSG_ERROR(Can't find EFLTK-devel installation (missing bin/efltk-config?))
else
    EFLTK_DIR=${ac_cv_efltk}
    AC_MSG_RESULT(found $EFLTK_DIR)
fi

])

AC_DEFUN([SC_PATH_COMEDI], [

dnl COMEDI dir passed to the command line overrides the Kconfig setting

AC_ARG_WITH(comedi, [  --with-comedi              directory containing COMEDI], with_comedi=${withval})

dnl Defaults to the Kconfig setting (CONFIG_RTAI_COMEDI_DIR) if unset

test x$with_comedi = x && with_comedi=$1

AC_MSG_CHECKING([for COMEDI])

AC_CACHE_VAL(ac_cv_comedi,[

    # First check to see if --with-comedi was specified.
    if test x"${with_comedi}" != x ; then
	if test -f "${with_comedi}/include/linux/comedilib.h" ; then
	    ac_cv_comedi=`(cd ${with_comedi}; pwd)`
	else
	    AC_MSG_ERROR([${with_comedi}/include directory doesn't contain linux/comedilib.h])
	fi
    fi

    # then check for a private COMEDI installation
    if test x"${ac_cv_comedi}" = x ; then
	for i in \
		../comedi \
		`ls -dr ../comedi* 2>/dev/null` \
		../../comedi \
		`ls -dr ../../comedi* 2>/dev/null` \
		../../../comedi \
		`ls -dr ../../../comedi* 2>/dev/null` ; do
	    if test -f "$i/include/linux/comedilib.h" ; then
		ac_cv_comedi=`(cd $i; pwd)`
		break
	    fi
	done
    fi

    # check in a few common install locations
    if test x"${ac_cv_comedi}" = x ; then
	for i in ${prefix}/comedi* /usr/local/comedi* /usr/pkg/comedi* /usr \
		`ls -dr /usr/lib/comedi* 2>/dev/null` ; do
	    if test -f "$i/include/linux/comedilib.h" ; then
		ac_cv_comedi=`(cd $i; pwd)`
		break
	    fi
	done
    fi

    # check in a few other private locations
    if test x"${ac_cv_comedi}" = x ; then
	for i in \
		${srcdir}/../comedi \
		`ls -dr ${srcdir}/../comedi* 2>/dev/null` ; do
	    if test -f "$i/include/linux/comedilib.h" ; then
	    ac_cv_comedi=`(cd $i; pwd)`
	    break
	fi
	done
    fi
    ])

if test x"${ac_cv_comedi}" = x ; then
    COMEDI_DIR="# no COMEDI installation found"
    AC_MSG_ERROR(Can't find COMEDI installation ( missing linux/comedilib.h ) )
else
    COMEDI_DIR=${ac_cv_comedi}
    AC_MSG_RESULT(found $COMEDI_DIR)
fi

])
