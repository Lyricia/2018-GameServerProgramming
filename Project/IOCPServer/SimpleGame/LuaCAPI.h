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
	Server_Instance->SendChatToAll(chatter, wmess);
	return 0;
}

int CAPI_Server_MoveNPC(lua_State* L) 
{
	int NPCID = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	Server_Instance->MoveNPC(NPCID);
	Server_Instance->AddTimerEvent(NPCID, op_Move, MOVE_TIME);
	return 0;
}

int CAPI_get_x(lua_State* L)
{
	int id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x;
	if (id >= NPC_START) {
		x = Server_Instance->GetNPC(id).x;
	}
	else {
		x = Server_Instance->GetClient(id).x;
	}
	lua_pushnumber(L, x);
	return 1;
}

int CAPI_get_y(lua_State* L)
{
	int id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y;
	if (id >= NPC_START) {
		y = Server_Instance->GetNPC(id).y;
	}
	else {
		y = Server_Instance->GetClient(id).y;
	}
	lua_pushnumber(L, y);
	return 1;
}