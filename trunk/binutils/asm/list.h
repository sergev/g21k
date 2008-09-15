/* @(#)list.h	1.5 1/4/91 1 */

typedef struct list LIST;

struct list
{
    long  index;
    char *symbol;
    long  tag;
    long  offset;
    LIST  *next;
};

#define LIST_SIZE sizeof(LIST)

/* tag types */

#define STRING_TABLE_INDEX  1
#define NAME_IN_MEMORY      2

extern void list_insert_index( SYMBOL *symbol, short sclass, long index );
extern long list_get_index( char *symbol_name, short sclass );
