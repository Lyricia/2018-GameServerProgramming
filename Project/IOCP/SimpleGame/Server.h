#pragma once


#define SERVERPORT	9000
#define MSGSIZE		5

enum MSGTYPE
{
	HEARTBEAT,
	MOVE,
	REMOVECLIENT
};

enum enumOperation {Send, Recv};

struct stOverlappedEx 
{
	WSAOVERLAPPED	wsaOverlapped;
	WSABUF			wsaBuf;
	unsigned char	IOCPbuf[MSGSIZE]; 
	unsigned char	packet_buf[MSGSIZE];
	int				nRemainLen; 
	enumOperation	eOperation; 
	int				receiving_packet_size, received;
};

struct ClientInfo
{
	int				ID = NULL;
	SOCKET			Client_Sock;

	WSABUF			stbuf;
	char			buf[MSGSIZE];
	stOverlappedEx	OverlappedEx;
};


class Server
{
private:
	WSADATA			wsa;
	SOCKET			Listen_Sock;
	SOCKADDR_IN		Server_Addr;
	HANDLE			h_IOCP;

	std::map<UINT, ClientInfo*> Client_list;
	std::list<thread>		WorkerThreads;
	std::list<char*>		MsgQueue;

	bool			ReadyToGo = false;
	UINT			ClientCounter = 0;

	OVERLAPPED		Overlapped;

public:

	Server();
	~Server();

	void InitServer();
	void StartListen();
	void CloseServer();
	bool IsReady() { return ReadyToGo; };

	static void WorkThreadProcess(Server* server);

	std::list<char*> GetMsgqueue() { return MsgQueue; }
};

