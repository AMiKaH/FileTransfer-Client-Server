#pragma once
#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include "HelperFunctions.h"

#define SERVER_UDP_PORT		29000	// Default port
#define MAXLEN				65000   // Maximum Buffer length
#define DEFLEN				64		// Default Length

/*----------- Function Prototypes ------------------------*/
LPTHREAD_START_ROUTINE newThread(LPVOID packetsinfo);
int UDPClient(std::vector<char*> chunks, int datagramSize, int numOfTimes, char* serverAddress);
void UDPServerReceive(LPVOID inStruct);
void CALLBACK WorkerRoutineUDP(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI WorkerThreadUDP(LPVOID lpParameter);
long delay(SYSTEMTIME t1, SYSTEMTIME t2);


#endif