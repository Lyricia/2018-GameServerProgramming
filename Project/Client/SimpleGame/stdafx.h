#pragma once

#include "targetver.h"

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <tchar.h>
#include <chrono>
#include <vector>
#include <thread>
#include <string>
#include <list>
#include <mutex>
#include <queue>
#include <unordered_map>

#include <comdef.h>

#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"

#include "../../IOCPServer/SimpleGame/protocol.h"
#include "Vector3D.h"

#pragma comment(lib, "ws2_32")

#define WINDOW_HEIGHT	720
#define WINDOW_WIDTH	720

#define EPSILON			0.00001f
#define	WM_SOCKET		WM_USER + 1

using std::thread;
using std::list;
using std::vector;
using namespace std;

//enum DIR { LEFT, RIGHT, UP, DOWN};

enum PieceTYPE {
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
