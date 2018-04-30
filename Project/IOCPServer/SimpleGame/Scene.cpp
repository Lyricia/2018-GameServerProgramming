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
	memset(m_Board, INVALID, sizeof(int) * BOARD_WIDTH * BOARD_HEIGHT);
	m_pClientlist = m_Server->GetClientlist();

	GameStatus = GAMESTATUS::RUNNING;
}

bool Scene::isCollide(int x, int y)
{
	if (m_Board[x][y] == INVALID)
		return false;
	else 
		return true;
}

void Scene::releaseScene()
{
	delete		g_Timer;
}

void Scene::ProcessPacket(int clientid, char * packet)
{
	int type = packet[1];
	ClientInfo& client = m_Server->GetClient(clientid);
	int x = client.x, y = client.y;
	switch (type) 
	{
	case CS_UP:
		client.y--;
		if (isCollide(client.x, client.y)) {
			client.y++; break;
		}
		else 
			m_Board[x][y] = INVALID;

		if (0 > client.y)
			client.y = 0;
		break;

	case CS_DOWN:
		client.y++;
		if (isCollide(client.x, client.y)) {
			client.y--; break;
		}
		else 
			m_Board[x][y] = INVALID;

		if (BOARD_HEIGHT <= client.y)
			client.y = BOARD_HEIGHT - 1;
		break;

	case CS_RIGHT:
		client.x++;
		if (isCollide(client.x, client.y)) {
			client.x--; break;
		}
		else
			m_Board[x][y] = INVALID;
		if (BOARD_WIDTH <= client.x)
			client.x = BOARD_WIDTH - 1;
		break;

	case CS_LEFT:
		client.x--;
		if (isCollide(client.x, client.y)) {
			client.x++; break;
		}
		else
			m_Board[x][y] = INVALID;

		if (0 > client.x)
			client.x = 0;
		break;
	default:
		cout << "unknown protocol from client [" << clientid << "]" << endl;
		return;
	}

	m_Board[client.x][client.y] = clientid;

	sc_packet_pos sp;
	sp.id = clientid;
	sp.size = sizeof(sc_packet_pos);
	sp.type = SC_POS;
	sp.x = client.x;
	sp.y = client.y;

	// 새로 viewList에 들어오는 객체 처리
	unordered_set<int> new_view_list;
	for (int i = 0; i < MAX_USER; ++i) {
		if (i == clientid) continue;
		if (m_pClientlist[i].inUse == false) continue;
		if (m_Server->CanSee(clientid, i) == false) continue;
		new_view_list.insert(i);
	}

	// viewList에 계속 남아있는 객체 처리
	for (auto id : new_view_list) {
		if (m_pClientlist[clientid].viewlist.count(id) == 0) {
			m_pClientlist[clientid].viewlist.insert(id);
			m_Server->SendPutObject(clientid, id);

			if (m_pClientlist[id].viewlist.count(clientid) == 0) {
				m_pClientlist[id].viewlist.insert(clientid);
				m_Server->SendPutObject(id, clientid);
			}
			else {
				m_Server->SendPacket(id, &sp);
			}
		}
		else {
			if (m_pClientlist[id].viewlist.count(clientid) == 0) {
				m_pClientlist[id].viewlist.insert(clientid);
				m_Server->SendPutObject(id, clientid);
			}
			else {
				m_Server->SendPacket(id, &sp);
			}
		}
	}

	// viewList에서 나가는 객체 처리
	for (auto iter = m_pClientlist[clientid].viewlist.begin(); iter != m_pClientlist[clientid].viewlist.end();) 
	{
		int id = *iter;
		if (clientid == id) {
			++iter; continue;
		}
		if (new_view_list.count(id) == 0) {
			iter = m_pClientlist[clientid].viewlist.erase(iter);
			m_Server->SendRemoveObject(clientid, id);
			if (m_pClientlist[id].viewlist.count(clientid) != 0) {
				m_pClientlist[id].viewlist.erase(clientid);
				m_Server->SendRemoveObject(id, clientid);
			}
			continue;
		}
		if (iter == m_pClientlist[clientid].viewlist.end())
			break;
		else
			++iter;
	}

	m_Server->SendPacket(clientid, &sp);
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

