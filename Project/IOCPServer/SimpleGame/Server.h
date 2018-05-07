#pragma once

class Scene;

enum MSGTYPE
{
	HEARTBEAT,
	MOVE,
	REMOVECLIENT
};

enum enumOperation {op_Send, op_Recv, op_Move};

struct stOverlappedEx 
{
	WSAOVERLAPPED	wsaOverlapped;
	WSABUF			wsaBuf;
	char			io_Buf[MAX_BUFF_SIZE];
	enumOperation	eOperation; 
};

struct ClientInfo
{
	stOverlappedEx	OverlappedEx;
	SOCKET			Client_Sock;
	int				ID = NULL;

	bool			inUse;
	bool			bActive;
	int				packetsize;
	int				prev_packetsize;
	char			prev_packet[MAX_PACKET_SIZE];
	SHORT			x, y;

	std::unordered_set<int>	viewlist;
	std::mutex				viewlist_mutex;
};

struct NPCInfo {
	int				ID = NULL;
	SHORT			x, y;
	bool			inUse;
};

struct sEvent 
{
	long long		startTime = 0;
	enumOperation	operation;
	UINT			id;
};

struct cmp{
    bool operator()(sEvent t, sEvent u){
        return t.startTime > u.startTime;
    }
};

class Server
{
private:
	WSADATA				wsa;
	SOCKET				Listen_Sock;
	SOCKADDR_IN			Server_Addr;
	HANDLE				h_IOCP;

	ClientInfo			Clientlist[NUM_OF_NPC];

	std::thread			AccessThread;
	std::thread			TimerThread;
	std::list<thread>	WorkerThreads;

	std::priority_queue<sEvent, vector<sEvent>, cmp>	EventQueue;
	std::mutex					EventMutex;

	UINT			ClientCounter = 0;

	Scene*			m_pScene = NULL;

public:
	Server();
	~Server();

	void InitServer();
	void StartListen();
	void CloseServer();
	void RegisterScene(Scene* scene) { m_pScene = scene; }

	void SendPacket(int clientkey, void* packet);
	void SendPutObject(int client, int objid);
	void SendRemoveObject(int client, int objid);

	ClientInfo& GetClient(int id) { return Clientlist[id]; }
	ClientInfo* GetClientlist() { return Clientlist; }
	void AddTimer(UINT id, enumOperation op, long long time);

	long long GetTime();

	bool CanSee(int a, int b);
	void DisConnectClient(int id);
	void MoveNPC(int key);

	static void WorkThreadProcess(Server* server);
	void TimerThreadProcess();
};

