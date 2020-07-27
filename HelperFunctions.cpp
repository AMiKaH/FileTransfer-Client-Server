/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Application.cpp - This file contains helper functions that are used throughout the application
--
--
--
-- PROGRAM: COMP4985-Assignment2
--
-- PROGRAM DESCRIPTION: Depending on what the user chooses from the UI the functions in here will be called to either start a
--                      server to receive TCP Packets or transmit packets if the user chose to start as a "Client"
--
-- FUNCTIONS:
--                      void parser(char* input, char* ip, int* port);
--                      void writeFile(char* buf);
--                      void writeStatsFile(bool isStarted, int size);
--                      char* getWindowValueText(HWND wnd);
--                      int getWindowValueNum(HWND wnd);
--
-- DATE: February 16, 2020
--
-- REVISIONS: 
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/

#include "HelperFunctions.h"


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Parser
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void parser(char* input, char* ip, int* port)
--
-- PARAMETERS:
--          char* input
--          char* ip
--          int* port
--
-- RETURNS: void
--
-- NOTES: This functions takes in a reference to the IP and PORT combination that the user entered and then parses them into
-- data that the application can use to establish a connection with the server. The use of a reference here is used since we are
-- manipulating two variables at the same time that will not be altered at a later time.
----------------------------------------------------------------------------------------------------------------------*/
void parser(char* input, char* ip, int* port) {
    int index = 0;
    char* tempBuff;

    tempBuff = (char*)malloc(128);

    //The next 2 for loops separate service name from protocol
    for (int i = 0; input[i] != ' '; ++i) {
        ip[i] = input[i];
        index = i;
    }

    ip[index + 1] = '\0';
    index = index + 2;

    if (tempBuff) {
        for (int i = 0; input[i] != '\0'; i++) {
            tempBuff[i] = input[index];
            index++;
        }
        *port = atoi(tempBuff);
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeFile
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void writeFile(char* buf)
--
-- PARAMETERS:
--          char* buf
--
-- RETURNS: void
--
-- NOTES: Thsi function simply opens a file in appending mode and writes the data in the buff to the file. The function is called
-- in the completion routine to write and save all the files and bytes received. For now the application saves one file then overrides it
-- when a new set of data arrives.
----------------------------------------------------------------------------------------------------------------------*/
void writeFile(char* buf) {
    std::ofstream incomingFile;
    incomingFile.open("Received.txt", std::ofstream::out | std::ios::app);
    incomingFile << buf;
    incomingFile.close();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeStatsFile
--
-- DATE: February 17 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void writeFile(char* buf)
--
-- PARAMETERS:
--          bool isStarted - denotes if it is the start or end of transmission
--          int size - the size of packet being sent/received
--
-- RETURNS: void
--
-- NOTES: Thsi function simply opens a file in appending mode and writes the data in the buff to the file. The function is called
-- in the completion routine to write and save all the files and bytes received. For now the application saves one file then overrides it
-- when a new set of data arrives.
----------------------------------------------------------------------------------------------------------------------*/
void writeStatsFile(bool isStarted, int size, int packetsCount) {
    std::ofstream statsFile;

    struct timeval tv;
    time_t currentTime;

    struct tm* localTime;
    time(&currentTime);                   // Get the current time


    localTime = localtime(&currentTime);  // Convert the current time to the local time

    statsFile.open("Stats.txt", std::ofstream::out | std::ios::app);
    if (isStarted) {
        statsFile << "Started at: ";
        statsFile << localTime->tm_hour << ":" << localTime->tm_min << ":" << localTime->tm_sec << "\n";
        statsFile << "packet size: " << size << " bytes.\n";
    }
    else {
        statsFile << "Ended at: ";
        statsFile << localTime->tm_hour << ":" << localTime->tm_min << ":" << localTime->tm_sec << "\n";
        statsFile << "Packets received: " << packetsCount << "\n\n";

    }
    statsFile.close();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getWindowValueText
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: char* getWindowValueText(HWND wnd)
--
-- PARAMETERS: HWND wnd - the handle to the control window
--
-- RETURNS: char*
--
-- REVISION: NONE
--
-- NOTES:
-- Helper function to make it easy to get user-entered text values
--
----------------------------------------------------------------------------------------------------------------------*/

char* getWindowValueText(HWND wnd) {
	char* windowValue;
	int winTextLength = 0;

	if (wnd != 0) {
		winTextLength = GetWindowTextLength(wnd) + 1;
		windowValue = (char*)malloc(winTextLength);
		if (windowValue)
			GetWindowText(wnd, windowValue, winTextLength);
	}
	else {
		windowValue = (char*)malloc(sizeof("Value unavailable"));
		windowValue = (char*)"Value unavailable";
	}

	return windowValue;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getWindowValueNum
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: char* getWindowValueNum(HWND wnd)
--
-- PARAMETERS: HWND wnd - the handle to the control window
--
-- RETURNS: int
--
-- REVISION: NONE
--
-- NOTES:
-- This function calls the function above it except this one returns an integer value to be used by the application when a char*
-- is not needed
--
----------------------------------------------------------------------------------------------------------------------*/
int getWindowValueNum(HWND wnd) {
	return std::stoi(getWindowValueText(wnd));
}

