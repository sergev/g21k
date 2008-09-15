/* @comment marc.hoffman@analog.com

@chapter Hashing Based String Library

This is a simple generic symbol table that provides a flexible
foundation object for applications.  The alogirithim used is
open hashing whoose worse case lookup is U(n) but on average is
O(n).  The distribution or hash function is:

@example

  s = sum(key[i])
       i
@end example

The data structure used is:

@example
@cartouche

  +---------+
  | tabsize |
  +---------+
  | nelms   |
  +---------+     +----------+
  | entries |---->|          |
  +---------+     +----------+
                  |          |
                  +----------+     +-------+    +-------+    +-------+
                  |          |---->|       |--->|       |--->|       |--->0
                  +----------+     +-------+    +-------+    +-------+
                  /          /     | key   |    | key   |    | key   |
                  \          \     +-------+    +-------+    +-------+
                  +----------+     | value |    | value |    | value |
                  |          |     +-------+    +-------+    +-------+
                  +----------+
@end cartouche
@end example
@center The Open Hash Table

Example usage:

@example
{
  struct hash_control *h;
  h = hash_new ();
  hash_insert (h,"123",7);
  hash_insert (h,"abc",6);
  hash_die (h);
}
@end example

*/

#include <malloc.h>
#include <string.h>
#include <config.h>
#include <adi/hash.h>

#ifndef NULL
#define NULL (0L)
#endif

#define START_SIZE 101

/* Open hashing */
struct hash_control
{
  int tabsize;
  int nelms;
  struct hashelm
  {
    struct hashelm *next;
    char *spell;
    char *value;
  } **elms;
};

/* @function hash_new

This is the hash table constructor, it creates a hash table object
and initializes the control structure. */
struct hash_control *hash_new ()
{
  struct hash_control *ht;
  ht = calloc (1, sizeof (struct hash_control));
  ht->elms = calloc (ht->tabsize = START_SIZE, sizeof (struct hashelm *));
  if (ht == 0) {
    printf ("hash_new: memory allocation error\n");
    exit (-1);
  }
  return ht;
}

/* @function hash_die
Hash Table Destructor, deallocates all entries and control */
int hash_die (ht)
  struct hash_control *ht;
{
  int i;
  for (i = 0;  i < ht->tabsize;  i++) {
    struct hashelm *nxt, *bucket = ht->elms[i];
    while (bucket) {
      nxt = bucket->next;
      free (bucket);
      bucket = nxt;
    }
  }
  free (ht);
}


int hash_hash (ht, spell)
  struct hash_control *ht; char *spell;
{
  int h = 0;
  for (;*spell;  spell++)
    h += *spell;
  return h % ht->tabsize;
}

/* @function hash_find
Find an entry in the hash table, returns value if found otherwise null. */
char *hash_find (ht, spell)
  struct hash_control *ht; char *spell;
{
  int k;
  struct hashelm *bucket;
  k = hash_hash (ht, spell);
  bucket = ht->elms[k];
  while (bucket) {
    if (strcmp (bucket->spell, spell) == 0)
      return bucket->value;
    bucket = bucket->next;
  }
  return NULL;
}

/* @function hash_remove 
Remove the entry that has name from the hash table */
int hash_remove (ht, spell)
  struct hash_control *ht; char *spell;
{
  int k;
  struct hashelm *bucket, *cache = NULL;
  k = hash_hash (ht, spell);
  bucket = ht->elms[k];
  while (bucket) {
    if (strcmp (bucket->spell, spell) == 0) {
      if (cache)
        cache->next = bucket->next;
      else
        ht->elms[k] = bucket->next;
      free (bucket);
      return 1;
    }
    cache = bucket;
    bucket = bucket->next;
  }
  return 0;
}


/* @function hash_insert 
Insert an entry into the hash table if entry exists insert it
anyways because open hashing, will find the first entry on the chain
and in affect this means that the previous value doesn't exist. */
int hash_insert (ht, spell, data)
  struct hash_control *ht; char *spell; char *data;
{
  int k;
  struct hashelm *bucket;
  struct hashelm *new;
  k = hash_hash (ht, spell);
  bucket = ht->elms[k];
  new = malloc (sizeof (struct hashelm));
  if (new == 0) {
    printf ("hash_new: memory allocation error\n");
    exit (-1); 
  }

  new->spell = spell;
  new->value = data;
  new->next = bucket;
  ht->elms[k] = new;
  return 1;
}

/* @function hash_replace 
Replace an entry in the hash table if the entry exists. 
Otherwise, just insert it into the table.*/
int hash_replace (ht, spell, data)
  struct hash_control *ht; char *spell; char *data;
{
  int k;
  struct hashelm *bucket;
  k = hash_hash (ht, spell);
  bucket = ht->elms[k];
  while (bucket) {
    if (strcmp (bucket->spell, spell) == 0) {
      bucket->value = data;
      return 1;
    }
    bucket = bucket->next;
  }
  return hash_insert (ht,spell,data);
}

/* @function hash_apply
Map a function @code{fun} over all entries in the table.
The function that is called has the following protocol:

@example
  int fun (char *spell, char *value);
@end example

The interface is: */
int hash_apply (ht, fun)
  struct hash_control *ht; int (* fun) (/*char *, void * */);
{
  int i;
  for (i = 0;  i < ht->tabsize;  i++) {
    struct hashelm *bucket = ht->elms[i];
    while (bucket) {
      (* fun) (bucket->spell, bucket->value);
      bucket = bucket->next;
    }
  }
}

