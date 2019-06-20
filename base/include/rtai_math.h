#if 1

/*
 * Copyright (C) 2013 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef _RTAI_MATH_H
#define _RTAI_MATH_H

#ifdef __KERNEL__

#include <rtai_schedcore.h>

#define kerrno (_rt_whoami()->kerrno)

char *d2str(double d, int dgt, char *str);

#endif 

/**************** BEGIN MACROS TAKEN AND ADAPTED FROM NEWLIB ****************/

#ifndef HUGE_VAL
#define HUGE_VAL (__builtin_huge_val())
#endif

#ifndef HUGE_VALF
#define HUGE_VALF (__builtin_huge_valf())
#endif

#ifndef HUGE_VALL
#define HUGE_VALL (__builtin_huge_vall())
#endif

#ifndef INFINITY
#define INFINITY (__builtin_inff())
#endif

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

int __fpclassify(double x);
int __fpclassifyf(float x);
int __signbit(double x);
int __signbitf(float x);

#if CONFIG_RTAI_MATH_LIBM_TO_USE == 1
int __fpclassifyd(double x);
#define __fpclassify __fpclassifyd
int __signbitd(double x);
#define __signbit __signbitd
#endif

#define fpclassify(__x) \
	((sizeof(__x) == sizeof(float))  ? __fpclassifyf(__x) : \
	__fpclassify(__x))

#ifndef isfinite
#define isfinite(__y) \
	(__extension__ ({int __cy = fpclassify(__y); \
                           __cy != FP_INFINITE && __cy != FP_NAN;}))
#endif

#ifndef isinf
#define isinf(y) (fpclassify(y) == FP_INFINITE)
#endif

#ifndef isnan
#define isnan(y) (fpclassify(y) == FP_NAN)
#endif

#define isnormal(y) (fpclassify(y) == FP_NORMAL)
#define signbit(__x) \
	((sizeof(__x) == sizeof(float))  ?  __signbitf(__x) : \
		__signbit(__x))

#define isunordered(a, b) \
          (__extension__ ({__typeof__(a) __a = (a); __typeof__(b) __b = (b); \
                           fpclassify(__a) == FP_NAN || fpclassify(__b) == FP_NAN;}))

#define isgreater(x, y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x > __y);}))

#define isgreaterequal(x, y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x >= __y);}))

#define isless(x, y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x < __y);}))

#define islessequal(x, y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x <= __y);}))

#define islessgreater(x, y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x < __y || __x > __y);}))

/***************** END MACROS TAKEN AND ADAPTED FROM NEWLIB *****************/

#define fpkerr(x) (isnan(x) || isinf(x))

int matherr(void);

double acos(double x);
float acosf(float x);
double acosh(double x);
float acoshf(float x);
double asin(double x);
float asinf(float x);
double asinh(double x);
float asinhf(float x);
double atan(double x);
float atanf(float x);
double atan2(double y, double x);
float atan2f(float y, float x);
double atanh(double x);
float atanhf(float x);
double j0(double x);
float j0f(float x);
double j1(double x);
float j1f(float x);
double jn(int n, double x);
float jnf(int n, float x);
double y0(double x);
float y0f(float x);
double y1(double x);
float y1f(float x);
double yn(int n, double x);
float ynf(int n, float x);
double cbrt(double x);
float  cbrtf(float x);
double copysign (double x, double y);
float copysignf (float x, float y);
double cosh(double x);
float coshf(float x);
double erf(double x);
float erff(float x);
double erfc(double x);
float erfcf(float x);
double exp(double x);
float expf(float x);
double exp2(double x);
float exp2f(float x);
double expm1(double x);
float expm1f(float x);
double fabs(double x);
float fabsf(float x);
double fdim(double x, double y);
float fdimf(float x, float y);
double floor(double x);
float floorf(float x);
double ceil(double x);
float ceilf(float x);
double fma(double x, double y, double z);
float fmaf(float x, float y, float z);
double fmax(double x, double y);
float fmaxf(float x, float y);
double fmod(double x, double y);
float fmodf(float x, float y);
double frexp(double val, int *exp);
float frexpf(float val, int *exp);
double gamma(double x);
float gammaf(float x);
double lgamma(double x);
float lgammaf(float x);
double gamma_r(double x, int *signgamp);
float gammaf_r(float x, int *signgamp);
double lgamma_r(double x, int *signgamp);
float lgammaf_r(float x, int *signgamp);
double tgamma(double x);
float tgammaf(float x);
double hypot(double x, double y);
float hypotf(float x, float y);
int ilogb(double val);
int ilogbf(float val);
double infinity(void);
float infinityf(void);
double ldexp(double val, int exp);
float ldexpf(float val, int exp);
double log(double x);
float logf(float x);
double log10(double x);
float log10f(float x);
double log1p(double x);
float log1pf(float x);
double log2(double x);
float log2f(float x);
double logb(double x);
float logbf(float x);
long int lrint(double x);
long int lrintf(float x);
long long int llrint(double x);
long long int llrintf(float x);
long int lround(double x);
long int lroundf(float x);
long long int llround(double x);
long long int llroundf(float x);
double modf(double val, double *ipart);
float modff(float val, float *ipart);
double nan(const char *);
float nanf(const char *);
double nearbyint(double x);
float nearbyintf(float x);
double nextafter(double val, double dir);
float nextafterf(float val, float dir);
double pow(double x, double y);
float powf(float x, float y);
double remainder(double x, double y);
float remainderf(float x, float y);
double remquo(double x, double y, int *quo);
float remquof(float x, float y, int *quo);
double rint(double x);
float rintf(float x);
double round(double x);
float roundf(float x);
double scalbn(double x, int n);
float scalbnf(float x, int n);
double scalbln(double x, long int n);
float scalblnf(float x, long int n);
double sin(double x);
float  sinf(float x);
double cos(double x);
float cosf(float x);
double sinh(double x);
float  sinhf(float x);
double sqrt(double x);
float  sqrtf(float x);
double tan(double x);
float tanf(float x);
double tanh(double x);
float tanhf(float x);
double trunc(double x);
float truncf(float x);

#ifdef CONFIG_RTAI_MATH_KCOMPLEX

#define complex _Complex
#define _Complex_I (__extension__ 1.0iF)
#undef I
#define I _Complex_I

char *cd2str(complex double d, int dgt, char *str);

#define cfpkerr(x) (fpkerr(__real__ x) || fpkerr(__imag__ x))

double cabs(double _Complex z);
float cabsf(float _Complex z);
asmlinkage double _Complex cacos(double _Complex z);
asmlinkage float _Complex cacosf(float _Complex z);
asmlinkage double _Complex cacosh(double _Complex z);
asmlinkage float _Complex cacoshf(float _Complex z);
double carg(double _Complex z);
float cargf(float _Complex z);
asmlinkage double _Complex casin(double _Complex z);
asmlinkage float _Complex casinf(float _Complex z);
asmlinkage double _Complex casinh(double _Complex z);
asmlinkage float _Complex casinhf(float _Complex z);
asmlinkage double _Complex catan(double _Complex z);
asmlinkage float _Complex catanf(float _Complex z);
asmlinkage double _Complex catanh(double _Complex z);
asmlinkage float _Complex catanhf(float _Complex z);
asmlinkage double _Complex ccos(double _Complex z);
asmlinkage float _Complex ccosf(float _Complex z);
asmlinkage double _Complex ccosh(double _Complex z);
asmlinkage float _Complex ccoshf(float _Complex z);
asmlinkage double _Complex cexp(double _Complex z);
asmlinkage float _Complex cexpf(float _Complex z);
double cimag(double _Complex z);
float cimagf(float _Complex z);
asmlinkage double _Complex clog(double _Complex z);
asmlinkage float _Complex clogf(float _Complex z);
asmlinkage double _Complex clog10(double _Complex z);
asmlinkage float _Complex clog10f(float _Complex z);
asmlinkage double _Complex conj(double _Complex z);
asmlinkage float _Complex conjf(float _Complex z);
asmlinkage double _Complex cpow(double _Complex x, double _Complex y);
asmlinkage float _Complex cpowf(float _Complex x, float _Complex y);
asmlinkage double _Complex cproj(double _Complex z);
asmlinkage float _Complex cprojf(float _Complex z);
double creal(double _Complex z);
float crealf(float _Complex z);
asmlinkage double _Complex csin(double _Complex z);
asmlinkage float _Complex csinf(float _Complex z);
asmlinkage double _Complex csinh(double _Complex z);
asmlinkage float _Complex csinhf(float _Complex z);
asmlinkage double _Complex csqrt(double _Complex z);
asmlinkage float _Complex csqrtf(float _Complex z);
asmlinkage double _Complex ctan(double _Complex z);
asmlinkage float _Complex ctanf(float _Complex z);
asmlinkage double _Complex ctanh(double _Complex z);
asmlinkage float _Complex ctanhf(float _Complex z);

#endif

#endif /* !_RTAI_MATH_H */

#else

/*
 * Declarations for math functions.
 * Copyright (C) 1991,92,93,95,96,97,98,99,2001 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 *
 * The GNU C Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the GNU C Library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 */

/*
 * ISO C99 Standard: 7.12 Mathematics <math.h>
 */

#ifndef	_RTAI_MATH_H
#define	_RTAI_MATH_H	1
#define _MATH_H         1

#include <rtai_types.h>
#ifdef __attribute_pure__
#undef __attribute_pure__
#endif
#ifdef __attribute_used__
#undef __attribute_used__
#endif
#include <features.h>

__BEGIN_DECLS

/* Get machine-dependent HUGE_VAL value (returned on overflow).
   On all IEEE754 machines, this is +Infinity.  */
#include <bits/huge_val.h>

/* Get machine-dependent NAN value (returned for some domain errors).  */
#ifdef	 __USE_ISOC99
# include <bits/nan.h>
#endif
/* Get general and ISO C99 specific information.  */
#include <bits/mathdef.h>


/* The file <bits/mathcalls.h> contains the prototypes for all the
   actual math functions.  These macros are used for those prototypes,
   so we can easily declare each function as both `name' and `__name',
   and can declare the float versions `namef' and `__namef'.  */

#define __MATHCALL(function,suffix, args)	\
  __MATHDECL (_Mdouble_,function,suffix, args)
#define __MATHDECL(type, function,suffix, args) \
  __MATHDECL_1(type, function,suffix, args); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args)
#define __MATHCALLX(function,suffix, args, attrib)	\
  __MATHDECLX (_Mdouble_,function,suffix, args, attrib)
#define __MATHDECLX(type, function,suffix, args, attrib) \
  __MATHDECL_1(type, function,suffix, args) __attribute__ (attrib); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args) __attribute__ (attrib)
#define __MATHDECL_1(type, function,suffix, args) \
  extern type __MATH_PRECNAME(function,suffix) args __THROW

#define _Mdouble_ 		double
#define __MATH_PRECNAME(name,r)	__CONCAT(name,r)
// added for gcc-3.2
#define _Mdouble_BEGIN_NAMESPACE __BEGIN_NAMESPACE_STD
#define _Mdouble_END_NAMESPACE   __END_NAMESPACE_STD
// end added for gcc-3.2
#include <bits/mathcalls.h>
#undef	_Mdouble_
// added for gcc-3.2
#undef _Mdouble_BEGIN_NAMESPACE
#undef _Mdouble_END_NAMESPACE
// end added for gcc-3.2
#undef	__MATH_PRECNAME

#if defined __USE_MISC || defined __USE_ISOC99


/* Include the file of declarations again, this time using `float'
   instead of `double' and appending f to each function name.  */

# ifndef _Mfloat_
#  define _Mfloat_		float
# endif
# define _Mdouble_ 		_Mfloat_
# ifdef __STDC__
#  define __MATH_PRECNAME(name,r) name##f##r
# else
#  define __MATH_PRECNAME(name,r) name/**/f/**/r
# endif
// added for gcc-3.2
#define _Mdouble_BEGIN_NAMESPACE __BEGIN_NAMESPACE_C99
#define _Mdouble_END_NAMESPACE   __END_NAMESPACE_C99
// end added for gcc-3.2
# include <bits/mathcalls.h>
# undef	_Mdouble_
// added for gcc-3.2
# undef _Mdouble_BEGIN_NAMESPACE
# undef _Mdouble_END_NAMESPACE
// end added for gcc-3.2
# undef	__MATH_PRECNAME

# if (__STDC__ - 0 || __GNUC__ - 0) && !defined __NO_LONG_DOUBLE_MATH
/* Include the file of declarations again, this time using `long double'
   instead of `double' and appending l to each function name.  */

#  ifndef _Mlong_double_
#   define _Mlong_double_	long double
#  endif
#  define _Mdouble_ 		_Mlong_double_
#  ifdef __STDC__
#   define __MATH_PRECNAME(name,r) name##l##r
#  else
#   define __MATH_PRECNAME(name,r) name/**/l/**/r
#  endif
// added for gcc-3.2
#define _Mdouble_BEGIN_NAMESPACE __BEGIN_NAMESPACE_C99
#define _Mdouble_END_NAMESPACE   __END_NAMESPACE_C99
// end added for gcc-3.2
#  include <bits/mathcalls.h>
#  undef _Mdouble_
// added for gcc-3.2
# undef _Mdouble_BEGIN_NAMESPACE
# undef _Mdouble_END_NAMESPACE 
// end added for gcc-3.2
#  undef __MATH_PRECNAME

# endif /* __STDC__ || __GNUC__ */

#endif	/* Use misc or ISO C99.  */
#undef	__MATHDECL_1
#undef	__MATHDECL
#undef	__MATHCALL


#if defined __USE_MISC || defined __USE_XOPEN
/* This variable is used by `gamma' and `lgamma'.  */
extern int signgam;
#endif


/* ISO C99 defines some generic macros which work on any data type.  */
#if defined(__USE_ISOC99) && __USE_ISOC99

/* Get the architecture specific values describing the floating-point
   evaluation.  The following symbols will get defined:

    float_t	floating-point type at least as wide as `float' used
		to evaluate `float' expressions
    double_t	floating-point type at least as wide as `double' used
		to evaluate `double' expressions

    FLT_EVAL_METHOD
		Defined to
		  0	if `float_t' is `float' and `double_t' is `double'
		  1	if `float_t' and `double_t' are `double'
		  2	if `float_t' and `double_t' are `long double'
		  else	`float_t' and `double_t' are unspecified

    INFINITY	representation of the infinity value of type `float'

    FP_FAST_FMA
    FP_FAST_FMAF
    FP_FAST_FMAL
		If defined it indicates that the `fma' function
		generally executes about as fast as a multiply and an add.
		This macro is defined only iff the `fma' function is
		implemented directly with a hardware multiply-add instructions.

    FP_ILOGB0	Expands to a value returned by `ilogb (0.0)'.
    FP_ILOGBNAN	Expands to a value returned by `ilogb (NAN)'.

    DECIMAL_DIG	Number of decimal digits supported by conversion between
		decimal and all internal floating-point formats.

*/

/* All floating-point numbers can be put in one of these categories.  */
enum
  {
    FP_NAN,
# define FP_NAN FP_NAN
    FP_INFINITE,
# define FP_INFINITE FP_INFINITE
    FP_ZERO,
# define FP_ZERO FP_ZERO
    FP_SUBNORMAL,
# define FP_SUBNORMAL FP_SUBNORMAL
    FP_NORMAL
# define FP_NORMAL FP_NORMAL
  };

/* Return number of classification appropriate for X.  */
# ifdef __NO_LONG_DOUBLE_MATH
#  define fpclassify(x) \
     (sizeof (x) == sizeof (float) ? __fpclassifyf (x) : __fpclassify (x))
# else
#  define fpclassify(x) \
     (sizeof (x) == sizeof (float)					      \
      ? __fpclassifyf (x)						      \
      : sizeof (x) == sizeof (double)					      \
      ? __fpclassify (x) : __fpclassifyl (x))
# endif

/* Return nonzero value if sign of X is negative.  */
# ifdef __NO_LONG_DOUBLE_MATH
#  define signbit(x) \
     (sizeof (x) == sizeof (float) ? __signbitf (x) : __signbit (x))
# else
#  define signbit(x) \
     (sizeof (x) == sizeof (float)					      \
      ? __signbitf (x)							      \
      : sizeof (x) == sizeof (double)					      \
      ? __signbit (x) : __signbitl (x))
# endif

/* Return nonzero value if X is not +-Inf or NaN.  */
# ifdef __NO_LONG_DOUBLE_MATH
#  define isfinite(x) \
     (sizeof (x) == sizeof (float) ? __finitef (x) : __finite (x))
# else
#  define isfinite(x) \
     (sizeof (x) == sizeof (float)					      \
      ? __finitef (x)							      \
      : sizeof (x) == sizeof (double)					      \
      ? __finite (x) : __finitel (x))
# endif

/* Return nonzero value if X is neither zero, subnormal, Inf, nor NaN.  */
# define isnormal(x) (fpclassify (x) == FP_NORMAL)

/* Return nonzero value if X is a NaN.  We could use `fpclassify' but
   we already have this functions `__isnan' and it is faster.  */
# ifdef __NO_LONG_DOUBLE_MATH
#  define isnan(x) \
     (sizeof (x) == sizeof (float) ? __isnanf (x) : __isnan (x))
# else
#  define isnan(x) \
     (sizeof (x) == sizeof (float)					      \
      ? __isnanf (x)							      \
      : sizeof (x) == sizeof (double)					      \
      ? __isnan (x) : __isnanl (x))
# endif

/* Return nonzero value is X is positive or negative infinity.  */
# ifdef __NO_LONG_DOUBLE_MATH
#  define isinf(x) \
     (sizeof (x) == sizeof (float) ? __isinff (x) : __isinf (x))
# else
#  define isinf(x) \
     (sizeof (x) == sizeof (float)					      \
      ? __isinff (x)							      \
      : sizeof (x) == sizeof (double)					      \
      ? __isinf (x) : __isinfl (x))
# endif

/* Bitmasks for the math_errhandling macro.  */
# define MATH_ERRNO	1	/* errno set by math functions.  */
# define MATH_ERREXCEPT	2	/* Exceptions raised by math functions.  */

#endif /* Use ISO C99.  */

#ifdef	__USE_MISC
/* Support for various different standard error handling behaviors.  */
typedef enum
{
  _IEEE_ = -1,	/* According to IEEE 754/IEEE 854.  */
  _SVID_,	/* According to System V, release 4.  */
  _XOPEN_,	/* Nowadays also Unix98.  */
  _POSIX_,
  _ISOC_	/* Actually this is ISO C99.  */
} _LIB_VERSION_TYPE;

/* This variable can be changed at run-time to any of the values above to
   affect floating point error handling behavior (it may also be necessary
   to change the hardware FPU exception settings).  */
extern _LIB_VERSION_TYPE _LIB_VERSION;
#endif


#ifdef __USE_SVID
/* In SVID error handling, `matherr' is called with this description
   of the exceptional condition.

   We have a problem when using C++ since `exception' is a reserved
   name in C++.  */
# ifdef __cplusplus
struct __exception
# else
struct exception
# endif
  {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
  };

# ifdef __cplusplus
extern int matherr (struct __exception *__exc) throw ();
# else
extern int matherr (struct exception *__exc);
# endif

# define X_TLOSS	1.41484755040568800000e+16

/* Types of exceptions in the `type' field.  */
# define DOMAIN		1
# define SING		2
# define OVERFLOW	3
# define UNDERFLOW	4
# define TLOSS		5
# define PLOSS		6

/* SVID mode specifies returning this large value instead of infinity.  */
# define HUGE		3.40282347e+38F

#else	/* !SVID */

# ifdef __USE_XOPEN
/* X/Open wants another strange constant.  */
#  define MAXFLOAT	3.40282347e+38F
# endif

#endif	/* SVID */


/* Some useful constants.  */
#if defined __USE_BSD || defined __USE_XOPEN
# define M_E		2.7182818284590452354	/* e */
# define M_LOG2E	1.4426950408889634074	/* log_2 e */
# define M_LOG10E	0.43429448190325182765	/* log_10 e */
# define M_LN2		0.69314718055994530942	/* log_e 2 */
# define M_LN10		2.30258509299404568402	/* log_e 10 */
# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */
# define M_1_PI		0.31830988618379067154	/* 1/pi */
# define M_2_PI		0.63661977236758134308	/* 2/pi */
# define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
# define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
# define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#endif

/* The above constants are not adequate for computation using `long double's.
   Therefore we provide as an extension constants with similar names as a
   GNU extension.  Provide enough digits for the 128-bit IEEE quad.  */
#ifdef __USE_GNU
# define M_El		2.7182818284590452353602874713526625L  /* e */
# define M_LOG2El	1.4426950408889634073599246810018922L  /* log_2 e */
# define M_LOG10El	0.4342944819032518276511289189166051L  /* log_10 e */
# define M_LN2l		0.6931471805599453094172321214581766L  /* log_e 2 */
# define M_LN10l	2.3025850929940456840179914546843642L  /* log_e 10 */
# define M_PIl		3.1415926535897932384626433832795029L  /* pi */
# define M_PI_2l	1.5707963267948966192313216916397514L  /* pi/2 */
# define M_PI_4l	0.7853981633974483096156608458198757L  /* pi/4 */
# define M_1_PIl	0.3183098861837906715377675267450287L  /* 1/pi */
# define M_2_PIl	0.6366197723675813430755350534900574L  /* 2/pi */
# define M_2_SQRTPIl	1.1283791670955125738961589031215452L  /* 2/sqrt(pi) */
# define M_SQRT2l	1.4142135623730950488016887242096981L  /* sqrt(2) */
# define M_SQRT1_2l	0.7071067811865475244008443621048490L  /* 1/sqrt(2) */
#endif


/* When compiling in strict ISO C compatible mode we must not use the
   inline functions since they, among other things, do not set the
   `errno' variable correctly.  */
#if defined __STRICT_ANSI__ && !defined __NO_MATH_INLINES
# define __NO_MATH_INLINES	1
#endif

/* Get machine-dependent inline versions (if there are any).  */
#ifdef __USE_EXTERN_INLINES
# include <bits/mathinline.h>
#endif


#if defined(__USE_ISOC99) && __USE_ISOC99
/* ISO C99 defines some macros to compare number while taking care
   for unordered numbers.  Since many FPUs provide special
   instructions to support these operations and these tests are
   defined in <bits/mathinline.h>, we define the generic macros at
   this late point and only if they are not defined yet.  */

/* Return nonzero value if X is greater than Y.  */
# ifndef isgreater
#  define isgreater(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x > __y; }))
# endif

/* Return nonzero value if X is greater than or equal to Y.  */
# ifndef isgreaterequal
#  define isgreaterequal(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x >= __y; }))
# endif

/* Return nonzero value if X is less than Y.  */
# ifndef isless
#  define isless(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x < __y; }))
# endif

/* Return nonzero value if X is less than or equal to Y.  */
# ifndef islessequal
#  define islessequal(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x <= __y; }))
# endif

/* Return nonzero value if either X is less than Y or Y is less than X.  */
# ifndef islessgreater
#  define islessgreater(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && (__x < __y || __y < __x); }))
# endif

/* Return nonzero value if arguments are unordered.  */
# ifndef isunordered
#  define isunordered(u, v) \
  (__extension__							      \
   ({ __typeof__(u) __u = (u); __typeof__(v) __v = (v);			      \
      fpclassify (__u) == FP_NAN || fpclassify (__v) == FP_NAN; }))
# endif

#endif

__END_DECLS

/* Missing declarations */

struct complex {
	double x;
	double y;
};

double cabs __P((struct complex));

double gamma_r(double x, int *signgamp); /* wrapper lgamma_r */

long int rinttol(double x);

long int roundtol(double x);

#endif /* !_RTAI_MATH_H  */

#endif
