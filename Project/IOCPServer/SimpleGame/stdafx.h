#pragma once

#include "targetver.h"

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <tchar.h>
#include <chrono>
#include <fstream>
#include <vector>
#include <thread>
#include <list>
#include <mutex>
#include <map>
#include <WS2tcpip.h>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <locale>
#include <iostream>
#include <sqlext.h> 


#include "Vector3D.h"
#include "protocol.h"

extern "C" {
#include "Lua\lua.h"
#include "Lua\lauxlib.h"
#include "Lua\lualib.h"
}

#pragma comment(lib, "Lua/lua53.lib")
#pragma comment(lib, "ws2_32")

#define EPSILON			0.00001f

using std::thread;
using std::list;
using std::vector;
using std::cout;
using std::endl;

enum GAMESTATUS {
	STOP
	, RUNNING
	, PAUSE
	, BLACKWIN
	, WHITEWIN	
};


inline Vector3D<float> Vec3i_to_Vec3f(Vector3D<int>& ivec)
{
	return Vector3D<float>{ static_cast<float>(ivec.x), static_cast<float>(ivec.y) ,static_cast<float>(ivec.z) };
}

inline Vector3D<int> Vec3f_to_Vec3i(Vector3D<float>& fvec)
{
	return Vector3D<int>{ static_cast<int>(fvec.x), static_cast<int>(fvec.y), static_cast<int>(fvec.z) };
}

inline int CalcDist(int ax, int ay, int bx, int by) {
	int dist = -1;

	dist = 
		(ax - bx) * (ax - bx) +
		(ay - by) * (ay - by);

	return dist;
}

inline long long GetSystemTime() {
	const long long _Freq = _Query_perf_frequency();	// doesn't change after system boot
	const long long _Ctr = _Query_perf_counter();
	const long long _Whole = (_Ctr / _Freq) * 1000;
	const long long _Part = (_Ctr % _Freq) * 1000 / _Freq;
	return _Whole + _Part;
}