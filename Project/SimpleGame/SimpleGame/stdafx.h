#pragma once

#include "targetver.h"

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <tchar.h>
#include <chrono>
#include <vector>

#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"
#include "Vector3D.h"

#define WINDOW_HEIGHT	800
#define WINDOW_WIDTH	800

#define EPSILON			0.00001f

using namespace std;

enum DIR { LEFT, RIGHT, TOP, BOTTOM };

enum OBJTYPE {
	PAWN,
	KING,
	QUEEN,
	BISHOP,
	ROCK,
	KNIGHT
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

