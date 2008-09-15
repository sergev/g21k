typedef struct findsymbols { char *name;  long *paddr; } findsymbol_t;

extern void find_symbol_addr (bfd * abfd, findsymbol_t *needsyms);
