/* @(#)expr.h	1.1 11/8/90 1 */


typedef struct
{
    short expr_type;

    union
    {
	long int_val;
        double float_val;
    } value;
} EXPR;

