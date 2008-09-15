#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(MSDOS) || defined(_WIN32)
#include <io.h>
#ifdef __GNUC__
#ifndef __EMX__
#include <std.h>
#endif
#include <unistd.h>
#endif
#ifndef R_OK
#define R_OK 4
#endif
#else
#include <sys/file.h>
#include <unistd.h>
#endif

#include "achparse.h"
#include "gcc_ach.h"
#include "app.h"
#include "atfile.h"
#include "util.h"
#include "tm.h"

static CONS 	*ach_arg_list = NULL;
static SW_TOKEN	 target_options[]  = TARGET_OPTIONS;
static SW_TOKEN	 target_switches[] = TARGET_SWITCHES;
static char	*reg_set[2] = {NULL, NULL};
static char	*default_ach="21k.ach";
static char	*default_runhdr="020_hdr.obj";
static int      target_z3 = 0;
extern char *startfile_spec;

/* 0 == uninitialized, non-zero == initialzed */
static int	 have_value[4] = {0,0,0,0};

static int	 print_anal_ach_file_ignore_warnings = 0;

static void ignore(char *);
static void add_reg(char **, char *);
static void make_switch(SW_TYPE, char *);
static void add_switch(char *sw);
void ach_overlay (char *, char *, char *); /* need dummy fcn here */

extern void fatal( char *,...);
extern void error( char *,...);


#define NUM_TARGET_SWITCHES	(sizeof(target_switches)/sizeof(SW_TOKEN))
#define NUM_TARGET_OPTIONS	(sizeof(target_options)/sizeof(SW_TOKEN))

#define ARCH_FLAG "-arch"
#define ARCH_FLAG_P(X) (!strcmp(X,"-a")||!strcmp(X,"-arch"))

static void default_z3 (void)
{
  default_ach = "21060.ach";
  default_runhdr = "060_hdr.obj";
  target_z3 = 1;
}

#ifdef TEST_ACHFILE
main(int argc, char **argv)
{
    printf("------ Args before : ------\n");
    dumpargs(argc, argv);

    process_architecture_file(&argc,&argv);
    printf("------ Args after processing atfiles and ach file : ------\n");
    dumpargs(argc, argv);
}

static void error(msg, a1, a2, a3)
     char *msg;
{
    fprintf(stderr, "error: ");
    fprintf(stderr, msg, a1, a2, a3);
    fprintf(stderr, "\n");
}

static void fatal(msg, a1, a2, a3)
     char *msg;
{
    fprintf(stderr, "fatal error: ");
    fprintf(stderr, msg, a1, a2, a3);
    fprintf(stderr, "\n");
    exit(1);
}

#endif /* TEST_ACHFILE */

#define obsolete(x) ach_error ("switch is obsolete " x " refer to manual")

/**************************************************************/


void process_architecture_file(int *pargc, char ***pargv)
{
    char	*ach_file = NULL;
    char	 buffer[BUFSIZ];
    static char  tmp_file_spec[BUFSIZ];
    int		 i;
    int		 nnargs;
    int		 runhdr = 0;

    /*
     * find the architecture file
     */
    for(i = 1 ; i < *pargc ; i++)
    {
	if(ARCH_FLAG_P((*pargv)[i]))
	{
	    if(NULL != ach_file)
		fatal("-a given more than once");

	    (*pargv)[i] = ARCH_FLAG;

	    if(++i == *pargc)
		fatal("no architecture file specified with -a");

	    ach_file = (*pargv)[i];
	}

	if(!strcmp("-runhdr", (*pargv)[i]))
	{
	    if(++i == *pargc)
		fatal("no runtime header specified with -runhdr");

	    runhdr = 1;
	}
    }

    /*
     * save the last switch because 'expand_args()' needs to REPLACE it
     */
    add_switch((*pargv)[*pargc - 1]);

    if (ach_file == NULL)
    {
	/*
	 * no architecture file given ... look for it first in the
	 * current directory, then in $ADI_DSP
	 */

	if (!access(default_ach, R_OK))
	    ach_file = default_ach;
	else if (!access ("21060.ach", R_OK))
	  ach_file = "21060.ach";
	else
	  {
	    char	*buf = buffer;
	    char	*cp;
#ifdef MSDOS
	    char	 term = '\\';
#else
	    char	 term = '/';
#endif

	    if ((cp = getenv("ADI_DSP")))
	      {
		strcpy(buf, cp);
		buf += strlen(buf);
		if (buf[-1] != term)
		  *buf++ = term;

		sprintf(buf, "%s%c%s%c", "21k", term, "lib", term);
		buf += strlen(buf);

		strcpy(buf, default_ach);
		if (!access(buffer, R_OK))
		  ach_file = buffer;
		else
		  {
		    strcpy(buf, "21060.ach");
		    if (!access(buffer, R_OK))
		      ach_file = buffer;
		  }
	      }
	  }

	if (ach_file)
	  {
	    /* if an architecture file is not explicit add it to ARGV */
	    add_switch (ARCH_FLAG);
	    add_switch (ach_file);
	  }
      }

    if(NULL != ach_file)
      {
	/*
	 * we found one somewhere - parse it and add the
	 * appropriate switches
	 */
	if(ach_parse(ach_file))
	    fatal("failed to parse architecture file \"%s\"", ach_file);
      }
    else error("Couldn't find an architecture file.");


#if 0
    /* The following thunk of code adds the appropriate local version of the
       runtime header.  We use target_z3 flag which is set when .processor
       sees o6o. Note the position of this thunk is important it needs to come after
       the ach_parse. */
    if(!runhdr)
      {
	/*
	 * if -runhdr wasn't given on the command line, look for
	 * the runtime header in the current directory
	 */
	if (!target_z3 && !access (default_runhdr, R_OK))
	  {
	    add_switch ("-runhdr");
	    add_switch (default_runhdr);
	  }
	else if (target_z3 && !access ("060_hdr.obj", R_OK))
	  {
	    add_switch ("-runhdr");
	    add_switch ("060_hdr.obj");
	  }
      }
#endif

/*
   AS - changed this b'cse it only allows for the runhdr to be included if
   it is in your local directory.  The driver should be able to take the one
   in the lib directory also. */
    if(!runhdr)
      {
	if (!access(default_runhdr, R_OK))
	  {
	    sprintf (tmp_file_spec, "%%{!runhdr:%s}", default_runhdr);
	    startfile_spec=tmp_file_spec;
	  }
	else
	  {
	    /* If -runhdr specified is not in the local directory (done above),
	       go look for it in the lib directory of the ADI_DSP tree */

	    sprintf (tmp_file_spec, "%%{!nostdlib:%%{!runhdr:%s%%s}%%{runhdr*:%%*}}", default_runhdr);
	    startfile_spec=tmp_file_spec;
	  }
      }

    /* Check for .REGISTER/RESERVED introducing a -mreserved= switch... */

    if (reg_set[RESERVED]) {
      char *buf=malloc (strlen (reg_set[RESERVED])+1+sizeof ("-mreserved="));

      strcpy( buf, "-mreserved=");
      strcat( buf, reg_set[RESERVED] );
      add_switch (buf);
      free (buf);
    }

    /*
     * add any new switches ... remember that there the last argument
     * will always have been added as the first switch (hence the >1)
     */
    if((nnargs = atfile_get_length(ach_arg_list)) > 1)
    {
	int last = *pargc - 1;

	*pargv = atfile_expand_argv(*pargc, *pargv, last, nnargs);
	atfile_copy_list(*pargv + last, ach_arg_list);
	*pargc += nnargs - 1;
    }

}


/**************************************************************/



static void add_switch(char *sw)
{
    CONS *new_consp;
    static CONS *last_cons = NULL;

    if(NULL == (new_consp = (CONS *)malloc(sizeof(CONS) + strlen(sw))))
    	fatal("malloc() failed");

    new_consp->next = NULL;
    strcpy(CAR(new_consp), sw);

    if(NULL == ach_arg_list)
	last_cons = ach_arg_list = new_consp;
    else
    {
	CDR(last_cons) = new_consp;
	last_cons = new_consp;
    }

}



/**************************************************************/


void add_reg(char **reg_str, char *new_reg)
{
    char *cp;

    if(NULL == *reg_str)
	*reg_str = (char *)strdup(new_reg);
    else
    {
	*reg_str = realloc(*reg_str, strlen(*reg_str) + strlen(new_reg) + 2);
	strcat(*reg_str, ",");
	strcat(*reg_str, new_reg);
    }

    for(cp = *reg_str ; *cp ; cp++)
	if(isupper(*cp))
	    *cp += 32; /* tolower(*cp); */
}


/**************************************************************/


static void ignore(char *msg)
{
    if(print_anal_ach_file_ignore_warnings)
	error("directive \"%s\" ignored", msg);
}


/**************************************************************/


static void make_switch(SW_TYPE sw, char *val)
{
    int i;
    int found = 0;
    char buf[BUFSIZ];

    for(i = 0 ; i < NUM_TARGET_SWITCHES ; i++)
	if(target_switches[i].id == sw)
	{
	    add_switch(target_switches[i].sw_name);
	    found = 1;
	}

    for(i = 0 ; i < NUM_TARGET_OPTIONS ; i++)
	if(target_options[i].id == sw)
	{
	    if((strlen(target_options[i].sw_name) + strlen(val) + 1) > BUFSIZ)
		fatal("string too long");

	    strcpy(buf, target_options[i].sw_name);
	    strcat(buf, val);
	    add_switch(buf);
	    found = 1;
	}

    if(!found)
    	fatal("unknown SW_%d:%s switch", sw, val);

}


/**************************************************************/


void ach_cstack (unsigned long length, int type, int access, char *label)
{
    if(have_value[STACK_LOCATION]++)
	error("stack location redefinition ignored");
    else if(ACH_PM == type)
	    error("Cannot put stack in program memory");
}


/**************************************************************/


void ach_system (char *label)
{
    ignore(".SYSTEM");
}


/**************************************************************/


void ach_bank (unsigned long begin, unsigned long pgsize, unsigned long wtstates, unsigned short type, unsigned short wtmode, unsigned short which_bank, unsigned short pgwten)
{
    ignore(".BANK");
}


/**************************************************************/


void ach_processor(int processor_token)
{
  switch (processor_token)
    {
    case ACH_ADSP21010:
      make_switch(SW_ADSP21010, "");
      break;

    case ACH_ADSP21020:
      make_switch(SW_ADSP21020, "");
      break;

    case ACH_ADSP210Z3:
      make_switch(SW_ADSP210Z3, "");
      default_z3 ();
      break;

    case ACH_ADSP21061:
      make_switch(SW_ADSP21061, "");
      default_z3 ();
      break;

    case ACH_ADSP210Z4:
      make_switch(SW_ADSP210Z4, "");
      default_z3 ();
      break;
    }
}


/**************************************************************/


void ach_segment(unsigned long begin, unsigned long end, int type_token, int access_token, char *symbol, unsigned short have_uninit, unsigned long width)
{
    ignore(".SEGMENT");
}


/**************************************************************/


void ach_compiler(short map_double_to_float, short jjb, short param_passing, short chip_rev)
{
    if(map_double_to_float)
	if(have_value[FLOAT_MODEL]++)
	    error("floating point representation redefinition ignored");
	else
	    if(map_double_to_float < 0)
		make_switch(SW_DAD, "");
	    else
	      obsolete("doubles-are-floats");


    if(jjb)
	if(have_value[CALL_MODEL]++)
	    error("calling model redefinition ignored");
	else
	    if(jjb < 0)
	      obsolete ("pc-rts");
	    else
		make_switch(SW_RTRTS, "");


    if(param_passing)
	if(have_value[PARAMETER_MODEL]++)
	    error("parameter passing model redefinition ignored");
	else
	    if(param_passing < 0)
	      obsolete ("paramaters on stack");
	    else
		make_switch(SW_PPINREG, "");


    if(chip_rev)
	ignore(".COMPILER/chip_rev");

}


/**************************************************************/


void ach_scratch_register(char *reg)
{
    add_reg(&reg_set[SCRATCH], reg);
}


/**************************************************************/


void ach_endsys(void)
{
    ignore(".ENDSYS");
}


/**************************************************************/


void ach_cdefaults (ACH_TOKEN which_def, int type, char *label)
{
    switch(which_def)
    {
      case ACH_CCODE:
	if(ACH_PM == type)
	    make_switch(SW_PMCODE, label);
	else
	    error("/CCODE must be in PM");
	break;

      case ACH_CDEFAULT:
	if(ACH_PM == type)
	    make_switch(SW_PMDATA, label);
	else
	    make_switch(SW_DMDATA, label);
	break;

      default:
	ignore("/Csegment");
	break;
    }
}


/**************************************************************/


void ach_cheap (unsigned long length, int type, int access, char *label)
{
    ignore("/CHEAP");
}


/**************************************************************/


void ach_error(char *msg)
{
    fatal("%s", msg);
}


/**************************************************************/


void ach_reserved_register(char *reg)
{
    add_reg(&reg_set[RESERVED], reg);
}


/**************************************************************/


void ach_circular_register(char *dag_register, char *variable)
{
    ignore(".REGISTER/CIRC");
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      strtoupper                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void strtoupper(str)                                            *
*      char *str;                                                      *
*                                                                      *
*   Description                                                        *
*      This routine converts a string to uppercase.                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/16/89       created                  -----             *
***********************************************************************/

void strtoupper( register char *str )
{
    while( *str != '\0' )
    {
	   if( islower(*str) )
	       *str = toupper(*str);
	   str++;
    }
}

/********************************************************************

Name:           remove_new_lines

Function:       Removes new line characters, <cr>, <lf>
                Allows unix/dos compatibility

Return Value:   none

********************************************************************/

void remove_new_lines (char *string) {

/* Do it */
   while (*string) {
     if ((*string == '\n') || (*string == '\r')) {
       *string = '\0';
     }
     string++;
   }

/* Return */
 }

/* Need a dummy function here.  The linker actually uses this function for
something.  g21k could care less about this, but in order to resolve
definitions, we need to have something - see GAS for more info.  Updated on
1/95 */

void ach_overlay (char *x, char *y, char *z)
{}
