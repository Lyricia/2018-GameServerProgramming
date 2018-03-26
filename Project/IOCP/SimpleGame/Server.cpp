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

	//Listen_Sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
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
		ZeroMemory(&Overlapped, sizeof(Overlapped));

		clientinput->Client_Sock = clisock;
		clientinput->ID = ClientCounter++;
		clientinput->stbuf.len = MSGSIZE;
		clientinput->stbuf.buf = clientinput->buf;

		CreateIoCompletionPort((HANDLE)clientinput->Client_Sock, h_IOCP, clientinput->ID, 0);

		Client_list.insert(std::make_pair(clientinput->ID, clientinput));
		printf("Client %d Connected", clientinput->ID);
		
		DWORD RecvByte = 0;
		DWORD flag = 0;

		WSARecv(clientinput->Client_Sock, &clientinput->stbuf, 1, NULL, &flag, &Overlapped, NULL);
	}
}

void Server::CloseServer()
{
	for (auto& t : WorkerThreads)
		t.join();

	Client_list.clear();
	MsgQueue.clear();

	closesocket(Listen_Sock);

	WSACleanup();
}

void Server::WorkThreadProcess(Server* server)
{
	DWORD msgbyte = 0;
	ULONG keyinput = NULL;
	while (1)
	{
		if (GetQueuedCompletionStatus(server->h_IOCP, &msgbyte, reinterpret_cast<PULONG_PTR>(&keyinput), reinterpret_cast<LPOVERLAPPED*>(&server->Overlapped), INFINITE) == false)
		{
			std::cout << "Error :: GetQueuedCompletionStatus" << std::endl;
			return;
		}
		else if (msgbyte == 0)
		{
			std::cout << "Error :: recvbyte 0" << std::endl;
			continue;
		}
		else
		{
			int a = 0;

		}

		//unsigned char *buf_ptr = pOverlappedEx->m_IOCPbuf;
		//int restDataSize = dwIoSize;
		//while (restDataSize) {
		//	if (0 == pOverlappedEx->receiving_packet_size) // 패킷사이즈 0 이면?
		//		pOverlappedEx->receiving_packet_size = (int)buf_ptr[0];

		//	int required = pOverlappedEx->receiving_packet_size - pOverlappedEx->received;	

		//	if (restDataSize < required) { // 더이상패킷을만들수없다. 루프를중지한다.
		//		memcpy(pOverlappedEx->m_packet_buf + pOverlappedEx->received,
		//			buf_ptr, restDataSize);
		//		pOverlappedEx->received += restDataSize;
		//		break;
		//	}
		//	else { // 패킷을완성할수있다.
		//		memcpy(pOverlappedEx->m_packet_buf + pOverlappedEx->received,
		//			buf_ptr, required);
		//		bool ret = PacketProcess(pOverlappedEx->m_packet_buf, pClientInfo);
		//		pOverlappedEx->received = 0;
		//		restDataSize -= required;
		//		buf_ptr += required;
		//		pOverlappedEx->receiving_packet_size = 0;
		//	}
		//}
		//BindRecv(pClientInfo);
	}
}