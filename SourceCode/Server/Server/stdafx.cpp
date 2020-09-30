#include "stdafx.h"

// ���� �Լ� ���� ���
void err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, // ���� �޼����� �ý����� �˾Ƽ� ����
		NULL, // ���� �޼��� ���� ��ü(�ɼ��� ���� ���� �����ϸ� NULL��.)
		WSAGetLastError(), // ������ �����ڵ�
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ���� �޼��� ��� ��� ����
		(LPTSTR)&lpMsgBuf, 0, NULL);

	// ���� �޼��� ���
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, reinterpret_cast<wchar_t*>(msg), MB_ICONERROR);

	// ���� �޼��� �޸� ����
	LocalFree(lpMsgBuf);

	exit(1);
}

void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, // ���� �޼����� �ý����� �˾Ƽ� ����
		NULL, // ���� �޼��� ���� ��ü(�ɼ��� ���� ���� �����ϸ� NULL��.)
		WSAGetLastError(), // ������ �����ڵ�
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ���� �޼��� ��� ��� ����
		(LPTSTR)&lpMsgBuf, 0, NULL);

	// ���� �޼��� ���
	printf_s("[%s] %ws", msg, (LPCTSTR)lpMsgBuf);

	// ���� �޼��� �޸� ����
	LocalFree(lpMsgBuf);
}

int getRandomNumber(int min, int max)
{
	//< 1�ܰ�. �õ� ����
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2�ܰ�. ���� ���� ( ���� )
	uniform_int_distribution<int> range(min, max);

	//< 3�ܰ�. �� ����
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
//wchar_t ���� char ���� ����ȯ �Լ�
char* ConvertWCtoC(wchar_t* str)
{
	//��ȯ�� char* ���� ����
	char* pStr;

	//�Է¹��� wchar_t ������ ���̸� ����
	int strSize = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);

	//char* �޸� �Ҵ�
	pStr = new char[strSize];

	//�� ��ȯ 
	WideCharToMultiByte(CP_ACP, 0, str, -1, pStr, strSize, 0, 0);

	return pStr;
}

///////////////////////////////////////////////////////////////////////
//char ���� wchar_t ���� ����ȯ �Լ�
wchar_t* ConverCtoWC(char* str)
{
	//wchar_t�� ���� ����
	wchar_t* pStr;
	//��Ƽ ����Ʈ ũ�� ��� ���� ��ȯ
	int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);

	//wchar_t �޸� �Ҵ�
	pStr = new WCHAR[strSize];
	//�� ��ȯ
	MultiByteToWideChar(CP_ACP, 0, str, (int)strlen(str) + 1, pStr, strSize);

	return pStr;
}
