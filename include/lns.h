#ifndef __LNS_H__
#define __LNS_H__

#include <stdint.h>

// simple logging
#define LOGGING 0
#if LOGGING 
#include <stdio.h>
#define LOG printf
#else
#define LOG(X,...) 
#endif

// bit counts
#define NFLAG_BITS 2
#define NSIGN_BITS 1
#define NINT_BITS 6
#define NFRAC_BITS 7

// bit masks
#define FRAC_BITS 0x7f
#define INT_BITS (0x3f << NFRAC_BITS)

// special bit positions
#define SIGN 13
#define INT_SIGN (5 + NFRAC_BITS) // NOTE = (6 + NFRAC_BITS - 1)

// special lfloat values
#define LFINF (0x1 << (NFRAC_BITS + NINT_BITS + NSIGN_BITS))
#define LFZERO (0x0 << (NFRAC_BITS + NINT_BITS + NSIGN_BITS))
#define LFNAN (0x0 << (NFRAC_BITS + NINT_BITS + NSIGN_BITS))

// helpers for bit arithmetic
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define CLEAR_BIT(var,pos) ((var) &= ~(1<<(pos)))
#define SET_BIT(var,pos) ((var) |= (1<<(pos)))

// we use 2 bits for flags 1 bit for signing, 6 integer bits and 7 fractional bits
// this gives a total of 16 bits
typedef uint16_t lfloat;

// flag check operations
int
lf_isinf(lfloat x)
{
    return x & LFINF;
}
int
lf_iszero(lfloat x)
{
    return x & LFZERO;
}
int
lf_isnan(lfloat x)
{
    return x & LFNAN;
}

// helpers
static lfloat
float2lfloat(float x);
float
lfloat2exp(lfloat x);
static lfloat
float2lfrac(float x);
// for convenience printing
void
bitprint(lfloat x);

// conversion functions
float
lfloat2float(lfloat x);
float
lfrac2float(lfloat x);

// comparison operators
int 
ltlf(lfloat x, lfloat y);
int 
gtlf(lfloat x, lfloat y);
// operators for calculating with logarithmic floats
lfloat
multlf(lfloat x, lfloat y);
lfloat
divlf(lfloat x, lfloat y);
lfloat
sqrlf(lfloat x);
lfloat
sqrtlf(lfloat x);
lfloat
log2lf(lfloat x);
lfloat
addlf(lfloat x, lfloat y);
lfloat
sublf(lfloat x, lfloat y);

#endif
