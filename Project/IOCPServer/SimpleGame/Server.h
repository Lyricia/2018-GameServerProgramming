#pragma once

class Scene;

void DisplayLuaError(lua_State* L, int error);

enum MSGTYPE
{
	HEARTBEAT,
	MOVE,
	REMOVECLIENT
};

enum enumOperation {
	op_Send, op_Recv, op_Move,
	db_login, db_logout,
	npc_player_move, npc_bye, npc_respawn,
	pc_heal
};

enum ServerOperationMode {
	MODE_NORMAL = 1,
	MODE_TEST_NORMAL,
	MODE_TEST_HOTSPOT
};

struct stOverlappedEx 
{
	WSAOVERLAPPED	wsaOverlapped;
	WSABUF			wsaBuf;
	char			io_Buf[MAX_BUFF_SIZE];
	enumOperation	eOperation;
	int				EventTarget;
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
	unsigned char	prev_packet[MAX_PACKET_SIZE];
	SHORT			x, y;

	std::unordered_set<int>	viewlist;
	std::mutex				dviewlist_mutex;
};

class CObject{
public:
	int				ID = NULL;
};

class CNPC :public CObject {
public:
	bool			inUse;
	bool			bActive;

	WORD			hp;
	SHORT			x, y;
	BYTE			level;
	lua_State*		L;
	ObjType			ObjectType;
	DWORD			exp;

	void getDamaged(int damage, int attacker) {
		int error = lua_getglobal(L, "event_get_Damaged");
		lua_pushnumber(L, damage);
 		lua_pushnumber(L, attacker);
		error = lua_pcall(L, 2, 1, 0);
		hp = lua_tointeger(L, lua_gettop(L));
	}
	bool isDead() {
		return hp <= 0;
	}
};

class CClient : public CNPC {
public:
	stOverlappedEx	OverlappedEx;
	SOCKET			Client_Sock;
	WCHAR			UserName[10] = {};

	std::unordered_set<int>	viewlist;
	std::mutex				viewlist_mutex;

	int				packetsize;
	int				prev_packetsize;
	unsigned char	prev_packet[MAX_PACKET_SIZE];
	
	DWORD			exp;
	DWORD			explimit = 100;

	long long		lastattacktime = 0;
	char			attackrange = 1;

	void getHealed() {
		if (hp < level * 100)
			hp += level * 10;
		else
			hp = level * 100;
	}

	void getDamaged(int damage, int attacker) {
		if (hp < damage) {
			hp = 0;
			return;
		}
		else {
			hp -= damage;
		}
	}

	void EarnEXP(int _exp) {
		exp += _exp;
		while (true) {
			if (exp > explimit) {
				level++;
				if (level % 50 == 0)
					attackrange++;
				if (level > 254) {
					level = 254;
					hp = 25500;
				}
				else
					hp = level * 100;
				exp -= explimit;
				if (exp < 0) exp = 0;
				if (explimit >= MAXDWORD) {
					explimit = MAXDWORD;
				}
				else
					explimit *= 2;
			}
			else {
				break;
			}
		}
	}
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
	SQLWCHAR	nName[10];
};

class Server
{
private:
	int					Mode;

	WSADATA				wsa;
	SOCKET				Listen_Sock;
	SOCKADDR_IN			Server_Addr;
	HANDLE				h_IOCP;

	SQLHENV				h_env;
	SQLHDBC				h_dbc;
	SQLHSTMT			h_stmt = 0;

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
	void CreateConnection(UINT clientkey);

	void SendPacket(int clientkey, void* packet);
	void SendPacketToViewer(int clientkey, void* packet);
	void SendPutObject(int client, int objid);
	void SendRemoveObject(int client, int objid);
	void SendChatPacket(int to, WCHAR * message);
	void SendChatToAll(WCHAR * message);
	void SendStatusPacket(int clientkey);

	CClient& GetClient(int id) { return ClientArr[id]; }
	CClient* GetClientArr() { return ClientArr; }
	CNPC& GetNPC(int id) { return NPCList[id]; }
	CNPC* GetNPClist() { return NPCList; }
	HANDLE GetIOCP() { return h_IOCP; }
	Scene* GetScene() { return m_pScene; }


	void AddTimerEvent(UINT id, enumOperation op, long long time);
	void AddDBEvent(UINT IOCPKey, UINT id, enumOperation op);
	void AddDBEvent(UINT IOCPKey, WCHAR* str, enumOperation op);

	bool CanSee(int a, int b, int range = VIEW_RADIUS);
	void DisConnectClient(int key);
	void MoveNPC(int key, int dir = -1);
	void RemoveNPC(int key, std::unordered_set<int>& viewlist);

	int GetSector(int idx);
	std::unordered_set<int>& GetSpace(int idx) { return m_Space[idx];}
	std::mutex& GetSpaceMutex(int idx) { return m_SpaceMutex[idx]; }
	int GetSpaceIndex(int id) {
		CNPC* obj = nullptr;
		if (id >= NPC_START)		obj = &NPCList[id];
		else if (id < NPC_START)	obj = &ClientArr[id];
		int res =  (obj->x / SPACESIZE) + (obj->y / SPACESIZE) * SPACE_X;
		if (res > 1600) {
			int a = 0;
		}
		return res;
	}

	bool ChkInSpace(int clientid, int targetid);

	void WorkThreadProcess();
	void TimerThreadProcess();
	void DBThreadProcess();

	void DoTest();
	void ReadServerGround();
};