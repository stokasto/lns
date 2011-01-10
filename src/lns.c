#include "lns.h"

#include <stdio.h>
#include <math.h>

/* for caching often recomputed values */
int POW2CACHEINIT = 0;
float APPROX_MARGIN = 0.;
float POW2CACHE[NFRAC_BITS];

float FETCHPOW2(int i) 
    { 
        int j; 
        if (!POW2CACHEINIT) 
            { 
                for(j = 0; j < NFRAC_BITS; ++j) 
                    POW2CACHE[j] = 0.; 
                POW2CACHEINIT = 1; 
            } 
        if (POW2CACHE[i] == 0.) 
            { 
                POW2CACHE[i] = pow(2., -1 * (i+1)); 
            } 
        return POW2CACHE[i]; 
    }
float FETCHMARGIN()
{
    if (APPROX_MARGIN == 0. )
        APPROX_MARGIN = pow(2., -NFRAC_BITS);
    return APPROX_MARGIN;
}

/*TODO: this function is bloody inefficient :) --> pow etc. */
lfloat
float2lfrac(float x)
{
    float tmp = 0.;
    lfloat res = 0;
    int i;
    for (i = 1; i <= NFRAC_BITS; ++i)
        {
            tmp = FETCHPOW2(i-1);
            LOG("test f2lfrac x: %f - %d %f %f\n",x, i, tmp, x - tmp);
            // NOTE we cap the precision here  using 2^(frac bits)
            if((fabs(x - tmp) < FETCHMARGIN() ? 0. : x - tmp) >= 0.) 
                {
                    res |= 1 << (NFRAC_BITS - i);
                    x -= tmp;
                }
        }
    return res;
}

lfloat
float2lfloat(float x)
{
    int sign = x < 0 ? 1 : 0;
    lfloat res = 0;
    lfloat ip = 0;
    lfloat ip_nover = 0;
    float frac = 0.;
    int ifrac = 0;
    float fp = 1.;
    float tol = 1e-13;

    x = fabs(x);
    
    while (x < 1) 
        {
            ip -= 1;
            x  *= 2;
        }
    
    while (x >= 2) 
        {
            ip += 1;
            x  /= 2;
        }
    
    while (fp >= tol) 
        {
            fp /= 2;
            x  *= x;
            if (x >= 2) 
                {
                    x   /= 2;
                    frac += fp;
                }
        }
    
    // we need the most significant bits of the fraction
    ifrac = float2lfrac(frac);
    LOG("ip: %d\n", ip);
    // ip without overflow
    ip_nover = (ip & 0x3f);
    if (ip > ip_nover)
        { // overflow detected 
            //TODO fix this case!
        }
    res = (ip_nover << NFRAC_BITS) | ifrac; // append fraction to integral part
    LOG("ip: %d\n", ip<<NFRAC_BITS);
    // sign if necessary
    if ( sign )
        SET_BIT(res, SIGN);
    // TODO set special zero flag etc.
    return res;
}

/* TODO this might also be inefficient */
float 
lfrac2float(lfloat x)
{
    int i;
    float res = 0;
    for (i = 1; i <= NFRAC_BITS; ++i)
        {
           if ( CHECK_BIT(x, NFRAC_BITS - i) )
            {
                res += FETCHPOW2(i-1);
            }
        }
    return res;
}

float
lfloat2float(lfloat x)
{
    int sign = CHECK_BIT(x, SIGN) ? -1 : 1;
    int8_t ip = 0.;
    float ifrac = 0.;
    ip = (x & INT_BITS) >> NFRAC_BITS;
    if ( CHECK_BIT(ip, 5) )
        { // negative number
            // set other bits for proper complement
            ip |= 0x3 << 6;
            LOG("ip: %d\n", ip);
        }
    ifrac = lfrac2float(x & FRAC_BITS);
    return sign * pow(2., ip + ifrac);
}


/* lossless operations */
lfloat
multlf(lfloat x, lfloat y)
{
    lfloat res = x + y;
    lfloat sign;
    CLEAR_BIT(res, SIGN); // TODO: this should not be necessary
    sign = CHECK_BIT(x, SIGN) ^ CHECK_BIT(y, SIGN);
    return res | sign;
}


lfloat
divlf(lfloat x, lfloat y)
{
    lfloat res = x - y;
    lfloat sign;
    CLEAR_BIT(res, SIGN); // TODO: this should not be necessary
    sign = CHECK_BIT(x, SIGN) ^ CHECK_BIT(y, SIGN);
    return res | sign;
}

lfloat
sqrlf(lfloat x)
{
    lfloat res = x << 1;
    CLEAR_BIT(res, SIGN); 
    // TODO: beware of overflows
    return res;
}

lfloat
sqrtlf(lfloat x)
{
    // TODO: check sign before op 
    lfloat res = x >> 1;
    CLEAR_BIT(res, SIGN); 
    return res;
}

#define SAFE_CHECK(X,Y,OP,DEF) \
                { \
                    if(CHECK_BIT(X, INT_SIGN)) \
                       { \
                            if(CHECK_BIT(Y, INT_SIGN)) \
                                return X OP Y; \
                            else \
                                return DEF; \
                        } \
                    if(CHECK_BIT(Y, INT_SIGN)) \
                        return 1-DEF; \
                    return X OP Y; \
                }
int 
gtlf(lfloat x, lfloat y)
{
    // TODO: check sign and flags before op 
    if (CHECK_BIT(x, SIGN))
        { // negative number found --> special comparison
          // as we store numbers in 2ary complement
            if(CHECK_BIT(y, SIGN))
                { // y < 0 --> invert comparison
                    SAFE_CHECK(x,y,<,1);
                }
            // y > 0 --> trivial case
            return 0;
        }
    if(CHECK_BIT(y, SIGN))
        { // y < 0 --> trivial case 
            return 1;
        }
    SAFE_CHECK(x,y,>,0);
}


int 
ltlf(lfloat x, lfloat y)
{
    // TODO: check sign and flags before op 
    if (CHECK_BIT(x, SIGN))
        { // negative number found --> special comparison
          // as we store numbers in 2ary complement
            if(CHECK_BIT(y, SIGN))
                { // y < 0 --> invert comparison
                    SAFE_CHECK(x,y,>,0);
                }
            // y > 0 --> trivial case
            return 1;
        }
    if(CHECK_BIT(y, SIGN))
        { // y < 0 --> trivial case 
            return 0;
        }
    SAFE_CHECK(x,y,<,1);
}

/* imprecise operations */

lfloat
log2lf(lfloat x)
{
    // TODO: check sign bit
    // calculate the exponent 
    float tmp = ((int8_t) ((x & INT_BITS) >> NFRAC_BITS)) + lfrac2float(x & FRAC_BITS);
    // re-encode as log val -> IMPRECISE OP
    return float2lfloat(tmp);
}

void
bitprint(lfloat x)
{
    int i = 16;
    //int shift;
    char buf[17];
    while(i)
        {
            --i;
            buf[i] = (x & 1) + '0';
            x >>= 1;
        }
    buf[16] = '\0';
    printf("%s\n", buf);
}

int 
main(void)
{
    float val = 2.37841;
    float val2 = 4.5;
    float val3 = 0.35355;
    float valbig = 7304.2386;
    lfloat lval = float2lfloat(val);
    lfloat lval2 = float2lfloat(val2);
    lfloat lval3 = float2lfloat(val3);
    lfloat lvalof2 = float2lfloat(2.);
    lfloat lvalbig = float2lfloat(valbig);
    printf("Test conversion from float to lfloat:\n");
    printf("float: %f => lfloat: 0x%x\n", val, lval);
    printf("bit print: ");
    bitprint(lval);
    printf("float: %f => lfloat: 0x%x\n", val2, lval2);
    printf("bit print: ");
    bitprint(lval2);
    printf("float: %f => lfloat: 0x%x\n", val3, lval3);
    printf("bit print: ");
    bitprint(lval3);
    printf("float: %f => lfloat: 0x%x\n", valbig, lvalbig);
    printf("bit print: ");
    bitprint(lvalbig);
    
    printf("\nTest conversion from lfloat to float:\n");
    printf("lfloat: 0x%x => float: %f == %f\n", lval, lfloat2float(lval), val);
    printf("lfloat: 0x%x => float: %f == %f\n", lval2, lfloat2float(lval2), val2);
    printf("lfloat: 0x%x => float: %f == %f\n", lval3, lfloat2float(lval3), val3);
    printf("lfloat: 0x%x => float: %f == %f\n", lvalbig, lfloat2float(lvalbig), valbig);
    
    printf("\nTest ops for positive floats:\n");
    printf("Multiplication: %f * %f = %f => 0x%x == 0x%x == %f\n",val, 2., val * 2., float2lfloat(val*2.), multlf(lval, lvalof2), lfloat2float(multlf(lval, lvalof2)));  
    printf("Multiplication: %f * %f = %f => 0x%x == 0x%x == %f\n",val, val2, val * val2, float2lfloat(val*val2), multlf(lval, lval2), lfloat2float(multlf(lval, lval2)));  
    printf("Division: %f / %f = %f => 0x%x == 0x%x == %f\n",val, 2., val/2., float2lfloat(val/2.), divlf(lval, lvalof2), lfloat2float(divlf(lval, lvalof2)));  
    printf("Power: %f ^ 2 = %f => 0x%x == 0x%x == %f\n",val, val*val, float2lfloat(val*val), sqrlf(lval), lfloat2float(sqrlf(lval)));  
    printf("Power: %f ^ 2 = %f => 0x%x == 0x%x == %f\n",valbig, valbig*valbig, float2lfloat(valbig*valbig), sqrlf(lvalbig), lfloat2float(sqrlf(lvalbig)));  
    printf("SQRT: sqrt(%f) = %f => 0x%x == 0x%x == %f\n",val, sqrt(val), float2lfloat(sqrt(val)), sqrtlf(lval), lfloat2float(sqrtlf(lval)));  
    printf("log2: log2(%f) = %f => 0x%x == 0x%x == %f\n",val, log2(val), float2lfloat(log2(val)), log2lf(lval), lfloat2float(log2lf(lval)));  
    printf("<: %f < %f = %d == %d\n", val, val2, val < val2, ltlf(lval, lval2));
    printf("<: %f < %f = %d == %d\n", val3, val2, val3 < val2, ltlf(lval3, lval2));
    printf(">: %f < %f = %d == %d\n", val, val2, val > val2, gtlf(lval, lval2));
    printf(">: %f < %f = %d == %d\n", val3, val2, val3 > val2, gtlf(lval3, lval2));
    printf(">: %f > %f = %d == %d\n", val2, val3, val2 > val3, gtlf(lval2, lval3));
    return 0;
}
