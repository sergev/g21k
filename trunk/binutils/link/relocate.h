/* @(#)relocate.h	1.5  6/11/91 */

typedef union
{
    long l;
    char c[4];
} VALUE;


#define DATA32_TOP8(x)         (((x) & 0xFF000000L) >> 24L)
#define DATA32_TOP_MIDDLE8(x)  (((x) & 0x00FF0000L) >> 16L)
#define DATA32_MIDDLE8(x)      (((x) & 0x0000FF00L) >> 8L)
#define DATA32_BOTTOM8(x)      ((x) & 0x000000FFL)
