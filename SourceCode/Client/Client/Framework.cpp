#include "stdafx.h"
#include "Framework.h"

CFramework::CFramework()
{
}

CFramework::~CFramework()
{
}

//������ �ڵ� ���� �� ����, ������Ʈ �ʱ�ȭ
bool CFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	//������ �ڵ� ����
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//������ ����
	CFramework::BuildFrameBuffer();

	//������Ʈ ����
	CFramework::BuildObjects();

	return(true);
}

//������ �� ����, ������Ʈ �޸� ����
void CFramework::OnDestroy()
{
	//�ĸ���� ����
	if (m_hBitmapFrameBuffer) DeleteObject(m_hBitmapFrameBuffer);
	if (m_hDCFrameBuffer) DeleteDC(m_hDCFrameBuffer);

	//������ ����
	if (m_hWnd) DestroyWindow(m_hWnd);

	//������Ʈ ����
	CFramework::ReleaseObjects();
}

//������ ����
void CFramework::FrameAdvance()
{
	// ���� ���(Scene) index�� ���� �����ȯ
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
		//�����찡 Ȱ��ȭ�Ǿ� ���� ������ �Է�ó������ ���� (�����κ��� ���� �����Ͱ� ������ ����)
		if (m_bActive) CFramework::ProcessInput();
	}

	//�ĸ���� �ʱ�ȭ
	CFramework::ClearFrameBuffer(RGB(0, 0, 0));

	//������
	m_pScenes[m_CurrentScene]->Render(m_hWnd, m_hDCFrameBuffer);

	//������ۿ� �ĸ���� ����
	CFramework::PresentFrameBuffer();
}


// �������� ���� �ĸ� ���� ���� �� �ʱ�ȭ
void CFramework::BuildFrameBuffer()
{
	//���� �������α׷��� ������κ��� ������� �ڵ��� ������
	HDC hDC = GetDC(m_hWnd);

	//�������α׷��� ������� ���� ��������
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);

	//������ۿ� ������ ũ���� �ĸ���� ����
	//�ĸ���ۿ� �׸� ��Ʈ�� ������ �ؽ��� ����
	m_hDCFrameBuffer = CreateCompatibleDC(hDC);
	m_hBitmapFrameBuffer = CreateCompatibleBitmap(hDC, (rcClient.right - rcClient.left) + 1, (rcClient.bottom - rcClient.top) + 1);
	SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);

	//������� �ڵ� ���� �� �ĸ������ �׸��� ��� ����
	ReleaseDC(m_hWnd, hDC);
	SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
}

// �������� ���� �ĸ� ���� �ʱ�ȭ
void CFramework::ClearFrameBuffer(DWORD dwColor)
{
	//�������α׷��� ������� ���� ��������
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	m_pScenes[m_CurrentScene]->m_rcClient = rcClient;

	//�ĸ���۸� fillcolor �������� �ʱ�ȭ
	HBRUSH fillcolor = CreateSolidBrush(dwColor);
	FillRect(m_hDCFrameBuffer, &rcClient, fillcolor);
	DeleteObject(fillcolor);
}

//�ĸ���ۿ� ������� ����
void CFramework::PresentFrameBuffer()
{
	//���� �������α׷��� ������κ��� ������� �ڵ��� ������
	HDC hDC = GetDC(m_hWnd);

	//�������α׷��� ������� ���� ��������
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	m_pScenes[m_CurrentScene]->m_rcClient = rcClient;

	//������� ũ��
	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;

	//�ĸ������ ������ ������۷� BitBlt(��Ӻ���) ����
	BitBlt(hDC, rcClient.left, rcClient.top, nWidth, nHeight, m_hDCFrameBuffer, rcClient.left, rcClient.top, SRCCOPY);
	ReleaseDC(m_hWnd, hDC);//������� �ڵ� ����

	//������۸� �ٽ� �׸���� �޼����� �޼���ť�� ����
	InvalidateRect(m_hWnd, NULL, FALSE);
}

// ���α׷��� ���� ��ü �� ������ �ʱ�ȭ
void CFramework::BuildObjects()
{
	//�ĸ���ۿ� �׸� ��� ����
	if (m_pScenes.count(m_CurrentSceneRef) == 0)
	{
		if (m_CurrentSceneRef == TITLE_SCENE)
			m_pScenes[m_CurrentSceneRef] = new CTitleScene(m_hWnd, &m_CurrentSceneRef);
		else if (m_CurrentSceneRef == PLAY_SCENE)
			m_pScenes[m_CurrentSceneRef] = new CPlayScene(m_hWnd, &m_CurrentSceneRef);
	}

	// ������ �������� ������Ʈ ����
	m_pScenes[m_CurrentSceneRef]->ReleaseObjects();

	// Scene �� �����ڿ� ����
	BYTE* SharedData = m_pScenes[m_PrevScene]->GetSharedData();
	if (SharedData != nullptr)
	{
		rsize_t SharedByteSize = m_pScenes[m_PrevScene]->GetSharedDataByteSize();
		// Reference Only
		m_pScenes[m_CurrentSceneRef]->SetSharedData(SharedData, SharedByteSize, false, true);
	}

	//��鿡 ����� ������Ʈ�� ���� �� �ʱ�ȭ
	m_pScenes[m_CurrentSceneRef]->BuildObjects();

	//�������α׷��� ������� ���� ��������
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);

	//����� ������ ���� �������α׷��� ��������� ������ ����
	m_pScenes[m_CurrentSceneRef]->m_rcClient = rcClient;

	// Scene�� �ĸ���� ���
	m_pScenes[m_CurrentSceneRef]->SetFrameBuffer(m_hDCFrameBuffer);
}

//���α׷��� ������ ��ü �� ������ ����
void CFramework::ReleaseObjects()
{
	//�ĸ� ���ۿ� �׷����� ��� ����
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

//�� �����ӿ��� ó���� �Է�ó��
void CFramework::ProcessInput()
{
	m_pScenes[m_CurrentScene]->ProcessInput();
}

//������ ���콺 �޼��� ó��
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

//������ Ű���� �޼��� ó��
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

//������ �޼��� ó��
LRESULT CFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScenes.count(m_CurrentScene) != 0)
	{
		if(m_pScenes[m_CurrentScene]) m_pScenes[m_CurrentScene]->OnProcessingWindowMessage(hWnd, nMessageID, wParam, lParam);
	}
	//������ ���콺 �� Ű���� �Է� �޼��� ó��
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
