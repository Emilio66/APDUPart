#ifndef APDU_H
#define APDU_H
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

//common APDU response
struct RESPONSE{
	BYTE * DATA;
	BYTE SW1,
		 SW2;
};

////////////////////////////////////
/// GLOBLE VARIABLES DECLARATION //
//////////////////////////////////

extern SCARDCONTEXT scardContext;
extern SCARDHANDLE	 scardHandle;
extern SCARD_IO_REQUEST ioRequest;
extern WCHAR * readerName;
extern DWORD	activeProtocal;
extern DWORD	scardStatus;
extern DWORD UNKNOWN;	//DON'T ASSIGN ANY VALUE
extern APDU GET_RESPONSE;	//COMMON GET RESPONSE APDU


////////////////////////////////
/// APDU FUNCTIONS DEFINITION //
///////////////////////////////

LONG establishContext();
LONG cardConnect();
LONG cardReconnect();
LONG process(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len);
LONG write(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len);
void startTransaction();
void endTransaction();
DWORD getStatus();

#endif