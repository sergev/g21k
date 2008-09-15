#ifndef _OBSTACK_H
#define _OBSTACK_H
#include <ansidecl.h>

#define OBSTACK_HAS_DESTROY

typedef char FAR *POINTER;

struct obstack_chunk
{
  struct obstack_chunk *prev;
  int dot;
  int size;
  char contents[4];
};

struct obstack
{
  long   chunk_size;    /* preferred size to allocate chunks in */
  struct obstack_chunk * chunk; /* address of current struct obstack_chunk */
  char  *object_base;   /* address of object we are building */
  char  *next_free;     /* where to add next char to current object */
  char  *chunk_limit;   /* address of char after current chunk */
};               

char DLLINTERFACE obstack_begin PARAMS((struct obstack *obstack, int size));
char DLLINTERFACE obstack_destroy PARAMS((struct obstack *obstack));
char DLLINTERFACE obstack_init PARAMS((struct obstack *obstack));
char *DLLINTERFACE obstack_alloc PARAMS((struct obstack *obstack, int size));
char *DLLINTERFACE obstack_grow PARAMS((struct obstack *obstack, char *data, int size));
int DLLINTERFACE obstack_grow1 PARAMS((struct obstack *, int ));
char * DLLINTERFACE obstack_finish PARAMS((struct obstack *obstack));
char *DLLINTERFACE obstack_copy0 PARAMS((struct obstack *obstack, char *x, int len));
int DLLINTERFACE obstack_free PARAMS((struct obstack *obstack, char *p));
#endif
