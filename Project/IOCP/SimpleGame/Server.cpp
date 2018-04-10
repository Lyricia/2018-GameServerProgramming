#include "stdafx.h"
#include "Server.h"

Server::Server()
{
}

Server::~Server()
{
}

void Server::InitServer()
{
	for (auto & client : Clientlist) {
		client.OverlappedEx.eOperation = op_Recv;
		client.OverlappedEx.wsaBuf.buf = client.OverlappedEx.io_Buf;
		client.OverlappedEx.wsaBuf.len = sizeof(client.OverlappedEx.io_Buf);
		client.inUse = false;
		client.packetsize = 0;
		client.prev_packetsize = 0;
		client.ID = -1;
	}

	int retval;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	// IOCP
	h_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (h_IOCP == NULL) {
		std::cout << "Error :: init IOCP" << std::endl;
		return;
	}

	// ThreadPool
	for (int i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		WorkerThreads.emplace_back(std::thread{ WorkThreadProcess, this });
	}

	Listen_Sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
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
}

void Server::StartListen()
{
	int retval = listen(Listen_Sock, SOMAXCONN);

	// accept()
	int addrlen = sizeof(SOCKADDR_IN);
	SOCKET clsock;
	SOCKADDR_IN cliaddr;
	std::cout << "Wait Client Accept" << std::endl;

	while (1) {
		clsock = WSAAccept(Listen_Sock, (SOCKADDR *)&cliaddr, &addrlen, NULL, NULL);
		if (clsock == INVALID_SOCKET) {
			std::cout << "Error :: Client Accept" << std::endl;
			return;
		}
		
		int ClientKey = -1;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (Clientlist[i].inUse == false) {
				ClientKey = i;
				break;
			}
		}
		if (-1 == ClientKey) {
			cout << "Max User Accepted" << endl;
			continue;
		}

		ClientInfo* clientinput = new ClientInfo();
		clientinput->Client_Sock = clsock;
		clientinput->ID = ClientCounter++;
		ZeroMemory(&Clientlist[ClientKey].OverlappedEx.wsaOverlapped, sizeof(WSAOVERLAPPED));

		CreateIoCompletionPort(
			reinterpret_cast<HANDLE*>(clsock),
			h_IOCP,
			ClientKey,
			0
		);

		printf("Client %d Connected", clientinput->ID);
		
		DWORD RecvByte = 0;
		DWORD flag = 0;

		clientinput->inUse = true;

		WSARecv(
			clientinput->Client_Sock, 
			&clientinput->OverlappedEx.wsaBuf, 
			1, 
			NULL,
			&flag, 
			&clientinput->OverlappedEx.wsaOverlapped, 
			NULL
		);

	}
}

void Server::CloseServer()
{
	for (auto& t : WorkerThreads)
		t.join();

	//Client_list.clear();
	MsgQueue.clear();

	closesocket(Listen_Sock);

	WSACleanup();
}

void Server::WorkThreadProcess(Server* server)
{
	unsigned long datasize;
	unsigned long long key;
	WSAOVERLAPPED* pOverlapped;
	stOverlappedEx* oEx;

	while (1)
	{
		bool isSuccess = GetQueuedCompletionStatus(
			server->h_IOCP,
			&datasize,
			&key,
			reinterpret_cast<LPOVERLAPPED*>(&oEx),
			INFINITE
		);

		if (isSuccess == 0)
		{
			std::cout << "Error :: GetQueuedCompletionStatus" << std::endl;
			return;
		}
		else if (datasize == 0)
		{
			std::cout << "Error :: recvbyte 0" << std::endl;
			continue;
		}

		if (op_Recv == oEx->eOperation)
		{
			int r_size = datasize;
			char* ptr = oEx->io_Buf;
			ClientInfo* cl = &server->Clientlist[key];
			
			while (0 < r_size)
			{
				if (cl->packetsize == 0)
					cl->packetsize = ptr[0];

				int remainsize = cl->packetsize - cl->prev_packetsize;

				if (remainsize <= r_size) {
					memcpy(cl->prev_packet + cl->prev_packetsize, ptr, remainsize);
					//ProcessPacket(key, cl->prev_packet);
					r_size -= remainsize;
					ptr += remainsize;
					cl->packetsize = 0;
					cl->prev_packetsize = 0;
				}
				else {
					memcpy(cl->prev_packet + cl->prev_packetsize, ptr, r_size);
					cl->prev_packetsize += r_size;
				}

			}
			unsigned long rFlag = 0;
			ZeroMemory(&oEx->wsaOverlapped, sizeof(OVERLAPPED));
			WSARecv(cl->Client_Sock, &oEx->wsaBuf, 1, NULL, &rFlag, &oEx->wsaOverlapped, NULL);
		}
		else
		{
			delete oEx;
		}
	}
}