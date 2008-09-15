/* @(#)action_table.c	1.5 2/21/91 1 */

#include "app.h"
#include "symbol.h"
#include "codegen.h"
#include "a_out.h"
#include "action.h"

#include "internal_reloc.h"
#include "reloc_fp.h"

void (*(array_ptr_functions[]))()=
{
   0,
   define_file_symbol,
   define_symbol,
   define_symbol_value,
   define_symbol_sclass,
   define_symbol_tag,
   define_symbol_line,
   define_symbol_size,
   define_section_auxent,
   define_dim1,
   define_dim2,
   end_symbol_define,
   line_number,
   reloc_addr_var,
   reloc_addr24,
   reloc_addr32,
   reloc_pc_rel_short,
   reloc_pc_rel_long,
   define_symbol_type,
   new_statement,
   0
};
