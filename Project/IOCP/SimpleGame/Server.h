#pragma once

#define SERVERPORT		4000
#define MSGSIZE			5
#define MAX_PACKET_SIZE 255
#define MAX_BUF_SIZE	4096
#define MAX_USER		10

class Scene;

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

	std::unordered_set<int>	viewlist;
	std::mutex				viewlist_mutex;
};


class Server
{
private:
	WSADATA				wsa;
	SOCKET				Listen_Sock;
	SOCKADDR_IN			Server_Addr;
	HANDLE				h_IOCP;

	ClientInfo			Clientlist[MAX_USER];

	std::thread			AccessThread;
	std::list<thread>	WorkerThreads;

	UINT			ClientCounter = 0;

	Scene*			m_pScene = NULL;

public:
	Server();
	~Server();

	void InitServer();
	void StartListen();
	void CloseServer();
	void SendPacket(int clientkey, void* packet);
	void RegisterScene(Scene* scene) { m_pScene = scene; }

	ClientInfo& GetClient(int id) { return Clientlist[id]; }
	ClientInfo* GetClientlist() { return Clientlist; }

	bool CanSee(int a, int b);

	static void WorkThreadProcess(Server* server);
};

