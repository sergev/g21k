/* @(#)memory_map_util.c        1.11 5/13/91 1 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "app.h"
#include "a_out.h"
#include "input_file.h"
#include "input_section.h"
#include "output_section.h"
#include "mem_map_symbol.h"
#include "mem_map.h"
#include "error.h"
#include "util.h"

#include "error_fp.h"
#include "memory_map_util_fp.h"


extern char *C_filename;

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      mem_map_init                                                    *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Initialize a memory map.                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/27/89       created                  -----            *
***********************************************************************/

MEM_MAP *mem_map_init( char *filename, int num_symbols )
{
    register MEM_MAP *p;

    p = (MEM_MAP *) my_malloc( (long)sizeof(MEM_MAP) );
    p->symbol_table = (MEM_MAP_SYMBOL **) my_malloc( (long)(sizeof(MEM_MAP_SYMBOL *) * num_symbols));
    if( C_filename != filename )
	p->filename = filename;
    else
	p->filename = "C Support Objects";
    p->num_symbols = num_symbols;
    p->next = (MEM_MAP *) NULL;
    p->symbol_index = 0;
    return( p );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      mem_map_add_symbol                                              *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Add a symbol to the memory map.                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/27/89       created                  -----             *
***********************************************************************/

void mem_map_add_symbol( register SYMENT *obj_sym_ptr, long address, INPUT_SECT *input_sect_ptr, 
			 MEM_MAP *map_ptr, long mem_type )
{
   register MEM_MAP_SYMBOL *map_symbol;
   char           *name;
   char            local_symname[SYMNMLEN+1];

   if( map_ptr->symbol_index < map_ptr->num_symbols )
   {
       map_ptr->symbol_table[map_ptr->symbol_index] = (MEM_MAP_SYMBOL *) my_malloc((long)sizeof(MEM_MAP_SYMBOL) );
       map_symbol = map_ptr->symbol_table[map_ptr->symbol_index];

	
       if( 0L == OBJ_SYM_ZEROES(obj_sym_ptr) )
	 name = obj_sym_ptr->_n._n_p._n_nptr;
       else 
       {
	 strncpy(local_symname, obj_sym_ptr->_n._n_name, SYMNMLEN);
	 local_symname[SYMNMLEN] = '\0';  
	 name = local_symname;
       }

#ifdef DEBUG
       printf( "mem_map_add_symbol: name = %s\n", name );
       fflush( stdout );
#endif
       map_symbol->symbol_name = my_malloc( (long)(strlen(name) + 1) );
       strcpy( map_symbol->symbol_name, name );

       map_symbol->address = address;
       map_symbol->type = obj_sym_ptr->n_type;
       map_symbol->mem_type = mem_type;
       map_symbol->sclass = obj_sym_ptr->n_sclass;
       map_symbol->num_aux = obj_sym_ptr->n_numaux;
       map_symbol->input_sect = input_sect_ptr;
       (map_ptr->symbol_index)++;
   }
   else
   {
       LINKER_ERROR( "Memory map symbol table overflow." );
   }
}
