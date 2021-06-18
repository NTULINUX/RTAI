/* -*- Mode: C -*- */
#ifndef MACHINE_H 
#define MACHINE_H 

/* Nsp
 * Copyright (C) 1998-2015 Jean-Philippe Chancelier Enpc/Cermics
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *--------------------------------------------------------------------------*/

#include "config.h" 

/* Define for using dld for sunos */
#undef SUNOSDLD

/**
 *  WLU:
 *
 * Defined if leading underscores are needed in dld
 */

/**
 *  WTU:
 *
 * Defined if trailing underscores are needed in dld
 */

/**
 *  USE_SHARP_SIGN:
 *
 * Defined if sharpsigns are recognized by cpp
 */

/**
 * HAVE_EXP10:
 * 
 * Defined if exp10 exists
 */

/**
 * HAVE_TGAMMA:
 * 
 * Defined if tgamma (true gamma) exists
 */

/**
 * HAVE_LGAMMA:
 * 
 * Defined if lgamma (log gamma) exists
 */

/**
 * HAVE_GETWD:
 * 
 * Defined if getwd exists
 */

/**
 * HAVE_GETCWD:
 * 
 * Defined if getcwd exists
 */

/**
 * HAVE_MKSTEMP:
 * 
 * Defined if mkstemp exists
 */

/**
 * HAVE_SLEEP:
 *
 * Defined if sleep exists
 */

/**
 *  HAVE_STRERROR:
 *
 * Defined if strerror exists 
 */

/**
 *  HAVE_TERMCAP:
 *
 * Defined if termcap library exists 
 */

/**
 *  HAVE_USLEEP:
 *
 * Defined if usleep exists 
 */

/**
 *  HAVE_GETRUSAGE:
 *
 * Defined if getrusage exists 
 */


/**
 *  HAVE_CLOCK:
 *
 * Defined if clock exists 
 */

/**
 *  HAVE_TIMES:
 *
 * Defined if times exists 
 */

/**
 *  HAVE_ISINF:
 *
 * Defined if isinf exists 
 */

/**
 *  HAVE_ISNAN:
 *
 * Defined if isnan exists 
 */

/**
 *  HAVE_FINITE:
 *
 * Defined if finite exists 
 */

/**
 *  WITH_TK:
 *
 * Defined if tcl/tk is used 
 */

/**
 *  WITH_PVM:
 *
 * Defined if pvm is used 
 */

/**
 *  WITH_GTKGLEXT:
 *
 * Defined if gtkglext is used 
 */

/**
 *  WITH_GTKHTML:
 *
 * Defined if gtkhtml is used 
 */

/**
 *  WITH_MAXPLUSLIB:
 *
 * Defined if maxplus is used 
 */

/**
 *  WITH_UMFPACK:
 *
 * Defined if umfpack is used 
 */

/**
 *  WITH_CHOLMOD:
 *
 * Defined if cholmod is used 
 */

/**
 *  WITH_FFTW3:
 *
 * Defined if fftw3 is used 
 */

/**
 *  WITH_SNDFILE:
 *
 * Defined if sndfile library is used 
 */

/**
 *  WITH_ASOUND:
 *
 * Defined if asound is used 
 */

/**
 *  WITH_GMP:
 *
 * Defined if gmp library is used 
 */

/**
 *  POINTER_INT :
 *
 * Defined if sizeof(int*)==sizeof(int) 
 */

/**
 *  POINTER_LONG :
 *
 * Defined if sizeof(int*)==sizeof(long) 
 */

/* used to store and retrieve an int in a pointer 
 */

/* NSP_POINTER_CAST_TO_INT is used when an integer is 
 * to be casted to a pointer: (void*) x 
 * we first cast the integer to a proper size integer 
 * using (void*) NSP_POINTER_CAST_TO_INT x
 * Note: intptr_t which is in <stdint.h> should do the same 
 */

#if defined(POINTER_INT) 
#define NSP_INT_TO_POINTER(i) ((int *) (i)) 
#define NSP_POINTER_TO_INT(i) ((int) (i))
#define NSP_POINTER_CAST_TO_INT (unsigned int)
#else 
#if defined(POINTER_LONG) 
#define NSP_INT_TO_POINTER(i) ((int *) (long) (i))
#define NSP_POINTER_TO_INT(i) ((int) (long) (i))
#define NSP_POINTER_CAST_TO_INT (unsigned long)
#else
#define NSP_INT_TO_POINTER(i) ((int *) (long long) (i))
#define NSP_POINTER_TO_INT(i) ((int) (long long) (i))
#define NSP_POINTER_CAST_TO_INT (unsigned long long)
#endif 
#endif
/* CNAME(x,y) ==> xy **/

#if defined(USE_SHARP_SIGN)
#define CNAME(name1,name2) name1##name2
#else 
#define CNAME(name1,name2) name1/**/name2
#endif

/* Define  C2F and F2C entry point conversion */
#if defined(WTU)
#if defined(USE_SHARP_SIGN)
#define C2F(name) name##_
#define F2C(name) name##_
#else 
#define C2F(name) name/**/_
#define F2C(name) name/**/_
#endif
#else
#define C2F(name) name
#define F2C(name) name
#endif

/* Define some functions */

#if !defined(HAVE_EXP10)
#define exp10(x) pow((double) 10.0,x)
#else 
/* missing in some math.h : */
extern double exp10(double);
#endif

#if !defined(HAVE_GETWD)
#define getwd(x) getcwd(x,1024) /* you must define char x[1024] */
#endif

/*
 * obsolete 
 */

typedef int integer;

/* Intel Blas library on win32 
 */

#ifdef MKL 
#include "MKL.h" 
#endif 


#if defined(HAVE_FLOAT_H) && defined(HAVE_LIMITS_H)
#   include <limits.h>
#   include <float.h>
#else 
#   define  DBL_EPSILON             2.2204460492503131e-16   
#   define  DBL_MAX                 1.7976931348623158e+308  
#   define  DBL_MIN                 2.2250738585072014e-308
#   define  SHRT_MAX                32767
#   define  LONG_MAX                2147483647
#   define  INT_MAX                 2147483647

#endif 

#   define  MACHINE_RESOLUTION      DBL_EPSILON
#   define  LARGEST_REAL            DBL_MAX
#   define  SMALLEST_REAL           DBL_MIN
#   define  LARGEST_SHORT_INTEGER   SHRT_MAX
#   define  LARGEST_LONG_INTEGER    LONG_MAX

/* symbol table in functions */
#define WITH_SYMB_TABLE 1 

/* matlab mode */
#define MTLB_MODE 1

/* use relative exec directories */
#define UPDATE_EXEC_DIR 1

#if defined(__MSC__) && defined(_MSC_VER)
#define IMPORT extern  __declspec (dllimport)
#define EXPORT __declspec (dllexport) 
#elif defined(__GNUC__) && defined(__declspec)
#define IMPORT extern __attribute__ ((__dllimport__))
#define EXPORT extern 
#else 
#define IMPORT extern
#define EXPORT extern
#endif

/* symbols in libnsp which are to be exported 
 * can be preceeded by INLIBNSP. 
 * This is only usefull for data that could possibly 
 * be used in shared libraries 
 */

#if defined(FORDLL) 
#define INLIBNSP IMPORT
#else 
#define INLIBNSP EXPORT
#endif 

/* msvc compiler */

#if defined(__MSC__) && defined(_MSC_VER)
#undef HAVE_VALUES_H
#undef HAVE_INLINE 
#endif 


#endif /* MACHINE_H  */
