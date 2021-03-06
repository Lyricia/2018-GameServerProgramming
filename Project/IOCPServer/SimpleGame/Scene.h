#pragma once

#define INVALID -1

class Timer;
class Server;
class Object;
struct ClientInfo;

class CClient;
class CNPC;

class Scene
{
private:
	Timer*		g_Timer = nullptr;

	GAMESTATUS	GameStatus = GAMESTATUS::STOP;

	int			m_Board[BOARD_WIDTH][BOARD_HEIGHT] = {-1};

	Server*			m_Server;
	ClientInfo*		m_pClientlist;

	CClient*		m_pClientArr;
	CNPC*			m_pNPCList;

public:
	Scene();
	~Scene();
	void releaseScene();

	void SetServer(Server* serv) { m_Server = serv; }

	void buildScene();
	bool isCollide(int x, int y);
	void setTimer(Timer* t) { g_Timer = t; }

	GAMESTATUS GetGamestatus() { return GameStatus; }
	void SetGamestatus(GAMESTATUS s) { GameStatus = s; }

	void ProcessPacket(int id, unsigned char* packet);
	void MoveObject(int clientid, int oldSpaceIdx);
	void MoveByCoord(int x, int y, int clientid, int oldSpaceIdx);
	void AttackObject(int objectid, int att_range, int targetid = INVALID);
	void SetSector(CNPC& npc);

	void RemovePlayerOnBoard(const int x, const int y) { m_Board[x][y] = INVALID; }
	void update();
	void ReadGroundPos();
};