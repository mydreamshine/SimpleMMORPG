#include "stdafx.h"

// 윈속 함수 에러 출력
void err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, // 오류 메세지를 시스템이 알아서 생성
		NULL, // 오류 메세지 생성 주체(옵션을 위와 같인 설정하면 NULL로.)
		WSAGetLastError(), // 마지막 오류코드
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 오류 메세지 출력 언어 종류
		(LPTSTR)&lpMsgBuf, 0, NULL);

	// 오류 메세지 출력
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);

	// 오류 메세지 메모리 해제
	LocalFree(lpMsgBuf);

	exit(1);
}

// 윈속 함수 에러 출력
void err_msgbox(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, // 오류 메세지를 시스템이 알아서 생성
		NULL, // 오류 메세지 생성 주체(옵션을 위와 같인 설정하면 NULL로.)
		WSAGetLastError(), // 마지막 오류코드
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 오류 메세지 출력 언어 종류
		(LPTSTR)&lpMsgBuf, 0, NULL);

	// 오류 메세지 출력
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);

	// 오류 메세지 메모리 해제
	LocalFree(lpMsgBuf);
}

void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, // 오류 메세지를 시스템이 알아서 생성
		NULL, // 오류 메세지 생성 주체(옵션을 위와 같인 설정하면 NULL로.)
		WSAGetLastError(), // 마지막 오류코드
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 오류 메세지 출력 언어 종류
		(LPTSTR)&lpMsgBuf, 0, NULL);

	// 오류 메세지 출력
	printf_s("[%s] %s", msg, (LPCTSTR)lpMsgBuf);

	// 오류 메세지 메모리 해제
	LocalFree(lpMsgBuf);
}

int REF2R(COLORREF rgb)
{
	int nBitmaskRed = 0x0000ff;  //255
	int nRed = rgb & nBitmaskRed;

	return nRed;
}

int REF2G(COLORREF rgb)
{
	int nBitmaskGreen = 0x00ff00; //65280
	int nGreen = (rgb & nBitmaskGreen) >> 8;

	return nGreen;
}

int REF2B(COLORREF rgb)
{
	int nBitmaskBlue = 0xff0000;  //16711680
	int nBlue = (rgb & nBitmaskBlue) >> 16;

	return nBlue;
}

ColorMatrix GetColorMatrix(DWORD Color)
{
	float Red = (float)REF2R(Color) / 255.0f; // Red
	float Green = (float)REF2G(Color) / 255.0f; // Green
	float Blue = (float)REF2B(Color) / 255.0f; // Blue
	float Alpha = 1.0f;
	ColorMatrix colorMatrix =
	{
		Red,  0.0f,  0.0f,  0.0f,  0.0f,
		0.0f, Green, 0.0f,  0.0f,  0.0f,
		0.0f, 0.0f,  Blue,  0.0f,  0.0f,
		0.0f, 0.0f,  0.0f,  Alpha, 0.0f,
		0.0f, 0.0f,  0.0f,  0.0f,  1.0f
	};
	return colorMatrix;
}

char* ConvertWCtoC(wchar_t* str)
{
	//반환할 char* 변수 선언
	char* pStr;

	//입력받은 wchar_t 변수의 길이를 구함
	int strSize = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);

	//char* 메모리 할당
	pStr = new char[strSize];

	//형 변환 
	WideCharToMultiByte(CP_ACP, 0, str, -1, pStr, strSize, 0, 0);

	return pStr;
}

wchar_t* ConvertCtoWC(char* str)
{
	//wchar_t형 변수 선언
	wchar_t* pStr;
	//멀티 바이트 크기 계산 길이 반환
	int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);

	//wchar_t 메모리 할당
	pStr = new WCHAR[strSize];
	//형 변환
	MultiByteToWideChar(CP_ACP, 0, str, (int)strlen(str) + 1, pStr, strSize);

	return pStr;
}

HWND CreateControlBox(HWND hwndParent, LPSTR lpClassName, LPSTR lpWindowName, int xcoord, int ycoord, int nWidth, int nHeight, HMENU hMenu)
{
	//if (strcmp(lpClassName, WC_IPADDRESS) == 0)
	//{
	//	INITCOMMONCONTROLSEX icex;

	//	// Ensure that the common control DLL is loaded. 
	//	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	//	icex.dwICC = ICC_INTERNET_CLASSES;
	//	InitCommonControlsEx(&icex);
	//}

	// Create the IPAddress control.
	HWND hWndIPAddressFld = CreateWindow(lpClassName,
		lpWindowName,
		WS_VISIBLE | WS_CHILD | WS_BORDER,
		xcoord,
		ycoord,
		nWidth,
		nHeight,
		hwndParent,
		hMenu,
		NULL,
		NULL);

	HFONT hFont = CreateFont((int)(nHeight * 0.8f), 0, 0, 0, FW_DONTCARE, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, "Arial");
	SendMessage(hWndIPAddressFld, WM_SETFONT, WPARAM(hFont), TRUE);

	return (hWndIPAddressFld);
}

void PrintText(HDC hDC, int FontSize, LPCSTR FontName, COLORREF FontColor, int x, int y, LPCWSTR Text)
{
	HFONT myFont = CreateFont(FontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, FontName);
	HFONT oldFont = (HFONT)SelectObject(hDC, myFont);
	SetTextColor(hDC, FontColor);
	TextOutW(hDC, x, y, Text, (int)wcsnlen_s(Text, 256));
	SelectObject(hDC, oldFont);
	DeleteObject(myFont);
}
