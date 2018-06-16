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
	m_pClientArr = m_Server->GetClientArr();
	m_pNPCList = m_Server->GetNPClist();

	ReadGroundPos();

	GameStatus = GAMESTATUS::RUNNING;

	for (int i = NPC_START; i < NUM_OF_NPC; ++i) {
		do {
			m_pNPCList[i].x = rand() % (BOARD_WIDTH - 100);
			m_pNPCList[i].y = rand() % (BOARD_HEIGHT - 100);
		} while (!isCollide(m_pNPCList[i].x, m_pNPCList[i].y));
		int newSpaceIdx = m_Server->GetSpaceIndex(i);
		m_Server->GetSpace(newSpaceIdx).insert(i);
	}					 
}

bool Scene::isCollide(int x, int y)
{
	if (x < 0 || y < 0 || x > BOARD_WIDTH || y > BOARD_HEIGHT) return true;
	if (m_Board[x][y] == INVALID)
		return true;
	else 
		return false;
}

void Scene::releaseScene()
{
	delete		g_Timer;
}

void Scene::ProcessPacket(int clientid, unsigned char * packet)
{
	int type = packet[1];
	CClient& client = m_Server->GetClient(clientid);
	int x = client.x, y = client.y;

	int oldSpaceIdx = m_Server->GetSpaceIndex(clientid);

	switch (type)
	{
	case CS_MOVE:
	{
		auto p = (cs_packet_move*)packet;
		switch (p->dir) {
		case CS_UP:
			client.y--;
			if (!isCollide(client.x, client.y)) {
				client.y++; break;
			}
			break;

		case CS_DOWN:
			client.y++;
			if (!isCollide(client.x, client.y)) {
				client.y--; break;
			}
			break;

		case CS_RIGHT:
			client.x++;
			if (!isCollide(client.x, client.y)) {
				client.x--; break;
			}
			break;

		case CS_LEFT:
			client.x--;
			if (!isCollide(client.x, client.y)) {
				client.x++; break;
			}
			break;
		}
		break;
	}
	case CS_LOGIN:
	{
		cs_packet_login * p = (cs_packet_login*)packet;
		m_Server->AddDBEvent(clientid, p->id, db_login);
		return;
	}
	default:
		cout << "unknown protocol from client [" << clientid << "]" << endl;
		return;
	}
	
	MoveObject(clientid, oldSpaceIdx);
}

void Scene::MoveObject(int clientid, int oldSpaceIdx)
{
	CClient& client = m_Server->GetClient(clientid);

	int newSpaceIdx = m_Server->GetSpaceIndex(clientid);
	if (oldSpaceIdx != newSpaceIdx) {
		m_Server->GetSpaceMutex(oldSpaceIdx).lock();
		m_Server->GetSpace(oldSpaceIdx).erase(clientid);
		m_Server->GetSpaceMutex(oldSpaceIdx).unlock();

		m_Server->GetSpaceMutex(newSpaceIdx).lock();
		m_Server->GetSpace(newSpaceIdx).insert(clientid);
		m_Server->GetSpaceMutex(newSpaceIdx).unlock();
	}

	//m_Board[client.x][client.y] = clientid;

	sc_packet_pos sp;
	sp.id = clientid;
	sp.size = sizeof(sc_packet_pos);
	sp.type = SC_POSITION_INFO;
	sp.x = client.x;
	sp.y = client.y;

	// 새로 viewList에 들어오는 객체 처리
	unordered_set<int> new_view_list;
	int idx = 0;
	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			idx = newSpaceIdx + i + (j * SPACE_X);
			if (idx < 0 || idx >= SPACE_X * SPACE_Y) continue;

			m_Server->GetSpaceMutex(idx).lock();
			auto Space_objlist = m_Server->GetSpace(idx);
			m_Server->GetSpaceMutex(idx).unlock();

			for (auto objidx : Space_objlist)
			{
				if (objidx == clientid) continue;
				if (objidx < NPC_START && m_pClientArr[objidx].inUse == false) continue;
				if (m_Server->CanSee(clientid, objidx) == false) continue;
				// 시야 내에 있는 플레이어가 이동했다는 이벤트 발생
				if (objidx >= NPC_START) {
					stOverlappedEx *e = new stOverlappedEx();
					e->eOperation = npc_player_move;
					e->EventTarget = clientid;
					PostQueuedCompletionStatus(m_Server->GetIOCP(), 1, objidx, &e->wsaOverlapped);
				}
				new_view_list.insert(objidx);
			}
		}
	}

	// viewList에 계속 남아있는 객체 처리
	for (auto id : new_view_list) {
		m_pClientArr[clientid].viewlist_mutex.lock();
		if (m_pClientArr[clientid].viewlist.count(id) == 0) {
			m_pClientArr[clientid].viewlist.insert(id);
			m_pClientArr[clientid].viewlist_mutex.unlock();

			m_Server->SendPutObject(clientid, id);
		}
		else {
			m_pClientArr[clientid].viewlist_mutex.unlock();
		}

		if (id >= NPC_START)
		{
			if (m_pNPCList[id].bActive == false) {
				m_pNPCList[id].bActive = true;
				//m_Server->AddTimerEvent(id, enumOperation::op_Move, MOVE_TIME);
			}
		}
		else
		{
			m_pClientArr[id].viewlist_mutex.lock();
			if (m_pClientArr[id].viewlist.count(clientid) == 0) {
				m_pClientArr[id].viewlist.insert(clientid);
				m_pClientArr[id].viewlist_mutex.unlock();

				m_Server->SendPutObject(id, clientid);
			}
			else {
				m_pClientArr[id].viewlist_mutex.unlock();
				m_Server->SendPacket(id, &sp);
			}
		}
	}

	// 빠져나간 객체
	m_pClientArr[clientid].viewlist_mutex.lock();
	unordered_set<int> oldviewlist = m_pClientArr[clientid].viewlist;
	m_pClientArr[clientid].viewlist_mutex.unlock();
	for (auto id : oldviewlist) {
		if (clientid == id)
			continue;
		if (new_view_list.count(id) == 0) {
			m_pClientArr[clientid].viewlist_mutex.lock();
			m_pClientArr[clientid].viewlist.erase(id);
			m_pClientArr[clientid].viewlist_mutex.unlock();

			m_Server->SendRemoveObject(clientid, id);

			if (id >= NPC_START) 
			{
				if (m_pNPCList[id].bActive == true) {
					m_pNPCList[id].bActive = false;
				}
			}
			else
			{
				m_pClientArr[id].viewlist_mutex.lock();
				if (m_pClientArr[id].viewlist.count(clientid) != 0) {
					m_pClientArr[id].viewlist.erase(clientid);
					m_pClientArr[id].viewlist_mutex.unlock();

					m_Server->SendRemoveObject(id, clientid);
				}
				else
					m_pClientArr[id].viewlist_mutex.unlock();
			}
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

struct pos {
	int x, y;
};

void Scene::ReadGroundPos()
{
	int nStride = sizeof(pos);
	int nVerts = 0;

	std::string Filepath = "./Assets/Ground.pos";
	FILE* fp = nullptr;
	fopen_s(&fp, Filepath.c_str(), "rb");
	fread_s(&nVerts, sizeof(UINT), sizeof(UINT), 1, fp);
	pos* pVerts = new pos[nVerts];
	fread_s(pVerts, nStride * nVerts, nStride, nVerts, fp);

	for (int i = 0; i < nVerts; ++i) {
		m_Board[pVerts[i].x][pVerts[i].y] = 1;
		//cout << pVerts[i].x << ", " << pVerts[i].y << "\n";
	}
}

