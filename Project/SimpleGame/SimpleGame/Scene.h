#pragma once
#include "Object.h"

#define MAX_OBJECT 100

class Timer;
class Sound;

class Scene
{
private:
	Renderer*	m_Renderer = nullptr;
	Timer*		g_Timer = nullptr;

	int			ChessBoard;
	int			ChessPiece;

	GAMESTATUS	GameStatus = GAMESTATUS::STOP;

	Sound		*m_Sound;
	int			m_SoundIdx[10] = {};

public:
	Scene();
	~Scene();
	void releaseScene();

	void buildScene();
	//void setRenderer(Renderer* g_render) { g_renderer = g_render; }
	void setTimer(Timer* t) { g_Timer = t; }

	void keyinput(unsigned char key);
	void keyspcialinput(int key);
	void mouseinput(int button, int state, int x, int y);

	GAMESTATUS GetGamestatus() { return GameStatus; }
	void SetGamestatus(GAMESTATUS s) { GameStatus = s; }

	void update();
	void render();
};