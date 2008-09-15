#ifndef _CONFIGURE_H
#define _CONFIGURE_H

#ifndef DLLINTERFACE
#if defined (_WINDLL) || defined (_WINDOWS)
#ifndef WIN32
#define DLLINTERFACE _loadds
#else
#define DLLINTERFACE 
#endif /* end of #ifndef WIN32 */
#else
#define DLLINTERFACE 
#endif
#endif

#ifdef unix
#define VOID void
#define DWORD unsigned long
#define WORD int
#define BOOL int
#define HANDLE int
#define LPSTR  char*
#ifndef TRUE
#define TRUE 1
#endif
#define FALSE 0
#define HWND int
#define LPARAM long
#endif

#endif
