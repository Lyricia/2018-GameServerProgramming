#include "stdafx.h"
#include "Renderer.h"
#include "Timer.h"
#include "Sound.h"
#include "Scene.h"

using namespace std;

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::buildScene()
{
	// Initialize Renderer
	m_Renderer = new Renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
	if (!m_Renderer->IsInitialized())
	{
		std::cout << "Renderer could not be initialized.. \n";
	}
	ChessBoard = m_Renderer->CreatePngTexture("Assets/Image/chess board.png");
}

void Scene::releaseScene()
{
	delete		m_Renderer;
	delete		g_Timer;
}


void Scene::keyinput(unsigned char key)
{
	switch (key)
	{
	case 'p':
	case 'P':
		if (GameStatus == GAMESTATUS::RUNNING)
		{
			GameStatus = GAMESTATUS::PAUSE;
		}
		else if (GameStatus == GAMESTATUS::PAUSE)
		{
			GameStatus = GAMESTATUS::RUNNING;
		}
		break;

	default:
		break;
	}
}

void Scene::keyspcialinput(int key)
{
	switch (key)
	{
	default:
		break;
	}
}

// 멀티태스킹을 가정하고 업 다운 글로벌을 가지고 있어야한다.
// 밖에서 누르고 안에서 업 할 수도 있기 때문에
void Scene::mouseinput(int button, int state, int x, int y)
{
	//if (y < 0) 
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && GAMESTATUS::RUNNING)
		{
			
		}
	}
}

void Scene::update()
{
	if (GameStatus == GAMESTATUS::RUNNING) 
	{
		
	}
	else if (GameStatus == GAMESTATUS::PAUSE)
	{
		g_Timer->getTimeset();
		double timeElapsed = g_Timer->getTimeElapsed();
	}

	else
	{
		g_Timer->getTimeset();
		double timeElapsed = g_Timer->getTimeElapsed();
	}
}

void Scene::render()
{
	m_Renderer->DrawTexturedRect(0, 0, 0, 820, 0.5f, 0.5f, 0.5f, 1.0f, ChessBoard, 0.9);

	if (GameStatus != GAMESTATUS::STOP)
	{


	}

	else if (GameStatus == GAMESTATUS::STOP)
	{

	}
}