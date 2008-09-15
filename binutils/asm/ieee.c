/* @(#)ieee.c	1.6 1/4/91 1 */

#include "app.h"
#include "ieee.h"
#include <math.h>


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      convert_to_ieee                                                 *
*                                                                      *
*   Description                                                        *
*       Convert a double to ieee single precision floating point       *
*   format or to the 21000's extended precision format.                *
*                                                                      *
*   Revision History                                                   *
*     name    date          description              number            *
*     mkc     1/1/89        format and document        --              *
*                                                                      *
***********************************************************************/

void convert_to_ieee( double d, char rnd, char precision, unsigned long *mant, int *exp )
{
    double	    temp;
    unsigned long   sign;
    short	    bits;
 
    bits = (precision == SINGLE_PRECISION) ? SINGLE_MANT : EXTENDED_MANT;

    if( d == 0.0 )
    {
	*exp = 0;
        *mant = 0;
        return;
    }

    /* Get the normalized mantissa and the exponent */

    temp = frexp( d, exp );
    temp *= 2.0;
    (*exp)--;


    if( *exp > 127 )   /* This is infinity */
    {
        *exp = 128 + EXPONENT_BIAS;
        *mant = 0;

        if( d < 0.0 )
            *mant |= 0x80000000;

        return;
    }
    else if( *exp < -126 )  /* Too small to represent */
    {
        *exp = 0;
        *mant = 0;

        if( d < 0.0 )
            *mant |= 0x80000000;

        return;
    }
        
    temp = temp * ((unsigned long)(1L << bits));

    if( temp < 0.0 )
    {
	temp = -temp;
        sign = 0x80000000;
    }
    else
        sign = 0;

    *mant = (unsigned long) temp;
    temp = temp - *mant;

    /* Check to see if the mantissa was rounded up by the compiler */

    if( temp < 0.0 )
    {
        (*mant)--;
        temp += 1.0;
    }
    
    if( precision == SINGLE_PRECISION )
	*mant &= 0x007FFFFF;
    else
	*mant &= 0x7FFFFFFF;

    switch( rnd )
    {
	    case RND_ZERO:
               break;

            case RND_PLUS_INF:
               if( d > 0.0 )
                   if( temp != 0.0 )
                       (*mant)++;
               break;

            case RND_MINUS_INF:
               if( d < 0.0 )
                   if( temp != 0.0 )
                       (*mant)++;
               break;

            case RND_NEAREST:
               if( temp == 0.5 )
               {
                   if( *mant & 0x1 )
                       (*mant)++;		   
	       }
               else if( temp > 0.5 )
                   (*mant)++;
               break;
    }

    /* Check for overflow */

    if( precision == SINGLE_PRECISION )
    {
	if( *mant & 0xFF800000 )
	{
	    (*exp)++;
	    if (*exp > 127)
	    {
		*exp = 128;
		*mant = 0;
	    }
	}
    }
    else
    {
	if( *mant & 0x80000000 )
	{
	    (*exp)++;
	    if( *exp > 127 )
	    {
		*exp = 128;
		*mant = 0;
	    }
	}
    }

    *mant &= 0x7FFFFFFF;
    *mant |= sign;
    *exp += EXPONENT_BIAS;
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      format_ieee                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void format_ieee( unsigned long mant, int exp, char *number,    *
*                        char precision )                              *
*                                                                      *
*   Description                                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     10/20/89      created                  -----             *
***********************************************************************/

void format_ieee( unsigned long mant, int exp, unsigned char *number, char precision )
{
    if( !mant && !exp )
    {
        *((unsigned long *) number) = 0;
        if( precision == EXTENDED_PRECISION)
	    number[4] = 0;
        return;
    }

    if( mant & 0x80000000 )
        number[0] = 0x80;
    else
        number[0] = 0;

    if( precision == SINGLE_PRECISION )
    {
	number[0] |= (exp & 0xFEL) >> 1;
        number[1] = (unsigned char) (((exp & 0x1L) << 7) | ((mant & 0x007F0000L) >> 16));
        number[2] = (unsigned char) ((mant & 0x0000FF00L) >> 8);
        number[3] = (unsigned char) ((mant & 0x000000FFL));
    }
    else
    {
	number[0] |= (exp & 0xFEL) >> 1;
        number[1] = (unsigned char) (((exp & 0x1L) << 7) | ((mant & 0x7F000000L) >> 24));
        number[2] = (unsigned char) ((mant & 0x00FF0000L) >> 16);
        number[3] = (unsigned char) ((mant & 0x0000FF00L) >> 8);
        number[4] = (unsigned char) (mant & 0x000000FFL);
    }
        
}
