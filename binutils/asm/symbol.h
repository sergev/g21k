/* @(#)symbol.h	1.5 1/4/91 1 */


#define MAX_SYMBOL_NAME_SIZE 8

typedef struct symbol SYMBOL;

struct symbol 
{
    union
    {
        char name[MAX_SYMBOL_NAME_SIZE + 1]; /* Pointer to name in memory */
        struct
        {
            long zeroes;
            long offset;
        } strtab_entry;
    } what;

    long value;
    long section_num;	    /* number of section symbol is defined in */
    long type;
    long length;            /* length in words */
    long num_relocs;        /* number of relocation entries dependant on this symbol */
    int Debug_symbol;	    /* indicates debug symbol that has been written already*/

    SYMBOL *next;
};

#define NAME(symbol)            ((symbol)->what.name)
#define OFFSET(symbol)          ((symbol)->what.strtab_entry.offset)
#define ZEROES(symbol)          ((symbol)->what.strtab_entry.zeroes)
#define IN_STRING_TABLE(symbol) ((symbol)->what.strtab_entry.zeroes == 0)
#define THERE_IS_A_STRING_TABLE (string_table_index > 4)

/* symbol type values */

#define STYPE_UNDEFINED      0x0000L
#define STYPE_FILE_NAME      0x0001L
#define STYPE_LABEL          0x0002L
#define STYPE_DM             0x0003L
#define STYPE_PM             0x0004L
#define STYPE_ABS            0x0005L
#define STYPE_SECTION        0x0010L
#define STYPE_EXTERN         0x0020L
#define STYPE_GLOBAL         0x0040L

#define IS_STATIC(type)            (IS_DEFINED(type) && !IS_EXTERN(type) && !IS_GLOBAL(type))
#define IS_GLOBAL_DEFINED(type)    (IS_DEFINED(type) && IS_GLOBAL(type))
#define IS_GLOBAL_UNDEFINED(type)  (!IS_DEFINED(type) && IS_GLOBAL(type))

#define TYPE_MASK            0x0070L
#define BASIC_TYPE(x)        ((x) & 0x000FL)

#define IS_DEFINED(type)     (BASIC_TYPE(type) != STYPE_UNDEFINED)
#define IS_EXTERN(x)         (((x) & STYPE_EXTERN) == STYPE_EXTERN)
#define IS_GLOBAL(x)         (((x) & STYPE_GLOBAL) == STYPE_GLOBAL)
#define IS_SECTION_TYPE(x)   (((x) & STYPE_SECTION)  == STYPE_SECTION)

#define MAKE_IT_GLOBAL(x)    ((x)->type |= STYPE_GLOBAL)
#define MAKE_IT_EXTERN(x)    ((x)->type |= STYPE_EXTERN)


/* symbol attribute values */

#define ATTR_ABSOLUTE        0x0001
#define ATTR_RELOCATABLE     0x0002

extern short symbol_hash( char *name );
extern void symbol_make_null( void );
extern SYMBOL *symbol_lookup( char *name );
extern SYMBOL *symbol_insert(char *name, long value, long section_num, long type,
                             long length );
extern void    symbol_traverse( void (*function)() );
extern short 	Debug_symbol_definition;

