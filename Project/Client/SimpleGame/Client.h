#pragma once
#define DEFAULTIP	"127.0.0.1"

class Scene;

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
	int					ClientID = 0;
	WCHAR				ClientName[10];
	int					ClientLevel = 1;
	int					ClientHP = 100;
	int					ClientExp = 0;
	Scene*				pScene = nullptr;

	SOCKET				Server_Sock;
	SOCKADDR_IN			Server_Addr;
	char				Server_IP[22];

	WSAEVENT			hWsaEvent;
	WSANETWORKEVENTS	netEvents;
	thread				recvThread;

	WSABUF				send_wsabuf;
	char 				send_buffer[MAX_BUFF_SIZE];
	WSABUF				recv_wsabuf;
	char				recv_buffer[MAX_BUFF_SIZE];
	char				packet_buffer[MAX_BUFF_SIZE];
	DWORD				in_packet_size = 0;
	int					saved_packet_size = 0;

public:
	Client();
	~Client();
	void InitClient();
	void StartClient();
	void CloseClient();
	void CompletePacket();
	
	void RegisterScene(Scene* s) { pScene = s; }
	void SendPacket(char* packet);
	WCHAR* getUserName() { return ClientName; }
	int getUserLevel() { return ClientLevel; }
	int getUserHP() { return ClientHP; }
	int getUserExp() { return ClientExp; }
	int getUserID() { return ClientID; }

	void setUserData(int level, int hp, int exp) { ClientLevel = level; ClientHP = hp; ClientExp = exp; }
};
