#include "stdafx.h"
#include "Scene.h"
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
	for (UINT i = 0; i < std::thread::hardware_concurrency(); i++)
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
	retval = ::bind(Listen_Sock, (SOCKADDR *)&Server_Addr, sizeof(Server_Addr));
	if (retval == SOCKET_ERROR) {
		std::cout << "Error :: Bind Server Addr" << std::endl;
		return;
	}
	
	std::cout << "Server Initiated" << std::endl;
}

void Server::StartListen()
{
	int retval = listen(Listen_Sock, SOMAXCONN);
	std::cout << "Waiting..." << std::endl;

	while (1) {
		SOCKADDR_IN ClientAddr;
		ZeroMemory(&ClientAddr, sizeof(SOCKADDR_IN));
		ClientAddr.sin_family = AF_INET;
		ClientAddr.sin_port = htons(SERVERPORT);
		ClientAddr.sin_addr.s_addr = INADDR_ANY;
		int addrlen = sizeof(SOCKADDR_IN);

		SOCKET ClientAcceptSocket = WSAAccept(Listen_Sock, (SOCKADDR*)&ClientAddr, &addrlen, NULL, NULL);
		if (ClientAcceptSocket == INVALID_SOCKET) {
			std::cout << "Error :: Client Accept" << std::endl;
			return;
		}
		else
			std::cout << "Client Accepted" << std::endl;
		
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

		Clientlist[ClientKey].Client_Sock = ClientAcceptSocket;
		Clientlist[ClientKey].x = 4;
		Clientlist[ClientKey].y = 4;
		ZeroMemory(&Clientlist[ClientKey].OverlappedEx.wsaOverlapped, sizeof(WSAOVERLAPPED));

		CreateIoCompletionPort(
			reinterpret_cast<HANDLE*>(ClientAcceptSocket),
			h_IOCP,
			ClientKey,
			0
		);

		printf("Client %d Connected\n", ClientKey);
		
		DWORD RecvByte = 0;
		DWORD flag = 0;

		Clientlist[ClientKey].inUse = true;
		Clientlist[ClientKey].viewlist.clear();

		WSARecv(
			Clientlist[ClientKey].Client_Sock,
			&Clientlist[ClientKey].OverlappedEx.wsaBuf,
			1, 
			NULL,
			&flag, 
			&Clientlist[ClientKey].OverlappedEx.wsaOverlapped,
			NULL
		);


		sc_packet_put_player p;
		p.id = ClientKey;
		p.size = sizeof(sc_packet_put_player);
		p.type = SC_PUT_PLAYER;
		p.x = Clientlist[ClientKey].x;
		p.y = Clientlist[ClientKey].y;
		// to all players
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == Clientlist[i].inUse) {
				if (!CanSee(i, ClientKey))
					continue;

				Clientlist[i].viewlist_mutex.lock();
				Clientlist[i].viewlist.insert(ClientKey);
				Clientlist[i].viewlist_mutex.unlock();
				SendPacket(i, &p);
			}
		}
		// to me
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (i != ClientKey && true == Clientlist[i].inUse)
			{
				if (!CanSee(ClientKey, i))
					continue;

				Clientlist[i].viewlist_mutex.lock();
				Clientlist[ClientKey].viewlist.insert(i);
				Clientlist[i].viewlist_mutex.unlock();

				p.id = i;
				p.x = Clientlist[i].x;
				p.y = Clientlist[i].y;

				SendPacket(ClientKey, &p);
			}
		}
	}
}

void Server::CloseServer()
{
	for (auto& t : WorkerThreads)
		t.join();

	closesocket(Listen_Sock);

	WSACleanup();
}

void Server::SendPacket(int clientkey, void * packet)
{
	stOverlappedEx* o = new stOverlappedEx();
	char* p = reinterpret_cast<char*>(packet);
	memcpy(o->io_Buf, packet, p[0]);
	o->eOperation = op_Send;
	o->wsaBuf.buf = o->io_Buf;
	o->wsaBuf.len = p[0];
	ZeroMemory(&o->wsaOverlapped, sizeof(WSAOVERLAPPED));

	WSASend(Clientlist[clientkey].Client_Sock, &o->wsaBuf, 1, NULL, 0, &o->wsaOverlapped, NULL);
}

void Server::SendPutObject(int client, int objid)
{
	sc_packet_put_player p;
	p.id = objid;
	p.size = sizeof(p);
	p.type = SC_PUT_PLAYER;
	p.x = Clientlist[objid].x;
	p.y = Clientlist[objid].y;

	SendPacket(client, &p);
}

void Server::SendRemoveObject(int client, int objid)
{
	sc_packet_remove_player p;
	p.id = objid;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	SendPacket(client, &p);
}

void Server::WorkThreadProcess(Server* server)
{
	unsigned long datasize;
	unsigned long long key;
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
			std::cout << "Client " << key << " Disconnected" << std::endl;
			server->DisConnectClient(key);
			continue;
		}
		else if (datasize == 0)
		{
			std::cout << "Client " << key << " Disconnected" << std::endl;
			server->DisConnectClient(key);
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

					server->m_pScene->ProcessPacket(key, cl->prev_packet);

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

bool Server::CanSee(int a, int b)
{
	int distance =
		(Clientlist[a].x - Clientlist[b].x) * (Clientlist[a].x - Clientlist[b].x) +
		(Clientlist[a].y - Clientlist[b].y) * (Clientlist[a].y - Clientlist[b].y);

	return distance <= VIEW_RADIUS * VIEW_RADIUS;
}

void Server::DisConnectClient(int key)
{
	closesocket(Clientlist[key].Client_Sock);

	sc_packet_remove_player p;
	p.id = key;
	p.size = sizeof(sc_packet_remove_player);
	p.type = SC_REMOVE_PLAYER;
	for (int id : Clientlist[key].viewlist)
	{
		if (Clientlist[id].inUse == true)
		{
			if (Clientlist[id].viewlist.count(key) != 0) {
				SendPacket(id, &p);
			}
		}
	}
	Clientlist[key].viewlist.clear();
	Clientlist[key].inUse = false;
	m_pScene->RemovePlayerOnBoard(Clientlist[key].x, Clientlist[key].y);
}
