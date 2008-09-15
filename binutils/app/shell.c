/* @(#)shell.c  1.3 9/16/94 */

/*
 * Shell which calls cpp and a21000
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef  MSDOS
#include <process.h>
#undef  stderr
#define stderr  stdout
#endif

#include "release.h"
#include "app.h"
#include "util.h"
#define NAME_BUFF_SIZE 200


void usage(char *exename);
int main(int argc, char *argv[]);
static void banner (void);
static short banner_printed = 0;

#define     MAXARG      15

static char system_string[132];
static char     file_name[NAME_BUFF_SIZE];
static char     a21000_name[NAME_BUFF_SIZE];
static char     cpp_name[NAME_BUFF_SIZE];

extern char    *getenv();
static char    *include_path = (char *) NULL;
static int      dopp = FALSE;
static char    *execargv[MAXARG] = 

{
#ifdef  MSDOS
    "a21000.exe",
#else
    "a21000",
#endif

    (char *) 0,
};
static char    *cppargv[MAXARG] = 

{
#ifdef  MSDOS
    "cpp.exe",
#else
    "cpp",
#endif

    (char *) 0,
};


/*
 * Main driver
 */

void usage(char *exename)
{
    exename;
    fprintf( stderr, "\n" );    
    fprintf(stderr, "Usage: cc21k [-adsp21060 -Dname -h -l -o name -pp -sp -v] filename\n");
		      /*EK* ^^^^^ changed from "asm21k" which really must be wrong... */
    fprintf( stderr, "Where:\n");
    fprintf( stderr, "\t-adsp21060\tAllow 21060 specific instructions\n");
    fprintf( stderr, "\t-Dname\t\tDefine 'name' to the assembler preprocessor\n" );
    fprintf( stderr, "\t-h\t\tDisplay usage\n" );
    fprintf( stderr, "\t-l\t\tGenerate a listing file\n" );
    fprintf( stderr, "\t-o name\t\tName the output object file\n" );
    fprintf(stderr, "\t-pp\t\tPreprocessor only\n");
    fprintf( stderr, "\t-sp\t\tSkip the assembler preprocessor\n" );
    fprintf( stderr, "\t-v\t\tVerbose\n" );
    fprintf( stderr, "\n" );    
    exit(-1);
}

int  main(int argc, char *argv[])
{

    short           i;
    short           arg;
    short           cpparg;
    short           skip_pp = FALSE;
    short           pponly  = FALSE;
    short           verbose = FALSE;
    short           bogus = FALSE;
    short           filecnt;
    char            *tp1 = NULL;
    char            *apc = NULL;
    char            cpp_infile[NAME_BUFF_SIZE];
    char            a21000_infile[NAME_BUFF_SIZE];
    char            root_ptr[NAME_BUFF_SIZE];
    char            outfile[NAME_BUFF_SIZE];
    char            *named_outfile = NULL;
    char            *infile = NULL;
    char            *path;
    FILE            *fp;



/* Check for verbose flag first */
   for( i = 1; i < argc; i++) {
     if( *argv[i] == '-' ) {
       switch(*(argv[i] + 1) ) {
	 case 'v':
	 case 'V':
	 case 'h':
	 case 'H':
	   verbose = TRUE;
	   break;
	 }
     }
   }
   if (verbose) {
     if (!banner_printed) {
       banner ();
       banner_printed = 1;
     }
   }


/* Process arguments */
    for( i = 1, arg = 1,cpparg=1; i < argc; i++ ) 
    {
	 if( *argv[i] == '-' ) 
	 {
	     switch(*(argv[i] + 1) ) 
	     {
		     case 'p':
		     case 'P':
			if( (*(argv[i] + 2) == 'p') || (*(argv[i] + 2) == 'P') )
			    pponly = TRUE;
			break;
		     case 'b':
		     case 'B':
			if( (*(argv[i] + 2) == 'o') || (*(argv[i] + 2) == 'O') )
			    bogus = TRUE;
			break;

		     case 's':
		     case 'S':
			if( (*(argv[i] + 2) == 'p') || (*(argv[i] + 2) == 'P') )
			    skip_pp = TRUE;
			break;

		     case 'l':
		     case 'L':
			execargv[arg++] = argv[i];
			break;

		     case 'z':
		     case 'Z':
			execargv[arg++] = argv[i];      
			break;

		     case 'u':
		     case 'U':
			execargv[arg++] = argv[i];
			break;

		     case 'c':
		     case 'C':
			execargv[arg++] = argv[i];
			break;
		
		     case 'v':
		     case 'V':
			cppargv[cpparg++] = argv[i];  /* Pass it thru to cpp */
			execargv[arg++] = argv[i];  /* Pass it thru to assembler */
			break;

		     case 'd':
		     case 'D':
			cppargv[cpparg++] = argv[i];  /* Pass it thru to cpp */
			break;

		     case 'o':
		     case 'O':
			execargv[arg++] = argv[i];
			i++;
			execargv[arg++] = argv[i];
			named_outfile = make_my_slash(argv[i], FALSE);
			break;
		     case 'a':
		     case 'A':
		       if(!strncmp((char *)(argv[i]+5),"21060",5))
			   execargv[arg++] = argv[i];
		       else {
			   usage( argv[0] );
		       }
		       break;

		     case 'h':
		     case 'H':
		     default:
			usage( argv[0] );
	     }
	 }
	 else 
	 {
	     if( infile == NULL ) 
	     {
		 infile = make_my_slash(argv[i], FALSE);
		 filecnt = 1;
	     }
	     else
		 fprintf( stderr,"file already specified", "" );
	 }
    }


    if( (filecnt == 0) || (argc == 1) ) {
      if (!banner_printed) {
	banner ();
	banner_printed = 1;
      }
      usage(argv[0]);
    }

    if (!infile)
	exit (0);

    if( strlen(infile) > NAME_BUFF_SIZE - 2 )
    {
	fprintf( stderr, "Filename %s is too long\n", argv[i] );
	exit( 1 );
    }

    /* set up input to cpp and input to a21000 */
    strcpy(cpp_infile,infile);
    append_ext(cpp_infile,".asm");

    strcpy(root_ptr,infile);
    strip_ext(root_ptr);
    strcpy( a21000_infile, root_ptr );
    strcat( a21000_infile, ".is" );
	
    /* get the final output file name for check */

    if (named_outfile)
	strcpy(outfile,named_outfile);
    else {
	strcpy(outfile, root_ptr);
	strcat(outfile,".obj");
    }

    if(pponly && named_outfile) 
	strcpy( a21000_infile, named_outfile); /* this will be final output */

#ifdef DEBUG
    i = 0;
    while( execargv[i] )
    {
	   fprintf( stderr, "%d\t%s\n", i, execargv[i] );
	   i++;
    }
#endif

    if (!bogus)
	cppargv[cpparg++] = "-lang-asm";    
    cppargv[cpparg++] = "-P";    
    cppargv[cpparg++] = "-undef";    
    cppargv[cpparg++] = cpp_infile;    
    cppargv[cpparg++] = a21000_infile;    
    cppargv[cpparg++] = (char *)NULL;    

    execargv[arg++] = "-r";    
    execargv[arg++] = a21000_infile;
    execargv[arg++] = (char *)NULL;



    if( (path = get_adi_dsp()) != NULL )  /*EK*/
    {
	i = 0;
	while( *path && *path != ';' )
	{
	    file_name[i] = *path;
	    path++;
	    i++;
	}

	if( file_name[i-1] != '\\' && file_name[i-1] != '/' )
	{
	    file_name[i] = FILENAME_SEPARATOR;
	    ++i;
	}
	file_name[i] = '\0';
#ifdef MSDOS
	strcat (file_name,"21k\\etc\\");
#else
	strcat (file_name,"21k/etc/");
#endif
	strcpy(cpp_name,file_name);
	strcpy(a21000_name,file_name);
	strcat( cpp_name, cppargv[0] );
	strcat( a21000_name, execargv[0] );
    }
    else {
	fprintf(stderr,"ADI_DSP not set\n");
	fprintf(stderr,"Execution terminated\n");
	exit(-1);
    }

/* Build the system string */
   strcpy (system_string, cpp_name);
   strip_ext(system_string); /* don't want .exe */
#ifdef MSDOS
/*
   printf(system_string);
   printf("\n");
for (i=0;i<cpparg;i++) {
   printf(cppargv[i]);
   printf("\n");
}
*/
   i = spawnv(P_WAIT,system_string,cppargv);
#else
   strcat (system_string, " ");
   i = 1;
   while (cppargv[i]) {
     strcat (system_string, cppargv[i]);
     strcat (system_string, " ");
     i++;
   }
/* Call cpp */
    i = system (system_string);
#endif /* MSDOS */
    if (i) {
      fprintf(stderr, "Error(s) found when Pre-Processing\n");
      exit(-1);
    }
    /* now see if cpp completed */
    if ((fp = fopen(a21000_infile, "r")) == (FILE *) NULL) {
	fprintf(stderr,"Pre-Processor did not complete. Execution terminated.\n");
	exit(-1);
    } else {
	fclose(fp);
    }


    if( pponly )
	exit( 0);

/* Build the system string */
   strcpy (system_string, a21000_name);
   strip_ext(system_string); /* don't want .exe */
#ifdef MSDOS
   i = spawnv(P_WAIT,system_string,execargv);
#else
   strcat (system_string, " ");
   i = 1;
   while (execargv[i]) {
     strcat (system_string, execargv[i]);
     strcat (system_string, " ");
     i++;
   }
/* Call the real assembler */
    i = system (system_string);

#endif /* MSDOS */
    if (i) {
      fprintf(stderr, "Error found executing assembler. Execution terminated\n");
      exit(-1);
    }

    /* now see if a21000 completed */
    if ((fp = fopen(outfile, "r")) == (FILE *) NULL) {
	fprintf(stderr,"Assembler did not complete. Execution terminated.\n");
	return -1;
    } else {
	fclose(fp);
	return 0;
    }
}

/*******************************/
static void banner (void) {
     printf( "\nAnalog Devices ADSP-210x0 Assembler \n");
     printf( "Release %s, Version %s\n", get_release_number(), get_version_number());
     printf( "%s\n", get_copyright_notice() );
     printf( "%s\n", get_warranty_notice() ); /*EK*/
     printf( "%s\n\n", get_license_notice() ); /*EK*/
     }

 int* D16HugeAlloc(long x)
{
	return malloc((short)x);
}

