#pragma once
#include "Object.h"

#define PIECESIZE 80
#define INVALID -1

class Timer;
class Sound;
class Server;

class Scene
{
private:
	Renderer*	m_Renderer = nullptr;
	Timer*		g_Timer = nullptr;

	int			ChessBoard;
	int			ChessPiece;

	GAMESTATUS	GameStatus = GAMESTATUS::STOP;

	Sound*		m_Sound;
	int			m_SoundIdx[10]{};
	Object*		m_Target = nullptr;
	int			m_Board[9][9] = {-1};

	vector<Object*>		m_BlackPiece;
	vector<Object*>		m_WhitePiece;

	Server*		m_Server;

public:
	Scene();
	~Scene();
	void releaseScene();

	void SetServer(Server* serv) { m_Server = serv; }

	void buildScene();
	void InitPieces(vector<Object*>& pieceset, TEAM side);
	int PieceChk(int x, int y);
	void setTimer(Timer* t) { g_Timer = t; }

	void keyinput(unsigned char key);
	void keyspcialinput(int key);
	void mouseinput(int button, int state, int x, int y);

	GAMESTATUS GetGamestatus() { return GameStatus; }
	void SetGamestatus(GAMESTATUS s) { GameStatus = s; }

	void update();
	void render();

	void ProcessMsg();
	//void SendToClient();
};