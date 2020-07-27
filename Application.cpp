/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Application.cpp - This is the main file of this program. It contains the WinMain function which
--								  runs the program and initializes all the controls, windows, variables and 
--								  structs needed for the UI to interact with the core of the application. 
--								  
--
--
-- PROGRAM: COMP4985-Assignment2
--
-- PROGRAM DESCRIPTION: The application will take user input and initialize either a client or a server using either a TCP or UDP.
--						The user will enter the values to connect to the other ed or to accept connections such as the 
--						IP Address and PORT to connect to. The packet size to send and the number of times to send it.
--
-- FUNCTIONS:
--						int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
--						LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--						void prepWindow(HINSTANCE hInst)
--						void start()
--						void transferData()
--						void createPackets(int chunkSize, int times)
--						char* strConcat(char* str1, char* str2)
--						boolean getFilePath(OPENFILENAME& ofn
--						void packetizeFile(OPENFILENAME& ofn)
--						char* getWindowValueText(HWND wnd)
--						int getWindowValueNum(HWND wnd)						
--
-- DATE: February 8, 2020
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

#include "Application.h"
#pragma warning (disable: 4096)
char szFile[FILESIZE];
Data* programData = new Data();

struct threadStruct {
	int packetSize;
	int port;
};

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- 
-- PARAMETERS:
--			HINSTANCE hInst
--			HINSTANCE hprevInstance
--			LPSTR lspszCmdParam
--			int nCmdShow
--
-- RETURNS: int
--
-- NOTES:
-- This is the driver of the program, it starts by creating the main window and registering it. The function also calls
-- the prepWindow function to create all the sub-window and control elements. It then runs a loop to receive and dispatch
-- the various Windows "Messages"
----------------------------------------------------------------------------------------------------------------------*/

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	programData->isClient = false;
	programData->isServer = false;
	programData->isTCP = false;
	programData->isUDP = false;
	programData->ipObtained = false;
	packets.clear();



	//Set default values
	programData->packetSizeInput = 1024;
	programData->numOfTimesInput = 100;
	static TCHAR Name[] = TEXT("COMP 4985 Assignment 2 - Amir Kbah");


	MSG Msg{ 0 };
	WNDCLASSEX Wcl;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(240, 50, 50)));//(RGB(240, 240, 240)
	Wcl.lpszClassName = Name;
	Wcl.lpszMenuName = TEXT("TerminalMenu"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	//Creates and centers the window
	programData->hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, (GetSystemMetrics(0) / 2 - 300), (GetSystemMetrics(1) / 2),
		615, 209, NULL, NULL, hInst, NULL);

	//prepWindow(hInst);
	prepWindow(hInst);

	ShowWindow(programData->hwnd, nCmdShow);
	UpdateWindow(programData->hwnd);


	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: Fenruary 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--
-- PARAMETERS:
--			HWND hwnd
--			UINT Message
--			WPARAM wParam
--			LPARAM lParam
--
-- RETURNS: LRESULT
--
-- REVISION: February 8, 2020
--			 Made changes to accommodate the new program such as handling the new buttons to upload the file and handle
--			 all the new user input such as IP, PORT, Packet Size... etc.
--
-- NOTES:
-- This is the main and only call back function to process Windows messages in this file. It handles the various Windows
-- "Messages" and user interactions through a switch statement. It handles the call backs for setting the operation that
-- the user chooses and more importantly takes the user input and passes it to the corresponding functions for processing
----------------------------------------------------------------------------------------------------------------------*/

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	int newLength = 0;
	int newbuf = 0;
	newLength = GetWindowTextLength(ipPort) + 1;
	int response;
	int s = 0;
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case RADIO_CLIENT:
			programData->isClient = true;
			programData->isServer = false;
			EnableWindow(radioTCP, true);
			EnableWindow(radioUDP, true);
			SendMessage(ipPortLabel, WM_SETTEXT, 0, (LPARAM)"IP Port");
			break;
		case RADIO_SERVER:
			programData->isClient = false;
			programData->isServer = true;
			EnableWindow(radioTCP, true);
			EnableWindow(radioUDP, true);
			EnableWindow(numOfTimes, false);
			SendMessage(ipPortLabel, WM_SETTEXT, 0, (LPARAM)"Port");
			break;

		case RADIO_TCP:
			programData->isTCP = true;
			programData->isUDP = false;
			EnableWindow(ipPort, true);
			if (programData->isServer)
				EnableWindow(numOfTimes, false);
			else 
				EnableWindow(numOfTimes, true);
			EnableWindow(addFile, true);
			EnableWindow(startApp, true);

			break;

		case RADIO_UDP:
			programData->isTCP = false;
			programData->isUDP = true;
			EnableWindow(ipPort, true);
			if (programData->isServer)
				EnableWindow(numOfTimes, false);
			else
				EnableWindow(numOfTimes, true);
			EnableWindow(addFile, true);
			EnableWindow(startApp, true);
			break;

		case BTN_TRANSFER:
			if (GetWindowTextLength(ipPort) <= 0) {
				MessageBox(NULL, TEXT("Please enter IP Port"), TEXT("Error!"), MB_OK);
				break;
			}
			else if (GetWindowTextLength(packetSize) <= 0) {
				MessageBox(NULL, TEXT("Please select a packet size!"), TEXT("Error!"), MB_OK);
				break;
			}
			else if (!programData->fileUploaded && GetWindowTextLength(numOfTimes) <= 0) {
				if (!programData->isServer) {
					MessageBox(NULL, TEXT("Please enter the number of times to send a packet or upload a file"), TEXT("Error!"), MB_OK);
					break;
				}
			}
			start();
			break;

		case BTN_ADDFILE:
			if (GetWindowTextLength(packetSize) <= 0) {
				MessageBox(NULL, TEXT("Please select a packet size first!"), TEXT("Error!"), MB_OK);
				break;
			}
			if (programData->isServer) {
				MessageBox(NULL, TEXT("You cannot upload a file as a server!"),TEXT("Server!"), MB_OK);
			} else if (getFilePath(ofn)) {
				packetizeFile(ofn);
				programData->fileUploaded = true;
				EnableWindow(numOfTimes, false);				
			}
			break;

		case IDM_HELP:
			//TODO
			MessageBox(NULL, TEXT("1) Select a behaviour first (Client or Server)\n2) Select a protocol (TCP or UDP)\n"
				"3) Enter the IP address and Port or just port if you are using a server\n"
				"4) Enter the packet size to send (in bytes) (Client only)\n"
				"5) Enter the number of times to send a packet or upload a file (Client only)\n"
				"6) Click start\n"
				"**Make sure that a server is started first and that you enter the correct server address!"),
				TEXT("Help"), MB_OK);
			break;

		case IDM_EXIT:
		case WM_LBUTTONDOWN:
			response = MessageBox(hwnd, TEXT("Are you sure you want to terminate the program?"), TEXT("Exit?"),
				MB_ICONEXCLAMATION | MB_YESNO);
			switch (response)
			{
			case IDYES:
				PostQuitMessage(0);

				break;
			case IDNO:

				break;
			}

		}
		break;

	case WM_DESTROY:	// Terminate program
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: preWindow
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: int prepWindow(HINSTANCE hInst)
--
-- PARAMETERS:
--			HINSTANCE hInst
--
-- RETURNS: int
--
-- NOTES:
-- This function only creates the main windows' controls such as labels, text boxes and buttons. It is called from the
-- WinMain function in order to create all the necessary controls for this program
----------------------------------------------------------------------------------------------------------------------*/

int prepWindow(HINSTANCE hInst) {

	//*** Start window background patches **//
	CreateWindow("STATIC", "",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		0, 2, 299, 146, programData->hwnd, NULL, hInst, NULL);
		
	CreateWindow("STATIC", "",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		301, 2, 299, 146, programData->hwnd, NULL, hInst, NULL);
	//*** End window background patches **//

	//Behavior Radio buttons
	CreateWindow("STATIC", "Please select role:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		15, 5, 275, 25, programData->hwnd, NULL, hInst, NULL);

	radioClient = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Client",
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 15, 25, 75, 25,
		programData->hwnd, (HMENU)RADIO_CLIENT, hInst, NULL);

	radioServer = CreateWindowExW(WS_EX_WINDOWEDGE, L"BUTTON", L"Server",
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 15, 45, 75, 25,
		programData->hwnd, (HMENU)RADIO_SERVER, hInst, NULL);

	//IP Port
	ipPortLabel = CreateWindow("STATIC", "IP Port:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		15, 75, 104, 18, programData->hwnd, NULL, hInst, NULL);

	ipPort = CreateWindow("EDIT", "",
		WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER,
		120, 75, 175, 18, programData->hwnd, NULL, hInst, NULL);
	EnableWindow(ipPort, false);

	//Packet size
	CreateWindow("STATIC", "Packet Size:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		15, 100, 104, 18, programData->hwnd, NULL, hInst, NULL);

	packetSize = CreateWindow("EDIT", "",
		WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER ,
		120, 100, 175, 18, programData->hwnd, NULL, hInst, NULL);

	//numOfTimes
	CreateWindow("STATIC", "# of times:",
		WS_VISIBLE | WS_CHILD | SS_LEFT ,
		15, 125, 104, 18, programData->hwnd, NULL, hInst, NULL);

	numOfTimes = CreateWindow("EDIT", "",
		WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER,
		120, 125, 175, 18, programData->hwnd, NULL, hInst, NULL);
	EnableWindow(numOfTimes, false);


	//Protocols
	CreateWindow("STATIC", "Please select protocol:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		315, 5, 275, 25, programData->hwnd, NULL, hInst, NULL);

	//TCP UDP radio button group
	radioTCP = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "TCP",
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 315, 25, 75, 25,
		programData->hwnd, (HMENU)RADIO_TCP, hInst, NULL);
	EnableWindow(radioTCP, false);

	radioUDP = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "UDP",
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 315, 45, 75, 25,
		programData->hwnd, (HMENU)RADIO_UDP, hInst, NULL);
	EnableWindow(radioUDP, false);

	// Add file button
	addFile = CreateWindow((LPCSTR)"BUTTON", (LPCSTR)"Add File",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		350, 85, 75, 25, programData->hwnd, (HMENU)BTN_ADDFILE, hInst, NULL);
	EnableWindow(addFile, false);

	//Transfer button
	startApp = CreateWindow((LPCSTR)"BUTTON", (LPCSTR)"Start",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		475, 85, 75, 25, programData->hwnd, (HMENU)BTN_TRANSFER, hInst, NULL);
	EnableWindow(startApp, false);

	applicationStateLbl = CreateWindow("STATIC", "Behaviour / Protocol / IP:Port",
		WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER,
		315, 125, 275, 18, programData->hwnd, NULL, hInst, NULL);

	return 1;

}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: transferData
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void transferData() 
--
-- PARAMETERS: NONE
--
-- RETURNS: void
--
-- REVISION: NONE
--
-- NOTES:
-- This function is called when the application behaviour is a "Client". It will take all the inputs of the user and map them
-- appropriately to the global programdata struct. For example it will assign the provided port, number of packets, 
-- number of times and then call the function to start the corresponding client, eitehr a TCP or a UDP client to send to either
-- a UDP or a TCP server, if a file is selected then the user cannot provide the number of times as the number of times will
-- be decided by the packet size and the file size.
----------------------------------------------------------------------------------------------------------------------*/
void transferData() {
	//Get IP and Port
	programData->ipPortInput = (char*)malloc(GetWindowTextLength(ipPort));
	programData->ipPortInput = (char*) getWindowValueText(ipPort);

	//programData->ipPortInput = (char*) "192.168.1.104 29000";
	//programData->ipPortInput= (char*)"192.168.1.134 29000";
	//programData->ipPortInput = (char*)"198.47.45.237 29000";

	//programData->ipPortInput = (char*)"127.0.0.1 29000";

	//henry
	//programData->ipPortInput = (char*)"205.250.198.182 5150";
	//ellaine
	//programData->ipPortInput = (char*) "70.71.136.56 5150";


	//get number of times
	if (!programData->fileUploaded)
		programData->numOfTimesInput = getWindowValueNum(numOfTimes);

	if (packets.size() > 0) {

	}
	else {
		//packets.reserve(numTimes);
		createPackets(programData->packetSizeInput, programData->numOfTimesInput);
	}

	if (programData->isTCP) {
		if (TCPClient(packets, programData->packetSizeInput, programData->numOfTimesInput, programData->ipPortInput) <= 1) {
			SetWindowText(applicationStateLbl, "Failed to connect to the server");
		}
		return;
	}
	else {
		UDPClient(packets, programData->packetSizeInput, programData->numOfTimesInput, programData->ipPortInput);
		return;
	}
	//free(IPPort);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateMachineState
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: bool updateMachineState()
--
-- PARAMETERS: NONE
--
-- RETURNS: bool
--
-- REVISION: NONE
--
-- NOTES:
-- This function is not operation as much as it is cosmetic. It will display the state of the machine as selected by the user
-- more importantly it will display the IP address of the machine on the local network through the call of another function.
-- This is to help other computers to know the machines address and server or clietn type when testing so that others can
-- interact with it accordingly.
----------------------------------------------------------------------------------------------------------------------*/
bool updateMachineState() {
	char* behaviour;
	char* protocol;
	char* ipAddress;

	//Only get the ip once per run
	//if (!programData->ipObtained) {
	ipAddress = (char*)malloc(sizeof(getIP()) + 1);
	ipAddress = getIP();
	protocol = (char*)malloc(128);
	behaviour = (char*)malloc(128);

	if (programData->isServer) {
		behaviour = (char*)realloc(behaviour, sizeof("Server started at:") + 1);
		behaviour = (char*)"Server started at:";
	}
	else {
		behaviour = (char*)realloc(behaviour, sizeof("Client from:") + 1);
		behaviour = (char*)"Client from:";
	}

	if (programData->isTCP) {
		protocol = (char*)realloc(protocol, sizeof("TCP ") + 1);
		protocol = (char*)"TCP ";
	}
	else {
		protocol = (char*)realloc(protocol, sizeof("UDP ") + 1);
		protocol = (char*)"UDP ";
	}

	programData->applicationStatus = (char*)malloc(255);

	programData->applicationStatus = strConcat(protocol, behaviour);

	programData->applicationStatus = strConcat(programData->applicationStatus, ipAddress);
	SetWindowText(applicationStateLbl, programData->applicationStatus);
	
	free(ipAddress);
	//free(behaviour);
	//free(protocol);
	return true;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: strConcat
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: char* strConcat(char* str1, char* str2)
--
-- PARAMETERS: 
--			char* str1
--			char* str2
--
-- RETURNS: char* 
--
-- REVISION: NONE
--
-- NOTES:
-- This is a helper function used to concatenate strings as required by the application
--
----------------------------------------------------------------------------------------------------------------------*/

char* strConcat(char* str1, char* str2) {
	int originalSize = strlen(str1);
	int newSize = originalSize + strlen(str2);

	char* concatedStr = (char*) malloc(newSize + 1);


	if (concatedStr) {
		for (int i = 0; i < originalSize; ++i) {
			concatedStr[i] = str1[i];
		}

		concatedStr[originalSize] = ' ';

		for (int i = originalSize + 1, j = 0; i <= newSize; ++i, ++j) {
			concatedStr[i] = str2[j];
		}
		concatedStr[newSize + 1] = '\0';
	}


	return concatedStr;

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getFilePath
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: boolean getFilePath(OPENFILENAME& ofn)
--
-- PARAMETERS:
--			PENFILENAME& ofn
--
-- RETURNS: boolean
--
-- REVISION: NONE
--
-- NOTES:
-- This is a helper function used to get the path of a file optained from the the user browsing and using Windows File
-- Explorer. The path is then used in another function to open the file and get the data out of it
--
----------------------------------------------------------------------------------------------------------------------*/
boolean getFilePath(OPENFILENAME& ofn) {
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = ("All\0*.*\0Text\0*.TXT\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	return GetOpenFileNameA((LPOPENFILENAMEA)&ofn);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: createPackets
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void createPackets(int chunkSize, int times)
--
-- PARAMETERS:
--			int chunkSize - the size of a packet as entere by the user
--			int times	  - the number of times to send the packets as entered by the user
--
-- RETURNS: void
--
-- REVISION: NONE
--
-- NOTES:
-- This function is responsible for populating the packets vector, which is a vector of char pointers passed to the TCP or UDP
-- clients to send to the server. The packets are created with alternating values, even packets are populated with 'X' characters
-- and odd packets are populated with 'I' characters. Those two characters were particualy chosen as it is easier for testing the
-- received file on the server side and distinguish the packets and see how many actually arrived.
--
----------------------------------------------------------------------------------------------------------------------*/
void createPackets(int chunkSize, int times) {
	int chunkSz = chunkSize;
	int num = times;
	packets = std::vector<char*>(times);

	for (int i = 0; i < num; ++i) {
		packets[i] = (char*)calloc(chunkSz, sizeof(char));

		for (int j = 0; j < chunkSz; ++j) {
			if ((i % 2) == 0)
				packets[i][j] = 'X';
			else
				packets[i][j] = 'I';				
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: packetizeFile
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void packetizeFile(OPENFILENAME &ofn)
--
-- PARAMETERS:
--			OPENFILENAME &ofn - a refernce to the file that was opened by the getFilePath function
--
-- RETURNS: void
--
-- REVISION: NONE
--
-- NOTES:
-- Similar to the function above, except this one gets the characters from a text file chosen by the client.
--
----------------------------------------------------------------------------------------------------------------------*/

void packetizeFile(OPENFILENAME &ofn) {
	LPSTR filePath = ofn.lpstrFile;
	std::ifstream file(filePath, std::ios::ate);
	std::streamoff fileLength = file.tellg();
	programData->packetSizeInput = getWindowValueNum(packetSize);


	int i = 0;
	//Set value to Default and prevent division by 0

	programData->numOfTimesInput = ceil((double) fileLength / programData->packetSizeInput);
	int seek = 0;
	char* buf;

	file.seekg(0, std::ios::beg);
	std::streamoff currG = 0;
	std::streamoff lastIndex = 0;

	packets = std::vector<char*>(programData->numOfTimesInput);
	
	while (!file.eof()) {

		packets[i] = (char*)malloc(programData->packetSizeInput + 1);

		packets[i][programData->packetSizeInput] = '\0';

		file.read(packets[i], programData->packetSizeInput);
		currG = file.tellg();

		//Last chunk of file may not have the same length as the rest of the packets
		if (currG < 0) {
			packets[i][lastIndex - 2] = '\0';
		}
		else {
			packets[i][programData->packetSizeInput] = '\0';
		}
		lastIndex = fileLength - currG;
		i++;
	}

	file.close();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: start
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void start()
--
-- PARAMETERS: NONE
--
-- RETURNS: void
--
-- REVISION: NONE
--
-- NOTES:
-- This function is called when the user clicks the "Start" button. It will check what the user chose and call the apropriate functions
-- As servers are more demanding, they are started with their own thread and are passed a struct that contains the size of the packets
-- that the server should expect as well as the port number to open. If the user chose "Client" instead of server then the transferData
-- function is called. in either case the updateMachineStatus is called to display the machine status on the screen
--
----------------------------------------------------------------------------------------------------------------------*/
void start() {
	HANDLE ThreadHandle;
	DWORD ThreadId;
	int pktSize = getWindowValueNum(packetSize);
	programData->packetSizeInput = getWindowValueNum(packetSize);
	updateMachineState();
	if (programData->isServer) {

		threadStruct* tStruct = (threadStruct*)malloc(sizeof(threadStruct));
		tStruct->packetSize = programData->packetSizeInput;
		tStruct->port = getWindowValueNum(ipPort);


		if (programData->isTCP) {
			//Start TCP Server
			ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TCPServerReceive, (LPVOID)tStruct, 0, &ThreadId);
		}
		else {
			//Start UDP Server
			ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UDPServerReceive, (LPVOID)tStruct, 0, &ThreadId);
		}
	}
	else {
		transferData();
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getIP
--
-- DATE: February 8, 2020
--
-- REVISIONS: NONE
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: char* getIP()
--
-- PARAMETERS: NONE
--
-- RETURNS: char*
--
-- REVISION: NONE
--
-- NOTES:
-- Helper function to get the IP of the machine on the local network
--
----------------------------------------------------------------------------------------------------------------------*/
char* getIP()
{
	std::string line;
	std::ifstream IPFile;
	int offset;
	char* IPAddress;
	char* search0 = (char*)"IPv4 Address. . . . . . . . . . . :";      // search pattern

	system("ipconfig > ip.txt");
	//cmdLineNoWindow("ipconfig > ip.txt");

	IPFile.open("ip.txt");
	IPAddress = (char*)malloc(128);
	if (IPFile.is_open())
	{
		while (!IPFile.eof())
		{
			getline(IPFile, line);
			if ((offset = line.find(search0, 0)) != std::string::npos)
			{
				// IPv4 Address. . . . . . . . . . . : 1
				line.erase(0, 39);
				IPAddress = (char*)realloc(IPAddress, sizeof(line) + 1);

				std::cout << line << std::endl;
				if (IPAddress)
					strcpy(IPAddress, line.c_str());
				programData->ipObtained = true;

				return IPAddress;
				IPFile.close();
			}
		}
	}
	return 0;
}