/* @(#)mem_map.h	1.4 6/11/91 1 */

typedef struct mem_map MEM_MAP;

 struct mem_map
{
    char            *filename;     /* file the symbol was defined or referenced in */
    MEM_MAP_SYMBOL **symbol_table;    /* The symbol table for this file */
    int              num_symbols;
    int              symbol_index;    
    MEM_MAP         *next;
};
