/* Floating point porting toolkit. 
   Define the needed features in tm.h

   NEED_ATOF
   NEED_MODF
   NEED_FREXP
   NEED_LDEXP

*/
#include "config.h"
#include <math.h>
#include <ctype.h>

#ifdef NEED_ATOF
double	twoe56	= 72057594037927936.; /*2^56*/
double	exp5[]	= {5.,25.,625.,390625.,152587890625.,23232710361480712890625.};

double atof (char *p)
{
  extern double ldexp();
  register c, exp = 0, eexp = 0;
  double fl = 0, flexp = 1.0;
  int bexp, neg = 1, negexp = 1;

  while((c = *p++) == ' ');
  if (c == '-') neg = -1;	else if (c == '+'); else --p;

  while ((c = *p++), isdigit(c))
    if (fl < twoe56) fl = 10*fl + (c-'0'); else exp++;
  if (c == '.')
    while ((c = *p++), isdigit(c))
      if (fl < twoe56)
	{
	  fl = 10*fl + (c-'0');
	  exp--;
	}
  if ((c == 'E') || (c == 'e'))
    {
      if ((c= *p++) == '+'); else if (c=='-') negexp = -1; else --p;
      while ((c = *p++), isdigit(c)) eexp = 10*eexp + (c-'0');
      if (negexp < 0) eexp = -eexp; exp += eexp;
    }
  bexp = exp;
  if (exp < 0) exp = -exp;

  for (c = 0; c < 6; c++)
    {
      if (exp & 01) flexp *= exp5[c];
      exp >>= 1; if (exp == 0) break;
    }

  if (bexp < 0) fl /= flexp; else fl *= flexp;
  fl = ldexp(fl, bexp);
  if (neg < 0) return(-fl); else return(fl);
}
#endif

#ifdef NEED_FREXP
double frexp (double value, int *eptr)
{
  double fr=value;
  int exp=0;

  if(fr!=0.0) {
    while(fr >= 1.0 || fr <= -1.0) {
      fr/=2;
      exp++;
    }
    while(fr < 0.5 && fr > -0.5) {
      fr*=2;
      exp--;
    }
  }
  *eptr=exp;
  return fr;
}
#endif

#ifdef NEED_LDEXP
double ldexp (double x, int n)
{
  if(n>0)
    while(n--)
      x*=2;
  else
    while(n++)
      x/=2;
  return x;
}
#endif


#ifdef NEED_MODF
double modf (double value, double *iptr)
{
  int i=floor(value);
  *iptr=(double)i;
  return value-(double)i;
}

#endif
