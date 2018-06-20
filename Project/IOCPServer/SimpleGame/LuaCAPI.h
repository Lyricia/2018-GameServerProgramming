#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "Server.h"

Server* Server_Instance;

int CAPI_sendMessage(lua_State* L)
{
	char* message = (char *)lua_tostring(L, -1);
	int chatter = (int)lua_tointeger(L, -2);
	int player = (int)lua_tointeger(L, -3);
	lua_pop(L, 4);

	wchar_t wmess[MAX_STR_SIZE];
	size_t len = strlen(message), wlen;
	if (len >= MAX_STR_SIZE) {
		len = MAX_STR_SIZE - 1;
	}
	mbstowcs_s(&wlen, wmess, len, message, _TRUNCATE);
	wmess[MAX_STR_SIZE - 1] = (wchar_t)0;

	//Server_Instance->SendChatPacket(player, chatter, wmess);
	Server_Instance->SendChatToAll(wmess);
	return 0;
}

int CAPI_Server_MoveNPC(lua_State* L) 
{
	int dir = (int)lua_tointeger(L, -1);
	int NPCID = (int)lua_tointeger(L, -2);
	lua_pop(L, 3);

	Server_Instance->MoveNPC(NPCID, dir);
	Server_Instance->AddTimerEvent(NPCID, op_Move, MOVE_TIME);
	return 0;
}

int CAPI_get_x(lua_State* L)
{
	int id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = Server_Instance->GetClient(id).x;
	lua_pushnumber(L, x);
	return 1;
}

int CAPI_get_y(lua_State* L)
{
	int id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = Server_Instance->GetClient(id).y;
	lua_pushnumber(L, y);
	return 1;
}

int CAPI_Attack_Player(lua_State* L)
{
	int damage = (int)lua_tointeger(L, -1);
	int targetid = (int)lua_tointeger(L, -2);
	int NPCID = (int)lua_tointeger(L, -3);
	lua_pop(L, 4);

	auto& cl = Server_Instance->GetClient(targetid);
	std::wstring str;

	sc_packet_attack p;
	p.size = sizeof(sc_packet_attack);
	p.type = SC_ATTACK;
	p.id = NPCID;
	p.targetid = targetid;

	if (damage == -2) {			// warning
		str = L"BOSS Attack In 3 Seconds";
		
		p.att_type = 100;
		Server_Instance->SendPacket(targetid, &p);
	}
	else if (damage == -1) {	// real attack
		cl.getDamaged(10000, NPCID);

		str = L"you hit by Boss! Damage : " + std::to_wstring(10000);

		p.att_type = 101;
		Server_Instance->SendPacket(targetid, &p);
	}
	else {
		cl.getDamaged(damage, NPCID);

		str = L"you hit by " + std::to_wstring(NPCID) + L"! Damage : " + std::to_wstring(damage);

		p.att_type = Server_Instance->GetNPClist()[NPCID].ObjectType;
		Server_Instance->SendPacket(targetid, &p);
	}

	if (cl.hp <= 0) {
		cl.hp = cl.level * 100;
		cl.exp *= 0.5;
		Server_Instance->GetScene()->MoveByCoord(100, 350, targetid, Server_Instance->GetSpaceIndex(targetid));
		return 0;
	}

	Server_Instance->SendChatPacket(targetid, &str[0]);

	Server_Instance->SendStatusPacket(targetid);

	return 0;
}


int CAPI_get_playeractive(lua_State* L) {
	int playerid = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	
	if(playerid< NPC_START)
		lua_pushnumber(L, Server_Instance->GetClient(playerid).bActive);
	else
		lua_pushnumber(L, false);

	return 1;
}