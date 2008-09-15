/* $Id: atfile.h,v 1.6 1996/06/27 20:37:04 lehotsky Exp $ %R% */

/*
 * To use atfile's just include this header file in your main file.
 * i.e. the file which has main definied.
 */

typedef struct  cons {
    struct cons *next;
    char str[1];
} CONS;


#define CDR(C) ((C)->next)
#define CAR(C) ((C)->str)

extern void process_atfiles (int *pargc, char ***pargv);
extern char **atfile_expand_argv (int, char *[], int, int);
extern int atfile_get_length(CONS* );
extern void atfile_copy_list (char *[], CONS *);
extern void free_argv(void);


int    get_length(CONS*);
char **expand_argv(int, char**, int, int);
void   copy_list(char**, CONS*);
