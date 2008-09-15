/* @c marc.hoffman@analog.com           12/31/95

@chapter Initialization File Parsing

Read and initializes program paramaters from a file, the point
of this is to standardize program initialization processes.  It
provides a simple initialization via the clargpar module which
integrates the command line switches into an ini file.  The format
of the file is similar if not exact to the format found on the
WINDOWS OS.

The file has a very simple format each non empty line is either a
comment or an assignment.  Comments begin with ; or # there is no difference
which style you choose.  I added # to keep in parallel with UNIX
scripting comments.  The section directives used in the WINDOWS
ini files are basicaly comments so [ treats the line as a comment also.
The rational for why we ignore sections is due to the fact that this
module is electric.  It doesn't use the same mechanism that windows
does to get values.  Instead, it sets variables in your main program
directly elliminating needless code.

The other types of lines are definition lines, they are of the form
x=y or x:y, they are both the same.  If y is more than one word then
it must be quoted.

Boolean values are represented with the words [on,off,yes,no,true,false]

examples:
@example
verbose: on
verbose: off
include-path=foo/bar
output-format=a21xx-coff
target-mach=21csp
adidsp=c:/adi
name-mangle: yes
keep: no
@end example

The default file is found by taking the program name and changing its
suffix to .ini.

*/

#include <stdio.h>
#include <ctype.h>
#include <config.h>
#include <adi/clargpar.h>

static
char *kill_space(p)
  char *p;
{
  while (p && *p) 
   if (isspace(*p)) p++;
   else break;
  return p;
}

/* @function parse_from_file
Read initializations from file ptr inf via cargs */
parse_from_file (cargs, f)
  cmdline_p cargs;
  FILE* f;
{
  char scratch[1024];
  char c;
  char *p;
  char *def, *val;

  while (1) {
    p = fgets (scratch, 1024, f);
    if (p == 0)
      break;
    c = *p;
    if (c == '#' || c == ';' || c == '[')
      continue;

    if (!isspace(c)) {
      while (p && *p)
       if (isspace(*p) || *p == ':' || *p == '=')
         break;
       else
         p++;
      
      def = scratch;
      val = kill_space (p);
      c = *val;
      *p = 0;
      val++;
      val = kill_space (val);

/* @subsection Value representation
unless enclosed in single quotes the value is white space delimited.

i.e.
foo=45 4
results in 45

but
foo='45 4'
results in 45 4
*/
      p=val;
      if (*p != '\'') {
        while (*p && !isspace(*p)) p++;
        *p++ = 0;
      }
      else {
        p=++val;
        while (*p && *p != '\'') p++;
        *p++ = 0;
      }
#ifdef DEBUG
      printf ("\t%s   ==> %s\n", def, val);
#endif
      clargparse_set_arg_value (cargs, def, val);
    }
  }
}

/* @function read_inifile
Read initialization values from file.  If ovfile is not null then
it is read first.  Otherwise, progname.ini is read in.  */
read_inifile (cargs, ovfile)
  cmdline_p cargs;
  char *ovfile;
{
  FILE *inf;
  int l;
  char *fme;

  inf = 0;
  fme = 0;

  if (ovfile)
    inf = fopen (ovfile, "r");
  if (inf == 0) {
    l = strlen (cargs->progname);
    fme = ovfile = malloc (l + 5); /* l + .ini + 0 */
    strcpy (ovfile, cargs->progname);
    strcpy (&ovfile[l], ".ini");
    
    inf = fopen (ovfile, "r");
  }

  if (inf) {
    parse_from_file (cargs, inf);
    fclose (inf);   
  }

  if (fme)
    free (fme);
}
