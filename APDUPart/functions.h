#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "apdu.h"
#include "common.h"

/////////////////////////
// external reference //
extern SCARDCONTEXT scardContext;
extern SCARDHANDLE	 scardHandle;
extern SCARD_IO_REQUEST ioRequest;
extern WCHAR * readerName;
extern DWORD	activeProtocal;
extern DWORD	scardStatus;
extern DWORD UNKNOWN;	//DON'T ASSIGN ANY VALUE
extern APDU GET_RESPONSE;	//COMMON GET RESPONSE APDU

LONG CRC_CALCULATE(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len);
LONG CRC_CHECK(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len);

LONG DES_ENCRYPT(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len);
LONG DES_DECRYPT(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len);

#endif