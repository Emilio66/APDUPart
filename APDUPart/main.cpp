#include <iostream>
#include <WinSCard.h>
#include <string>

#pragma comment(lib,"winscard.lib")

using namespace std;

/*
*
1. Get the reader
2. Get the APDU
3. Connect to the smart card
4. Write the APDU
5. Get Response
*/

struct APDU{
	unsigned char CLA;
	unsigned char INS;
	unsigned char P1;
	unsigned char P2;
	unsigned char LENGTH;
	unsigned char * DATA;
};

struct RESPONSE{
	unsigned char SW1;
	unsigned char SW2;
	unsigned char * DATA;
};

class SConnector{
public :
	string reader;
	APDU * cmd;
	APDU * ret;

	SConnector();
	~SConnector();
	
	void connect();
	void read();
	void write();
	void release();
};

void main(){


}