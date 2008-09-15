/* @(#)syms_fp.h	1.4 6/11/91 1 */

extern int hash( char *name );
extern LINKER_SYM *findsym( char *name );
extern LINKER_AUX *findaux( LINKER_SYM *sym, int index );
extern LINKER_SYM  *makesym( SYMENT *sym, INPUT_FILE *file_ptr );
extern LINKER_AUX * makeaux( LINKER_SYM *sym, AUXENT *aux, int num_aux );
extern LINKER_SYM *getsym( long acid );
extern LINKER_AUX *getaux( long acid );
extern long process_symbol_table( FILE *fd, long numsyms, INPUT_FILE *file_ptr, 
                                  SYMENT *symbuf, char *string_table_ptr );
extern LINKER_AUX *find_aux_entry( LINKER_SYM *sym, LINKER_AUX *aux, 
                                  int changed );
