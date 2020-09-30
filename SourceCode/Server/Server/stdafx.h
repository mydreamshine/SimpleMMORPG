#pragma once
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <string>
#include <locale.h>

#include <queue>
#include <map>
#include <unordered_map>
#include <unordered_set>
//#include <concurrent_unordered_set.h> // PPL(Parallel Patterns Library) -> 게임서버엔 부적합
#include <random>
#include <algorithm>
#include <mutex>

#include "Global.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>

// 윈속 함수 에러 출력
void err_quit(char* msg);
void err_display(char* msg);


// Random Number
int getRandomNumber(int min, int max);

// Check Rect in
bool RectIn(int left, int top, int right, int bottom, int x, int y);

//wchar_t 에서 char 로의 형변환 함수
char* ConvertWCtoC(wchar_t* str);

//char 에서 wchar_t 로의 형변환 함수
wchar_t* ConverCtoWC(char* str);

inline int Clamp(int minimum, int x, int maximum) { return max(minimum, min(x, maximum)); }
