/* @(#)main.h   1.5 1/4/91 1 */


extern short delete_preprocessor_output;
extern short listing_flag;
extern short warn_flag;
extern char  round_mode;
extern char obj_name[];
extern char src_name[];
extern char list_name[];
extern FILE *duplicate_src_stream;
extern FILE *src_stream;
extern FILE *listing_file;

extern short Ansi_c_code;
extern short Dsp_c_code;
extern short Crts;
extern short Pmstack;
extern short True_doubles;
extern short Gcc_compiled;

extern int main(int argc, char **argv);
