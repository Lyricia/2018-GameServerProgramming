#pragma once


#define SERVERPORT	9000
#define MSGSIZE		5

enum MSGTYPE
{
	HEARTBEAT,
	MOVE,
	ADDCLIENT,
	REMOVECLIENT
};

struct ClientInfo
{
	int				ID = NULL;
	SOCKET			Client_Sock;
	SOCKADDR_IN		Client_Addr;
	char			Client_IP[INET_ADDRSTRLEN];

	bool			IsAlive = false;
	std::thread*	RecvThread;

	int x, y;
	void SetPosition(int _x, int _y) { x = _x, y = _y; }
	Vec3i GetPosition() { return Vec3i{ x, y, 0 }; }
};

class Server
{
private:
	WSADATA			wsa;
	SOCKET			Listen_Sock;
	SOCKADDR_IN		Server_Addr;

	std::map<UINT, ClientInfo*> Client_list;
	std::list<thread*>		RecvThread_list;
	std::thread				ListeningThread;

	bool			ReadyToGo = false;
	UINT			ClientCounter;
public:
	std::list<char*>		MsgQueue;

	Server();
	~Server();

	void InitServer();
	void StartListen();
	void CloseServer();
	bool IsReady() { return ReadyToGo; };
	
	void AliveChecker();
	void RemoveClient(int ID);

	int SendMsg(char* buf);
	void SendClientInitData(ClientInfo* acceptclient);
	void SendClientAccept(ClientInfo* AccetpClient);

	void RecvThreadFunc(ClientInfo* client);

	ClientInfo* getClient(int id) { return Client_list.find(id)->second; }
};

int recvn(SOCKET s, char *buf, int len, int flags);

