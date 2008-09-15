
#ifndef _OBSTACK_H
#define _OBSTACK_H

#define OBSTACK_HAS_DESTROY

#ifndef DLLINTERFACE
#ifdef _WINDLL
#ifdef WIN32
#define DLLINTERFACE
#else
#define DLLINTERFACE _loadds
#endif /* end of #ifdef WIN32 */ 
#else
#define DLLINTERFACE
#endif
#endif

#ifndef FAR
#ifdef MSDOS
#define FAR far
#else
#define FAR
#endif
#endif

typedef void FAR *POINTER;

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

void DLLINTERFACE obstack_begin (struct obstack *obstack, int size);
void DLLINTERFACE obstack_destroy (struct obstack *obstack);
void DLLINTERFACE obstack_init (struct obstack *obstack);
void * DLLINTERFACE obstack_alloc (struct obstack *obstack, int size);
void DLLINTERFACE obstack_grow (struct obstack *obstack, void *data, int size);
void * DLLINTERFACE obstack_finish (struct obstack *obstack);
void *DLLINTERFACE obstack_copy0 (struct obstack *obstack, void *x, int len);
int DLLINTERFACE obstack_free (struct obstack *obstack, void *p);
#endif
