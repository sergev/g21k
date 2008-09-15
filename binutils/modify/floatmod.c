#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#ifndef FALSE 
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

short	IEEEfloatModifyForMSC(char * pszFloatStr, short nMaxStrLen);


// 
// Function:	IEEEfloatModifyForMSC(char * pszFloatStr, short nMaxStrLen)
// input:		char *	pszFloatStr
//				short	nMaxStrLen
// output		char *	pszFloatStr
// return:		TRUE,	input string changed
//				FALSe,	input string not changed
// Author:		BL
//	

// VC++ and Borland C++ printf() print IEEE float number differenet
// For example:
//		double dRes = pow(2.0, 127.0); 
//		printf("dRes [% .8e]", dRes); 
//
//		Borland C++ output,	dRes [ 1.70141183e+38] 
//		VC++ output,		dRes [ 1.70141183e+038] 
// Same thing for simple number:
//		printf("dRes [% .8e]", 2.0);
//		Borland C++ output,	dRes [ 2.00000000e+00] 
//		VC++ output,		dRes [ 2.00000000e+000] 
//
// I believe Borland C++ output same as UNIX SUN output.
//
// In 21060 original simulator, which was ported to Borland C++ first. 
// Coder hardcoded many windows size to match string length of Borland
// C++. The extra '0' in VC++ cause array overflow and many subsequent
// result. The following simple function is used to get rid of
// extra '0' from VC++ output and reduce string length by one byte. 
//
// The function is coded in a very defensive way. Unless absolutely necessay,
// We do not want to change the input string, that is why there is some
// check point in the function.
  
short	IEEEfloatModifyForMSC(char * pszFloatStr, short nMaxStrLen)
{
// Only for Microsoft Compiler
#ifdef _MSC_VER

	short	nExtraZeroPos;
	short	i;
	char *	pszSubStr = NULL;
	char	szToFind1[]="e+0";
	char	szToFind2[]="e-0";
	short	nStrLen;
	
	assert(pszFloatStr != NULL);
	assert(nMaxStrLen >0);

	// If input string len is less than nMaxStrLen, do not change the
	// the input string, return
	nStrLen = strlen(pszFloatStr);
	if(nStrLen <= nMaxStrLen)	
	{
		return FALSE;
	}

	// find "e+0" or "e-0" in string, if not find "e+0",
	// try "e-0"
	pszSubStr = strstr(pszFloatStr, szToFind1);
	if(pszSubStr == NULL)
 	{
		pszSubStr = strstr(pszFloatStr, szToFind2);
	}

	//	if none of "e+0" and"e-0" in the input string,
	//	do not change the input string and return
	//  There are still some risk here, because string
	//	length >= nMaxStrLen. It will very unlike to
	//  happen. For 32bit IEEE float 2^255=5.78....e+76
	//  I do not think We need three digit for exponents.
	//  It may happen in 64bit, but that will be a different
	//  story, and We will take different approach then.
	if(pszSubStr == NULL)
	{	
		return FALSE;
	}

	else
	{
		// find the offset position in the input string for
		// the extra "0", which is the "0" in "e+0" or "e-0"
		nExtraZeroPos = pszSubStr - pszFloatStr + 2;

		// It must be '0', and two character before it must be 'e'		
		assert(pszFloatStr[nExtraZeroPos]=='0');
		assert(pszFloatStr[nExtraZeroPos - 2] =='e');

		i = 1;
		while(pszFloatStr[nExtraZeroPos +i])
		{
			pszFloatStr[nExtraZeroPos +i-1] = pszFloatStr[nExtraZeroPos + i];
			i++;
		}

		// terminate the string.
		pszFloatStr[nExtraZeroPos +i -1] =0;
		return TRUE;
	}

#else
	pszFloatStr;
	nMaxStrLen;	
	return FALSE;
#endif
}


#ifdef	TESTFLOATMODIFY

// The following was extracted from flt_pt.c
#define SET_NAN(x)           \
   *((char *)x + 7) = 0x7f;  \
   *((char *)x + 6) = 0xf8

#define SET_POS_INF(x)       \
   *((char *)x + 7) = 0x7f;  \
   *((char *)x + 6) = 0xf0;  \
   *((char *)x + 5) = 0x00;  \
   *((char *)x + 4) = 0x00;  \
   *((char *)x + 3) = 0x00;  \
   *((char *)x + 2) = 0x00;  \
   *((char *)x + 1) = 0x00;  \
   *((char *)x + 0) = 0x00

#define SET_NEG_INF(x)       \
   *((char *)x + 7) = 0xff;  \
   *((char *)x + 6) = 0xf0;  \
   *((char *)x + 5) = 0x00;  \
   *((char *)x + 4) = 0x00;  \
   *((char *)x + 3) = 0x00;  \
   *((char *)x + 2) = 0x00;  \
   *((char *)x + 1) = 0x00;  \
   *((char *)x + 0) = 0x00

#define SET_DENORM(x)        \
   *((char *)x + 7) = 0xff;  \
   *((char *)x + 6) = 0xf8;  \
   *((char *)x + 5) = 0x00;  \
   *((char *)x + 4) = 0x00;  \
   *((char *)x + 3) = 0x00;  \
   *((char *)x + 2) = 0x00;  \
   *((char *)x + 1) = 0x00;  \
   *((char *)x + 0) = 0x00


int main(void)
{
	double dRes;
	double dSimple1 = 2.0;
	double dSimple2 = 0.0;
	double dTest;

	char szTmp[80];

	// First test
	dRes = pow(2.0, 127.0);
	sprintf(szTmp, "% .8e", dRes);
	printf("szTmp %s \r\n", szTmp);
#ifdef _MSC_VER
	IEEEfloatModifyForMSC(szTmp, 14);
#endif
	printf("szTmp %s \r\n", szTmp);

	// Second test
	sprintf(szTmp, "% .8e", dSimple1);
	printf("szTmp %s \r\n", szTmp);
#ifdef _MSC_VER
	IEEEfloatModifyForMSC(szTmp, 14);
#endif
	printf("szTmp %s \r\n", szTmp);

	// Third test
	sprintf(szTmp, "% .8e", dSimple2);
	printf("szTmp %s \r\n", szTmp);
#ifdef _MSC_VER
	IEEEfloatModifyForMSC(szTmp, 14);
#endif
	printf("szTmp %s \r\n", szTmp);

	// Fourth test
	sprintf(szTmp, " %s", "1.2345678901234");
	printf("szTmp %s \r\n", szTmp);
#ifdef _MSC_VER
	IEEEfloatModifyForMSC(szTmp, 14);
#endif
	printf("szTmp %s \r\n", szTmp);

	// Test of NAN, INF, DENORM. Behavior are different from 
	// BC++ and VC++
	SET_NAN(&dTest);
	printf("%8s ",	"NAN");       printf("% .8e", dTest); printf("\r\n");

	SET_POS_INF(&dTest);    
	printf("%8s ",	"POS_INF");   printf("% .8e", dTest); printf("\r\n");

	SET_NEG_INF(&dTest);
	printf("%8s ",	"NEG_INF");   printf("% .8e", dTest); printf("\r\n");

	SET_DENORM(&dTest);
	printf("%8s ",	"DENORM");    printf("% .8e", dTest); printf("\r\n");

/*	Test of NAN, INF, DENORM result:	
//Output from VC++
     NAN  1.#QNAN000e+000
 POS_INF  1.#INF0000e+000
 NEG_INF -1.#INF0000e+000
  DENORM -1.#IND0000e+000


//Output from BC++
     NAN  +NAN
 POS_INF  +INF
 NEG_INF -INF
  DENORM -NAN
*/

	return 0;
}

#endif	// TESTFLOATMODIFY
