/* @(#)main.h	1.2  5/14/91 */

extern char *current_file_name;
extern char **argptr;
extern int argcnt;
void usage (void);
extern char map_file[];


void init_pass1();
void interrupt_handler();

#define PASSED 0
#define FAIL 1

