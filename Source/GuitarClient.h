#pragma once

#undef UNICODE

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SOCKET int
#define SD_SEND SHUT_RD
#define closesocket close
#endif
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <functional>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "24639"
#define DEFAULT_SERVER "raspberrypi"

class GuitarClient
{
protected:
	std::thread* runThread;
	std::function<std::string(std::string const&)> lineCallback;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	SOCKET ConnectSocket = INVALID_SOCKET;

	void Connect();

public:
	GuitarClient();
	~GuitarClient() {}
	void Start();
	void Stop();
	void SetLineCallback(std::function<std::string(std::string)> callback);
	void SendData(std::string const& data);
};

