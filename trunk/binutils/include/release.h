/* @(#)release.h        2.2 5/26/95 2 */

#if defined (__STDC__) || defined (WIN31)
char *get_version_number (void);
char *get_release_date (void);
char *get_release_number (void);
char *get_copyright_notice (void);
char *get_warranty_notice (void);
char *get_license_notice (void);
char *get_build_stamp (void);
#else
char *get_version_number ();
char *get_release_date ();
char *get_release_number ();
char *get_copyright_notice ();
char *get_warranty_notice ();
char *get_license_notice ();
char *get_build_stamp ();
#endif

