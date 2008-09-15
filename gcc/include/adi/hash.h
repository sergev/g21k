#ifndef HASH_H
#define HASH_H
#include <ansidecl.h>

extern struct hash_control *hash_new ();
extern int hash_die PARAMS((struct hash_control *));
extern int hash_hash PARAMS((struct hash_control *ht, char *spell));
extern char *hash_find PARAMS((struct hash_control *ht, char *spell));
extern int hash_insert PARAMS((struct hash_control *ht, char *spell,char *data));
int hash_replace PARAMS((struct hash_control *ht, char *spell,char *data));
extern int hash_apply PARAMS((struct hash_control *ht, int (*fun) ()));

#endif
