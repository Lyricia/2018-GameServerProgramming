//#include "stdafx.h"
#define SERVERPORT		4000

#define MAX_BUFF_SIZE	4096
#define MAX_PACKET_SIZE 255

#define BOARD_WIDTH		400
#define BOARD_HEIGHT	400
#define SPACESIZE		10

#define VIEW_RADIUS		7
#define ATT_RAD_MELEE	1
#define ATT_RAD_RANGE	5

#define	MOVE_TIME		1000
#define RESPAWN_TIME	1000

#define MAX_USER		10000

#define NPC_START		10001
#define NUM_OF_NPC		NPC_START + 10000

#define MAX_STR_SIZE	100
#define MAX_NAME_LEN	50

#define CS_LOGIN		1
#define CS_LOGOUT		2
#define CS_MOVE			3
#define CS_ATTACK		4
#define CS_CHAT			5

#define SC_LOGINOK			1
#define SC_LOGINFAIL		2
#define SC_POSITION_INFO	3
#define SC_CHAT				4
#define SC_STAT_CHANGE		5
#define SC_REMOVE_OBJECT	6
#define SC_ADD_OBJECT		7
#define SC_ATTACK			8

constexpr int SPACE_X = BOARD_WIDTH / SPACESIZE;
constexpr int SPACE_Y = BOARD_HEIGHT / SPACESIZE;

enum DIR {
		UP = 0
	,	DOWN
	,	LEFT
	,	RIGHT
};

enum ObjType {
		Player				= 01
	,	MOB_Peaceful_melee	= 10
	,	MOB_Peaceful_ranged	= 15
	,	MOB_Chaotic_melee	= 20
	,	MOB_Chaotic_ranged	= 25
	,	MOB_Boss			= 30
};

#pragma pack (push, 1)

struct cs_packet_login {
	BYTE size;
	BYTE type;
	WCHAR ID_STR[10];
};

struct cs_packet_move {
	BYTE size;
	BYTE type;
	BYTE dir;
};

struct cs_packet_chat {
	BYTE size;
	BYTE type;
	WCHAR message[MAX_STR_SIZE];
};

struct cs_packet_attack {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_loginok {
	BYTE size;
	BYTE type;
	WORD id;
	WORD x;
	WORD y;
	WORD hp;
	BYTE level;
	DWORD exp;
};

struct sc_packet_loginfail {
	BYTE size;
	BYTE type;
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	SHORT x;
	SHORT y;
};

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE ObjType;
	SHORT x;
	SHORT y;
};

struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
	WCHAR message[MAX_STR_SIZE];
};

struct sc_packet_stat_change {
	BYTE size;
	BYTE type;
	WORD id;
	WORD hp;
	BYTE lvl;
	DWORD exp;
};

struct sc_packet_attack {
	BYTE size;
	BYTE type;
	WORD id;
	WORD targetid;
	BYTE att_type;
};
#pragma pack (pop)