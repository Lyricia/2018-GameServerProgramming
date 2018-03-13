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
	ChessPiece = m_Renderer->CreatePngTexture("Assets/Image/chess piece.png");

	InitPieces(m_BlackPiece, TEAM::BLACK);
	InitPieces(m_WhitePiece, TEAM::WHITE);

}

void Scene::InitPieces(vector<Object*>& pieceset, TEAM Side)
{
	Vec3f pos;
	pieceset.reserve(16);
	for (int i = 0; i < 16; ++i)
	{
		Object* piece = new Object();
		piece->setTeam(Side);
		if (Side == TEAM::WHITE)
			pos = Vec3f((i % 8) + 1, 8 - ((15 - i) / 8), 0);
		else if (Side == TEAM::BLACK)
			pos = Vec3f((i % 8) + 1, ((15 - i) / 8) + 1, 0);
		piece->setPosition(pos);
		piece->setID(10 * pos.x + pos.y);

		if (i < 8)						piece->setType(OBJTYPE::PAWN);
		else if (i == 8 || i == 15)		piece->setType(OBJTYPE::ROCK);
		else if (i == 9 || i == 14)		piece->setType(OBJTYPE::BISHOP);
		else if (i == 10 || i == 13)	piece->setType(OBJTYPE::KNIGHT);

		m_Board[(int)pos.x][(int)pos.y] = piece->getID();
		pieceset.push_back(piece);
	}
	pieceset[11]->setType(OBJTYPE::KING);
	pieceset[12]->setType(OBJTYPE::QUEEN);
}

int Scene::PieceChk(int x, int y)
{
	if (m_Board[x][y] == 0)
		return false;
	else if (m_Board[x][y] != 0)
		return m_Board[x][y];
}


void Scene::releaseScene()
{
	delete		m_Renderer;
	delete		g_Timer;
}


void Scene::keyinput(unsigned char key)
{
	if (m_Target == nullptr)	return;
	

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

	case 'w':
		if (PieceChk(m_Target->getPosition().x, m_Target->getPosition().y - 1) != 0)
			break;
		m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = 0;
		m_Target->move(DIR::UP);
		m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = m_Target->getID();
		break;

	case 'a':
		if (PieceChk(m_Target->getPosition().x - 1, m_Target->getPosition().y) != 0)
			break;
		m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = 0;
		m_Target->move(DIR::LEFT);
		m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = m_Target->getID();
		break;

	case 's':
		if (PieceChk(m_Target->getPosition().x, m_Target->getPosition().y + 1) != 0)
			break;
		m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = 0;
		m_Target->move(DIR::DOWN);
		m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = m_Target->getID();
		break;

	case 'd':
		if (PieceChk(m_Target->getPosition().x + 1, m_Target->getPosition().y) != 0)
			break;
		m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = 0;
		m_Target->move(DIR::RIGHT);
		m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = m_Target->getID();
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
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && GAMESTATUS::RUNNING)
	{
		Vec3i pos((x + 320) / 80 + 1, (320 - y) / 80 + 1, 0);
		if (m_Board[pos.x][pos.y] != 0)
		{
			for (int i = 0; i < 16; ++i)
			{
				if (m_BlackPiece[i]->getPosition().x == pos.x && m_BlackPiece[i]->getPosition().y == pos.y)
				{
					m_Target = m_BlackPiece[i];
					return;
				}
			}
			for (int i = 0; i < 16; ++i)
			{
				if (m_WhitePiece[i]->getPosition().x == pos.x && m_WhitePiece[i]->getPosition().y == pos.y)
				{
					m_Target = m_WhitePiece[i];
					return;
				}
			}
		}
		else if (m_Board[pos.x][pos.y] == 0)
		{
			m_Board[(int)m_Target->getPosition().x][(int)m_Target->getPosition().y] = 0;
			m_Target->setPosition(pos.x, pos.y, 0);
			m_Board[pos.x][pos.y] = m_Target->getID();
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
	m_Renderer->DrawTexturedRect(0, 0, 0, 720, 1.0f, 1.0f, 1.0f, 1.0f, ChessBoard, 0.9);

	for (int i = 0; i < 16; ++i)
	{
		m_Renderer->DrawTexturedRectSeq(
			-360 + m_BlackPiece[i]->getPosition().x * 80,
			360 - m_BlackPiece[i]->getPosition().y * 80,
			0, PIECESIZE, 1, 1, 1, 1, ChessPiece,
			m_BlackPiece[i]->getType(),
			TEAM::BLACK,
			6, 2, 0.5);
	}
	for (int i = 0; i < 16; ++i)
	{
		m_Renderer->DrawTexturedRectSeq(
			-360 + m_WhitePiece[i]->getPosition().x * 80,
			360 - m_WhitePiece[i]->getPosition().y * 80,
			0, PIECESIZE, 1, 1, 1, 1, ChessPiece,
			m_WhitePiece[i]->getType(),
			TEAM::WHITE,
			6, 2, 0.5);
	}
	if (m_Target != nullptr)
		m_Renderer->DrawSolidRect(
			-360 + m_Target->getPosition().x * 80,
			360 - m_Target->getPosition().y * 80,
			0, 80, 0.5f, 0.5f, 0, 0.5f, 0.7);

	if (GameStatus != GAMESTATUS::STOP)
	{


	}

	else if (GameStatus == GAMESTATUS::STOP)
	{

	}
}