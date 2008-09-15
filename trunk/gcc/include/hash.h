/* hash.h - for hash.c
   Copyright (C) 1987, 1992 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef hashH
#define hashH

struct hash_entry
	{
	char *hash_string;		/* points to where the symbol string is */
  /* NULL means slot is not used */
  /* DELETED means slot was deleted */
	char *hash_value;			/* user's datum, associated with symbol */
	};


#define HASH_STATLENGTH	(6)
struct hash_control
	{
	struct hash_entry *hash_where;	/* address of hash table */
	int hash_sizelog;			/* Log of ( hash_mask + 1 ) */
	int hash_mask;				/* masks a hash into index into table */
	int hash_full;				/* when hash_stat[STAT_USED] exceeds this, */
  /* grow table */
	struct hash_entry *hash_wall;	/* point just after last (usable) entry */
  /* here we have some statistics */
	int hash_stat[HASH_STATLENGTH];	/* lies & statistics */
  /* we need STAT_USED & STAT_SIZE */
	};


/*						returns		  */
struct hash_control *hash_new (void);	/* [control block]	  */
/* hash_die is defined in file "HASH.C" */
extern void hash_die (struct hash_control *handle);

/* hash_say is defined in file "HASH.C" */
void hash_say (struct hash_control *handle,
               int buffer[ /*bufsiz*/ ],
               int bufsiz);

char *             /* NULL if string not in table, else */
/* returns value of deleted symbol */
hash_delete (struct hash_control *handle,
             char *string);	/* previous value         */

char *
hash_replace (struct hash_control *handle,
              char *string,
              char *value);	/* previous value         */

char *             /* return error string */
hash_insert (struct hash_control *handle,
             char *string,
             void *value);	/* error string           */

char *
hash_apply (struct hash_control *handle,
            char *(*function) ());	/* 0 means OK             */

void *
hash_find (struct hash_control *handle, char *string);	/* return char* or NULL */
			/* value                  */
char *
hash_jam (struct hash_control *handle,
          char *string,
          void *value);	/* error text (internal)  */
#endif /* #ifdef hashH */

/* end of hash.c */
