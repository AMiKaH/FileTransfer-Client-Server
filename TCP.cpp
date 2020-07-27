/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Application.cpp - This file is responsible for all the TCP communications, server and client. 
--
--
--
-- PROGRAM: COMP4985-Assignment2
--
-- PROGRAM DESCRIPTION: Depending on what the user chooses from the UI the functions in here will be called to either start a 
--                      server to receive TCP Packets or transmit packets if the user chose to start as a "Client"  
--
-- FUNCTIONS:
--                      int TCPClient(std::vector<char*> chunks, int packetSize,  int numOfTimes, char* serverAddress)
--                      void TCPServerReceive(LPVOID inStruct)
--                      void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
--                            LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--                      DWORD WINAPI WorkerThread(LPVOID lpParameter)

--
-- DATE: February 8, 2020
--
-- REVISIONS: February 8, 2020 - merged the file to have both TCP client and server in one file as opposed to 2 separate files.
                                 Moved helper functions into a new file helperFucntions.h
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/

#include "TCP.h"

int numPackets;
int started;
int ended;
int packetSize;
SOCKET AcceptSocket;
std::ofstream incomingFile;
int totalsize;


typedef struct _SOCKET_INFORMATION {
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[DATA_BUFSIZE];
    WSABUF DataBuf;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

struct threadStruct {
    int packetSize;
    int port;
};

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCPClient
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: int TCPClient(std::vector<char*> packets, int packetSize, int numOfTimes, char* serverAddress)
--
-- PARAMETERS:
--          std::vector<char*> packets - vector of packets to send
--          int packetSize - the size of each packet
--          int numOfTimes - number of times to send the packets
--          char* serverAddress - Server address (IP and Port)
--
-- RETURNS: int
--
-- NOTES:
-- This function starts the TCP client, creates the socket, binds it and then connect to the server address provided by the
-- user. It then loops to send all the provided packets or file until it is done and after that it terminates the connection
-- and closes the socket
----------------------------------------------------------------------------------------------------------------------*/

int TCPClient(std::vector<char*> packets, int packetSize, int numOfTimes, char* serverAddress)
{
	int ns, bytes_to_read;
	int port, err;
	SOCKET sd;
	struct hostent* hp;
	struct sockaddr_in server;
	char* host, * bp, ** pptr;
	int bytesSent = 0;
	int bytesToSend = 0;

	WSADATA WSAData;
	WORD wVersionRequested;

	port = (int) malloc(sizeof(int));
	host = (char*) malloc(128);

	//Parse server and IP address from severAddress
	parser(serverAddress, host, &port);


	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &WSAData);

	if (err != 0) //No usable DLL
	{
		OutputDebugString("DLL not found!\n");
		printf("DLL not found!\n");
		return 1;
	}

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		OutputDebugString("Cannot create socket");
		perror("Cannot create socket");
        return 1;
	}

	// Initialize and set up the address structure
	memset((char*)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	hp = (hostent*)malloc(sizeof(hostent*));

	if (host) {
		if ((hp = gethostbyname(host)) == NULL)
		{
			fprintf(stderr, "Unknown server address\n");
            return 1;
		}
	}

	// Copy the server address
	memcpy((char*)&server.sin_addr, hp->h_addr, hp->h_length);

	// Connecting to the server
	if (connect(sd, (struct sockaddr*) & server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
        return 1;
	}
	
	for (int i = 0; i < numOfTimes; ++i){
		if ((ns = send(sd, packets[i], packetSize, 0)) == SOCKET_ERROR) {
			printf("send() failed with error %d\n", WSAGetLastError());
		}
		ns = 0;
	}

	closesocket(sd);
	WSACleanup();
	return 2;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCPServerReceive
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void TCPServerReceive(LPVOID inStruct)
--
-- PARAMETERS:
--          LPVOID inStruct - the struct that contains the port to open and the size of the packet the server should expect
--
-- RETURNS: void
--
-- NOTES:
-- This function starts the TCP server with the provided user input (port and packet size). The server will then listen 
-- for TCP connections on the created socket
----------------------------------------------------------------------------------------------------------------------*/
void TCPServerReceive(LPVOID inStruct)
{
    WSADATA wsaData;
    SOCKET ListenSocket;
    SOCKADDR_IN InternetAddr;
    INT Ret;
    HANDLE ThreadHandle;
    DWORD ThreadId;
    WSAEVENT AcceptEvent;
    threadStruct* tStruct = (threadStruct*)inStruct;
    int port;
    packetSize = tStruct->packetSize;
    port = tStruct->port;
    started = 0;
    ended = 0;
    numPackets = 0;

    incomingFile.open("Received.txt");
    incomingFile.close();


    if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
    {
        printf("WSAStartup failed with error %d\n", Ret);
        WSACleanup();
        return;
    }

    if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return;
    }

    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);

    if (bind(ListenSocket, (PSOCKADDR)&InternetAddr,
        sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return;
    }

    if (listen(ListenSocket, 5))
    {
        printf("listen() failed with error %d\n", WSAGetLastError());
        return;
    }

    if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return;
    }

    // Create a worker thread to service completed I/O requests. 

    if ((ThreadHandle = CreateThread(NULL, 0, WorkerThread, (LPVOID)AcceptEvent, 0, &ThreadId)) == NULL)
    {
        printf("CreateThread failed with error %d\n", GetLastError());
        return;
    }

    while (TRUE)
    {
        AcceptSocket = accept(ListenSocket, NULL, NULL);

        if (WSASetEvent(AcceptEvent) == FALSE)
        {
            printf("WSASetEvent failed with error %d\n", WSAGetLastError());
            return;
        }

    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerThread
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void TCPServerReceive(LPVOID inStruct)
--
-- PARAMETERS:
--          LPVOID lpParameter - a handler to an overlapped event
--
-- RETURNS: DWORD
--
-- NOTES:
-- This function starts a new thread to wait for the event passed in lpParameter to occur, it calls the cmopletion routine 
-- once something arrives to the socket and the event is triggered.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
    DWORD Flags;
    LPSOCKET_INFORMATION SocketInfo;
    WSAEVENT EventArray[1];
    DWORD Index;
    DWORD RecvBytes;

    // Save the accept event in the event array.

    EventArray[0] = (WSAEVENT)lpParameter;

    while (TRUE)
    {
        // Wait for accept() to signal an event and also process WorkerRoutine() returns.

        while (TRUE)
        {

            Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

            if (Index == WSA_WAIT_FAILED)
            {
                printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
                return FALSE;
            }

            if (Index != WAIT_IO_COMPLETION)
            {
                // An accept() call event is ready - break the wait loop
                break;
            }
        }

        //Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, INFINITE, TRUE);

        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);


        // Create a socket information structure to associate with the accepted socket.

        if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
            sizeof(SOCKET_INFORMATION))) == NULL)
        {
            printf("GlobalAlloc() failed with error %d\n", GetLastError());
            return FALSE;
        }

        // Fill in the details of our accepted socket.

        SocketInfo->Socket = AcceptSocket;
        ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        SocketInfo->BytesSEND = 0;
        SocketInfo->BytesRECV = 0;
        SocketInfo->DataBuf.len = packetSize;
        SocketInfo->DataBuf.buf = SocketInfo->Buffer;

        Flags = 0;
        if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
            &(SocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
                return FALSE;
            }
        }
        printf("Socket %d connected\n", AcceptSocket);
    }

    return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerRoutine
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void TCPServerReceive(LPVOID inStruct)
--
-- PARAMETERS:
--          DWORD Error
--          DWORD BytesTransferred
--          LPWSAOVERLAPPED Overlapped
--          DWORD InFlags
--
-- RETURNS: void
--
-- NOTES:
-- This is the callback for the completion routine, it fires up when the overlapped event is triggered and receives the 
-- data and writes it to a file on the server
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD RecvBytes;
    DWORD Flags;

    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

    if (started <= 0) {
        writeStatsFile(true, packetSize, numPackets);
        ++started;
    }
    //Client disconnected
    if (Error != WSAECONNRESET && BytesTransferred > 0) {
        numPackets++;
        SI->Buffer[BytesTransferred] = '\0';

        if (ended == 0) {
            writeStatsFile(false, packetSize, numPackets);
            ++ended;
        }
        
        writeFile(SI->Buffer);
        
    }
    if (Error == WSAECONNRESET) {
        closesocket(SI->Socket);
    }

    if (Error != 0)
    {
        printf("I/O operation failed with error %d\n", Error);
    }

    //Transmission completed
    if (BytesTransferred == 0)
    {

        printf("Closing socket %d\n", SI->Socket);
    }

    if (Error != 0 || BytesTransferred == 0)
    {
        closesocket(SI->Socket);
        GlobalFree(SI);
        return;
    }


    SI->BytesRECV = 0;
    Flags = 0;

    // Now that there are no more bytes to send post another WSARecv() request.

    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
    SI->DataBuf.len = packetSize;
    SI->DataBuf.buf = SI->Buffer;


    if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
        &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            int ss = WSAGetLastError();
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
        }
    }
    else {

    }
}
