#pragma once
#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include "HelperFunctions.h"

int TCPClient(std::vector<char*> chunks, int packetSize,  int numOfTimes, char* serverAddress);
void TCPServerReceive(LPVOID inStruct);
void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI WorkerThread(LPVOID lpParameter);

#endif