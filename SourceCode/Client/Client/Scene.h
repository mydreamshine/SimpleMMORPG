#pragma once
#include "C_SIMPLE_MMORPG.h"
#include "ProcessSocket.h"

#define FRAME_WIDTH  VIEWPORT_SIZE
#define FRAME_HEIGHT VIEWPORT_SIZE

#define SHARED_CONFIG_IS_NON        0
#define SHARED_CONFIG_IS_COPYED     1
#define SHARED_CONFIG_IS_REFERENCED 2

class CScene
{
public:
	CScene() = default;
	CScene(HWND hWnd, int* CurrentScne) :m_hWnd(hWnd), m_pCurrentSceneIndex(CurrentScne) {}
	virtual ~CScene();

	virtual void    BuildObjects()   = 0;    // ������Ʈ ����
	virtual void    ReleaseObjects() = 0;    // ������Ʈ ����
	virtual void    ProcessInput()   = 0;    // �� �����ӿ��� ó���� �Է�ó��

	virtual void    SetFrameBuffer(HDC hDCFrameBuffer) = 0;
	virtual void    Render(HWND hWnd, HDC hDCFrameBuffer) = 0;                                                  // ������
	virtual void    OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)    = 0;  // ������ ���콺 �޼��� ó��
	virtual void    OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) = 0;  // ������ Ű���� �޼��� ó��
	virtual LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)   = 0;  // ������ �޼��� ó��
	virtual void    ProcessPacketFrom(char* BufferStream) = 0;                                                  // ��Ŷ ó��

	void            SetSharedData(BYTE* ByteStream, rsize_t ByteSize, bool isCopy = true, bool RefOnly = false); // Scene �ܺο��� �����ϰų�, �����Ǿ����� �����͸� ���� (with �����̷�)
	BYTE*           GetSharedData() { return m_SharedData; } // ���� Scene������ Ư�� ������(�ܺο��� ���� ������)�� �ܺη� �Ѱ��ִ� �Լ�.
	rsize_t         GetSharedDataByteSize() { return m_SharedDataByteSize; };

	RECT       m_rcClient;       // �������α׷� ������� ����
	POINT      m_mousePos;       // ���콺 ��ǥ (Screen Coordinate)
	BOOL       m_LButtonDown = FALSE;
	BOOL       m_KeepOnLButtonDown = TRUE;
protected:
	HDC        m_hDCFrameBuffer;
	HWND       m_hWnd;

	int*       m_pCurrentSceneIndex = nullptr; // 0: Title(Menu Scene), 1: GameplayScene

	BYTE*      m_SharedData = nullptr;
	rsize_t    m_SharedDataByteSize = 0;
	BYTE       m_SharedConfig = SHARED_CONFIG_IS_NON;
};

class CTitleScene : public CScene
{
public:
	CTitleScene() = default;
	CTitleScene(HWND hWnd, int* CurrentScne);
	virtual ~CTitleScene();

	virtual void    BuildObjects();    // ������Ʈ ����
	virtual void    ReleaseObjects();  // ������Ʈ ����
	virtual void    ProcessInput() {}; // �� �����ӿ��� ó���� �Է�ó��

	virtual void    SetFrameBuffer(HDC hDCFrameBuffer);
	virtual void    Render(HWND hWnd, HDC hDCFrameBuffer);                                                    // ������
	virtual void    OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}     // ������ ���콺 �޼��� ó��
	virtual void    OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}  // ������ Ű���� �޼��� ó��
	virtual LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);      // ������ �޼��� ó��

	virtual void    ProcessPacketFrom(char* BufferStream);                                                    // ��Ŷ ó��
protected:
	Graphics*              m_pScreen      = nullptr;
	CachedBitmap*          m_pCachedImage = nullptr;// ��� �������� ���ҽ�

	HWND                   m_hInputBox[2]; // 0: id input, 1: password input
	HWND                   m_hButton;
	HWND                   m_hIPAddressControl;
	DWORD                  m_IPAddress;
};

class CPlayScene : public CScene
{
public:
	CPlayScene() = default;
	CPlayScene(HWND hWnd, int* CurrentScne) :CScene(hWnd, CurrentScne) {}
	virtual ~CPlayScene();

	virtual void BuildObjects();    // ������Ʈ ����
	virtual void ReleaseObjects();  // ������Ʈ ����
	virtual void ProcessInput();    // �� �����ӿ��� ó���� �Է�ó��

	virtual void    SetFrameBuffer(HDC hDCFrameBuffer);
	virtual void    Render(HWND hWnd, HDC hDCFrameBuffer);  												// ������
	virtual void    OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);		// ������ ���콺 �޼��� ó��
	virtual void    OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	// ������ Ű���� �޼��� ó��
	virtual LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	// ������ �޼��� ó��

	virtual void    ProcessPacketFrom(char* BufferStream);                                                  // ��Ŷ ó��
protected:
	C_SIMPLE_MMORPG* m_pCSIMPEMMORPG = nullptr; // �������α׷� ������Ʈ

	bool             m_keys[256];
	HWND             m_hButton[2]; // 0: Ÿ��Ʋ ������� ��ȯ�ϱ�, 1: �����ϱ�
};