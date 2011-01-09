#ifndef __LNS_H__
#define __LNS_H__

#include <stdint.h>

// simple logging
#define LOGGING 0
#if LOGGING 
#define LOG printf
#else
#define LOG(X,...) 
#endif


#define SIGN 13

#define NFLAG_BITS 2
#define NSIGN_BITS 1
#define NINT_BITS 6
#define NFRAC_BITS 7

#define FRAC_BITS 0x7f
#define INT_BITS (0x3f << NFRAC_BITS)

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define CLEAR_BIT(var,pos) ((var) & ~(1<<(pos)))
#define SET_BIT(var,pos) ((var) | (1<<(pos)))

// we use 2 bits for flags 1 bit for signing, 6 integer bits and 7 fractional bits
typedef uint16_t lfloat;

// for conversion
lfloat
float2lfloat(float x);
lfloat
float2lfrac(float x);

float
lfloat2float(lfloat x);
float
lfrac2float(lfloat x);

// for calculing with logarithmic floats
lfloat
multlf(lfloat x, lfloat y);
lfloat
divlf(lfloat x, lfloat y);

// for convenience printing
void
bitprint(lfloat x);

#endif
