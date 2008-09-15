/* @(#)pass2.c  1.6 6/11/91 1 */

#include "app.h"
#include "a_out.h"
#include "input_file.h"
#include "input_section.h"
#include "output_section.h"
#include "mem_map_symbol.h"
#include "process_args.h"

#include "allocate_fp.h"
#include "memory_fp.h"
#include "memory_map_fp.h"
#include "pass2_fp.h"
#include "process_section_fp.h"
#include "update_fp.h"
#include "output_fp.h"


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      pass2                                                           *
*                                                                      *
*   Synopsis                                                           *
*      void pass2( void )                                              *
*                                                                      *
*   Description                                                        *
*      Responsible for coordinating all activity related to creating   *
*   the final executable.                                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/23/89       created                  -----             *
***********************************************************************/

void pass2( void )
{
   memory_check();

   process_section_list();

   allocate();

   update_linker_information();

   output_object();

   if( mflag )
       memory_map();
}
