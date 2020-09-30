#include "stdafx.h"
#include "Framework.h"

CFramework::CFramework()
{
}

CFramework::~CFramework()
{
}

//윈도우 핸들 지정 및 버퍼, 오브젝트 초기화
bool CFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	//윈도우 핸들 지정
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//프레임 생성
	CFramework::BuildFrameBuffer();

	//오브젝트 생성
	CFramework::BuildObjects();

	return(true);
}

//윈도우 및 버퍼, 오브젝트 메모리 해제
void CFramework::OnDestroy()
{
	//후면버퍼 해제
	if (m_hBitmapFrameBuffer) DeleteObject(m_hBitmapFrameBuffer);
	if (m_hDCFrameBuffer) DeleteDC(m_hDCFrameBuffer);

	//윈도우 해제
	if (m_hWnd) DestroyWindow(m_hWnd);

	//오브젝트 해제
	CFramework::ReleaseObjects();
}

//프레임 갱신
void CFramework::FrameAdvance()
{
	// 현재 장면(Scene) index에 따른 장면전환
	if (m_CurrentSceneRef != m_PrevScene)
	{
		switch (m_PrevScene)
		{
		case TITLE_SCENE:
			m_pScenes[m_PrevScene]->ReleaseObjects();
			break;
		case PLAY_SCENE:
			m_pScenes[m_PrevScene]->ReleaseObjects();
			break;
		}
		CFramework::BuildObjects();
		m_PrevScene = m_CurrentScene = m_CurrentSceneRef;
	}
	else
	{
		//윈도우가 활성화되어 있지 않으면 입력처리하지 않음 (서버로부터 받은 데이터가 있으면 갱신)
		if (m_bActive) CFramework::ProcessInput();
	}

	//후면버퍼 초기화
	CFramework::ClearFrameBuffer(RGB(0, 0, 0));

	//렌더링
	m_pScenes[m_CurrentScene]->Render(m_hWnd, m_hDCFrameBuffer);

	//전면버퍼와 후면버퍼 스왑
	CFramework::PresentFrameBuffer();
}


// 렌더링에 쓰일 후면 버퍼 생성 및 초기화
void CFramework::BuildFrameBuffer()
{
	//현재 응용프로그램의 윈도우로부터 전면버퍼 핸들을 가져옴
	HDC hDC = GetDC(m_hWnd);

	//응용프로그램의 전면버퍼 영역 가져오기
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);

	//전면버퍼와 동일한 크기의 후면버퍼 생성
	//후면버퍼에 그릴 비트맵 형식의 텍스쳐 생성
	m_hDCFrameBuffer = CreateCompatibleDC(hDC);
	m_hBitmapFrameBuffer = CreateCompatibleBitmap(hDC, (rcClient.right - rcClient.left) + 1, (rcClient.bottom - rcClient.top) + 1);
	SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);

	//전면버퍼 핸들 해제 후 후면버퍼의 그리기 방식 지정
	ReleaseDC(m_hWnd, hDC);
	SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
}

// 렌더링에 쓰일 후면 버퍼 초기화
void CFramework::ClearFrameBuffer(DWORD dwColor)
{
	//응용프로그램의 전면버퍼 영역 가져오기
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	m_pScenes[m_CurrentScene]->m_rcClient = rcClient;

	//후면버퍼를 fillcolor 색상으로 초기화
	HBRUSH fillcolor = CreateSolidBrush(dwColor);
	FillRect(m_hDCFrameBuffer, &rcClient, fillcolor);
	DeleteObject(fillcolor);
}

//후면버퍼와 전면버퍼 스왑
void CFramework::PresentFrameBuffer()
{
	//현재 응용프로그램의 윈도우로부터 전면버퍼 핸들을 가져옴
	HDC hDC = GetDC(m_hWnd);

	//응용프로그램의 전면버퍼 영역 가져오기
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	m_pScenes[m_CurrentScene]->m_rcClient = rcClient;

	//전면버퍼 크기
	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;

	//후면버퍼의 내용을 전면버퍼로 BitBlt(고속복사) 스왑
	BitBlt(hDC, rcClient.left, rcClient.top, nWidth, nHeight, m_hDCFrameBuffer, rcClient.left, rcClient.top, SRCCOPY);
	ReleaseDC(m_hWnd, hDC);//전면버퍼 핸들 해제

	//전면버퍼를 다시 그리라는 메세지를 메세지큐에 보냄
	InvalidateRect(m_hWnd, NULL, FALSE);
}

// 프로그램에 쓰일 객체 및 변수를 초기화
void CFramework::BuildObjects()
{
	//후면버퍼에 그릴 장면 생성
	if (m_pScenes.count(m_CurrentSceneRef) == 0)
	{
		if (m_CurrentSceneRef == TITLE_SCENE)
			m_pScenes[m_CurrentSceneRef] = new CTitleScene(m_hWnd, &m_CurrentSceneRef);
		else if (m_CurrentSceneRef == PLAY_SCENE)
			m_pScenes[m_CurrentSceneRef] = new CPlayScene(m_hWnd, &m_CurrentSceneRef);
	}

	// 이전에 쓰여졌던 오브젝트 삭제
	m_pScenes[m_CurrentSceneRef]->ReleaseObjects();

	// Scene 간 공유자원 설정
	BYTE* SharedData = m_pScenes[m_PrevScene]->GetSharedData();
	if (SharedData != nullptr)
	{
		rsize_t SharedByteSize = m_pScenes[m_PrevScene]->GetSharedDataByteSize();
		// Reference Only
		m_pScenes[m_CurrentSceneRef]->SetSharedData(SharedData, SharedByteSize, false, true);
	}

	//장면에 연출될 오브젝트들 생성 및 초기화
	m_pScenes[m_CurrentSceneRef]->BuildObjects();

	//응용프로그램의 전면버퍼 영역 가져오기
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);

	//장면의 영역은 현재 응용프로그램의 전면버퍼의 영역과 동일
	m_pScenes[m_CurrentSceneRef]->m_rcClient = rcClient;

	// Scene에 후면버퍼 등록
	m_pScenes[m_CurrentSceneRef]->SetFrameBuffer(m_hDCFrameBuffer);
}

//프로그램에 쓰였던 객체 및 변수를 해제
void CFramework::ReleaseObjects()
{
	//후면 버퍼에 그려졌던 장면 해제
	for (auto pScene : m_pScenes)
	{
		if (pScene.second != nullptr)
		{
			delete pScene.second;
			pScene.second = nullptr;
		}
	}
	m_pScenes.clear();
}

//한 프레임에서 처리할 입력처리
void CFramework::ProcessInput()
{
	m_pScenes[m_CurrentScene]->ProcessInput();
}

//윈도우 마우스 메세지 처리
void CFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScenes.count(m_CurrentScene) != 0)
	{
		if (m_pScenes[m_CurrentScene]) m_pScenes[m_CurrentScene]->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	}
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	}
}

//윈도우 키보드 메세지 처리
void CFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScenes.count(m_CurrentScene) != 0)
	{
		if (m_pScenes[m_CurrentScene]) m_pScenes[m_CurrentScene]->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	}
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		break;
	case WM_KEYUP:
		break;
	}
}

//윈도우 메세지 처리
LRESULT CFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScenes.count(m_CurrentScene) != 0)
	{
		if(m_pScenes[m_CurrentScene]) m_pScenes[m_CurrentScene]->OnProcessingWindowMessage(hWnd, nMessageID, wParam, lParam);
	}
	//윈도우 마우스 및 키보드 입력 메세지 처리
	OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_COMMAND:
		break;
	default:
		break;
	}
	return(0);
}
