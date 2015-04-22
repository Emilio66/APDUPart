#pragma once
#include<winscard.h>

#pragma comment(lib,"winscard.lib")

/////////////////////////////////////////
//// CONSTANT APDU DEFINITION //////////
///////////////////////////////////////

//normal APDU with data, if length == 256 , LC = 0
struct APDU{
	BYTE CLA,
		INS,
		P1,
		P2,
		LC;
};

//simple APDU without data
struct APDU_N{
	BYTE CLA,
		INS,
		P1,
		P2;
};


//////////////////////////////////
/// GLOBLE VARIABLES DEFINITION //
/////////////////////////////////

SCARDCONTEXT scardContext;
SCARDHANDLE	 scardHandle;
SCARD_IO_REQUEST ioRequest;
WCHAR * readerName;
DWORD	activeProtocal;
DWORD	cardStatus;




////////////////////////////////
/// APDU FUNCTIONS DEFINITION //
///////////////////////////////

LONG establishContext();
LONG cardConnect();
LONG process(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len);
LONG write(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len);
DWORD getStatus();

