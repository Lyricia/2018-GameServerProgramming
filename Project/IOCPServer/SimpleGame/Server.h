#pragma once

class Scene;

enum MSGTYPE
{
	HEARTBEAT,
	MOVE,
	REMOVECLIENT
};

enum enumOperation {
	op_Send, op_Recv, op_Move,
	db_login, db_logout
};

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

class CObject{
public:
	int				ID = NULL;

	std::unordered_set<int>	viewlist;
	std::mutex				viewlist_mutex;
};

class CNPC :public CObject {
public:
	bool			inUse;
	bool			bActive;

	SHORT			x, y;
};

class CClient : public CNPC {
public:
	stOverlappedEx	OverlappedEx;
	SOCKET			Client_Sock;

	int				packetsize;
	int				prev_packetsize;
	char			prev_packet[MAX_PACKET_SIZE];
};

struct sEvent 
{
	long long		startTime = 0;
	enumOperation	operation;
	UINT			id;
	UINT			IOCPKey = -1;
	void*			data = nullptr;
};

struct cmp{
    bool operator()(sEvent t, sEvent u){
        return t.startTime > u.startTime;
    }
};

struct DBUserData {
	UINT Key;
	SQLINTEGER 
		nID, 
		nCHAR_LEVEL, 
		nPosX, 
		nPosY, 
		nHP,
		nExp;
};

class Server
{
private:
	WSADATA				wsa;
	SOCKET				Listen_Sock;
	SOCKADDR_IN			Server_Addr;
	HANDLE				h_IOCP;

	SQLHENV				h_env;
	SQLHDBC				h_dbc;
	SQLHSTMT			h_stmt = 0;

	//ClientInfo			Clientlist[NUM_OF_NPC];

	CNPC				NPCList[NUM_OF_NPC];
	CClient				ClientArr[MAX_USER];

	std::thread			AccessThread;
	std::thread			TimerThread;
	std::thread			DBThread;
	std::list<thread>	WorkerThreads;

	std::priority_queue<sEvent, vector<sEvent>, cmp>	TimerEventQueue;
	std::mutex					TimerEventMutex;

	std::queue<sEvent>			DBEventQueue;
	std::mutex					DBEventMutex;

	UINT			ClientCounter = 0;

	Scene*			m_pScene = NULL;

	std::mutex					m_SpaceMutex[SPACE_X * SPACE_Y];
	std::unordered_set<int>		m_Space[SPACE_X * SPACE_Y];

public:
	Server();
	~Server();

	void InitServer();
	void InitDB();
	void InitObjectList();

	void StartListen();
	void CloseServer();
	void RegisterScene(Scene* scene) { m_pScene = scene; }

	void SendPacket(int clientkey, void* packet);
	void SendPutObject(int client, int objid);
	void SendRemoveObject(int client, int objid);

	//ClientInfo& GetClient(int id) { return Clientlist[id]; }
	//ClientInfo* GetClientlist() { return Clientlist; }


	CClient& GetClient_(int id) { return ClientArr[id]; }
	CClient* GetClientArr() { return ClientArr; }
	CNPC& GetNPC(int id) { return NPCList[id]; }
	CNPC* GetNPClist() { return NPCList; }



	void AddTimerEvent(UINT id, enumOperation op, long long time);
	void AddDBEvent(UINT IOCPKey, UINT id, enumOperation op);

	long long GetTime();

	bool CanSee(int a, int b);
	void DisConnectClient(int key);
	void MoveNPC(int key);

	std::unordered_set<int>& GetSpace(int idx) { return m_Space[idx];}
	std::mutex& GetSpaceMutex(int idx) { return m_SpaceMutex[idx]; }
	int GetSpaceIndex(int id) {
		CNPC* obj = nullptr;
		if (id >= NPC_START)		obj = &NPCList[id];
		else if (id < NPC_START)	obj = &ClientArr[id];
		return (obj->x / SPACESIZE) + (obj->y / SPACESIZE) * SPACE_X;
		//return (Clientlist[id].x / SPACESIZE) + (Clientlist[id].y / SPACESIZE) * SPACE_X;
	}

	bool ChkInSpace(int clientid, int targetid);

	void WorkThreadProcess();
	void TimerThreadProcess();
	void DBThreadProcess();

	void DoTest();
};