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
		
		SetSector(m_pNPCList[i]);

		lua_getglobal(m_pNPCList[i].L, "setPosition");
		lua_pushnumber(m_pNPCList[i].L, m_pNPCList[i].x);
		lua_pushnumber(m_pNPCList[i].L, m_pNPCList[i].y);
		int error = lua_pcall(m_pNPCList[i].L, 2, 0, 0);
		lua_pop(m_pNPCList[i].L, 1);
		if (error != 0)
			DisplayLuaError(m_pNPCList[i].L, error);

		lua_getglobal(m_pNPCList[i].L, "setStatus");
		lua_pushnumber(m_pNPCList[i].L, m_pNPCList[i].hp);
		lua_pushnumber(m_pNPCList[i].L, m_pNPCList[i].ObjectType);
		lua_pushnumber(m_pNPCList[i].L, m_pNPCList[i].level);
		error = lua_pcall(m_pNPCList[i].L, 3, 0, 0);
		if(error != 0)
			DisplayLuaError(m_pNPCList[i].L, error);
		lua_pop(m_pNPCList[i].L, 1);
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
		case DIR::UP:
			client.y--;
			if (!isCollide(client.x, client.y)) {
				client.y++; break;
			}
			break;

		case DIR::DOWN:
			client.y++;
			if (!isCollide(client.x, client.y)) {
				client.y--; break;
			}
			break;

		case DIR::RIGHT:
			client.x++;
			if (!isCollide(client.x, client.y)) {
				client.x--; break;
			}
			break;

		case DIR::LEFT:
			client.x--;
			if (!isCollide(client.x, client.y)) {
				client.x++; break;
			}
			break;
		}
		MoveObject(clientid, oldSpaceIdx);
		break;
	}

	case CS_LOGIN:
	{
		cs_packet_login * p = (cs_packet_login*)packet;
		m_Server->AddDBEvent(clientid, p->ID_STR, db_login);
		return;
	}

	case CS_ATTACK:
	{
		cs_packet_attack* p = (cs_packet_attack*)packet;
		if(m_pClientArr[p->id].lastattacktime + 500 < GetSystemTime())
			AttackObject(p->id, 2);
		break;
	}

	default:
		cout << "unknown protocol from client [" << clientid << "]" << endl;
		return;
	}
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

	sc_packet_pos sp;
	sp.id = clientid;
	sp.size = sizeof(sc_packet_pos);
	sp.type = SC_POSITION_INFO;
	sp.x = client.x;
	sp.y = client.y;

	// 새로 viewList에 들어오는 객체 처리
	unordered_set<int> new_view_list;
	int idx = 0;
	int sectoridx = m_Server->GetSector(clientid);
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
				if ((m_Server->GetSector(objidx) - sectoridx) * (m_Server->GetSector(objidx) - sectoridx) != 1)continue;
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

void Scene::AttackObject(int attcker_id, int att_range, int targetid)
{
	if (targetid != INVALID)
	{
		if (attcker_id < NPC_START) {
			if (targetid < NPC_START) {
				if (CalcDist(
					m_pClientArr[attcker_id].x, m_pClientArr[attcker_id].y,
					m_pClientArr[targetid].x, m_pClientArr[targetid].y) < att_range) {
					m_pClientArr[targetid].hp -= 10;
				}
			}
			else if (attcker_id >= NPC_START) {
				if (CalcDist(
					m_pClientArr[attcker_id].x, m_pClientArr[attcker_id].y,
					m_pNPCList[targetid].x, m_pNPCList[targetid].y) < att_range) {
					m_pNPCList[targetid].getDamaged(10, attcker_id);
				}
			}
		}
		else if (attcker_id >= NPC_START) {

		}
	}

	if (attcker_id < NPC_START) {
		CClient& client = m_Server->GetClient(attcker_id);
		int newSpaceIdx = m_Server->GetSpaceIndex(attcker_id);

		unordered_set<int> inRange_list;
		unordered_set<int> view_list;
		int idx = 0;
		for (int i = -1; i <= 1; ++i) {
			for (int j = -1; j <= 1; ++j) {
				idx = newSpaceIdx + i + (j * SPACE_X);
				if (idx < 0 || idx >= SPACE_X * SPACE_Y) continue;

				m_Server->GetSpaceMutex(idx).lock();
				auto& Space_objlist = m_Server->GetSpace(idx);
				m_Server->GetSpaceMutex(idx).unlock();

				for (int space_obj_idx : Space_objlist)
				{
					if (space_obj_idx == attcker_id) continue;
					if (space_obj_idx < NPC_START && m_pClientArr[space_obj_idx].inUse == false) continue;
					if (m_Server->CanSee(attcker_id, space_obj_idx) == false) continue;
					view_list.insert(space_obj_idx);
					if (m_Server->CanSee(attcker_id, space_obj_idx, att_range) == false) continue;
					inRange_list.insert(space_obj_idx);
				}
			}
		}
		view_list.insert(attcker_id);

		for (auto id_inRange : inRange_list) {
			if (id_inRange < NPC_START) 
			{
				auto& target = m_pClientArr[id_inRange];
				target.hp -= 10;
			}
			else if (id_inRange >= NPC_START) 
			{
				auto& target = m_pNPCList[id_inRange];
				target.getDamaged(50, attcker_id);
				if (target.isDead())
				{
					m_Server->RemoveNPC(id_inRange, view_list);
					m_Server->GetClient(attcker_id).EarnEXP(target.level*10);
				}
			}
		}
	}
	else if (attcker_id >= NPC_START) {
	}

	sc_packet_attack p;
	p.size = sizeof(sc_packet_attack);
	p.type = SC_ATTACK;
	p.id = attcker_id;
	p.att_type = 0;
	m_Server->SendPacketToAll(&p);

	m_pClientArr[attcker_id].lastattacktime = GetSystemTime();
}

void Scene::SetSector(CNPC & npc)
{
	int x = npc.x;
	int y = npc.x;

	//section 4
	if (y <= 120) {
		npc.ObjectType = ObjType::MOB_Peaceful_ranged;
		npc.level = rand() % 20;
	}
		
	//section 1
	if (0 <= x && x < 139) {
		if (rand() % 4 == 0)
			npc.ObjectType = ObjType::MOB_Peaceful_ranged;
		else
			npc.ObjectType = ObjType::MOB_Peaceful_melee;

		npc.level = rand() % 20;
	}
	
	//section 2
	if (139 <= x && x < 270) {
		if (rand() % 3 == 0) {
			npc.ObjectType = ObjType::MOB_Chaotic_melee;
			npc.level = 20 + rand() % 20;
		}
		else {
			if (rand() % 2)
				npc.ObjectType = ObjType::MOB_Peaceful_melee;
			else
				npc.ObjectType = ObjType::MOB_Peaceful_ranged;

			npc.level = 5 + rand() % 20;
		}
	}

	//section 3
	if (270 <= x && x < 399) {
		if (rand() % 3 == 0) {
			if (rand() % 2)
				npc.ObjectType = ObjType::MOB_Peaceful_melee;
			else
				npc.ObjectType = ObjType::MOB_Peaceful_ranged;
			npc.level = 30 + rand() % 20;
		}
		else {
			if (rand() % 2)
				npc.ObjectType = ObjType::MOB_Chaotic_melee;
			else
				npc.ObjectType = ObjType::MOB_Chaotic_ranged;
			npc.level = 40 + rand() % 20;
		}
	}
	npc.hp = npc.level * 100;
	npc.exp = npc.level * npc.ObjectType * 5;
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
	}
}

