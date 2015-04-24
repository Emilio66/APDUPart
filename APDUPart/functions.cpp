#include "functions.h"
//just for test
#include<iostream>
using namespace std;


//ONE CYCLE CALCULATION, data length less than 128 Bytes
LONG CRC_CALCULATE_ONECE(APDU* CRC_APDU, LPCBYTE data, LPBYTE receiver, LPDWORD len){
	int wholeLen = CRC_APDU->LC + 5;
	BYTE RETURN[2] = { 0 };
	DWORD retLen = 2;
	BYTE* dataStream = (BYTE*)malloc(wholeLen);
	memcpy(dataStream, CRC_APDU, 5);
	memcpy(&dataStream[5], data, wholeLen - 5);	//assemble APDU struct with DATA

	LONG retCode;
	retCode = process(dataStream, wholeLen, RETURN, &retLen);
	if (SCARD_S_SUCCESS == retCode){
		//get response
		GET_RESPONSE.LC = 0x02;
		retCode = process((LPCBYTE)&GET_RESPONSE, 5, receiver, len);
	}

	free(dataStream);

	return retCode;
}


//calculate CRC of data, NOTE that only less than 128 bytes every cycle
LONG CRC_CALCULATE(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len){
	APDU CRC_APDU = {0};
	LONG retCode;
	CRC_APDU.CLA = 0x00;
	CRC_APDU.INS = 0x04;
	CRC_APDU.P1 = 0x00;		//16 bit mode, apply to 0-128 bytes' data
	CRC_APDU.P2 = 0x00;

	//cout << "data to be calculated length: " << length << endl;
	
	//APDU allowS 1-256 bytes, but in order to check CRC, 252 bytes is the maximum !!
	if (length > 252)
		return SCARD_E_NO_MEMORY;	
	
	//when data length > 128, divide them into 2 groups
	if (length < 129){
		CRC_APDU.LC = length;	

		startTransaction();
		retCode = CRC_CALCULATE_ONECE(&CRC_APDU, data, receiver, len);
		endTransaction();
	}
	else{
		DWORD len1 = 4;	//length of data received
		DWORD len2 = length - 128;
		BYTE receiver1[4];
		BYTE receiver2[4];
		CRC_APDU.LC = 128;

		startTransaction();
		retCode = CRC_CALCULATE_ONECE(&CRC_APDU, data, receiver1, &len1);
		if (SCARD_S_SUCCESS == retCode){
			//second part, length of the reminder
			CRC_APDU.LC = len2;
			retCode = CRC_CALCULATE_ONECE(&CRC_APDU, &data[128], receiver2, &len1);
			if (SCARD_S_SUCCESS == retCode){
				//return real data, 4 bytes for calculated data, 2 bytes indicate sw1, sw2
				*len = 6;
				receiver[0] = receiver1[0];
				receiver[1] = receiver1[1];
				receiver[2] = receiver2[0];
				receiver[3] = receiver2[1];
				receiver[4] = receiver2[2];
				receiver[5] = receiver2[3];
			}
		}
		endTransaction();
	}

	return retCode;
}

LONG CRC_CHECK_ONECE(APDU * apdu, LPCBYTE data, LPBYTE receiver, LPDWORD len){
	//cout << "data to be checked length: " << (int)apdu->LC << endl;
	//printf("0x%02X 0x%02X \n", data[0], data[1]);

	int wholeLength = apdu->LC + 5;
	LONG retCode; 
	BYTE* stream = (BYTE *)malloc(wholeLength);
	memcpy(stream, apdu, 5);
	memcpy(&stream[5], data, apdu->LC);	//copy data

	retCode = process(stream, wholeLength, receiver, len); //no response, only sw1 & sw2
	
	free(stream);
	return retCode;
}

//crc check, <=256 bytes
LONG CRC_CHECK(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len){
	APDU CRC_CHK_APDU;
	CRC_CHK_APDU.CLA = 0X00;
	CRC_CHK_APDU.INS = 0x04;
	CRC_CHK_APDU.P1 = 0x02;	//CRC check code	
	CRC_CHK_APDU.P2 = 0x00;
	
	LONG retCode;
	//only one group of check sum
	if (length < 131){
		CRC_CHK_APDU.LC = length;
		
		retCode = CRC_CHECK_ONECE(&CRC_CHK_APDU, data, receiver, len);
	}
	else{
		//first 2 bytes stands for the check sum of first group data, 3-4th bytes the second group
		DWORD le = 2;
		DWORD len2 = length - 130;
		BYTE * group1 = (BYTE *)malloc(130);
		BYTE * group2 = (BYTE *)malloc(len2);
		BYTE receiver1[2] = { 0 };
		BYTE receiver2[2] = { 0 };

		group1[0] = data[0];
		group1[1] = data[1];
		group2[0] = data[2];
		group2[1] = data[3];
		memcpy(&group1[2], &data[4], 128);			//assemble data for the first group
		memcpy(&group2[2], &data[132], len2 - 2);   //the second group
		CRC_CHK_APDU.LC = 130;						//first group (check sum + data) length

		startTransaction();
		retCode = CRC_CHECK_ONECE(&CRC_CHK_APDU, group1, receiver1, &le);
		if (SCARD_S_SUCCESS == retCode){
			CRC_CHK_APDU.LC = len2;
			retCode = CRC_CHECK_ONECE(&CRC_CHK_APDU, group2, receiver2, &le);
			if (SCARD_S_SUCCESS == retCode){
				//whether return the right code 0X9000 or the wrong code 0X9001
				*len = 2;
				receiver[0] = 0x90;
				receiver[1] = receiver1[1] | receiver2[1];	//if 01 occur, check failed
			/*	cout << " 2 group check result : " << endl;
				for (int i = 0; i < 2; i++)
					printf("0x%02x ", receiver1[i]);
				for (int i = 0; i < 2; i++)
					printf("0x%02x ", receiver2[i]);
				cout << endl;*/
			}
		}
		endTransaction();

		free(group1);
		free(group2);
	}
	return retCode;
}

//input 3 keys needed in 3DES at the first time
LONG DES_KEYS_IN(LPCBYTE keys, DWORD length, LPBYTE result, LPDWORD len){
	APDU KEYS;
	KEYS.CLA = 0X00;
	KEYS.INS = 0X03;
	KEYS.P1 = 0X00;
	KEYS.P2 = 0X07;	//INPUT 3 KEY, 0111
	KEYS.LC = 0X18; //8 bytes each

	if (length != 24)
		return SCARD_E_INVALID_PARAMETER;

	BYTE * data = (BYTE *)malloc(29);	//apdu + keys = 5 + 24 = 29
	memcpy(data, &KEYS, 5);
	memcpy(&data[5], keys, 24);

	LONG retCode = process(data, 29, result, len);
	free(data);

	return retCode;
}

//3DES encrytion, ecb mode avoiding the input of IV 
LONG DES_OPERATE(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len, bool isEncrypt){
	LONG retCode;
	APDU DES_APDU;
	DES_APDU.CLA = 0X00;
	DES_APDU.INS = 0X03;
	DES_APDU.P1 = 0X01;
	DES_APDU.P2 = isEncrypt ? 0X0A : 0X0B; //differ in a bit 

	//data length should be a multiple of 8, i.e. x % 8 == 0
	if (length & 0x07)
		return SCARD_E_INVALID_PARAMETER;

	DES_APDU.LC = length;
	BYTE * package = (BYTE*)malloc(length + 5);
	BYTE recev[2] = { 0 };
	DWORD len2 = 2;
	memcpy(package, &DES_APDU, 5);
	memcpy(&package[5], data, length);

	startTransaction();
	retCode = process(package, length + 5, recev, &len2);
	
	//after return the right code about encrytion
	if (SCARD_S_SUCCESS == retCode && recev[1] == length){
		GET_RESPONSE.LC = length; //encrypted data's length equals the original
		retCode = process((LPCBYTE)&GET_RESPONSE, 5, receiver, len);
	}
	else{
		//write the error code from card
		*len = len2;
		receiver[0] = recev[0];
		receiver[1] = recev[1];
	}
	endTransaction();

	free(package);

	return retCode;
}

LONG DES_ENCRYPT(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len){
	return DES_OPERATE(data, length, receiver, len, true);
}
LONG DES_DECRYPT(LPCBYTE data, DWORD length, LPBYTE receiver, LPDWORD len){
	return DES_OPERATE(data, length, receiver, len, false);
}