/* @(#)release.c        2.20 10/1/96 2 */

#include <stdio.h>
#include "release.h"

static char version_number[] = "2.30";                /* Old SCCS automatically fills in,  */
													  /* set by hand now, previous 2.20    */
static char release_date  [] = "12/1/96";             /* Old SCCS automatically fills in   */
													  /* set by hand now, previous 10/1/96 */
static char release_number[] = "3.3";           /* Set by hand, previous release is 3.23 */
						      /* release with version 3.3  MODIFICATION_BL */

static char copyright_notice[] = "Copyright (c) 1991-1997 Analog Devices, Inc.";
static char build_stamp[] = __DATE__" @ "__TIME__;  /* always build this file for good stamp */

static char warranty_notice[] = "This program comes with ABSOLUTELY NO WARRANTY.";                        /*EK*/
static char license_notice[] = "This is free software licensed under the GNU General Public License.";    /*EK*/

char *get_version_number(void) { 
   return (version_number); 
   }

char *get_release_date(void) { 
   return (release_date);
   }

char *get_release_number(void) { 
   return (release_number);
   }

char *get_copyright_notice (void) { 
   return (copyright_notice);
   }
char *get_warranty_notice(void) { /*EK*/
   return warranty_notice;
   }
char *get_license_notice(void) { /*EK*/
   return license_notice;
   }
char *get_build_stamp (void) {
   return (build_stamp);
   }

