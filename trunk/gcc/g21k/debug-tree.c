#include "config.h"
#include "machmode.h"
#include "rtl.h"
#include "tree.h"
#include "c-tree.h"

typedef struct {tree *nn_ptr; char *nn_name;} nn_entry;

nn_entry nn_table [] =  {
    
{ &short_integer_type_node,	"short_integer_type_node"},
{ &integer_type_node,	"integer_type_node"},
{ &long_integer_type_node,	"long_integer_type_node"},
{ &long_long_integer_type_node,	"long_long_integer_type_node"},
{ &short_unsigned_type_node,	"short_unsigned_type_node"},
{ &unsigned_type_node,	"unsigned_type_node"},
{ &long_unsigned_type_node,	"long_unsigned_type_node"},
{ &long_long_unsigned_type_node,	"long_long_unsigned_type_node"},
{ &ptrdiff_type_node,	"ptrdiff_type_node"},
{ &unsigned_char_type_node,	"unsigned_char_type_node"},
{ &signed_char_type_node,	"signed_char_type_node"},
{ &char_type_node,	"char_type_node"},
{ &wchar_type_node,	"wchar_type_node"},
{ &signed_wchar_type_node,	"signed_wchar_type_node"},
{ &unsigned_wchar_type_node,	"unsigned_wchar_type_node"},
{ &float_type_node,	"float_type_node"},
{ &double_type_node,	"double_type_node"},
{ &long_double_type_node,	"long_double_type_node"},
{ &void_type_node,	"void_type_node"},
{ &ptr_type_node,	"ptr_type_node"},
{ &const_ptr_type_node,	"const_ptr_type_node"},
{ &string_type_node,	"string_type_node"},
{ &const_string_type_node,	"const_string_type_node"},
{ &char_array_type_node,	"char_array_type_node"},
{ &int_array_type_node,	"int_array_type_node"},
{ &wchar_array_type_node,	"wchar_array_type_node"},
{ &default_function_type,	"default_function_type"},
{ &double_ftype_double,	"double_ftype_double"},
{ &double_ftype_double_double,	"double_ftype_double_double"},
{ &int_ftype_int,	"int_ftype_int"},
{ &long_ftype_long,	"long_ftype_long"},
{ &void_ftype_ptr_ptr_int,	"void_ftype_ptr_ptr_int"},
{ &int_ftype_ptr_ptr_int,	"int_ftype_ptr_ptr_int"},
{ &void_ftype_ptr_int_int,	"void_ftype_ptr_int_int"},
{ &string_ftype_ptr_ptr,	"string_ftype_ptr_ptr"},
{ &int_ftype_string_string,	"int_ftype_string_string"},
{ &int_ftype_cptr_cptr_sizet,	"int_ftype_cptr_cptr_sizet"},

{ 0, "--end-of-table---"}};


char *nodename(tree addr)
{
    nn_entry *p;
    for(p = nn_table; p->nn_ptr; p++)
	if (*(p->nn_ptr) == addr) return p->nn_name;
    return "<unnamed>";
}
