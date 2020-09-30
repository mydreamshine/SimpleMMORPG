// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 특정 포함 파일이 들어 있는
// 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// 여기서 프로그램에 필요한 추가 헤더를 참조합니다.
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <array>
#include <objidl.h>
#include <gdiplus.h>
#include "Global.h"

#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;

#include "Global.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>

#define	WM_SOCKET WM_USER + 1

using namespace std;

#include <CommCtrl.h>

// 윈속 함수 에러 출력
void err_quit(char* msg);
void err_msgbox(char* msg);
void err_display(char* msg);

int REF2R(COLORREF rgb);
int REF2G(COLORREF rgb);
int REF2B(COLORREF rgb);
ColorMatrix GetColorMatrix(DWORD Color);

//wchar_t 에서 char 로의 형변환 함수
char* ConvertWCtoC(wchar_t* str);
//char 에서 wchar_t 로의 형변환 함수
wchar_t* ConvertCtoWC(char* str);

inline int Clamp(int minimum, int x, int maximum) { return max(minimum, min(x, maximum)); }

HWND CreateControlBox(HWND hwndParent, LPSTR lpClassName, LPSTR lpWindowName, int xcoord, int ycoord, int nWidth = 150, int nHeight = 20, HMENU hMenu = NULL);

void PrintText(HDC hDC, int FontSize, LPCSTR FontName, COLORREF FontColor, int x, int y, LPCWSTR Text);