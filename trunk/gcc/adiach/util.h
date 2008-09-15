/* $Id: util.h,v 2.7 1996/01/02 13:45:09 mmh Exp $ %R% */
#if !defined ( MSDOS ) || defined (__GNUDOS__) || defined (__EMX__) || defined(__GO32__)
#define huge 
#endif

extern void           *my_malloc                  (long);
extern void           *my_calloc                  (long);
extern void           *my_realloc                 (void *, long);
extern void           *my_strdup                  (void *);
extern void            my_free                    (void *);

extern void strtoupper( char *str );
extern long port_get_long( unsigned char huge *buffer );
extern void port_put_long( long w, unsigned char huge *buffer );
extern short port_get_short( unsigned char huge *buffer );
extern void port_put_short( short w, unsigned char huge *buffer );
void append_ext (char *filename, char *ext);
void strip_ext (char *filename);
void strip_path (char *filename);

void uns_to_hex_string (void *number, char *string, size_t width);
void hex_string_to_uns (char *string, void *buffer, size_t width);
void add_40 (UNS40 *a, UNS40 *b, UNS40 *c);
void add_48 (UNS48 *a, UNS48 *b, UNS48 *c);
void align (unsigned short width_1, void *b1, unsigned short width_2, void *b2);

unsigned short get_mem_from_file (FILE *file_ptr, 
				  unsigned short autowrap,
				  unsigned short format,
				  size_t width,
				  void *buffer);
void remove_new_lines (char *string);
char *make_my_slash(char *, int);
