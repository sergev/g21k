

#define ESCAPE_IO       1

#define IO_FFLUSH       1
#define IO_FPUTC        2
#define IO_FGETC        3
#define IO_FOPEN        4
#define IO_GET_CLOCK    5
#define IO_INIT         6
#define IO_EXIT         7

#define IO_IO_SEND      100
#define IO_IO_TERM      101
#define IO_IO_DONE      102


#define io_set_int32(n,b) \
 ( (b)[3] = (n) & 0xff, (b)[2] = ( (n) >>  8 ) & 0xff, \
   (b)[1] = ( (n) >> 16 ) & 0xff, (b)[0] = ( (n) >> 24 ) & 0xff )

#define io_get_int32(b) \
 ( ( ( ( ( ( long )( (b)[0] << 8 ) + ( long )( (b)[1] & 0xff ) ) << 8 ) + ( long )( (b)[2] & 0xff ) ) << 8 ) + ( long )( (b)[3] & 0xff ) )

