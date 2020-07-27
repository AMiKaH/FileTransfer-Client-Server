#pragma once
#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <windows.h>
#include <errno.h>
#include <memory.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <memory.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <ctime>

#define DATA_BUFSIZE 100000


void parser(char* input, char* ip, int* port);
void writeFile(char* buf);
char* getWindowValueText(HWND wnd);
int getWindowValueNum(HWND wnd);
void writeStatsFile(bool isStarted, int size, int packetsCount);

#endif