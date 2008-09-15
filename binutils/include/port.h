/* @(#)port.h	1.2 11/22/89 1 */

extern void port_write_object_header( register OBJ_HEADER *header, FILE *fd );
extern void port_write_section_header( register SEC_HEADER *header, FILE *fd );
extern void port_write_symbol( register OBJ_SYM *ptr, FILE *fd );
extern void port_write_relocation_info( register RELOC *ptr, FILE *fd );
extern void port_write_aux_info( register AUXENT *ptr, FILE *fd, int aux_class );
extern void port_write_line_info( register LINE *ptr, FILE *fd );

extern int port_read_object_header( register OBJ_HEADER *header, FILE *fd );
extern int port_read_section_header( register SEC_HEADER *header, FILE *fd );
extern int port_read_symbol( register OBJ_SYM *ptr, FILE *fd );
extern int port_read_relocation_info( register RELOC *ptr, FILE *fd );
extern int port_read_aux_info( register AUXENT *ptr, FILE *fd, int aux_class );
extern int port_read_line_info( register LINE *ptr, FILE *fd );
