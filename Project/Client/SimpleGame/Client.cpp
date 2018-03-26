#include "stdafx.h"
#include "Client.h"

int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

void RecvThreadFunc(SOCKET clientsock, std::list<char*> *MsgQueue)
{
	int retval = 0;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);

	getpeername(clientsock, (SOCKADDR *)&clientaddr, &addrlen);

	while (1)
	{
		char* buf = new char[MSGSIZE];
		retval = recvn(clientsock, buf, MSGSIZE, 0);
		MsgQueueLocker.lock();
		MsgQueue->push_back(buf);
		MsgQueueLocker.unlock();
	}
	return;
}

Client::Client()
{
}


Client::~Client()
{
}

void Client::InitClient()
{
	int retval = 0;
	std::cout << "Enter ServerIP (default : 0) : ";
	std::cin >> Server_IP;
	if (!strcmp(Server_IP, "0"))
		strcpy_s(Server_IP, sizeof("127.0.0.1"), "127.0.0.1");

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Error :: init WinSock" << std::endl;
		exit(1);
	}

	Server_Sock = socket(AF_INET, SOCK_STREAM, 0);
	if (Server_Sock == INVALID_SOCKET) {
		std::cout << "Error :: init ServerSocket" << std::endl;
		exit(1);
	}

	Server_Addr;
	ZeroMemory(&Server_Addr, sizeof(Server_Addr));
	Server_Addr.sin_family = AF_INET;
	inet_pton(AF_INET, Server_IP, &Server_Addr.sin_addr);
	Server_Addr.sin_port = htons(SERVERPORT);

	retval = connect(Server_Sock, (SOCKADDR *)&Server_Addr, sizeof(Server_Addr));
	if (retval == SOCKET_ERROR) {
		std::cout << "Error :: Connect Server" << std::endl;
		exit(1);
	}

	RecvThread = std::thread{ RecvThreadFunc, Server_Sock, &MsgQueue };
}

void Client::StartClient()
{
}

void Client::CloseClient()
{
	RecvThread.join();

	closesocket(Server_Sock);
	printf("Client Disconnected :: IP : %s, PORT : %d\n", Server_IP, ntohs(Server_Addr.sin_port));

	closesocket(Server_Sock);

	WSACleanup();
}

int Client::SendMsg(int id, int x, int y)
{
	char buf[MSGSIZE];
	buf[0] = MSGTYPE::MOVE;
	buf[1] = id;
	buf[2] = x;
	buf[3] = y;

	return send(Server_Sock, buf, sizeof(buf), 0);
}

int Client::SendMsg(char * buf)
{
	return send(Server_Sock, buf, MSGSIZE, 0);
}



void Client::SendHeartBeat()
{
	char buf[MSGSIZE];
	buf[0] = MSGTYPE::HEARTBEAT;
	buf[1] = ID;

	send(Server_Sock, buf, sizeof(buf), 0);
}
