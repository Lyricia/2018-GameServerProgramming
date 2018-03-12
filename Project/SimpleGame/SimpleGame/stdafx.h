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

#define WINDOW_HEIGHT	720
#define WINDOW_WIDTH	720

#define EPSILON			0.00001f

using namespace std;

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
