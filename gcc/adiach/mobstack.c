/* @chapter Obstack
Object Stacks are data object heaps that perform efficient memory managment
for data structures.  

@enumerate
@item The life time of a collection of data objects can be grouped together 
and freed all at once.
@item Unknown Length Strings can be created on the fly.
@item the use of malloc and free can be minimized.
@end enumerate

@example
@cartouche

   +--------------+
   |    size      |
   +--------------+       +-------+       +-------+       +-------+
   |   chunks     |------>|dot,sz |------>|dot,sz |------>|dot,sz |->0
   +--------------+       +-------+       +-------+       +-------+
   |    base      |       |%%%%%%%|       |%%%%%%%|       |%%%%%%%|
   +--------------+       |%%%.   |       |%%%%%%%|       |%%%%%%%|
   |    free      |       +---^---+       +-------+       +-------+
   +--------------+           |dot
   |   limit      |
   +--------------+

@end cartouche
@end example
@center The Obstack, Heap Structure
object_base, next_free are used to allocate unkown sized objects.

*/
#include <stdio.h>
#include <stdlib.h>
#ifndef sun
#include <malloc.h>
#endif
#include <memory.h>

#include <config.h>
#include <adi/obstack.h>

#ifdef sun
#define ALIGN(x) (((x) & 0x3) ? ((x) | 0x3) + 1 : (x))
#else
#define ALIGN(x) x
#endif

/* @function obstack_begin
Initialize an obstack object (obstack, size) where size is default block
allocation size.  The invocation of the function sets up struct to be
terminated by nulls. */
char DLLINTERFACE obstack_begin (obstack, size)
  struct obstack *obstack; int size;
{
  obstack->chunk_size = size;
  obstack->chunk = NULL;
  obstack->object_base = NULL;
  obstack->next_free = NULL;
  obstack->chunk_limit = NULL;
}

/* @function obstack_destroy
Terminate an obstack freeing all allocated heap memory and setting
the obstack chunk chain to Null. */
char DLLINTERFACE obstack_destroy (obstack)
  struct obstack *obstack;
{
  struct obstack_chunk *chunk, *prev = NULL;

  for (chunk = obstack->chunk;  chunk;  chunk = prev) {
    char *block = &chunk->contents[0];
    prev = chunk->prev;
    free (chunk);
  }
  obstack->chunk = NULL;
}

/* @function obstack_init
Default init routine.  Just envoking the begin with a 4k block size */
char DLLINTERFACE obstack_init (obstack)
  struct obstack  *obstack;
{
  obstack_begin (obstack, 4096);
}


/* @function internal obstack_ckalloc
The work horse this routine is responsible for allocating space from the
heap, it allocates the blocks and set aside space based on the size argument.
*/
static struct obstack_chunk *DLLINTERFACE
obstack_ckalloc (obstack, size)
  struct obstack *obstack; int size;
{
  struct obstack_chunk *chunk = obstack->chunk;
  int len;

  /* Need to align for bus error */
  size = ALIGN(size);
  len = size < obstack->chunk_size ? obstack->chunk_size : size;

  if (chunk == NULL
      || (chunk->dot + size > chunk->size)) {
       /* allocate new block */
    chunk = (struct obstack_chunk  *) 
                    malloc (len + sizeof (struct obstack_chunk));
    if (chunk == 0) {
      printf ("obstack memory allocation error %d\n",
              len + sizeof (struct obstack_chunk));
      exit (-1);
    }
    chunk->dot = 0;
    chunk->size = len;
    chunk->prev = obstack->chunk;
    obstack->chunk = chunk;
  }
  return chunk;
}

/* @function obstack_alloc

Allocate a block of memory size n from the obstack, uses ckalloc to obtain
the fill block.  */
char *DLLINTERFACE obstack_alloc (obstack, n)
  struct obstack  *obstack; int n;
{
  char *p;
  struct obstack_chunk *chunk;
  chunk = obstack_ckalloc (obstack, n);
  p = (char *) (&chunk->contents[0]) + chunk->dot;
  chunk->dot += ALIGN(n);
  memset (p, 0, ALIGN(n));
  return (char *) p;
}


/* @function obstack_free

Free all memory allocated from the obstack since location p was allocated
search the previous chunks for the inclusion of location p. if found free
all previous blocks and set dot to p. */
int DLLINTERFACE obstack_free (obstack, p)
  struct obstack  *obstack; char *p;
{
  struct obstack_chunk *chunk, *prev = NULL;

  for (chunk = obstack->chunk;  chunk;  chunk = prev) {
    char *block = &chunk->contents[0];
    prev = chunk->prev;
    if (p >= block && p < block + chunk->dot) {
      chunk->dot = (char *) p - block;
      if (chunk->dot == 0) {
        free (chunk);
        obstack->chunk = prev;
      }
      else
      {
        obstack->chunk = chunk;
      }
      return 0;
    }
    free (chunk);
  }
  obstack->chunk = NULL;
}

/* @function obstack_copy0
NOT IMPLEMENTED */
char *DLLINTERFACE obstack_copy0 (obstack, x, len)
  struct obstack *obstack; char *x; int len;
{
}


/* @function obstack_finish */
char *DLLINTERFACE obstack_finish (obstack)
  struct obstack  *obstack;
{
  char *p;
  
  p = obstack_alloc (obstack, 1);
  *p++ = 0;
}

/* @function obstack_grow1 */
int DLLINTERFACE obstack_grow1 (obstack, c)
  struct obstack  *obstack;
  char c;
{
  char *p;
  
  p = obstack_alloc (obstack, 1);
  *p++ = c;
}


/* @function obstack_grow */
char *DLLINTERFACE obstack_grow (obstack, data, size)
  struct obstack *obstack; char *data; int size;
{
  char *p;
  int i;
  p = obstack_alloc (obstack, size);
  
  for (i=0;i<size;i++)
    p[i] = *data++;
  return p;
}

/* @function obstack_1grow 
NOT IMPLEMENTED */
char obstack_1grow (obstack, ch)
  struct obstack *obstack;
  char ch;
{
}


#ifdef _PCC_
memset(p,v,n)
  char *p;
  char v;
  int n;
{
  int i;
  for (i=0;i<n;i++) *p++ = v;
}
#endif
