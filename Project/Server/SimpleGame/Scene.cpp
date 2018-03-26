#include "stdafx.h"
#include "Server.h"
#include "Timer.h"
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
	memset(m_Board, INVALID, sizeof(int) * 81);
	
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
	if (x < 1 || y < 1 || x > 9 || y > 8) return false;

	if (m_Board[x][y] == INVALID)
		return true;
	else if (m_Board[x][y] != INVALID)
		return false;

	return false;
}

void Scene::releaseScene()
{
	delete		g_Timer;
}

void Scene::keyinput(unsigned char key)
{
}

void Scene::keyspcialinput(int key)
{
}

// 멀티태스킹을 가정하고 업 다운 글로벌을 가지고 있어야한다.
// 밖에서 누르고 안에서 업 할 수도 있기 때문에
void Scene::mouseinput(int button, int state, int x, int y)
{
}

void Scene::update()
{
	g_Timer->getTimeset();
	double timeElapsed = g_Timer->getTimeElapsed();

	if (GameStatus == GAMESTATUS::RUNNING) 
	{
		ProcessMsg();
	}
	else if (GameStatus == GAMESTATUS::PAUSE)
	{
		g_Timer->getTimeset();
		double timeElapsed = g_Timer->getTimeElapsed();
	}

	m_Server->AliveChecker();
}

void Scene::render()
{
}

void Scene::ProcessMsg()
{
	char buf[MSGSIZE];

	while (m_Server->MsgQueue.size() != 0)
	{
		memcpy(buf, m_Server->MsgQueue.front(), MSGSIZE);

		MsgQueueLocker.lock();
		delete m_Server->MsgQueue.front();
		m_Server->MsgQueue.pop_front();
		MsgQueueLocker.unlock();

		MSGTYPE type = (MSGTYPE)buf[0];
		switch (type)
		{
		case MSGTYPE::ADDCLIENT:
		{
			int id = buf[1], x = buf[2], y = buf[3];
			Object* p = new Object();
			p->setID(id);
			p->setPosition(x, y, 0);
			m_Board[x][y] = id;
			m_Players.insert(std::make_pair(id, p));
			break;
		}

		case MSGTYPE::MOVE:
		{
			int id = buf[1], x = buf[2], y = buf[3];
			m_Target = m_Players.find(id)->second;

			Vec3f pos{ x,y,0 };
			if (!PieceChk(x, y))
				continue;

			m_Board[int(m_Target->getPosition().x)][int(m_Target->getPosition().y)] = -1;
			m_Target->setPosition(pos);
			m_Board[x][y] = m_Target->getID();
			m_Server->getClient(m_Target->getID())->SetPosition(x, y);

			buf[0] = MSGTYPE::MOVE;
			buf[1] = m_Target->getID();
			buf[2] = m_Target->getPosition().x;
			buf[3] = m_Target->getPosition().y;

			m_Server->SendMsg(buf);
			break;
		}

		case MSGTYPE::REMOVECLIENT:
			m_Server->RemoveClient(buf[1]);
			break;

		case MSGTYPE::HEARTBEAT:
			//cout << buf[1] << "Heart Beat"<< endl;
			break;
		}
	}
}
