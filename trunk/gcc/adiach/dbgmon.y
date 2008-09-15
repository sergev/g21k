%{
/* This is a simple debug monitor for use with the TargetTrace API.
   It is a very simple command line debugger. providing minimal
   functionality and very simplistic behavior. 
   
   The use of this module is as follows.
   
      while (! dbgmon_done) {
         printf (dbgmon_prompt);
         dbgmon_parse ();
      }

   The module is initialized by a call to dbgmon_init (tproc, tid);


   Interface:
     dbgmon_init (TARGETTRACE_PROC tp, TARGETID ti, 
                  int (* dis)(), struct findsymbols *needs)     
     dbgmon_parse ()
     dbgmon_done ()
     dbgmon_loader ()

   */
#define yyparse dbgmon_parse

#include <config.h>
#include <stdarg.h>
#include <stdio.h>   
#include <signal.h>

#include <hash.h>
#include <bfd.h>
#include <adi/clargpar.h>
#include <adi/targtrac.h>
#include <adi/generat.h>
#include <dis-asm.h>

FILE *yyin = stdin;
static struct hash_control *symtab;

static TARGETID tid;
static TARGETTRACE_PROC tproc;

static int done = 0;

#define TGTBP_SIZE  1L
#define TGT_MAXADDR 0xFFFFFFL

static int (* dis1insn) ();

static struct findsymbols { char *name;  long *paddr; } *needsyms;

static struct breakpoint {
  struct breakpoint *nxt;
  int bnum;
  BOOL disabled;
  struct tgt_break_point tgtbp;
} *blist;

static int bpidno;

dbgmon_init (TARGETTRACE_PROC tp, TARGETID ti, int (* dis)(), struct findsymbols *needs)
{
  tproc = tp;
  tid = ti;
  dis1insn = dis;
  needsyms = needs;
}

int dbgmon_done () 
{
  return done;
}

static yylex ();
static yyerror ();

%}

%union {
  long      lval;
  char*     string;
  int       reg;
}

%type <lval> expr term

%token <string> FILENAME
%token <lval>   NUMBER SYMBOL
%token <reg>    REG

%token NL

%token LOAD RESET RUN CONTINUE BREAK LIST EXIT STEP PRINT REGS WHERE DM PM

%left '|'
%left '^'
%left '&'
%left LSHIFT RSHIFT
%left '+' '-'
%left '/' '*' '%'
%right '~' '!'

%start start
%%
start:    command

command:  
       |  LOAD FILENAME        { tproc (tid, TGT_RESET_COLD); dbgmon_loader ($2); }
       |  LOAD SYMBOL          { tproc (tid, TGT_RESET_COLD); dbgmon_loader ($2); }
       |  RESET                { tproc (tid, TGT_RESET_COLD);  }
       |  RUN                  { resume ();               }
       |  CONTINUE             { resume ();               }
       |  BREAK expr           { bp_new ($2);             }
       |  LIST BREAK           { bplist ();               }
       |  EXIT                 { done = 1;                }
       |  STEP expr            { int n=$2; while (--n) tproc (tid, TGT_SINGLESTEP); whereami (); }
       |  STEP                 { tproc (tid, TGT_SINGLESTEP); whereami (); }
       |  PRINT expr           { printf ("--> 0x%x %d\n", $2, $2); }
       |  PRINT REGS           { }
       |  WHERE                { whereami (); }
       ;

expr   :  expr '+' expr        { $$ = $1 + $3; }
       |  expr '-' expr        { $$ = $1 - $3; }
       |  expr '*' expr        { $$ = $1 * $3; }
       |  expr '/' expr        { $$ = $1 / $3; }
       |  expr '%' expr        { $$ = $1 % $3; }
       |  expr '|' expr        { $$ = $1 | $3; }
       |  expr '^' expr        { $$ = $1 ^ $3; }
       |  expr '&' expr        { $$ = $1 & $3; }
       |  expr LSHIFT expr     { $$ = $1 << $3; }
       |  expr RSHIFT expr     { $$ = $1 >> $3; }
       |  '~' expr             { $$ = ~ $2; }
       |  '-' expr             { $$ = - $2; }
       |  '!' expr             { $$ = ! $2; }
       |  '(' expr ')'         { $$ = $2; }
       |  term                 { $$ = $1; }
       ;

term   :  SYMBOL
       |  NUMBER
       |  DM expr              { tproc (tid, TGT_GETMEMDM, $2, &$$, 1l); }
       |  PM expr              { tproc (tid, TGT_GETMEMPM, $2, &$$, 1l); }
       |  '*' expr             { tproc (tid, TGT_GETMEM, $2, &$$, 1l); }
       |  REG                  { tproc (tid, TGT_GETREG, $1, &$$);     }
       ;


%%


/* Lexical Analysis */

#define DEFKEY(x) { #x, x}

static struct kw { char *key; int tok; } kwtab[] = {
  DEFKEY(RUN),
  DEFKEY(CONTINUE),
  DEFKEY(BREAK),
  DEFKEY(LIST),
  DEFKEY(STEP),
  DEFKEY(PRINT),
  DEFKEY(RESET),
  DEFKEY(WHERE),
  DEFKEY(EXIT),
  DEFKEY(PM),
  DEFKEY(DM),
  { "r", RUN  },
  { "c", CONTINUE  },
  { "p", PRINT  },
  { "s", STEP  },
  { "w", WHERE  },
  { 0, 0 }
};

static yyerror (char *s)
{
  fprintf (stderr, "error: %s\n", s);
}

static char back;


int nextchar ()
{
  if (back) {
    char c = back;
    back = 0;
    return c;
  }
  return fgetc (yyin);
}

int putback (char c)
{
  back = c;
}

static long parse_int (char fmt, long value)
{
  int not_done = 1;
  int shiftvalue = 0;
  char * char_bag;

  switch (fmt) {
  case 'h':
  case 'H':
    shiftvalue = 4;
    char_bag = "0123456789ABCDEFabcdef";
    break;
  case 'o':
  case 'O':
    shiftvalue = 3;
    char_bag = "01234567";
    break;
  case 'b':
  case 'B':
    shiftvalue = 1;
    char_bag = "01";
    break;

/* @subsection Fractional constants.
The assembler allows for fractional constants to be created
by either the 0.xxxx or the f#xxxx format 

i.e.   0.5 would result in 0x4000

note .5 would result in the identifier .5.

The assembler converts to fractional format 1.15
by the simple rule.

@example
  value = (short)(finput*(1<<15))
@end example
*/
  case 'f':
  case 'F':
  {
    double fval = 0.0;
    double pos = 10.0;
    while (1) {
      int c = nextchar ();

      if (isdigit (c)) {
        double digit = (c - '0') / pos;
        fval = fval + digit;
        pos = pos * 10.0;
      }
      else
      {
        putback (c);
        value = (short) (fval * (1 << 15));
        break;
      }
    }
    return value;
  }

  case 'd':
  case 'D':
  default:
  {
    while (1) {
      int c = nextchar ();
      if (isdigit (c))
        value = (value * 10) + (c - '0');
      else
      {
/* @subsection the k-suffix
Constants that are suffixed with k|K are multiplied by 1024

@table @code
@item 1k 
1024
@item 2k
2048
@item nk
etc.
@end table
This suffix is only allowed on decimal constants. */
        if (c == 'k' || c == 'K')
          value *= 1024;
        else
          putback (c);
        break;
      }
    }
    return value;
  }
  }

  while (not_done) {
    char c = nextchar ();
    if (c == 0 || !strchr (char_bag, c)) {
      not_done = 0;
      putback (c);
    }
    else
    {
      if (c >= 'a' && c <= 'z')
        c = c - ('a' - '9') + 1;
      else if (c >= 'A' && c <= 'Z')
        c = c - ('A' - '9') + 1;

      c -= '0';

      value = (value << shiftvalue) + c;
    }
  }
  return value;
}

static yylex ()
{
  while (1) {
    int c = nextchar ();

    switch (c) {
    case 0:
    case  -1:
      return 0;
    case '\r':          /* This is for Micro-Sloth */
    case ' ':
    case '\t':
      continue;
    case '\f':
      continue;
    case '\n':
      return -1; /* NL */

    case '<':
      if ((c = nextchar ()) == '<')
	return LSHIFT;
      else
	putback (c);
      return '<';

    case '>':
      if ((c = nextchar ()) == '>')
	return RSHIFT;
      else
	putback (c);
      return '>';

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      yylval.lval = parse_int ('d', c - '0');
      return NUMBER;


    case '0':           /* Accept diffrent formated integers hex octal and binary. */
      c = nextchar ();
      if (c == 'x' || c == 'X') /* hex input */
	yylval.lval = parse_int ('h', 0);
      else if (c == 'b' || c == 'B')
	yylval.lval = parse_int ('b', 0);
      else if (c == '.')
	yylval.lval = parse_int ('f', 0);
      else              /* octal */
      {
        putback (c);
	yylval.lval = parse_int ('o', 0);
      }
      return NUMBER;


    case 'd':
    case 'D':
    case 'h':
    case 'H':
    case 'o':
    case 'O':
    case 'b':
    case 'B':
    case 'f':
    case 'F':
    {
      int fmt = c;
      if ((c = nextchar ()) != '#') {
        putback (c);
        c = fmt;
        goto word;
      }
      else
      {
	yylval.lval = parse_int ((char)fmt, 0);
        return NUMBER;
      }
    }


    case '$':
    case 'A':
    case 'C':
    case 'E':
    case 'G':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case 'a':
    case 'c':
    case 'e':
    case 'g':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case '_':
    case '.':

word:
    {
      int i;
      int not_done = 1;
      char buf[256];
      char *cp = buf;
      while (not_done) {
        *cp++ = c;
        c = nextchar ();
        if (!(isalnum (c) || c == '_' || c == '%')) {
          putback (c);
          not_done = 0;
        }
      }
      *cp = 0;

      if (buf[0] == '.'
          && buf[1] == 0) /* If its a just a dot return it. */
        return '.';
      
      for (i=0; kwtab[i].key ; i++)
	if (strcasecmp (buf, kwtab[i].key) == 0)
	  return kwtab[i].tok;

      if ((yylval.lval = hash_find (symtab, buf)))
	return NUMBER;

      yylval.string = strdup (buf);
      return SYMBOL;
    }
    char_default:
    default:
      return c;
    }
  }
}



/*
  toplevel type things... 
  */
static resume ()
{
   bpdownload ();
   tproc (tid, TGT_RESUME);
   bpupload ();
   whereami ();
}

/* Break Points */
static struct breakpoint *bp_find (address_t addr)
{
   struct breakpoint *b = blist;
   while (b) {                    
     if (b->tgtbp.addr == addr)
        return b;
     b = b->nxt;
   }            
   return NULL;
}

static struct breakpoint *bp_new (address_t addr)
{
  struct breakpoint *b = bp_find (addr);
  if (b)
    return b;
  else {
    struct breakpoint *new = calloc (1, sizeof (struct breakpoint) + TGTBP_SIZE);
    new->bnum  = bpidno++;
    new->tgtbp.addr  = addr;
    new->nxt = blist; 
    blist = new;          
    return new;
  }
}

static void bp_del (int num)
{
   struct breakpoint *b = blist, *p = 0;
   while (b) {                    
     if (b->bnum == num) {
        if (p) 
           p->nxt = b->nxt;
        else
           blist = b->nxt;
        free (b);
        break;
     }
     p = b;
     b = b->nxt;
   }            
}

static int toggle_bp (long addr)
{
  struct breakpoint *b = bp_find (addr);
  if (!b)
    b  = bp_new (addr);
  else
    b->disabled = !b->disabled;

  return 1;
}

static int bplist ()
{
  struct breakpoint *b = blist;               
  while (b) {
    printf ("%d: 0x%06lx %s\n", b->bnum, b->tgtbp.addr, b->disabled ? "disabled" : "enabled");
    b = b->nxt;
  }
  return 1;
}

static int bpdownload ()
{
   struct breakpoint *b = blist;
   while (b) {      
     if (!b->disabled)
       tproc (tid, &b->tgtbp);
     b=b->nxt;
   }
   return 1;
}

static int bpupload ()
{
   struct breakpoint *b = blist;
   while (b) {      
     if (!b->disabled)
       tproc (tid, TGT_CLRBKPT, &b->tgtbp);
     b = b->nxt;
   }
   return 1;
}


/* Down Loader. */
static void 
find_symbol_addr (bfd * abfd, struct findsymbols *needsyms)
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
    hash_insert (symtab, sy[i]->name, sy[i]->value + sy[i]->section->vma);
    if (needsyms) {
      for (j = 0; needsyms[j].name ; j++)
	if (strcmp (sy[i]->name, needsyms[j].name) == 0) {
	  *(needsyms[j].paddr) = sy[i]->value + sy[i]->section->vma;
	  break;
	}
    }
  }
  free (sy);
}

static long
fill_memory_pages (bfd *obj)
{
  sec_ptr s;
  long totdl = 0;
  
  for (s = obj->sections;  s;  s = s->next) {
    long flags = bfd_get_section_flags (obj, s);
    long size = bfd_section_size (obj, s);
    bfd_vma adr = s->vma;
    bfd_byte *p, *buffer = 0;
    
    if (strcmp (s->name, "*achinf*") == 0)
      continue;

    if (flags & SEC_HAS_CONTENTS) {
      buffer = (char *)malloc (size);
      bfd_get_section_contents (obj, s, buffer, 0L, size);
      p = buffer;
      tproc (tid, TGT_SETMEMPACKED, adr, buffer, size,  (flags&SEC_PM) ? MEM24BIT : 0);
      free (buffer);
      totdl += size;
    } else if (size) { /* Then its a BSS section */
      buffer = (char *)malloc (size);
      memset (buffer, 0, size);
      tproc (tid, TGT_SETMEMPACKED, adr, buffer, size,  (flags&SEC_PM) ? MEM24BIT : 0);
      free (buffer);
    }
  }                 
  return totdl;
}

int dbgmon_loader (char *ifilename)
{
  bfd *obj = bfd_openr (ifilename, NULL);
  
  if (obj == NULL) {
    printf ("Error opening %s as object module\n", ifilename);
    return 0;
  }

  /* Ensure that this is an object module and not something else
     you know like an archive.  */

  if (bfd_check_format (obj, bfd_object)) {

    if (symtab)
      hash_die (symtab);

    symtab = hash_new ();

    find_symbol_addr (obj, needsyms);
    fill_memory_pages (obj);
  }
  else
  {
    bfd_close (obj);
    printf ("Error opening %s as coff-a21k object module\n", ifilename);
    return 0;
  }
  bfd_close (obj);
  return 1;
}


/* Disassembler Interface */
struct char_file {
  int length;
  char *base;
  char *cp;      
};

static int 
mysprintf (void *ignore, char *fmt, ...)
{           
  int rv;
  struct char_file *fp = ignore;
  va_list ap;
  va_start (ap, fmt);
  if (fp->cp == NULL) fp->cp = fp->base;
  vsprintf (fp->cp, fmt, ap);
  fp->cp += (rv = strlen (fp->cp));
  va_end (ap); 
  return rv;
}

void
objmod_print_address (vma, info)
     bfd_vma vma;
     struct disassemble_info *info;
{
  info->fprintf_func (info->stream, "0x%04lX", vma);
}

int 
dbgmon_sprintf_disasm (char *outputb, int szOutputb, long addr, long *buf)
{ 
  int nwords;
  disassemble_info sinfo;
  struct char_file char_file;
#if !defined( __SVR4) && !defined(__MSDOS__)
  extern int fprintf (FILE*, char*,...);
#endif
  char_file.length = szOutputb;
  char_file.base = outputb;
  char_file.base[0] = 0;
  char_file.cp = char_file.base;

  INIT_DISASSEMBLE_INFO (sinfo, (void *)&char_file);
  sinfo.print_address_func = objmod_print_address;
  sinfo.fprintf_func = mysprintf;
  sinfo.flags = 0;
  sinfo.buffer = (bfd_byte *) buf;
  sinfo.buffer_vma = addr;
  sinfo.buffer_length = 8;
  sinfo.private_data = NULL;
  
  nwords = dis1insn (addr, &sinfo);
  if (nwords)
     nwords /= 3; /* convert bytes to words. */
  return nwords;
}
