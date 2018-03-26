#pragma once
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32")

#define SERVERPORT	9000
#define MSGSIZE		5


enum MSGTYPE
{
	HEARTBEAT,
	MOVE,
	ADDCLIENT,
	REMOVECLIENT
};

class Client
{
	WSADATA				wsa;
	int					ID;

	SOCKET				Server_Sock;
	SOCKADDR_IN			Server_Addr;
	char				Server_IP[INET_ADDRSTRLEN];

	std::thread			RecvThread;

	Vec3i				Position;
public:
	std::list<char*>		MsgQueue;

	Client();
	~Client();
	void InitClient();
	void StartClient();
	void CloseClient();
	int SendMsg(int id, int x, int y);
	int SendMsg(char* buf);
	void SendHeartBeat();

	void SetPosition(int x, int y) { Position.x = x, Position.y = y; }
	Vec3i GetPosition() { return Position; }
	void SetID(int _ID) { ID = _ID; };
	int GetID() { return ID; }
};

int recvn(SOCKET s, char *buf, int len, int flags);
void RecvThreadFunc(SOCKET clientsock, std::list<char*> *MsgQueue);
