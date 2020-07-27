#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H


#include "TCP.h"
#include "UDP.h"
#include "Menu.h"


// IDs for the buttons
#define BTN_ADDFILE			500
#define BTN_TRANSFER		600

#define RADIO_CLIENT		700
#define RADIO_SERVER		701
#define RADIO_TCP			702
#define RADIO_UDP			703


//Error codes
#define	CLIENT_ABORTED		100
#define SERVER_NOT_FOUND	101
#define SERVER_COMPLETED	102
#define CLIENT_COMPLETED	103

#define FILESIZE			1000
#define LABELSBUFF			1024

struct Data {
	HWND hwnd;
	char* ipPortInput;
	char* applicationStatus;
	int packetSizeInput;
	int numOfTimesInput;
	bool isServer;
	bool isClient;
	bool isTCP;
	bool isUDP;
	bool fileUploaded;
	HWND labels;
	bool ipObtained;
};

struct CLientData {
	std::vector<char*> packets;
	int packetsToSend;
	int packetSize;
};


extern Data* programData;
std::vector<char*> packets;
HWND ipPort;
HWND packetSize;
HWND numOfTimes;
HWND ipPortLabel;

HWND startApp;
HWND addFile;


HWND radioClient;
HWND radioServer;

HWND radioTCP;
HWND radioUDP;

HWND applicationStateLbl;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
int prepWindow(HINSTANCE hInst);
bool updateMachineState();
void transferData();
void start();
void createPackets(int chunkSize, int times);
char* strConcat(char* str1, char* str2);
boolean getFilePath(OPENFILENAME& ofn);
void packetizeFile(OPENFILENAME& ofn);
char* getIP();


#endif