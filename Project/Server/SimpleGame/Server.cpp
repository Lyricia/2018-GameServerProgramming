#include "stdafx.h"
#include "Server.h"

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

Server::Server()
{
}

Server::~Server()
{
}

void Server::InitServer()
{
	int retval;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	Listen_Sock = socket(AF_INET, SOCK_STREAM, 0);
	if (Listen_Sock == INVALID_SOCKET) {
		std::cout << "Error :: init ListenSocket" << std::endl;
		return;
	}

	ZeroMemory(&Server_Addr, sizeof(Server_Addr));
	Server_Addr.sin_family = AF_INET;
	Server_Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Server_Addr.sin_port = htons(SERVERPORT);
	retval = bind(Listen_Sock, (SOCKADDR *)&Server_Addr, sizeof(Server_Addr));
	if (retval == SOCKET_ERROR) {
		std::cout << "Error :: Bind Server Addr" << std::endl;
		return;
	}

	std::cout << "Server Initiated" << std::endl;

	ListeningThread = std::thread{ [this] { this->StartListen(); } };
}

void Server::StartListen()
{
	int retval = listen(Listen_Sock, SOMAXCONN);

	// accept()
	int addrlen = sizeof(SOCKADDR_IN);
	SOCKET clisock;
	SOCKADDR_IN cliaddr;
	std::cout << "Wait Client Accept" << std::endl;

	while (1) {
		clisock = accept(Listen_Sock, (SOCKADDR *)&cliaddr, &addrlen);
		if (clisock == INVALID_SOCKET) {
			std::cout << "Error :: Client Accept" << std::endl;
			return;
		}

		ClientInfo* clientinput = new ClientInfo();
		clientinput->Client_Sock = clisock;
		clientinput->Client_Addr = cliaddr;
		clientinput->IsAlive = true;
		clientinput->ID = ClientCounter++;
		clientinput->SetPosition(1, 1);
		inet_ntop(AF_INET, &cliaddr, clientinput->Client_IP, INET_ADDRSTRLEN);
		int timeoutval = 2000;
		//setsockopt(clientinput->Client_Sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutval, sizeof(timeoutval));

		SendClientInitData(clientinput);
		SendClientAccept(clientinput);

		clientinput->RecvThread = new std::thread{ [&] {this->RecvThreadFunc(clientinput); } };
		printf("Client %lld Thread Created\n", clientinput->Client_Sock);

		Client_list.insert(std::make_pair(clientinput->ID, clientinput));
		printf("Client Connected :: IP : %s, PORT : %d\n", clientinput->Client_IP, ntohs(clientinput->Client_Addr.sin_port));

		if(!ReadyToGo)	ReadyToGo = true;
	}
}

void Server::CloseServer()
{
	ListeningThread.join();
	
	for (auto client : Client_list)
	{
		client.second->RecvThread->join();
		closesocket(client.second->Client_Sock);
	}

	Client_list.clear();
	MsgQueue.clear();

	closesocket(Listen_Sock);

	WSACleanup();
}

void Server::RemoveClient(int ID)
{
	for (auto client : Client_list)
		if (client.second->ID == ID)
			client.second->IsAlive = false;
}

int Server::SendMsg(char* buf)
{
	for (auto client : Client_list)
		send(client.second->Client_Sock, buf, MSGSIZE, 0);

	return 0;
}

void Server::SendClientInitData(ClientInfo* acceptclient)
{
	char buf[MSGSIZE];
	buf[0] = MSGTYPE::HEARTBEAT;
	buf[1] = acceptclient->ID;
	buf[2] = acceptclient->x;
	buf[3] = acceptclient->y;
	buf[4] = 0;
	send(acceptclient->Client_Sock, buf, MSGSIZE, 0);

	for (auto client : Client_list)
	{
		buf[0] = MSGTYPE::ADDCLIENT;
		buf[1] = client.second->ID;
		buf[2] = client.second->x;
		buf[3] = client.second->y;
		buf[4] = 0;
		send(acceptclient->Client_Sock, buf, MSGSIZE, 0);
	}
}

void Server::SendClientAccept(ClientInfo* AccetpClient)
{
	char buf[MSGSIZE];

	for (auto client : Client_list) {
		buf[0] = MSGTYPE::ADDCLIENT;
		buf[1] = AccetpClient->ID;
		buf[2] = AccetpClient->x;
		buf[3] = AccetpClient->y;
		send(client.second->Client_Sock, buf, MSGSIZE, 0);
	}
}

void Server::AliveChecker()
{
	for (auto it = Client_list.begin(); it != Client_list.end();) {
		if (!it->second->IsAlive) {
			char buf[MSGSIZE];
			buf[0] = MSGTYPE::REMOVECLIENT;
			buf[1] = it->first;
			SendMsg(buf);
	
			closesocket(it->second->Client_Sock);
			delete it->second;
			std::cout << "Client Deleted" << std::endl;
			it = Client_list.erase(it);
		}
		else
			++it;
	}
}

void Server::RecvThreadFunc(ClientInfo* client)
{
	int retval = 0;

	while (1)
	{
		if (client)
		{
			char* buf = new char[MSGSIZE];
			retval = recvn(client->Client_Sock, buf, MSGSIZE, 0);
			if (retval == SOCKET_ERROR) {
				client->IsAlive = false;
				std::cout << client->ID << " Client Thread Terminated" << std::endl;
				return;
			}
			MsgQueueLocker.lock();
			MsgQueue.push_back(buf);
			MsgQueueLocker.unlock();
		}
	}
}
