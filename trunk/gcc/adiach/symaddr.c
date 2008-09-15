/* This function is used to collect a set of needed symbol address
   from an already opened bfd.
   
   long ezaddr;
   long startaddr;

   findsymbol_t syms[] = { { "_ez", &ezaddr }, { "start", &startaddr }, 0 };

   foo () {
   ...
     find_symbol_addr (bfd, syms);
   ...
   }
   */
#ifndef MSC
#include <stdio.h>
#include <bfd.h>
#include <adi/symaddr.h>

void find_symbol_addr (bfd * abfd, findsymbol_t *needsyms)
{
  asymbol **sy = (asymbol **) NULL;
  int storage, symcount, i,j;
  int found;
  long adr;

  if (!(bfd_get_file_flags (abfd) & HAS_SYMS)) {
    (void) printf ("No symbols in \"%s\".\n", bfd_get_filename (abfd));
    return (NULL);
  }

  storage = get_symtab_upper_bound (abfd);

  if (storage) {
    sy = malloc (storage * sizeof (asymbol *));
    if (sy == NULL) {
      printf ( "%s: out of memory.\n", "dynamic-loader");
      exit (1);
    }
  }

  symcount = bfd_canonicalize_symtab (abfd, sy);
  found = 0;
  for (i = 0; i < symcount; i++) {
    for (j = 0; needsyms[j].name ; j++)
      if (strcmp (sy[i]->name, needsyms[j].name) == 0) {
	*(needsyms[j].paddr) = sy[i]->value + sy[i]->section->vma;
	break;
      }
  }
  free (sy);
}
#endif
