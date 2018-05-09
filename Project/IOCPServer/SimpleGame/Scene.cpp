#include "stdafx.h"
#include "Server.h"
#include "Timer.h"
#include "Object.h"
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
	if (x < 0 || y < 0 || x>BOARD_WIDTH || y > BOARD_HEIGHT) return false;
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

	int oldSpaceIdx = m_Server->GetSpaceIndex(clientid);

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

	int newSpaceIdx = m_Server->GetSpaceIndex(clientid);
	if (oldSpaceIdx != newSpaceIdx) {
		m_Server->GetSpaceMutex(oldSpaceIdx).lock();
		m_Server->GetSpace(oldSpaceIdx).erase(clientid);
		m_Server->GetSpaceMutex(oldSpaceIdx).unlock();

		m_Server->GetSpaceMutex(newSpaceIdx).lock();
		m_Server->GetSpace(newSpaceIdx).insert(clientid);
		m_Server->GetSpaceMutex(newSpaceIdx).unlock();
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
	int idx = 0;
	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			idx = newSpaceIdx + i + (j * SPACE_X);
			if (idx < 0 || idx > SPACE_X*SPACE_Y) continue;
	
			for (auto objidx : m_Server->GetSpace(idx)) 
			{
				if (objidx == clientid) continue;
				if (m_pClientlist[objidx].inUse == false) continue;
				if (m_Server->CanSee(clientid, objidx) == false) continue;
				new_view_list.insert(objidx);
			}
		}
	}

	//for (int i = 0; i < NUM_OF_NPC; ++i) {
	//	if (i == clientid) continue;
	//	if (m_pClientlist[i].inUse == false) continue;
	//	if (m_Server->CanSee(clientid, i) == false) continue;
	//	new_view_list.insert(i);
	//}

	// viewList에 계속 남아있는 객체 처리
	for (auto id : new_view_list) {
		m_pClientlist[clientid].viewlist_mutex.lock();
		if (m_pClientlist[clientid].viewlist.count(id) == 0) {
			m_pClientlist[clientid].viewlist.insert(id);
			m_pClientlist[clientid].viewlist_mutex.unlock();

			m_Server->SendPutObject(clientid, id);
		}
		else {
			m_pClientlist[clientid].viewlist_mutex.unlock();
		}

		m_pClientlist[id].viewlist_mutex.lock();
		if (m_pClientlist[id].viewlist.count(clientid) == 0) {
			m_pClientlist[id].viewlist.insert(clientid);
			if (id >= NPC_START && !m_pClientlist[id].bActive) {
				m_pClientlist[id].bActive = true;
				m_Server->AddEvent(id, enumOperation::op_Move, MOVE_TIME);
			}
			m_pClientlist[id].viewlist_mutex.unlock();

			m_Server->SendPutObject(id, clientid);
		}
		else {
			if (id >= NPC_START && !m_pClientlist[id].bActive) {
				m_pClientlist[id].bActive = true;
				m_Server->AddEvent(id, enumOperation::op_Move, MOVE_TIME);
			}
			m_pClientlist[id].viewlist_mutex.unlock();
			m_Server->SendPacket(id, &sp);
		}
	}

	// 빠져나간 객체
	m_pClientlist[clientid].viewlist_mutex.lock();
	unordered_set<int> oldviewlist = m_pClientlist[clientid].viewlist;
	m_pClientlist[clientid].viewlist_mutex.unlock();
	for (auto id : oldviewlist) {
		if (clientid == id)
			continue;
		if (new_view_list.count(id) == 0) {
			m_pClientlist[clientid].viewlist_mutex.lock();
			m_pClientlist[clientid].viewlist.erase(id);
			m_pClientlist[clientid].bActive = false;
			m_pClientlist[clientid].viewlist_mutex.unlock();

			m_Server->SendRemoveObject(clientid, id);

			m_pClientlist[id].viewlist_mutex.lock();
			if (m_pClientlist[id].viewlist.count(clientid) != 0) {
				m_pClientlist[id].viewlist.erase(clientid);
				m_pClientlist[id].bActive = false;
				m_pClientlist[id].viewlist_mutex.unlock();

				m_Server->SendRemoveObject(id, clientid);
			}
			else
				m_pClientlist[id].viewlist_mutex.unlock();
		}
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

