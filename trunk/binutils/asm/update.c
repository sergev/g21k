/* @(#)update.c	2.1 5/6/92 2 */

#include "app.h"
#include "symbol.h"
#include "update.h"

static long update_addr = 0l;
static long update_incr = 0l;
static long update_sect_num = 0l;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      update_symbol_table                                             *
*                                                                      *
*   Synopsis                                                           *
*      void update_symbol_table( long addr, long incr, long sect_num ) *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for updating a symbol by adding in  *
*   the address of it's section.                                       *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/20/89       created                  -----             *
***********************************************************************/

void update_symbol_table( long addr, long incr, long sect_num )
{
    update_addr = addr;
    update_incr = incr;
    update_sect_num = sect_num;

    symbol_traverse( update );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      update                                                          *
*                                                                      *
*   Synopsis                                                           *
*      void update( SYMBOL *sym )                                      *
*                                                                      *
*   Description                                                        *
*      This routine actually updates the value of the symbol.          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/20/89       created                  -----             *
***********************************************************************/

void update( SYMBOL *sym )
{
    if( (sym->section_num == update_sect_num) && (sym->value >= update_addr) )
         sym->value += update_incr;
}
