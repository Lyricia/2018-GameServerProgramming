#pragma once
#include "Object.h"

#define PIECESIZE 80
#define INVALID -1

class Timer;
class Server;
class ClientInfo;

class Scene
{
private:
	Timer*		g_Timer = nullptr;

	int			ChessBoard;
	int			ChessPiece;

	GAMESTATUS	GameStatus = GAMESTATUS::STOP;

	Object*		m_Target = nullptr;
	int			m_Board[9][9] = {-1};
	
	vector<Object*> m_Playerlist;

	Server*			m_Server;
	ClientInfo*		m_pClientlist;

public:
	Scene();
	~Scene();
	void releaseScene();

	void SetServer(Server* serv) { m_Server = serv; }

	void buildScene();
	void InitPieces(vector<Object*>& pieceset, TEAM side);
	int PieceChk(int x, int y);
	void setTimer(Timer* t) { g_Timer = t; }

	GAMESTATUS GetGamestatus() { return GameStatus; }
	void SetGamestatus(GAMESTATUS s) { GameStatus = s; }

	void ProcessPacket(int id, char* packet);

	void update();

	//void SendToClient();
};