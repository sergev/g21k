/* This is a helper library for use with the 
   TargetTrace API.  It provides to things.

   1. A 'debug_message' function which prints the message
      in humman readable form.

   2. It provides a 'dummy_target' to help build application
      with out the use of a real target.
*/
#include <stdio.h>
#include <adi/targtrac.h>
#include <adi/generat.h>
#include <stdarg.h>

static char *m_names[] = {
#define PROTOCOL(message, enum, ops, proto) #message,
#include <adi/protocol.def>
#undef PROTOCOL
};

static long m_map[] = {
#define PROTOCOL(message, enum, ops, proto) enum,
#include <adi/protocol.def>
#undef PROTOCOL
};

static char *m_protocol[] = {
#define PROTOCOL(message, enum, ops, proto) proto,
#include <adi/protocol.def>
#undef PROTOCOL
};

static char m2index[256];

static init_protocol ()
{
  int i;
  for (i=0;i<sizeof (m_map)/sizeof (m_map[0]);i++) {
    m2index[m_map[i]]=i;
  }
}

#define messagenames(x) m_names[m2index[x]]
#define protocol(x) m_protocol[m2index[x]]


/*
   a - address
   P - pointer
   < - input block
   > - output block
   l - long integer (length)
   i - long integer (arbitrary integer)
   r - register no
   s - zero terminated string.
   f - function hook 
   * - message is unavailable
   ? - marks next object as return entity
   @ - signifies that this message produces output.
   
   if < is not followed by l then 1 is implied. 
   if > is not followed by l then 1 is implied.
   */


debug_message (char *info, TARGETID tid, long op, va_list ap)
{
  static int init = 0;
  static int mid;
  char *p;

  if (!init) {
    init_protocol ();
    init ++;
  }

  printf ("<%s:%d %ld %s (", info, mid++, op, messagenames(op));

  p = protocol(op);
  if (ap) 
    while (p && *p) {
      switch (*p++) {
      case 'a':
	printf ("addr: %06lx ", va_arg (ap, long));
	break;
      case 'P':
      case '>':
      case '<':
	printf ("bptr: %06lx ", va_arg (ap, long));
	break;
      case 'r':
	printf ("reg: %d ", va_arg (ap, int));
	break;
      case 'l':
	printf ("len: %ld ", va_arg (ap, long));
	break;
      case 'i':
	printf ("int: %ld ", va_arg (ap, long));
	break;
      default: break;
      }
    }
  else
    printf (p);

  printf(")>\n");
}

unsigned long
dummy_target (TARGETID tid, long op, ...)
{
  va_list ap;
  va_start (ap, op);
  debug_message ("dummy", tid, op, ap);
  va_end (ap);
  return EUNSUPPORTED;
}
