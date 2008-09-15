/* Configuration for GCC for Intel i386 running Linux.
 *
 * Written by H.J. Lu (hlu@eecs.wsu.edu)
 */

#include "i386/xm-i386.h"

/*EK* modified - I think the Linux version distributed was pretty ancient */
/*EK* removed: #include "xm-svr3.h" */

/*EK* new bits follows... */
#include <stdlib.h>
#include <string.h>

#define bcopy(src,dst,len) memcpy ((dst),(src),(len))
#define bzero(dst,len) memset ((dst),0,(len))
#define bcmp(left,right,len) memcmp ((left),(right),(len))

#define rindex strrchr
#define index strchr

/* remove... #define USG */

#ifndef POSIX
#define POSIX
#endif
#define HAVE_VPRINTF
#define HAVE_PUTENV
#define HAVE_STRERROR

#ifndef SVR4
#define SVR4
#endif

/* SVR4 provides no sys_siglist,
   but does offer the same data under another name.  */
/* #define sys_siglist _sys_siglist */

/*EK* end of new bits */

#undef BSTRING
#define BSTRING
/*#undef bcmp*/
/*#undef bcopy*/
/*#undef bzero*/
#undef index
#undef rindex

#if 0 /* These conflict with stdlib.h in protoize, it is said,
	 and there's no evidence they are actually needed.  */
#undef malloc(n)
#define malloc(n)       malloc ((n) ? (n) : 1)
#undef calloc(n,e)
#define calloc(n,e)     calloc (((n) ? (n) : 1), ((e) ? (e) : 1))
#endif
