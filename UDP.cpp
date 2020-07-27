/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Application.cpp - This file is responsible for all the UDP communications, server and client.
--
--
--
-- PROGRAM: COMP4985-Assignment2
--
-- PROGRAM DESCRIPTION: Depending on what the user chooses from the UI the functions in here will be called to either start a
--                      server to receive UDP datagrams or transmit datagrams if the user chose to start as a "Client"
--
-- FUNCTIONS:
--                      LPTHREAD_START_ROUTINE newThread(LPVOID packetsinfo);
--                      int UDPClient(std::vector<char*> chunks, int datagramSize, int numOfTimes, char* serverAddress);
--                      void UDPServerReceive(LPVOID inStruct);
--                      void CALLBACK WorkerRoutineUDP(DWORD Error, DWORD BytesTransferred,
--                          LPWSAOVERLAPPED Overlapped, DWORD InFlags);
--                      DWORD WINAPI WorkerThreadUDP(LPVOID lpParameter);
--                      long delay(SYSTEMTIME t1, SYSTEMTIME t2);
--
-- DATE: February 8, 2020
--
-- REVISIONS: February 8, 2020 - merged the file to have both UDP client and server in one file as opposed to 2 separate files.
                                 Moved helper functions into a new file helperFucntions.h
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
#include "UDP.h"

char rbufu[MAXLEN];
char sbufu[MAXLEN];
struct	sockaddr_in serveru;
int server_lenu;
SOCKET sdu;
int numTimes;


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
-- FUNCTION: UDPClient
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: int UDPClient(std::vector<char*> packets, int packetSize, int numOfTimes, char* serverAddress)
--
-- PARAMETERS:
--          std::vector<char*> chunks - vector of chunks of bytes to send as datagrams
--          int packetSize - the size of each packet/datagram
--          int numOfTimes - number of times to send the packets
--          char* serverAddress - Server address (IP and Port)
--
-- RETURNS: int
--
-- NOTES:
-- This function starts the UDP client, creates the socket, binds it and then loops to send all the provided data or file 
-- until it is done then it closes the socket
----------------------------------------------------------------------------------------------------------------------*/

int UDPClient(std::vector<char*> chunks, int datagramSize, int numOfTimes, char* serverAddress)
{
	sdu = INVALID_SOCKET;
	char host[128];
	int	data_size = DEFLEN, port = 0;
	int	client_len;

	struct	hostent* hp;
	struct	sockaddr_in client;

	WSADATA stWSAData;
	WORD wVersionRequested = MAKEWORD(2, 2);


	//host = (char*)malloc(128);

	//Parse server and IP address from severAddress
	parser(serverAddress, host, &port);

	// Initialize the DLL with version Winsock 2.2
	if (WSAStartup(wVersionRequested, &stWSAData) != 0) {
		printf("WSASTartup failed with error %d\n", GetLastError());
		return -1;
	}
	// Create a datagram socket
	if ((sdu = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Can't create a socket\n");
        return 1;
    }

	// Store server's information
	memset((char*)&serveru, 0, sizeof(serveru));
	serveru.sin_family = AF_INET;
	serveru.sin_port = htons(port);


	if ((hp = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Can't get server's IP address\n");
        return 1;
    }


	//hp = (hostent*)malloc(sizeof(hostent*));

	//strcpy((char *)&server.sin_addr, hp->h_addr);

	memcpy((char*)&serveru.sin_addr, hp->h_addr, hp->h_length);

	// Bind local address to the socket
	memset((char*)&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(0);  // bind to any available port
	client.sin_addr.s_addr = htonl(INADDR_ANY);



	if (bind(sdu, (struct sockaddr*) & client, sizeof(client)) == -1)
	{
		perror("Can't bind name to socket");
        return 1;
    }

	// Find out what port was assigned and print it
	client_len = sizeof(client);
	if (getsockname(sdu, (struct sockaddr*) &client, &client_len) < 0)
	{
		perror("getsockname: \n");
        return 1;
    }

	printf("Port aasigned is %d\n", ntohs(client.sin_port));

	if (data_size > MAXLEN)
	{
		fprintf(stderr, "Data is too big\n");
        return 1;
    }

	server_lenu = sizeof(serveru);
	for (int i = 0; i < numOfTimes; ++i) {
		if (sendto(sdu, chunks[i], datagramSize, 0, (sockaddr*)& serveru, server_lenu) == -1)
		{
			perror("sendto failure");
        }

	}
	closesocket(sdu);
	WSACleanup();
	return 1;
}

// Compute the delay between tl and t2 in milliseconds
long delay(SYSTEMTIME t1, SYSTEMTIME t2)
{
	long d;

	d = (t2.wSecond - t1.wSecond) * 1000;
	d += (t2.wMilliseconds - t1.wMilliseconds);
	return(d);
}

//using namespace std;

#include "UDPServer.h"


int numPacketsUDP;
int packetSizeUDP;
SOCKET AcceptSocketUDP;
std::ofstream incomingFileUDP;
int totalsizeUDP;
SOCKET ListenSocket;

struct sockaddr_in SenderAddr;
struct sockaddr_in SenderAddrPeek;
int SenderAddrSize;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: UDPServerReceive
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void UDPServerReceive(LPVOID inStruct)
--
-- PARAMETERS:
--          LPVOID inStruct - the struct that contains the port to open and the size of the datagram the server should expect
--
-- RETURNS: void
--
-- NOTES:
-- This function starts the UDP server with the provided user input (port and packet size). The server will then listen
-- for any datagrams that arrive, as it is connectionless a blocking call is put using the recv() function to stop and wait for
-- datagrams before firing up the the event and subsequently, the completion routine
----------------------------------------------------------------------------------------------------------------------*/
void UDPServerReceive(LPVOID inStruct)
{
    WSADATA wsaData;
    SOCKADDR_IN InternetAddr;
    INT Ret;
    HANDLE ThreadHandle;
    DWORD ThreadId;
    WSAEVENT AcceptEvent;
    int port;
    threadStruct* tStruct = (threadStruct*)inStruct;

    packetSizeUDP = tStruct->packetSize;
    port = tStruct->port;
    numPacketsUDP = 0;
    SenderAddrSize = sizeof(SenderAddr);
    incomingFileUDP.open("Received.txt");

    incomingFileUDP.close();


    if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
    {
        printf("WSAStartup failed with error %d\n", Ret);
        WSACleanup();
        return;
    }

    // Check IPPROTO
    if ((ListenSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return;
    }

    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);

    if (bind(ListenSocket, (SOCKADDR*)&InternetAddr,
        sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return;
    }

    if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return;
    }

    // Create a worker thread to service completed I/O requests. 

    if ((ThreadHandle = CreateThread(NULL, 0, WorkerThreadUDP, (LPVOID)AcceptEvent, 0, &ThreadId)) == NULL)
    {
        printf("CreateThread failed with error %d\n", GetLastError());
        return;
    }

    char buf[1];

    while (TRUE)
    {
        if (recvfrom(ListenSocket, buf, 1, MSG_PEEK, (SOCKADDR*)&SenderAddrPeek, &SenderAddrSize)) {
            if (WSASetEvent(AcceptEvent) == FALSE)
            {
                printf("WSASetEvent failed with error %d\n", WSAGetLastError());
                return;
            }
        }
    }
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerThreadUDP
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void WorkerThreadUDP(LPVOID inStruct)
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

DWORD WINAPI WorkerThreadUDP(LPVOID lpParameter)
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

        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

        // Create a socket information structure to associate with the accepted socket.

        if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
            sizeof(SOCKET_INFORMATION))) == NULL)
        {
            printf("GlobalAlloc() failed with error %d\n", GetLastError());
            return FALSE;
        }
        SenderAddrSize = sizeof(SenderAddr);

        // Fill in the details of our accepted socket.
        SenderAddrSize = sizeof(SenderAddr);
        SocketInfo->Socket = ListenSocket;
        ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        SocketInfo->BytesSEND = 0;
        SocketInfo->BytesRECV = 0;
        SocketInfo->DataBuf.len = packetSizeUDP;
        SocketInfo->DataBuf.buf = SocketInfo->Buffer;

        Flags = 0;

        if (WSARecvFrom(ListenSocket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
            (SOCKADDR*)&SenderAddrPeek, &SenderAddrSize, &(SocketInfo->Overlapped), WorkerRoutineUDP) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
                return FALSE;
            }
        }
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
void CALLBACK WorkerRoutineUDP(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD SendBytes, RecvBytes;
    DWORD Flags;

    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

    writeFile(SI->Buffer);
    numPacketsUDP++;
    printf("packets %d\n", numPacketsUDP);

    if (Error != 0)
    {
        printf("I/O operation failed with error %d\n", Error);
    }

    if (BytesTransferred == 0)
    {
        printf("Closing socket %d\n", SI->Socket);
    }

    if (Error != 0 || BytesTransferred == 0)
    {
        closesocket(SI->Socket);
        //GlobalFree(SI);
        return;
    }

    // Check to see if the BytesRECV field equals zero. If this is so, then
    // this means a WSARecv call just completed so update the BytesRECV field
    // with the BytesTransferred value from the completed WSARecv() call.

    if (SI->BytesRECV == 0)
    {
        SI->BytesRECV = BytesTransferred;
        SI->BytesSEND = 0;

    }
    else
    {
        SI->BytesSEND += BytesTransferred;
    }


    SI->BytesRECV = 0;

    Flags = 0;

    SI->DataBuf.len = packetSizeUDP;
    SI->DataBuf.buf = SI->Buffer;

    if (WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
        (SOCKADDR*)&SenderAddr, &SenderAddrSize, &(SI->Overlapped), WorkerRoutineUDP) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
        }
    }
}