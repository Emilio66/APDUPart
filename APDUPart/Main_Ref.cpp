#include<iostream>
#include<winscard.h>

#pragma comment(lib,"winscard.lib")

using namespace std;

//normal APDU with data, if length == 256 , LC = 0
struct APDU{
	BYTE CLA,
		INS,
		P1,
		P2,
		LC;
};

struct APDU_N{
	BYTE CLA,
		INS,
		P1,
		P2;
};


void main_ref(){
	SCARDCONTEXT context;
	LONG Lreturn;

	Lreturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &context);

	if (SCARD_S_SUCCESS != Lreturn){
		cout << "Fail to establish connection" << endl;
		return;
	}
	
	WCHAR names[MAX_PATH];	//16bit char, use carefully
	DWORD len;
	Lreturn = SCardListReaders(context, NULL, names, &len);

	////////////////////////////////////
	//////**** Get Readers *****///////
	if (SCARD_S_SUCCESS == Lreturn){
		//only one reader
		WCHAR * content = new WCHAR[len];

		for (int i = 0; (content[i++] = names[i]) != '\0';)cout << (char)content[i - 1]; cout << endl;
		
		/*char * readers[10];
		char tmp[256];
		int cnt = 0;
		//unicode occupies 2 bytes
		for (int i = 0,j=0; i < len * 2-2; i += 2){
			//assign value to temporary buffer until the end of string
			if ((tmp[j++] = names[i]) == '\0'){
				readers[cnt] = (char *)malloc(j - 1);
				strcpy(readers[cnt], tmp);	//copy data to real container
				cnt++;	//increase count
				j = 0;	//initialize again
				memset(tmp, 1, 256);
			}	
		}

		char ** container = (char **)malloc(sizeof(char*)*cnt);
		for (int i = 0; i < cnt; i++){
			container[i] = readers[i];
			cout <<"Reader "<<i<<" : "<< container[i]<<endl;
		}*/

		///////////////////////////
		/// Connect Smart Card ///
		SCARDHANDLE cardHandle;
		DWORD acceptProtocals =0L;
		/*int len2 = sizeof(*container[0]);	//test the string that represents the reader
		WCHAR *wName = new WCHAR[len2];
		int len1 = MultiByteToWideChar(CP_ACP, 0, container[0], len2, NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, container[0], len2, wName, len1);*/

		//connect smart card with shared mode
		Lreturn = SCardConnect(context, content, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
			&cardHandle, &acceptProtocals);
		if (SCARD_S_SUCCESS == Lreturn){
			printf("Sucessfully connected to the card: 0X%X \n",cardHandle);
		
			////////////////////
			//// Send Data	///
			
			//only protocal T0 is active
			SCARD_IO_REQUEST request,response;
			request.dwProtocol	= acceptProtocals;
			request.cbPciLength = sizeof(SCARD_IO_REQUEST);	//essential, indicate the whole length of PCI, otherwise err
			response.cbPciLength= sizeof(SCARD_IO_REQUEST);
			response.dwProtocol = acceptProtocals;
			
			BYTE DATA[8] = { 0x22, 0x23, 0x34, 0x45, 0x22, 0x23, 0x34, 0x45 };
			APDU CRC_APDU;
			CRC_APDU.CLA =	0x00;
			CRC_APDU.INS =	0x04;
			CRC_APDU.P1	=	0x00;
			CRC_APDU.P2 =	0x00;
			CRC_APDU.LC = sizeof(DATA);
			

			int length = CRC_APDU.LC + 5;
			BYTE RETURN[4] = { 0};
			DWORD retLen = 4;
			BYTE* dataStream = (BYTE*)malloc(length);
			memcpy(dataStream, &CRC_APDU, 5);
			memcpy(&dataStream[5], DATA, length - 5);	//assemble APDU struct with DATA

			//////start transaction
			SCardBeginTransaction(cardHandle);
			Lreturn = SCardTransmit(cardHandle, &request, dataStream, length, &response, RETURN, &retLen);
			/*LPBYTE attr = NULL;
			DWORD    cByte = SCARD_AUTOALLOCATE;
			DWORD L = 2;
			BYTE VAL[2] = { 0XFF, 0X33 };
			SCardSetAttrib(cardHandle, SCARD_ATTR_CURRENT_W, VAL, L);
			Lreturn = SCardGetAttrib(cardHandle, SCARD_ATTR_CURRENT_W, (LPBYTE)&attr, &cByte);

			if (SCARD_S_SUCCESS != Lreturn)
			{
				if (ERROR_NOT_SUPPORTED == Lreturn)
					printf("Value not supported\n");
				else
				{
					// Some other error occurred.
					printf("Failed SCardGetAttrib - %x\n", Lreturn);
					exit(1);  // Or other appropriate action
				}
			}
			else
			{
				// Output the bytes.
				for (int i = 0; i < cByte; i++)
					printf("%c", *(attr + i));
				printf("\n");

				// Free the memory when done.
				// hContext was set earlier by SCardEstablishContext
				Lreturn = SCardFreeMemory(context, attr);
			}

			/////////////////////
			/// Receive Data ///
			printf("Return %d bytes: ", retLen);
			if (SCARD_S_SUCCESS == Lreturn)
				for (int i = 0; i < cByte; i++){
					printf("%c ", *(attr + i));
				}*/
			free(dataStream);
			if (SCARD_S_SUCCESS == Lreturn){
				cout << "Return APDU: ";
				for (int i = 0; i < retLen; i++){
					printf("%02X ", RETURN[i]);
				}
			}			
			else{
				printf("Go to hell Error Code 0X%X \n", Lreturn);
			}

			APDU GET_RESPONSE;
			GET_RESPONSE.CLA = 0x00;
			GET_RESPONSE.INS = 0xc0;
			GET_RESPONSE.P1 = 0x00;
			GET_RESPONSE.P2 = 0x00;
			GET_RESPONSE.LC = 0x02;	//length of expected DATA

			//length = GET_RESPONSE.LC + 5;
			//BYTE * chunk = (BYTE*)malloc(length);
			//memcpy(chunk, &GET_RESPONSE, 5);
			//memset(&chunk[5], 0, GET_RESPONSE.LC);	//fill in the fake data with 0

			BYTE retData[4] = {0};
			retLen = 4;		//length expected
			
			///////////////////////////
			//// GET reponse Data ////
			Lreturn = SCardTransmit(cardHandle, &request, (LPCBYTE)&GET_RESPONSE, 5, &response, retData, &retLen);
			//Lreturn = SCardControl(cardHandle, SCARD_CTL_CODE(3500), NULL, 0, RETURN, 4, &retLen);
			
			cout << endl;
			if (SCARD_S_SUCCESS == Lreturn){
				cout << "DATA: ";
				for (int i = 0; i < retLen; i++){
					printf("0x%02X ", retData[i]);
				}
			}
			else{
				printf("Error Code 0X%X \n", Lreturn);
			}
			cout << endl;

			//////////////////////
			/// verify data /////
			CRC_APDU.P1 = 0x02;			//instruction parameter changes	
			CRC_APDU.LC += retLen - 2; //plus check data
			length = CRC_APDU.LC + 5; //real send buffer size
			
			BYTE* sendBuffer = (BYTE *)malloc(length);
			memcpy(sendBuffer, &CRC_APDU, 5);	//assemble data
			memcpy(&sendBuffer[5], retData, retLen - 2);	//check code
			memcpy(&sendBuffer[7], DATA, 8);

			printf("We gonna check CRC code %02X %02X ", retData[0], retData[1]);
			Lreturn = SCardTransmit(cardHandle, &request, sendBuffer, length, &response, retData, &retLen);
			cout <<"Generated from DATA: "<< endl;
			for (int i = 0; i < CRC_APDU.LC; i++)
				printf(" %02X ", sendBuffer[i]);
			free(sendBuffer);
			cout << endl;
			if (SCARD_S_SUCCESS == Lreturn){
				for (int i = 0; i < retLen; i++){
					printf("0x%02X ", retData[i]);
				}
			}
			else{
				printf("Error Code 0X%X \n", Lreturn);
			}
			cout << endl;

			///End Transaction
			SCardEndTransaction(cardHandle, SCARD_LEAVE_CARD);
			//SCardDisconnect(cardHandle, SCARD_LEAVE_CARD);
			
			//Reestablishes an existing connection between the calling application and a smart card. 
			//This function moves a card handle from direct access to general access, or acknowledges
			//and clears an error condition that is preventing further access to the card
			Lreturn = SCardReconnect(cardHandle, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
				SCARD_LEAVE_CARD, &acceptProtocals);
			if (SCARD_S_SUCCESS != Lreturn){
				printf("Can't reconnect.\tError Code: 0X%02X \n",Lreturn );
			}

			//////////////////////////
			////Card Status Check ////
			WCHAR           szReader[200];
			DWORD           cch = 200;
			BYTE            bAttr[32];
			DWORD           cByte = 32;
			DWORD           dwState, dwProtocol;
			LONG            lReturn;

			lReturn = SCardStatus(cardHandle,
				szReader,
				&cch,
				&dwState,
				&dwProtocol,
				(LPBYTE)&bAttr,
				&cByte);

			if (SCARD_S_SUCCESS != lReturn)
			{
				printf("Failed SCardStatus\n");
				exit(1);     // or other appropriate action
			}

			// Examine retrieved status elements.
			// Look at the reader name and card state.
			printf("%S\n", szReader);
			switch (dwState)
			{
			case SCARD_ABSENT:
				printf("Card absent.\n");
				break;
			case SCARD_PRESENT:
				printf("Card present.\n");
				break;
			case SCARD_SWALLOWED:
				printf("Card swallowed, not powered.\n");
				break;
			case SCARD_POWERED:
				printf("Power is being provided to the card, but the reader driver is unaware of the mode of the card..\n");
				break;
			case SCARD_NEGOTIABLE:
				printf("Card reset and waiting PTS negotiation.\n");
				break;
			case SCARD_SPECIFIC:
				printf("Card has specific communication protocols set.\n");
				break;
			default:
				printf("Unknown or unexpected card state.\n");
				break;
			}
		}
		else
		{
			//cout << "Error code: " << Lreturn << endl;
			printf("Error Code 0X%X \n", Lreturn);
		}
	}
	else{
		cout << "Not reader found! " << endl;
	}
}