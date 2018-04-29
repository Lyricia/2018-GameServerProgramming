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
	m_pClientlist = m_Server->GetClientlist();
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
	if (x < 1 || y < 1 || x > 9 || y > 9) return false;

	if (m_Board[x][y] == INVALID)
		return true;
	else if (m_Board[x][y] != INVALID)
		return false;
}

void Scene::releaseScene()
{
	delete		g_Timer;
}

void Scene::ProcessPacket(int id, char * packet)
{
	int type = packet[1];
	ClientInfo& client = m_Server->GetClient(id);
	switch (type) 
	{
	case CS_UP:
		client.y--;
		if (0 > client.y)
			client.y = 0;
		break;
	case CS_DOWN:
		client.y++;
		if (BOARD_HEIGHT <= client.y)
			client.y = BOARD_HEIGHT - 1;
		break;
	case CS_RIGHT:
		client.x++;
		if (BOARD_WIDTH <= client.x)
			client.x = BOARD_WIDTH - 1;
		break;
	case CS_LEFT:
		client.x--;
		if (0 > client.x)
			client.x = 0;
		break;
	default:
		cout << "unknown protocol from client [" << id << "]" << endl;
		return;
	}

	sc_packet_pos sp;
	sp.id = id;
	sp.size = sizeof(sc_packet_pos);
	sp.type = SC_POS;
	sp.x = client.x;
	sp.y = client.y;




	m_Server->SendPacket(id, &sp);
	for (int i = 0; i < MAX_USER; ++i) {
		if (m_Server->GetClient(i).inUse == true)
			m_Server->SendPacket(i, &sp);
	}
}

void Scene::update()
{
	g_Timer->getTimeset();
	double timeElapsed = g_Timer->getTimeElapsed();

	if (GameStatus == GAMESTATUS::RUNNING) 
	{
	}
	else if (GameStatus == GAMESTATUS::PAUSE)
	{
		g_Timer->getTimeset();
		double timeElapsed = g_Timer->getTimeElapsed();
	}

}

