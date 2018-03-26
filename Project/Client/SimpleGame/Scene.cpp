#include "stdafx.h"
#include "Client.h"
#include "Renderer.h"
#include "Timer.h"
#include "Sound.h"
#include "Scene.h"
#include "mdump.h"

using namespace std;

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::buildScene()
{
	m_Renderer = new Renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
	// Initialize Renderer
	if (!m_Renderer->IsInitialized())
	{
		std::cout << "Renderer could not be initialized.. \n";
	}

	
	memset(m_Board, -1, sizeof(int) * 81);
	
	ChessBoard = m_Renderer->CreatePngTexture("Assets/Image/chess board.png");
	ChessPiece = m_Renderer->CreatePngTexture("Assets/Image/chess piece.png");

	//InitPieces(m_BlackPiece, TEAM::BLACK);
	//InitPieces(m_WhitePiece, TEAM::WHITE);
	
	GameStatus = GAMESTATUS::RUNNING;


}

void Scene::InitPieces(vector<Object*>& pieceset, TEAM Side)
{
	Vec3i pos;
	pieceset.reserve(16);

	for (int i = 0; i < 16; ++i)
	{
		Object* piece = new Object();
		piece->setTeam(Side);
		if (Side == TEAM::WHITE)
			pos = Vec3i((i % 8) + 1, 8 - ((15 - i) / 8), 0);
		else if (Side == TEAM::BLACK)
			pos = Vec3i((i % 8) + 1, ((15 - i) / 8) + 1, 0);
		piece->setPosition(pos);
		piece->setID(i);

		if (i < 8)						piece->setType(OBJTYPE::PAWN);
		else if (i == 8 || i == 15)		piece->setType(OBJTYPE::ROCK);
		else if (i == 9 || i == 14)		piece->setType(OBJTYPE::BISHOP);
		else if (i == 10 || i == 13)	piece->setType(OBJTYPE::KNIGHT);

		m_Board[pos.x][pos.y] = piece->getID();
		pieceset.push_back(piece);
	}
	pieceset[11]->setType(OBJTYPE::KING);
	pieceset[12]->setType(OBJTYPE::QUEEN);
}

int Scene::PieceChk(int x, int y)
{
	if (m_Board[x][y] == -1)
		return false;
	else if (m_Board[x][y] != -1)
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
		m_Client->SendMsg(m_Target->getID(), m_Target->getPosition().x, m_Target->getPosition().y - 1);
		break;

	case 'a':
		m_Client->SendMsg(m_Target->getID(), m_Target->getPosition().x - 1, m_Target->getPosition().y);
		break;

	case 's':
		m_Client->SendMsg(m_Target->getID(), m_Target->getPosition().x, m_Target->getPosition().y + 1);
		break;

	case 'd':
		m_Client->SendMsg(m_Target->getID(), m_Target->getPosition().x + 1, m_Target->getPosition().y);
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
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && GameStatus == GAMESTATUS::RUNNING)
	{
		Vec3i pos((x + 320) / 80 + 1, (320 - y) / 80 + 1, 0);
		if (pos.x < -1 || pos.y < -1 || pos.x > 9 || pos.y > 9) return;

		if (m_Board[pos.x][pos.y] != -1)
		{
			for (auto p : m_Players)
				if (p->getID() == m_Board[pos.x][pos.y])
					m_Target = p;
		}
		if(m_Target !=nullptr)
			m_Client->SendMsg(m_Client->GetID(), pos.x, pos.y);
	}
}

void Scene::update()
{
	if (GameStatus == GAMESTATUS::RUNNING) 
	{
		m_Client->SendHeartBeat();
		ProcessMsg();
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

	for (auto p : m_Players)
	{
		m_Renderer->DrawTexturedRectSeq(
			-360 + p->getPosition().x * 80,
			360 - p->getPosition().y * 80,
			0, PIECESIZE, 1, 1, 1, 1, ChessPiece,
			p->getType(),
			TEAM::BLACK,
			6, 2, 0.5);
	}
	m_Renderer->DrawSolidRectXY(
		-360 + Client_Object->getPosition().x * 80,
		360 - Client_Object->getPosition().y * 80,
		0, PIECESIZE, PIECESIZE, 1,1,0,0.5, 0.7);

	if (GameStatus != GAMESTATUS::STOP)
	{

	}

	else if (GameStatus == GAMESTATUS::STOP)
	{

	}
}

void Scene::ProcessMsg()
{
	char buf[MSGSIZE];

	while (m_Client->MsgQueue.size() != 0)
	{
		memcpy(buf, m_Client->MsgQueue.front(), MSGSIZE);

		MsgQueueLocker.lock();
		delete m_Client->MsgQueue.front();
		m_Client->MsgQueue.pop_front();
		MsgQueueLocker.unlock();

		MSGTYPE type = (MSGTYPE)buf[0];

		switch (type)
		{
		case MSGTYPE::MOVE:
		{
			int id = buf[1], x = buf[2], y = buf[3];
			for (auto p : m_Players)
				if (p->getID() == id)
					m_Target = p;

			Vec3f pos{ x,y,0 };

			m_Board[int(m_Target->getPosition().x)][int(m_Target->getPosition().y)] = -1;
			m_Target->setPosition(pos);
			m_Board[x][y] = m_Target->getID();
			break;
		}

		case MSGTYPE::HEARTBEAT:
		{
			int x = buf[2], y = buf[3];
			m_Client->SetID(buf[1]);
			m_Client->SetPosition(x, y);
			Object * p = new Object();
			p->setID(buf[1]);
			p->setPosition(Vec3i{ x, y, 0 });
			p->setType(OBJTYPE::PAWN);
			Client_Object = p;
			m_Target = Client_Object;
			m_Board[x][y] = buf[1];
			m_Players.push_back(p);

			char buf[MSGSIZE];
			buf[0] = MSGTYPE::ADDCLIENT;
			buf[1] = m_Client->GetID();
			buf[2] = 1;
			buf[3] = 1;
			m_Client->SendMsg(buf);

			std::cout << m_Client->GetID() << " Client Init" << std::endl;
			break;
		}

		case MSGTYPE::ADDCLIENT:
		{
			if (buf[1] == m_Client->GetID()) break;

			int x = buf[2], y = buf[3];
			Object * p = new Object();
			p->setID(buf[1]);
			p->setPosition(Vec3i{ x, y, 0 });
			p->setType(OBJTYPE::PAWN);
			m_Players.push_back(p);
			m_Board[x][y] = buf[1];

			std::cout << p->getID() << " client Added" << std::endl;
			break;
		}

		case MSGTYPE::REMOVECLIENT:
			m_Players.remove_if([&](Object* player)->bool {
				if (player->getID() == buf[1])
				{
					std::cout << player->getID() << "player Deleted" << std::endl;
					delete player;
					return true;
				}
				return false;
			});
			break;
		}

	}
}