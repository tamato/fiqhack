/*	SCCS Id: @(#)config1.h	3.4	1999/12/05	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG1_H
#define CONFIG1_H

#ifdef __TURBOC__
# define __MSC		/* increase Borland C compatibility in libraries */
#endif

/*
 * Mac Stuff.
 */

#if defined(__BEOS__)
# define DLB
# undef UNIX
#endif

#ifdef __BEOS__
# define NEED_VARARGS
#endif

#define NEARDATA

/*
 * Atari auto-detection
 */

#ifdef atarist
# undef UNIX
# ifndef TOS
# define TOS
# endif
#else
# ifdef __MINT__
#  undef UNIX
#  ifndef TOS
#  define TOS
#  endif
# endif
#endif

/*
 * Windows NT Autodetection
 */
#ifdef _WIN32_WCE
#define WIN_CE
# ifndef WIN32
# define WIN32
# endif
#endif

#ifdef WIN32
# undef UNIX
# define NHSTDC
# define USE_STDARG
# define NEED_VARARGS

#ifndef WIN_CE
# define STRNCMPI
# define STRCMPI
#endif

#endif


#if defined(__linux__) && defined(__GNUC__) && !defined(_GNU_SOURCE)
/* ensure _GNU_SOURCE is defined before including any system headers */
# define _GNU_SOURCE
#endif

#ifdef VMS	/* really old compilers need special handling, detected here */
# undef UNIX
# ifdef __DECC
#  ifndef __DECC_VER	/* buggy early versions want widened prototypes */
#   define NOTSTDC	/* except when typedefs are involved		*/
#   define USE_VARARGS
#  else
#   define NHSTDC
#   define USE_STDARG
#   define POSIX_TYPES
#   define _DECC_V4_SOURCE	/* avoid some incompatible V5.x changes */
#  endif
#  undef __HIDE_FORBIDDEN_NAMES /* need non-ANSI library support functions */
# else
#  ifdef VAXC	/* must use CC/DEFINE=ANCIENT_VAXC for vaxc v2.2 or older */
#   ifdef ANCIENT_VAXC	/* vaxc v2.2 and earlier [lots of warnings to come] */
#    define KR1ED	/* simulate defined() */
#    define USE_VARARGS
#   else		/* vaxc v2.3,2.4,or 3.x, or decc in vaxc mode */
#     if defined(USE_PROTOTYPES) /* this breaks 2.2 (*forces* use of ANCIENT)*/
#      define __STDC__ 0 /* vaxc is not yet ANSI compliant, but close enough */
#      define signed	/* well, almost close enough */
#include <stddef.h>
#     endif
#     define USE_STDARG
#   endif
#  endif /*VAXC*/
# endif /*__DECC*/
# ifdef VERYOLD_VMS	/* v4.5 or earlier; no longer available for testing */
#  define USE_OLDARGS	/* <varargs.h> is there, vprintf & vsprintf aren't */
#  ifdef USE_VARARGS
#   undef USE_VARARGS
#  endif
#  ifdef USE_STDARG
#   undef USE_STDARG
#  endif
# endif
#endif /*VMS*/

#ifdef vax
/* just in case someone thinks a DECstation is a vax. It's not, it's a mips */
# ifdef ULTRIX_PROTO
#  undef ULTRIX_PROTO
# endif
# ifdef ULTRIX_CC20
#  undef ULTRIX_CC20
# endif
#endif

#ifdef KR1ED		/* For compilers which cannot handle defined() */
#define defined(x) (-x-1 != -1)
/* Because:
 * #define FOO => FOO={} => defined( ) => (-1 != - - 1) => 1
 * #define FOO 1 or on command-line -DFOO
 *	=> defined(1) => (-1 != - 1 - 1) => 1
 * if FOO isn't defined, FOO=0. But some compilers default to 0 instead of 1
 * for -DFOO, oh well.
 *	=> defined(0) => (-1 != - 0 - 1) => 0
 *
 * But:
 * defined("") => (-1 != - "" - 1)
 *   [which is an unavoidable catastrophe.]
 */
#endif

#endif	/* CONFIG1_H */
