#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <thread>

#include "net_lib.h"

void print_error(const char* text)
{
	puts(text);
	DWORD dwError;
	dwError = WSAGetLastError();
	std::string message = std::system_category().message(dwError);
	printf("Error: %s - %d\n", message.c_str(), dwError);
}


// Global handle to stop event.
HANDLE hStopEvent;

void th_rcv(SOCKET ClientSock)
{
	char recvbuf[512];
	int iResult = 1, iSendResult;
	int recvbuflen = 512;

	// Array of event handles.
	HANDLE hEvents[2];

	hEvents[0] = hStopEvent;

	// Create event handle for socket.
	hEvents[1] = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Set event handle for socket to track socket.
	if (WSAEventSelect(ClientSock, hEvents[1], FD_READ | FD_CLOSE) != 0) {
		print_error("wsaeventselect");
		return;
	}

	DWORD dwEvent;
	do {
		// Wait for event
		dwEvent = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

		// Stop event.
		if (dwEvent == WAIT_OBJECT_0) {
			print_error("StopEvent");
			return;
		}

		// Socket event.
		else if (dwEvent == WAIT_OBJECT_0 + 1) {
			puts("INCOMING DATA");
		}

		// Shouldn't hit.
		else {
			print_error("No event");
			continue;
		}


		iResult = recv(ClientSock, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			iSendResult = send(ClientSock, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSock);
				WSACleanup();
				return;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			print_error("recv");
			closesocket(ClientSock);
			WSACleanup();
			return;
		}

	} while (iResult > 0);
}


void thr_loop(void)
{
	DWORD dwEvent;
	for (;;) {
		dwEvent = WaitForSingleObject(hStopEvent, 0);

		if (dwEvent == WAIT_OBJECT_0) {
			return;
		}

		puts("YEEE YEE");
		Sleep(1000);
	}
}


int main()
{
	SOCKET ListenSocket = create_socket();
	SOCKET ClientSock = listen_socket(ListenSocket);

	// Create stop event handle.
	hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


	std::thread b = std::thread(th_rcv, ClientSock);
	std::thread a = std::thread(thr_loop);

	Sleep(10000);

	// Send stop event.
	SetEvent(hStopEvent);

	b.join();
	a.join();

}
