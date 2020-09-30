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
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, reinterpret_cast<wchar_t*>(msg), MB_ICONERROR);

	// 오류 메세지 메모리 해제
	LocalFree(lpMsgBuf);

	exit(1);
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
	printf_s("[%s] %ws", msg, (LPCTSTR)lpMsgBuf);

	// 오류 메세지 메모리 해제
	LocalFree(lpMsgBuf);
}

int getRandomNumber(int min, int max)
{
	//< 1단계. 시드 설정
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2단계. 분포 설정 ( 정수 )
	uniform_int_distribution<int> range(min, max);

	//< 3단계. 값 추출
	return range(rnd);
}

bool RectIn(int left, int top, int right, int bottom, int x, int y)
{
	if ((left <= x && x <= right) && (top <= y && y <= bottom)) return true;
	return false;
	/*if (x < left || right < x) return false;
	if (y < top || bottom < y) return false;
	return true;*/
}

///////////////////////////////////////////////////////////////////////
//wchar_t 에서 char 로의 형변환 함수
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

///////////////////////////////////////////////////////////////////////
//char 에서 wchar_t 로의 형변환 함수
wchar_t* ConverCtoWC(char* str)
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
