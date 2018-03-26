#pragma once

#include "targetver.h"

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <tchar.h>
#include <chrono>
#include <vector>
#include <thread>
#include <list>
#include <mutex>
#include <map>
#include <WS2tcpip.h>

#include "Vector3D.h"

#pragma comment(lib, "ws2_32")

#define WINDOW_HEIGHT	720
#define WINDOW_WIDTH	720

#define EPSILON			0.00001f

#define GLRENDERON		0

using std::thread;
using std::list;
using std::vector;


enum DIR { LEFT, RIGHT, UP, DOWN};

enum OBJTYPE {
	KING,
	QUEEN,
	BISHOP,
	KNIGHT,
	ROCK,
	PAWN
};

enum TEAM { WHITE, BLACK };

enum GAMESTATUS {
	STOP
	, RUNNING
	, PAUSE
	, BLACKWIN
	, WHITEWIN	
};

enum SOUNDINDEX {
	BGM = 0,
	PAUSESOUND,
	WIN,
	LOSE,
	CRASHEFFECT
};

extern std::mutex	MsgQueueLocker;

inline Vector3D<float> Vec3i_to_Vec3f(Vector3D<int>& ivec)
{
	return Vector3D<float>{ static_cast<float>(ivec.x), static_cast<float>(ivec.y) ,static_cast<float>(ivec.z) };
}

inline Vector3D<int> Vec3f_to_Vec3i(Vector3D<float>& fvec)
{
	return Vector3D<int>{ static_cast<int>(fvec.x), static_cast<int>(fvec.y), static_cast<int>(fvec.z) };
}

/*
  1 2 3 4 5 6 7 8
1
2		B
3
4
5
6
7		W
8
*/
