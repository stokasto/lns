#include "lns.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

/*TODO: this function is bloody inefficient :) --> pow etc. */
lfloat
float2lfrac(float x)
{
    float tmp = 0.;
    lfloat res = 0;
    int i;
    for (i = 0; i < NFRAC_BITS; ++i)
        {
            tmp = pow(2, -1. * (i+1));
            LOG("test f2lfrac x: %f - %d %f %f\n",x, i, tmp, x - tmp);
            // NOTE we cap the precision here 
            if((fabs(x - tmp) < 0.00001 ? 0. : x - tmp) >= 0.) 
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
            x   *= 2;
        }
    
    while (x >= 2) 
        {
            ip += 1;
            x   /= 2;
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
    res = (ip_nover << (NFRAC_BITS+1)) | ifrac; // append fraction to integral part
    LOG("ip: %d\n", ip<<(NFRAC_BITS+1));
    // sign if necessary
    if ( sign )
        SET_BIT(res, SIGN);
    // TODO set special zero flag etc.
    return res;
}

/* TODO this is also inefficient */
float 
lfrac2float(lfloat x)
{
    int i;
    float res = 0;
    for (i = 0; i < NFRAC_BITS; ++i)
        {
           if ( CHECK_BIT(x, NFRAC_BITS - i) )
            {
                res += pow(2, -1. * (i+1));
            }

        }
    return res;
}

float
lfloat2float(lfloat x)
{
    printf("converting: ");
    bitprint(x & (1 << SIGN));
    int sign = CHECK_BIT(x, SIGN) ? -1 : 1;
    int8_t ip = 0.;
    float ifrac = 0.;
    ip = (x & INT_BITS) >> (NFRAC_BITS+1);
    if ( CHECK_BIT(ip, 5) )
        { // negative number
            // set other bits for proper complement
            ip |= 0x3 << 6;
            LOG("ip: %d\n", ip);
        }
    ifrac = lfrac2float(x & FRAC_BITS);
    return sign * pow(2., ip + ifrac);
}


lfloat
multlf(lfloat x, lfloat y)
{
    lfloat res = x + y;
    lfloat sign;
    res = CLEAR_BIT(res, SIGN); // TODO: this should not be necessary
    sign = CHECK_BIT(x, SIGN) ^ CHECK_BIT(y, SIGN);
    return res | sign;
}


lfloat
divlf(lfloat x, lfloat y)
{
    lfloat res = x - y;
    lfloat sign;
    res = CLEAR_BIT(res, SIGN); // TODO: this should not be necessary
    sign = CHECK_BIT(x, SIGN) ^ CHECK_BIT(y, SIGN);
    return res | sign;
}

void
bitprint(lfloat x)
{
    int i = 16;
    int shift;
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
    lfloat lval = float2lfloat(val);
    lfloat lval2 = float2lfloat(val2);
    lfloat lval3 = float2lfloat(val3);
    lfloat lvalof2 = float2lfloat(2.);
    printf("Test conversion from float to lfloat:\n");
    printf("float: %f => lfloat: 0x%x\n", val, lval);
    printf("bit print: ");
    bitprint(lval);
    printf("float: %f => lfloat: 0x%x\n", val3, lval3);
    printf("bit print: ");
    bitprint(lval3);
    
    printf("\nTest conversion from lfloat to float:\n");
    printf("lfloat: 0x%x => float: %f == %f\n", lval, lfloat2float(lval), val);
    printf("lfloat: 0x%x => float: %f == %f\n", lval2, lfloat2float(lval2), val2);
    printf("lfloat: 0x%x => float: %f == %f\n", lval3, lfloat2float(lval3), val3);
    
    printf("\nTest ops for positive floats:\n");
    printf("Multiplication: %f * %f => 0x%x == 0x%x\n",val, 2., float2lfloat(val*2.), multlf(lval, lvalof2));  
    printf("Multiplication: %f * %f => 0x%x == 0x%x\n",val, val2, float2lfloat(val*val2), multlf(lval, lval2));  
    printf("Division: %f / %f => 0x%x == 0x%x\n",val, 2., float2lfloat(val/2.), divlf(lval, lvalof2));  
    return 0;
}
