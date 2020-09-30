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

	virtual void    BuildObjects()   = 0;    // 오브젝트 생성
	virtual void    ReleaseObjects() = 0;    // 오브젝트 해제
	virtual void    ProcessInput()   = 0;    // 한 프레임에서 처리할 입력처리

	virtual void    SetFrameBuffer(HDC hDCFrameBuffer) = 0;
	virtual void    Render(HWND hWnd, HDC hDCFrameBuffer) = 0;                                                  // 렌더링
	virtual void    OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)    = 0;  // 윈도우 마우스 메세지 처리
	virtual void    OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) = 0;  // 윈도우 키보드 메세지 처리
	virtual LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)   = 0;  // 윈도우 메세지 처리
	virtual void    ProcessPacketFrom(char* BufferStream) = 0;                                                  // 패킷 처리

	void            SetSharedData(BYTE* ByteStream, rsize_t ByteSize, bool isCopy = true, bool RefOnly = false); // Scene 외부에서 공유하거나, 공유되어지는 데이터를 지정 (with 공유이력)
	BYTE*           GetSharedData() { return m_SharedData; } // 현재 Scene에서의 특정 데이터(외부와의 공유 데이터)를 외부로 넘겨주는 함수.
	rsize_t         GetSharedDataByteSize() { return m_SharedDataByteSize; };

	RECT       m_rcClient;       // 응용프로그램 전면버퍼 영역
	POINT      m_mousePos;       // 마우스 좌표 (Screen Coordinate)
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

	virtual void    BuildObjects();    // 오브젝트 생성
	virtual void    ReleaseObjects();  // 오브젝트 해제
	virtual void    ProcessInput() {}; // 한 프레임에서 처리할 입력처리

	virtual void    SetFrameBuffer(HDC hDCFrameBuffer);
	virtual void    Render(HWND hWnd, HDC hDCFrameBuffer);                                                    // 렌더링
	virtual void    OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}     // 윈도우 마우스 메세지 처리
	virtual void    OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}  // 윈도우 키보드 메세지 처리
	virtual LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);      // 윈도우 메세지 처리

	virtual void    ProcessPacketFrom(char* BufferStream);                                                    // 패킷 처리
protected:
	Graphics*              m_pScreen      = nullptr;
	CachedBitmap*          m_pCachedImage = nullptr;// 고속 렌더링용 리소스

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

	virtual void BuildObjects();    // 오브젝트 생성
	virtual void ReleaseObjects();  // 오브젝트 해제
	virtual void ProcessInput();    // 한 프레임에서 처리할 입력처리

	virtual void    SetFrameBuffer(HDC hDCFrameBuffer);
	virtual void    Render(HWND hWnd, HDC hDCFrameBuffer);  												// 렌더링
	virtual void    OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);		// 윈도우 마우스 메세지 처리
	virtual void    OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	// 윈도우 키보드 메세지 처리
	virtual LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	// 윈도우 메세지 처리

	virtual void    ProcessPacketFrom(char* BufferStream);                                                  // 패킷 처리
protected:
	C_SIMPLE_MMORPG* m_pCSIMPEMMORPG = nullptr; // 응용프로그램 오브젝트

	bool             m_keys[256];
	HWND             m_hButton[2]; // 0: 타이틀 장면으로 전환하기, 1: 종료하기
};