/* @(#)input_file.h	1.3 1/7/91 1 */

struct input_file
{
    char       *file_name;     /* Name of the file */
    INPUT_FILE *infile_next;   /* Next file on list */
    long        local_syms;    /* Number of local symbols in this file */
    long        symbol_index;  /* Index into output symbol table for local symbols from this file */
    INPUT_SECT *head;          /* Head of the input section list for this file */
    INPUT_SECT *tail;          /* End of the input section list for this file */
    long       arch_offset;    /* For archives: offset from beginning of library */
    char       *strings;       /* Pointer to string table of this file */
    short      file_index;     /* For archives: # of files to skip before this file */
    long       num_symbols;    /* Total number of symbols in this file */
    long       sym_table_ptr;  /* File offset of the symbol table */
    short      num_sections;   /* Number of input sections */
};

extern short 	Pmstack, Crts, Dubs, Regp, First_c_file, Input_file_types;
extern char 	First_c_file_name[];


