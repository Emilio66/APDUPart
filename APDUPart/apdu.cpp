
#include "apdu.h"
#include<iostream>

using namespace std;

//////////////////////////////////
/// GLOBLE VARIABLES DEFINITION //
/////////////////////////////////
SCARDCONTEXT scardContext;
SCARDHANDLE	 scardHandle;
SCARD_IO_REQUEST ioRequest;
WCHAR * readerName;
DWORD	activeProtocal;
DWORD	scardStatus;
DWORD UNKNOWN;	//DON'T ASSIGN ANY VALUE
APDU GET_RESPONSE;	//COMMON GET RESPONSE APDU

void init(){
	if (GET_RESPONSE.INS != 0xc0){
		GET_RESPONSE.CLA = 0x00;
		GET_RESPONSE.INS = 0xc0;
		GET_RESPONSE.P1 = 0x00;
		GET_RESPONSE.P2 = 0x00;
		GET_RESPONSE.LC = 0x02;	//length of expected DATA
	}
	ioRequest.dwProtocol = activeProtocal;
	ioRequest.cbPciLength = sizeof(SCARD_IO_REQUEST);
}

//establish the context before connecting to the smart card
LONG establishContext(){
	if (scardStatus == SCARD_SPECIFIC || scardContext != UNKNOWN)
		return SCARD_S_SUCCESS;
	return SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &scardContext); //establish context the first time
}

//connect to the card
LONG cardConnect(){
	LONG retCode = establishContext();
	if (SCARD_S_SUCCESS == retCode){
		//choose the first reader
		WCHAR name[MAX_PATH];
		DWORD length;
		retCode = SCardListReaders(scardContext, NULL, name, &length);
		if (SCARD_S_SUCCESS == retCode){
			readerName = new WCHAR[length];
			for (int i = 0; (readerName[i++] = name[i]) != '\0';);	//assign the reader name

			//establish the connection between reader and card
			retCode = SCardConnect(scardContext, readerName, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
				&scardHandle,&activeProtocal);

			init();	
		}
	}

	return retCode;
}

//reconnect the card in order to clear errors or acknowlege
LONG cardReconnect(){
	return  SCardReconnect(scardHandle, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		SCARD_LEAVE_CARD, &activeProtocal);
}

//transmit data to smart card and bring back response as well
LONG process(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len){
	LONG retCode;
	if (scardHandle == UNKNOWN){
		retCode = cardConnect();
	}

	retCode = SCardTransmit(scardHandle, &ioRequest, data, length, NULL, receiver, len);
	return retCode;
}

//get the smart card's status, normally return SCARD_SPECIFIC
DWORD getStatus(){
	if (scardStatus == UNKNOWN){
		if (scardHandle == UNKNOWN && SCARD_S_SUCCESS != cardConnect())
			return UNKNOWN;

		BYTE            bAttr[32];
		DWORD           cByte = 32;
		DWORD           dwProtocol;
		SCardStatus(scardHandle, readerName, NULL, &scardStatus, &dwProtocol, bAttr, &cByte);	
	}

	return  scardStatus;
}

//issue a transaction, nothing can interrupt the process 
void startTransaction(){
	SCardBeginTransaction(scardHandle);
}

//end a transaction but keep the connection alive
void endTransaction(){
	SCardEndTransaction(scardHandle, SCARD_LEAVE_CARD);
}
