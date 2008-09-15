#include <stdio.h>
#include <config.h>
#include <obstack.h>
#include <sys/types.h>
#include <adi/clargpar.h>

char *filename;
char *header;
char *footer;
int print_defaults;
int bye = 1;
int latex = 0;


/* @CL-ARG-PAR: */
struct arg args[] = {
  { 'v', "version",   AOP_VERSION, "print version number",   0, 0 },
  { 'h', "help",      AOP_HELP,    "print help information", 0, 0 },
  { '?', "syntax",    AOP_SYNTAX,  "print syntax information", 0, 0 },
  { 'V', "print-defaults", AOP_BOOL, "print defaults",       &print_defaults,0},
  { 'o', "ofilename", AOP_STRING,  "output filename",        &filename, 0 },
  { 'n', "omit-bye",  AOP_INVBOOL,    "do not output @bye",     &bye, 0},
  { 'l', "latex",     AOP_BOOL,    "uses latex formatting",  &latex, 0},
  { 'H', "include-header",   AOP_STRING,  "include header file",   &header, 0},
  { 'F', "include-footer",   AOP_STRING,  "include footer file",   &footer, 0},
  0
};

cmdline_t cargs = {
  "mungedoc",
  "1.0",
  "ADSP-documentation generation program",
  0,
  MAXDEFARGS,
  0,0,
  args
};


/*
@title Munge Code Into A Document.

@section Why
Code needs to be documented well or it will be at deaths door before
it sees the light of day.  Well, maybe it won't be at deaths door 
but if its not documented you can guarantee that you will be harsed by
maintence for ever.

This file should be an example of what to do.

Need to Add support for collecting @type into a section types.

Okay next we need to construct a table that describes how the 
registers are configured.

If a file contains the @Register documentation token then
a table will be constructed describing how the registers are
used.

@example 
  DEF_REGISTER (name, index, grp)
@end example


*/
static struct obstack type_stream;
static struct obstack markers_stream;
static struct obstack register_names;

int divert2type = 0;
int has_types = 0;
int line = 1;

#ifdef _PCC_
bzero (x,n)
  char *x;
  int n;
{
  while (n--)
   *x++ = 0;
}
#endif

outc(c,f)
  char c; FILE *f;
{
  if (divert2type)
    obstack_1grow (&type_stream, c);
  else 
    putc(c, f);
}

outs(s,f)
  char *s; FILE *f;
{
  while (*s)
    outc (*s++, f);
}

inc (f)
  FILE *f;
{
  int ch;
  ch = getc (f);
  if (ch == '\n')
    line++;
  return ch;
}

munge_comment (name,out)
  char *name; FILE *out;
{
  FILE *inf;
  char scratch[1024];
  int   ch;
  int  buildregtab = 0;
  int  buildclint = 0;
  int  buildfuncdef = 0;
  int  rtab_w, rtab_h;
  char **rtab;
  char *special_regs[100], **sprp = special_regs;

  inf = fopen (name, "r");
  if (inf == 0) {
    fprintf (stderr, "%s: can not open %s for reading\n", cargs.progname, name);
    exit (-1);
  }
    
  line = 1;
  fprintf (out, "@comment file %s\n", name);
  while (1)
    {
      if ((ch = inc (inf)) == -1)
	break;
      else if (ch == '/'
	       && inc (inf) == '*')
	{
	  while (1)
	    {
	      ch = inc (inf);
	      if (ch == '@' || ch == '\\')
		{
		  int i = 0;
		  scratch[i++] = '@';

/* @subsection Types
   here we collect all Types into a table to be produced at
   the end of the document. 
@example
@@Type-xxx
@end example
    some type of thing in the assembler each thing is an instruction
    type.
*/
		  if ((scratch[i++] = inc (inf)) == 'T'
		      && (scratch[i++] = inc (inf)) == 'y'
		      && (scratch[i++] = inc (inf)) == 'p'
		      && (scratch[i++] = inc (inf)) == 'e'
		      && (scratch[i++] = inc (inf)) == '-')
		    {
		      divert2type = 1;
		      has_types=1;
		      sprintf (scratch, 
			       "@comment file %s line %d\n@item ",
			       name, line);

		      i = strlen (scratch);
		      while ((scratch[i++] = inc (inf)) != ' ');
		      sprintf (&scratch[i], "\n@example\n");
		      i = strlen (scratch);
		    }
/* @subsection Register
Constructs a table of strings.  The table is formated via the width and height
arguments, and each item is then added to the table.

@example
@@Register<height,width>
@end example

All following items are extracted from the code with the

@example
DEF_REGISTER(...)
@end example

see DEF_REGISTER command forward.
*/
		  else if (!buildregtab
			   && scratch[1] == 'R'
			   && (scratch[i++] = inc (inf)) == 'e'
			   && (scratch[i++] = inc (inf)) == 'g'
			   && (scratch[i++] = inc (inf)) == 'i'
			   && (scratch[i++] = inc (inf)) == 's'
			   && (scratch[i++] = inc (inf)) == 't'
			   && (scratch[i++] = inc (inf)) == 'e'
			   && (scratch[i++] = inc (inf)) == 'r')
		    {
		      buildregtab = 1;
		      fscanf (inf, "<%d,%d>", &rtab_h, &rtab_w);
		      rtab = obstack_alloc (&register_names,
                                            sizeof (char*) * rtab_h * rtab_w);
		      bzero (rtab, sizeof (char*) * rtab_h * rtab_w);
		      bzero (special_regs, sizeof (special_regs));
		      strcpy (scratch, "@subsection Register ");
		      i = strlen (scratch);
		    }
/* @subsection @@CL-ARG-PAR: 
struct arg args[] = {
  { 'v', "version",   AOP_VERSION, "print version number",   0, 0 },
  ...
cmdline_t cargs = {
  "mungedoc",
  "1.0",
  "ADSP-documentation generation program",
*/
		  else if (!buildclint &&     scratch[1] == 'C'
			   && (scratch[i++] = inc (inf)) == 'L'
			   && (scratch[i++] = inc (inf)) == '-'
			   && (scratch[i++] = inc (inf)) == 'A'
			   && (scratch[i++] = inc (inf)) == 'R'
			   && (scratch[i++] = inc (inf)) == 'G'
			   && (scratch[i++] = inc (inf)) == '-'
			   && (scratch[i++] = inc (inf)) == 'P'
			   && (scratch[i++] = inc (inf)) == 'A'
			   && (scratch[i++] = inc (inf)) == 'R')
                    {
                      buildclint = 1;
		      strcpy (scratch, 
                        "@section Command Line Interface");
		      i = strlen (scratch);
                    }

/* @function hash_new

This is the hash table constructor, it creates a hash table object
and initializes the control structure. 

struct hash_control *hash_new ()
{
*/
		  else if (!buildfuncdef &&   scratch[1] == 'f'
			   && (scratch[i++] = inc (inf)) == 'u'
			   && (scratch[i++] = inc (inf)) == 'n'
			   && (scratch[i++] = inc (inf)) == 'c'
			   && (scratch[i++] = inc (inf)) == 't'
			   && (scratch[i++] = inc (inf)) == 'i'
			   && (scratch[i++] = inc (inf)) == 'o'
			   && (scratch[i++] = inc (inf)) == 'n')
                    {
                      buildfuncdef = 1;
		      strcpy (scratch, 
                        "@subsection ");
		      i = strlen (scratch);
                    }

/* @subsection Add-to-doc
This adds the next line of text from the input file. 
To the document. if the next line is a string
then use printf 
@example
Add-to-doc/
/foo bar goo/
@end example

Would add 'foo bar goo' to the document

*/
		  else if (scratch[1] == 'A'
			   && (scratch[i++] = inc (inf)) == 'd'
			   && (scratch[i++] = inc (inf)) == 'd'
			   && (scratch[i++] = inc (inf)) == '-'
			   && (scratch[i++] = inc (inf)) == 't'
			   && (scratch[i++] = inc (inf)) == 'o'
			   && (scratch[i++] = inc (inf)) == '-'
			   && (scratch[i++] = inc (inf)) == 'd'
			   && (scratch[i++] = inc (inf)) == 'o'
			   && (scratch[i++] = inc (inf)) == 'c')
		    {
		      char delim = inc (inf);
		      while ((ch = inc (inf)) != delim
			     && ch != EOF)
			;

		      if (ch != delim)
			{
			  fprintf (stderr, 
  "error occured while processing file %s, Add-to-doc %c\n", name, delim);
			  exit (-1);
			}
		      
		      while (1)
			{
			  ch = inc (inf);
			  if (ch == -1
			      || ch == delim)
			    break;
			  if (delim == '\"'
			      && ch == '\\')
			    switch (ch = inc (inf))
			      {
			      case 'n': ch = '\n'; break;
			      case 'f': ch = '\f'; break;
			      case '\n': continue;
			      }
			  outc (ch, out);
			}
		      outc ('\n', out);
		      outc ('\n', out);
		      continue;
		    }
		  {
		    int j;
		    for (j=0;j<i;j++)
		      outc (scratch[j], out);
		    
		    while (1)
		      {
			ch = inc (inf);
			if (ch == -1)
			  break;
			else if (ch == '*')
			  if ((ch = inc (inf)) == '/')
			    break;
			  else
			    outc ('*', out);
			outc (ch, out);
		      }
		    outc ('\n', out);
		    outc ('\n', out);
		    if (divert2type)
		      outs ("@end example\n", out);
		    divert2type = 0;
		    break;
		  }
		}
	      else if (ch == -1)
		break;
	      else if (ch == '*'
		       && inc (inf) == '/')
		break;
	    }
	}
/* @subsection DEF_REGISTER
This is special functionality for the generation of tables from the
fixpoint assembler sources.

The registers are stored in a matrix that is indexed by the code

@example
DEF_REGISTER(name,grp,ind ...)
@end example

@enumerate
@item name the register name to store
@itam grp the group index
@item ind the index
@end enumerate
*/
      else if (buildregtab
	       && ch == 'D'
	       && inc (inf) == 'E'
	       && inc (inf) == 'F'
	       && inc (inf) == '_'
	       && inc (inf) == 'R'
	       && inc (inf) == 'E'
	       && inc (inf) == 'G'
	       && inc (inf) == 'I'
	       && inc (inf) == 'S'
	       && inc (inf) == 'T'
	       && inc (inf) == 'E'
	       && inc (inf) == 'R'
	       && inc (inf) == ' '
	       && inc (inf) == '(')
	{
	  char name[30];
	  int index=-1, grp=-1;
	  int r = fscanf (inf, "%s %d, %d", name, &index, &grp);
	  
	  if (name[0] == '@')
	    {
	      char name1[30];
	      sprintf (name1, "@%s", name);
	      strcpy (name, name1);
	    }

	  if (index > -1 && grp > -1 && grp <= rtab_w)
	    rtab[index * rtab_w + grp] 
	      = obstack_copy0 (&register_names, name, strlen (name)-1);
	  else
	    *sprp++ = obstack_copy0 (&register_names, name, strlen (name)-1); 
	}

/*  { 'v', "version",   AOP_VERSION, "print version number",   0, 0 }, */
      else if (buildclint && ch == '{')
        {
          char sw;
          char name[50],kind[20];
          char doc[256];
          int cnt;
          if (buildclint++ == 1)
            fprintf (out, "\n\n@itemize\n");

          fgets (scratch, 1024, inf);

          cnt = sscanf (scratch, 
             " \'%c\', \"%[^\"]\", AOP_%[^,], \"%[^\"]\"", 
             &sw, name, kind, doc);

          if (cnt == 4) {
            fprintf (out, 
                "@item @code{-%c}, @code{--%s}\nArgument Type %s\n%s\n\n", 
                     sw, name, kind, doc);
          }
        }
      else if (buildclint && ch == '}' && inc (inf) == ';')
        {
          if (buildclint > 1)
            fprintf (out, "@end itemize\n\n\n");
          buildclint = 0;
        }
      else if (buildfuncdef && ch == '{')
        {
          if (buildfuncdef > 1)
            fprintf (out, "\n@end example\n\n");
          buildfuncdef = 0;
        }
      else if (buildfuncdef) 
        {
          if (buildfuncdef == 1) 
            {
              fprintf (out, "@example\n");
              buildfuncdef ++;
            }
          outc (ch,out);
        }
    }
  
  if (buildregtab)
    {
      int index,grp;
      fprintf (out, "@example\n");
      for (grp = 0; grp < rtab_w; grp++)
	fprintf (out, "\t%2d", grp);
      for (index = 0; index < rtab_h; index++)
	{
	  fprintf (out, "\n%2d:\t", index);
	  for (grp = 0; grp < rtab_w; grp++)
	    fprintf (out, "%s\t", rtab[index * rtab_w + grp]);
	}
      fprintf (out, "\n@end example\n");
      if (special_regs[0])
	fprintf (out, 
"\n\nThese registers have special meanings they might be aliases or nicities\n@example\n");

      for (index = 0; special_regs[index];index++)
	fprintf (out, "%s\t", special_regs [index]);

      if (special_regs[0])
	fprintf (out, "\n@end example\n");

      fprintf (out, "\n\n");
    }
  fclose (inf);
}

void *obstack_chunk_alloc (sz)
  size_t sz;
{
  return (void *)malloc (sz);
}
void obstack_chunk_free (x)
  char *x;
{
  free (x);
}

/* 
@section file handling
There is basicly none just open each file and munge it to stdout.
*/
main(argc,argv)
  int argc; char **argv;
{
  FILE *out,*in;
  int i,ch;
  obstack_init (&type_stream);
  obstack_init (&register_names);

  parse_args (&cargs, argc,argv);
  if (print_defaults)
    print_args (&cargs);

  if (filename)
    out = fopen (filename, "w");
  else
    out = stdout;

  if (out == 0) {
    fprintf (stderr, "%s: can not open file %s as output\n", 
             cargs.progname, filename);
    exit (0);
  }

  if (header) {
    in = fopen (header, "r");
    if (in) {
      do {
        ch = getc (in);
        if (ch != -1) putc (ch, out);
      } while (ch != -1);
      fclose (in);
    } 
  }
  else
    fprintf (out, "\\input texinfo			@c -*- texinfo -*-\n");

  for (i=0; i< cargs.dargc;i++)
    {
      munge_comment (cargs.dargv[i], out);
    }
  if (has_types)
    {
      char *buf = obstack_finish (&type_stream);
      fprintf (out, 
          "\n@section Instruction Types and there encodings.\n@table @b\n");
      while (*buf)
	putc (*buf++, out);
      fprintf (out, "\n@end table\n");
    }

  if (footer) {
    in = fopen (footer, "r");
    if (in) {
      do {
        ch = getc (in);
        if (ch != -1) putc (ch, out);
      } while (ch != -1);
      fclose (in);
    } 
  }
  else
    if (bye)
      fprintf (out, "\n@contents\n@bye\n");

  if (filename)
    fclose (out);
  return 0;
}
