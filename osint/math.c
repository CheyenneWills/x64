
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
 * math.c - extended math support for spitbol
 *
 * Routines not provided by hardware floating point.
 *
 * These routines are not called from other C routines.  Rather they
 * are called by inter.*, and by external functions.
 */

#include <errno.h>

#include <math.h>
#include <xmmintrin.h>

#include "port.h"

#ifndef errno
int errno;
#endif

extern double inf; /* infinity */

/*
 * f_atn - arctangent
 */
void
f_atn(struct minimal_regs *minregs)
{
    unsigned int mxcsr;

    SET_RA(atan(RA(double)));
    mxcsr = _mm_getcsr();
    if (mxcsr & _MM_EXCEPT_UNDERFLOW) {
        SET_RA(0.0);
    }
}

/*
 * f_chp - chop
 */
void
f_chp(struct minimal_regs *minregs)
{
    if (RA(double) >= 0.0) {
        SET_RA(floor(RA(double)));
    } else {
        SET_RA(ceil(RA(double)));
    }
}

/*
 * f_cos - cosine
 */
void
f_cos(struct minimal_regs *minregs)
{
    unsigned int mxcsr;
    SET_RA(cos(RA(double)));
    mxcsr = _mm_getcsr();
    if (mxcsr & _MM_EXCEPT_UNDERFLOW) {
        SET_RA(0.0);
    }
}

/*
 * f_etx - e to the x
 */
void
f_etx(struct minimal_regs *minregs)
{
    unsigned int mxcsr;
    errno = 0;
    SET_RA(exp(RA(double)));
    mxcsr = _mm_getcsr();
    if (mxcsr & _MM_EXCEPT_UNDERFLOW) {
        SET_RA(0.0);
    } else if (errno) {
        SET_RA(inf);
    }
}

/*
 * f_lnf - natureg_ral log
 */
void
f_lnf(struct minimal_regs *minregs)
{
    unsigned int mxcsr;
    errno = 0;

    SET_RA(log(RA(double)));
    mxcsr = _mm_getcsr();
    if (mxcsr & _MM_EXCEPT_UNDERFLOW) {
        SET_RA(0.0);
    } else if (errno) {
        SET_RA(inf);
    }
}

/*
 * f_sin - sine
 */
void
f_sin(struct minimal_regs *minregs)
{
    unsigned int mxcsr;
    SET_RA(sin(RA(double)));
    mxcsr = _mm_getcsr();
    if (mxcsr & _MM_EXCEPT_UNDERFLOW) {
        SET_RA(0.0);
    }
}

/*
 * f_sqr - square root  (reg_range checked by caller)
 */
void
f_sqr(struct minimal_regs *minregs)
{
    unsigned int mxcsr;
    SET_RA(sqrt(RA(double)));
    mxcsr = _mm_getcsr();
    if (mxcsr & _MM_EXCEPT_UNDERFLOW) {
        SET_RA(0.0);
    }
}

/*
 * f_tan - tangent
 */
void
f_tan(struct minimal_regs *minregs)
{
    unsigned int mxcsr;
    errno = 0;
    SET_RA(tan(RA(double)));
    mxcsr = _mm_getcsr();
    if (mxcsr & _MM_EXCEPT_UNDERFLOW) {
        SET_RA(0.0);
    } else if (errno) {
        SET_RA(inf);
    }
}
