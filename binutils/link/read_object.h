/* @(#)read_object.h	1.4  6/11/91 */

#define TABLE_SIZE  2000
#define MAXSYMS     60

extern long ar_size;
extern LIST input_file_list;
extern long max_section_size;

typedef union
{
    long l;
    char c[sizeof(long) / sizeof(char)];
} LONG_CHAR;
