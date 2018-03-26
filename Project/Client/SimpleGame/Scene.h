#pragma once
#include "Object.h"

#define PIECESIZE 80

class Timer;
class Sound;
class Client;

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
	int			m_Board[9][9]{};

	vector<Object*>		m_BlackPiece;
	vector<Object*>		m_WhitePiece;

	Client*				m_Client;

	list<Object*>		m_Players;
	Object*				Client_Object;

public:
	Scene();
	~Scene();
	void releaseScene();

	void buildScene();
	void InitPieces(vector<Object*>& pieceset, TEAM side);
	int PieceChk(int x, int y);
	void setTimer(Timer* t) { g_Timer = t; }

	void setClient(Client* cli) { m_Client = cli; }

	void keyinput(unsigned char key);
	void keyspcialinput(int key);
	void mouseinput(int button, int state, int x, int y);

	GAMESTATUS GetGamestatus() { return GameStatus; }
	void SetGamestatus(GAMESTATUS s) { GameStatus = s; }

	void update();
	void render();

	void ProcessMsg();
};