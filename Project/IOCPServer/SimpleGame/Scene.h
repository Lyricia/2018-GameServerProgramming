#pragma once
#include "Object.h"

#define PIECESIZE 80
#define INVALID -1

class Timer;
class Server;
struct ClientInfo;

class Scene
{
private:
	Timer*		g_Timer = nullptr;

	int			ChessBoard;
	int			ChessPiece;

	GAMESTATUS	GameStatus = GAMESTATUS::STOP;

	Object*		m_Target = nullptr;
	int			m_Board[BOARD_WIDTH][BOARD_HEIGHT] = {-1};

	Server*			m_Server;
	ClientInfo*		m_pClientlist;

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

	void ProcessPacket(int id, char* packet);

	void RemovePlayerOnBoard(const int x, const int y) { 
		m_Board[x][y] = INVALID;
	}
	void update();
};