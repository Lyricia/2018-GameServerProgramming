#pragma once


#define SERVERPORT		9000
#define MSGSIZE			5
#define MAX_PACKET_SIZE 255
#define MAX_BUF_SIZE	4096
#define MAX_USER		10

enum MSGTYPE
{
	HEARTBEAT,
	MOVE,
	REMOVECLIENT
};

enum enumOperation {op_Send, op_Recv};

struct stOverlappedEx 
{
	WSAOVERLAPPED	wsaOverlapped;
	WSABUF			wsaBuf;
	char			io_Buf[MAX_BUF_SIZE];
	enumOperation	eOperation; 
};

struct ClientInfo
{
	stOverlappedEx	OverlappedEx;
	SOCKET			Client_Sock;
	int				ID = NULL;

	bool			inUse;
	int				packetsize;
	int				prev_packetsize;
	char			prev_packet[MAX_PACKET_SIZE];
	CHAR			x, y;
};


class Server
{
private:
	WSADATA				wsa;
	SOCKET				Listen_Sock;
	SOCKADDR_IN			Server_Addr;
	HANDLE				h_IOCP;

	ClientInfo			Clientlist[MAX_USER];

	std::list<thread>		WorkerThreads;
	std::list<char*>		MsgQueue;

	bool			ReadyToGo = false;
	UINT			ClientCounter = 0;

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

