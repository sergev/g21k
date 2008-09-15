/* 
 * Oct. 29			CREATED_BL
 */

#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>       
#include <assert.h>    

#include "app.h"
#include "win_main.h"
#include "util.h"
#include "mem_adi.h"
#include "dis.h"
#include "reg.h"
#include "flt_pt.h"
#include "bp.h"
#include "sym.h"
#include "status.h"
#include "cach.h"
#include "common.h"


#include "21kasm.fp"

#ifdef SIM
#include "mem_sim.h"
#include "reg_sim.h"
#endif

#ifdef ICE
#include "error.h"
#include "stk.h"
#include "sms.h"
#include "mem_map.h"
#endif

// 32 line of cache plus two line of headings
// total 34 line, each line have ONELINEFORMATCACHE
#define ONELINEFORMATCACHE	120

typedef struct 
{
	char szOneLineEntry[2*CACH_SETS_PER_CACHE + 2][ONELINEFORMATCACHE +1];
} CACHESNAP;		

#define CACHEONELINEDISPLAYFORMAT1  " %3s   %3s    %5s  %8s   %12s"
#define CACHEONELINEDISPLAYFORMAT2  " %3d   %3d    %5d  %08lx   %12s"
#define CACHEONELINEDISPLAYFORMAT3  "              %5d  %08lx   %12s"

#define INSTRUCTIONLEN				12

// file I/O error stringtable entries
#define IDS_OPENFAILED			WM_USER+101
#define IDS_READFAILED			WM_USER+103
#define IDS_REALLOCFAILED		WM_USER+104   

#ifdef SIM
CACHESNAP cachesnap;
short TakeCacheSnap(CACHESNAP * const pCacheSnap, const CACH * const pCache );
extern CACH cache;
#endif

/* Create a new display windows, it is modified from addMemWnd */
DISPCONTROLAUX * 
addEditDispControlWnd (WPARAM wParam)
{
	MDICREATESTRUCT mcs;
	HWND hwnd;
	RECT rect;

#ifndef WIN32
	/* This is really a gross measure, and OS dependant besides.  */
	if (GetFreeSystemResources (GFSR_SYSTEMRESOURCES) < MINRESOURCES)
    {
		MessageBox (hClientWnd, "Please close some windows.", "Insufficient System Resources",
					MB_ICONINFORMATION | MB_OK);
		return (NULL);
    }
#endif

	GetClientRect (hFrameWnd, &rect);

	mcs.szClass = EDITDISPCONTROLWNDCLASS;
	mcs.szTitle = "Architecture file View";        //  NULL;
	mcs.hOwner  = hCurInst;
	mcs.x       = rect.left;
	mcs.y       = rect.top;
	mcs.cx      = rect.right/2;
	mcs.cy      = rect.bottom * 3 / 4;
	mcs.style   = 0;      
	mcs.lParam  = (LPARAM) wParam;

	hwnd = (HWND) SendMessage (	hClientWnd, WM_MDICREATE, 0,
								(long) (LPMDICREATESTRUCT) &mcs);
	if (!hwnd)
		return (NULL);

	return (DISPCONTROLAUX *) GetWindowLong (hwnd, 0);
}

/* Create a new display windows, it is modified from addMemWnd */
DISPCONTROLAUX * 
addListDispControlWnd (WPARAM wParam)
{
	MDICREATESTRUCT mcs;
	HWND hwnd;
	RECT rect;

#ifndef WIN32
	/* This is really a gross measure, and OS dependant besides.  */
	if (GetFreeSystemResources (GFSR_SYSTEMRESOURCES) < MINRESOURCES)
    {
		MessageBox (hClientWnd, "Please close some windows.", "Insufficient System Resources",
					MB_ICONINFORMATION | MB_OK);
		return (NULL);
    }
#endif

	GetClientRect (hFrameWnd, &rect);

	mcs.szClass = LISTDISPCONTROLWNDCLASS;
	mcs.szTitle = "Architecture file View";        //  NULL;
	mcs.hOwner  = hCurInst;
	mcs.x       = rect.left;
	mcs.y       = rect.top;
	mcs.cx      = rect.right/2;
	mcs.cy      = rect.bottom * 3 / 4;
	mcs.style   = 0;      
	mcs.lParam  = (LPARAM) wParam;

	hwnd = (HWND) SendMessage (	hClientWnd, WM_MDICREATE, 0,
								(long) (LPMDICREATESTRUCT) &mcs);
	if (!hwnd)
		return (NULL);

	return (DISPCONTROLAUX *) GetWindowLong (hwnd, 0);
}


/* 
	I think it is easier to write different version for SHARC EZKIT and normal
	program, compare using one version function for two version. that is why
	I use the following #ifndef EZKIT and #ifdef EZKIT preprocessor 
*/

#ifndef EZKIT
/* open a file and load into edit control */
int WINAPI LoadFileToEditCtrl (HWND    hWndEdit, char    *lpszName)
{
 
    HANDLE  hEditData;
    char    *lpEditData;
	char	szBuffer[20];

  	unsigned long           file_size;
  	unsigned long           initial_offset;
  	FILE                   *file_ptr = NULL;

    if ((file_ptr = fopen (lpszName, READ_BINARY)) == NULL)
        return (-1);

    /* Get the size of the file */
    initial_offset = ftell (file_ptr);
	fseek (file_ptr, 0L, SEEK_END);
    file_size = ftell (file_ptr) - initial_offset;
    fseek (file_ptr, initial_offset, SEEK_SET);


#if !defined (WIN32)           
    
	/* get the edit control's memory handle */
    if (!(hEditData = (HANDLE)SendMessage (hWndEdit, EM_GETHANDLE, 0, 0L)))
    {
		fclose(file_ptr);
		return -1;
    }                                                   
  
    /* realloc the memory to fit the new file size */
    if (((hEditData = LocalReAlloc(hEditData, file_size+1, LHND)) == NULL) ||

#else
    
    if (((hEditData = LocalAlloc (LHND, file_size+1)) == NULL) ||

#endif

    (!(lpEditData = (char *)LocalLock (hEditData))))
    {
		/* close file and return error */
		fclose(file_ptr);
		return 0-IDS_REALLOCFAILED;
    }                                          
    
    /* read the file into hEditData buffer */
    if (fread(lpEditData,sizeof(char), file_size, file_ptr) == -1)
    {
		/* close file and return error */
		fclose(file_ptr);
		return 0-IDS_READFAILED;
    }

    /* null terminate edit buffer */
    lpEditData[file_size] = 0;
    LocalUnlock (hEditData);

    /* load buffer into edit control and close file */

#if !defined(WIN32)

    SendMessage (hWndEdit, EM_SETHANDLE, (UINT)hEditData, 0L);

#else

    lpEditData = LocalLock (hEditData);
    SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM)lpEditData);
    GetLastError();
    LocalUnlock (hEditData);
    // LocalFree (hEditData); // Isn't there a synchronization issue with this?

#endif

    fclose(file_ptr);

    /* return success */
    return TRUE;
}

#endif			/* end of #ifndef EZKIT */

#ifdef EZKIT
extern char szEzSharcAchStr[];
/* open a file and load into edit control */
int WINAPI LoadFileToEditCtrl (HWND    hWndEdit, char    *lpszName)
{
 
    HANDLE  hEditData;
    char    *lpEditData;
  	unsigned long           file_size;

	assert(lpszName != NULL);

	file_size = strlen(szEzSharcAchStr);
  

#if !defined (WIN32)           
    
	/* get the edit control's memory handle */
    if (!(hEditData = (HANDLE)SendMessage (hWndEdit, EM_GETHANDLE, 0, 0L)))
    {
		return -1;
    }                                                   
  
    /* realloc the memory to fit the new file size */
    if (((hEditData = LocalReAlloc(hEditData, file_size+1, LHND)) == NULL) ||

#else
    
    if (((hEditData = LocalAlloc (LHND, file_size+1)) == NULL) ||

#endif

    (!(lpEditData = (char *)LocalLock (hEditData))))
    {
		return 0-IDS_REALLOCFAILED;
    }                                          
    
    /* read the file into hEditData buffer */
	memcpy(lpEditData, szEzSharcAchStr, file_size);

    /* null terminate edit buffer */
    lpEditData[file_size] = 0;
    LocalUnlock (hEditData);

    /* load buffer into edit control and close file */

#if !defined(WIN32)

    SendMessage (hWndEdit, EM_SETHANDLE, (UINT)hEditData, 0L);

#else

    lpEditData = LocalLock (hEditData);
    SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM)lpEditData);
    GetLastError();
    LocalUnlock (hEditData);
    // LocalFree (hEditData); // Isn't there a synchronization issue with this?

#endif


    /* return success */
    return TRUE;
}
#endif		/* end of #ifdef EZKIT */


/* IMPORTANT MDI NOTE:
   If cases are added for any of the following messages, they need to be
   passed on to the DefMDIChildProc() as well (in deference to the usual
   convention of returning zero) even if we handle them.  See the SDK
   Reference entry for DefMDIChildProc():

             WM_CHILDACTIVATE
             WM_GETMINMAXINFO
             WM_MENUCHAR
             WM_MOVE
             WM_NEXTMENU
             WM_SETFOCUS
             WM_SIZE
             WM_SYSCOMMAND
*/

long FAR PASCAL _export
EditDispControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND	hListBoxWnd;
	RECT	rect;
	short	nResult;
	char *  pszAchFile = NULL; 
	HWND    hWndEdit;
	DISPCONTROLAUX *aux = (DISPCONTROLAUX *) GetWindowLong (hwnd, 0);

	pszAchFile = ConfigInfo.szAchFile;

	switch (message)
    {
		case WM_CREATE:
			aux = (DISPCONTROLAUX *) my_malloc (sizeof (DISPCONTROLAUX));
			if (!aux)
				return (-1);
			memset (aux, '\0', sizeof (DISPCONTROLAUX));
			SetWindowLong (hwnd, 0, (LONG)aux);
			aux->owner = hwnd;

			/* Create an edit control */
			GetClientRect (hwnd, &rect);
			hWndEdit = CreateWindow	(	"edit",
										" ",
										WS_CHILD | WS_VISIBLE | DS_LOCALEDIT | WS_OVERLAPPED |
										WS_HSCROLL | WS_VSCROLL |
										ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_BORDER |
										ES_MULTILINE | ES_NOHIDESEL | ES_READONLY,
										rect.left,
										rect.top,
										rect.right,
										rect.bottom,
										hwnd,
										(HMENU) EDITMAGIC,
										hCurInst,
										0);
					
			/* if edit control failed, abort the function */
   			if (!IsWindow (hWndEdit))
			{
				return FALSE;
			}

			/* increase edit control max character limit beyond 30,000 default */
			SendMessage (hWndEdit, EM_LIMITTEXT, 0xFFFFFFFF, 0);

  			/* load filename passed at initialization */
			if ((nResult = LoadFileToEditCtrl (hWndEdit, pszAchFile)) > 0)
			{
				/* set window text title to be szAchFile */
				SetWindowText(hwnd, pszAchFile);
			}
			else
			{
				MessageBox(hwnd, "Can not load architecture file !", "Load Architecture", MB_OK);
				return FALSE;
			}
  
			SendMessage (hWndEdit, WM_SETFONT, (WPARAM) hStdFont, MAKELONG (FALSE, 0));
 			
			SetScrollRange (hWndEdit, SB_VERT, 0, (int)SCROLLRANGE, FALSE);
			SetScrollRange (hWndEdit, SB_HORZ, 0, (int)SCROLLRANGE, FALSE);
			
			aux->editor = hWndEdit;
			SetFocus (hwnd);
		break;
  
		case WM_SIZE:
			/* hwnd has already been sized, here we adjust the listbox */
			aux->rows = MAX (1, HIWORD (lParam) / wStdFontHt);  /* must be >= 1 line */
			
			// the last parameter in MoveWindow has to be true, otherwise, some part of 
			// of scroll bar will not re-draw
			MoveWindow (aux->editor, 0, 0,
						LOWORD (lParam), aux->rows * wStdFontHt, TRUE);

		break;

        
		/* adjust for new font */
		case UM_FONT_CHANGED:
			SendMessage (aux->editor, WM_SETFONT, (WPARAM) hStdFont, MAKELONG (FALSE, 0));
			InvalidateRect (hwnd, NULL, TRUE);
			GetClientRect (hwnd, &rect);
			SendMessage (hwnd, WM_SIZE, 0, MAKELONG (rect.right, rect.bottom));
        return 0;
        

		/* Windows kills the window and its children, here we kill our own stuff */
		case WM_DESTROY:
			my_free (aux);
		return 0;
	}

	return DefMDIChildProc (hwnd, message, wParam, lParam);
}  

/* IMPORTANT MDI NOTE:
   If cases are added for any of the following messages, they need to be
   passed on to the DefMDIChildProc() as well (in deference to the usual
   convention of returning zero) even if we handle them.  See the SDK
   Reference entry for DefMDIChildProc():

             WM_CHILDACTIVATE
             WM_GETMINMAXINFO
             WM_MENUCHAR
             WM_MOVE
             WM_NEXTMENU
             WM_SETFOCUS
             WM_SIZE
             WM_SYSCOMMAND
*/



long FAR PASCAL _export
ListDispControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

// Right Now, this function is only used to display cache, it is a simulator only
// function, to make it can be compiled OK with emulator project, we use #ifdef SIM
// make it empty function body for ICE

#ifdef SIM 
	RECT	rect;
	DISPCONTROLAUX *aux = (DISPCONTROLAUX *) GetWindowLong (hwnd, 0);
	char *	pszBuffer;

	char	szListTitle[]="Cache Contents";
 	HWND    hWndList;
	short	LoopCnt;
	char	szCacheNotEnableMsg[]="Cache is not enabled. ";
	short	nCacheEnableFlag = TRUE;

	switch (message)
    {
		case WM_CREATE:
			aux = (DISPCONTROLAUX *) my_malloc (sizeof (DISPCONTROLAUX));
			if (!aux)
				return (-1);
			memset (aux, '\0', sizeof (DISPCONTROLAUX));
			SetWindowLong (hwnd, 0, (LONG)aux);
			aux->owner = hwnd;

			/* Create an list control */
			GetClientRect (hwnd, &rect);
			hWndList = CreateWindow	(	"listbox",
										" ",
										WS_CHILD | WS_VISIBLE |
										WS_HSCROLL | WS_VSCROLL |WS_BORDER,
										rect.left,
										rect.top,
										rect.right,
										rect.bottom,
										hwnd,
										(HMENU) EDITMAGIC,
										hCurInst,
										0);
					
			/* if list control failed, abort the function */
   			if (!IsWindow (hWndList))
			{
				return FALSE;
			}
	
			// Take a snap of cache and send to the listbox
			nCacheEnableFlag = TakeCacheSnap(&cachesnap, &cache);
			if(nCacheEnableFlag)
			{
				for(LoopCnt = 0; LoopCnt < 2*CACH_SETS_PER_CACHE + 2; LoopCnt ++)
				{
					pszBuffer = (char *)& (cachesnap.szOneLineEntry[LoopCnt]);
					SendMessage (hWndList, LB_ADDSTRING, 0, (LPARAM) pszBuffer) ;
  				}
			}
			else
			{
				SendMessage (hWndList, LB_ADDSTRING, 0, (LPARAM) &szCacheNotEnableMsg) ;
			}
			
			SetWindowText(hwnd, szListTitle);

			SendMessage (hWndList, WM_SETFONT, (WPARAM) hStdFont, MAKELONG (FALSE, 0));
 			
			SetScrollRange (hWndList, SB_VERT, 0, (int)SCROLLRANGE, FALSE);
			SetScrollRange (hWndList, SB_HORZ, 0, (int)SCROLLRANGE, FALSE);

			aux->listbox = hWndList;

			SetFocus (hwnd);
		break;
  
		// MDI childwindow already re-sized, here, we resize the listbox 
		case WM_SIZE:

			aux->clean_slate = TRUE;
			aux->rows = MAX (1, HIWORD (lParam) / wStdFontHt);  /* must be >= 1 line */
  
			MoveWindow (aux->listbox, 0, 0,
						LOWORD (lParam), aux->rows * wStdFontHt, TRUE);

		break;

		/* handle updates from core...													  */
		/* Core call update_windows(), then update_windows() find all active childwindow  */
		/* and send UM_UPDATE message to them.                                            */
		/* Core call update_windows() at many different situation, like single step,      */
		/* N-step, or when receive timer message.                                         */
		/*																				  */
		/* Right now, We use a very brute approach to update cache contents, we reset all */
		/* contents of cache and reset all. In this case, speed will be a issue, because  */
		/* the program will send (2*CACH_SETS_PER_CACHE + 2)=34 message to the listbox,   */
		/* It may take too long, keep the eye open on this.                               */
		/*																				  */
		case UM_UPDATE:

			TakeCacheSnap(&cachesnap, &cache);
			SendMessage (aux->listbox, LB_RESETCONTENT, 0, 0) ;

			// Take a snap of cache and send to the listbox
			nCacheEnableFlag = TakeCacheSnap(&cachesnap, &cache);
			if(nCacheEnableFlag)
			{
				for(LoopCnt = 0; LoopCnt < 2*CACH_SETS_PER_CACHE + 2; LoopCnt ++)
				{
					pszBuffer = (char *)& (cachesnap.szOneLineEntry[LoopCnt]);
					SendMessage (aux->listbox, LB_ADDSTRING, 0, (LPARAM) pszBuffer) ;
  				}
			}
			else
			{
				SendMessage (aux->listbox, LB_ADDSTRING, 0, (LPARAM)&szCacheNotEnableMsg) ;
			}

		return 0;
        
		/* adjust for new font */
		case UM_FONT_CHANGED:
			SendMessage (aux->listbox, WM_SETFONT, (WPARAM) hStdFont, MAKELONG (FALSE, 0));
			InvalidateRect (hwnd, NULL, TRUE);
			GetClientRect (hwnd, &rect);
			SendMessage (hwnd, WM_SIZE, 0, MAKELONG (rect.right, rect.bottom));
        return 0;
        

		/* Windows kills the window and its children, here we kill our own stuff */
		case WM_DESTROY:
			my_free (aux);
        return 0;
    }

	return DefMDIChildProc (hwnd, message, wParam, lParam);

#endif
}   


// Function:	TakeCacheSnap
// Description: The function take a snap shot of current content in the cache
// Input:		CACH * pCache, pointer to cache data structure
// Output:		CACHESNAP * pCacheSnap, pointer to cache snap data structure, which
//				is already been format for displaying
// Global Var:	Not use any global variable
// return:		TRUE, cache enabled
//				FLASE, cache not enabled

// cache is a simulator only function
#ifdef SIM

short TakeCacheSnap(CACHESNAP * const pCacheSnap, const CACH * const pCache )
{
	short	set;
	short	entry;
    void	* pUNS48 = NULL;
	char	szTmpBuf[INSTRUCTIONLEN +1];
	unsigned long mode2;
	
	assert(pCacheSnap != NULL);
	assert(pCache != NULL);

	/* Be sure the cache is enabled */
	reg_get_effect (REG_TYPE_UNIV, REG_MODE2, &mode2);
	if (mode2 & REG_MODE2_CADIS) 
	{
		return (FALSE);
	}

	memset(szTmpBuf, 0, INSTRUCTIONLEN +1);
	
	sprintf(pCacheSnap->szOneLineEntry[0], CACHEONELINEDISPLAYFORMAT1, "SET", "LRU", "VALID", "ADDRESS", "OPCODE");
	pCacheSnap->szOneLineEntry[0][ONELINEFORMATCACHE] =0;

	sprintf(pCacheSnap->szOneLineEntry[1], CACHEONELINEDISPLAYFORMAT1, "---", "---", "-----", "-------",  "-----------");
	pCacheSnap->szOneLineEntry[1][ONELINEFORMATCACHE] =0;

	for (set = 0; set < CACH_SETS_PER_CACHE; set++) 
	{
		for (entry = 0; entry < CACH_ENTRIES_PER_SET; entry++) 
		{
			// Compiler complain about the pointer casting in the following line,
			// So I break it and use pUNS48
			// uns_to_hex_string((void *)pCache->set[set].opcode[entry], szTmpBuf, sizeof (UNS48));

			pUNS48 = (void *) &(pCache->set[set].opcode[entry]);
			uns_to_hex_string(pUNS48, szTmpBuf, sizeof (UNS48));

			// For the even and odd number of line, cache display use different format
			if (entry == 0)
			{
				sprintf(pCacheSnap->szOneLineEntry[set*2 + entry + 2], 
						CACHEONELINEDISPLAYFORMAT2, 
						set, 
						pCache->set[set].lru_bit, 
						pCache->set[set].valid_bit[entry], 
						pCache->set[set].address[entry], 
						szTmpBuf);
				pCacheSnap->szOneLineEntry[set*2 + entry + 2][ONELINEFORMATCACHE] =0;
			} 
				
			else
			{
				sprintf(pCacheSnap->szOneLineEntry[set*2 + entry + 2], 
						CACHEONELINEDISPLAYFORMAT3, 
						pCache->set[set].valid_bit[entry], 
						pCache->set[set].address[entry], 
						szTmpBuf);
				pCacheSnap->szOneLineEntry[set*2 + entry + 2][ONELINEFORMATCACHE] =0;
			}
		}
	}

	return TRUE;
}
#endif


