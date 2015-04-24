#include <iostream>
#include <WinSCard.h>
#include <string>
#include "functions.h"

#pragma comment(lib,"winscard.lib")

using namespace std;

/*
*
1. Get the reader
2. Connect to the smart card
3. Get the APDU
4. Write the APDU
5. Get the Response
*/

class SConnector{
public :
	string reader;
	APDU cmd;
	APDU ret;

	SConnector();
	~SConnector();
	
	void connect();
	void read();
	void write();
	void release();
};

#define SIZE 24

void main(){
	BYTE DATA[SIZE] = { 0x22, 0x23, 0x34, 0x45, 0x22, 0x23, 0x34, 0x45, 0x22, 0x23, 0x34, 0x45, 0x22, 0x23, 0x34, 0x45, 
		0x22, 0x23, 0x34, 0x45, 0x22, 0x23, 0x34, 0x45
	};
	BYTE RETURN[SIZE+2] = { 0 };
	DWORD retLen = SIZE+2;
	LONG retCode;
	/* retCode= CRC_CALCULATE(DATA, SIZE, RETURN, &retLen);
	if (SCARD_S_SUCCESS == retCode){
		cout << "DATA: ";
		for (int i = 0; i < retLen; i++){
			printf("0x%02X ", RETURN[i]);
		}
		cout << endl;
	}
	else{
		printf("Error Code 0X%X \n", retCode);
	}

	BYTE * sum = (BYTE *)malloc(SIZE+4);
	for (int i = 0; i < 4; i++)
		sum[i] = RETURN[i];
	memcpy(&sum[4], DATA, SIZE);

	retCode = CRC_CHECK(sum, SIZE+4, RETURN, &retLen);
	if (SCARD_S_SUCCESS == retCode){
		cout << "CHECK result: ";
		for (int i = 0; i < retLen; i++){
			printf("0x%02X ", RETURN[i]);
		}
		cout << endl;
	}
	else{
		printf("Error Code 0X%X \n", retCode);
	}
	free(sum);*/

	//check input 3 des keys
	retCode = DES_KEYS_IN(DATA, SIZE, RETURN, &retLen);
	if (SCARD_S_SUCCESS == retCode){
		cout << "RETURN CODE: ";
		for (int i = 0; i < retLen; i++){
			printf("0x%02X ", RETURN[i]);
		}
		cout << endl;
	}
	else{
		printf("Error Code 0X%X \n", retCode);
	}

	retLen = SIZE + 2;	// reassign
	//ENCRYPT & DECRYPT
	retCode = DES_ENCRYPT(DATA, SIZE, RETURN, &retLen);
	retCode = DES_DECRYPT(RETURN, SIZE, RETURN, &retLen);
	if (SCARD_S_SUCCESS == retCode){
		cout << "DEcrypted CODE: ";
		for (int i = 0; i < retLen; i++){
			printf("0x%02X ", RETURN[i]);
		}
		cout << endl;
	}
	else{
		printf("Error Code 0X%X \n", retCode);
	}

}