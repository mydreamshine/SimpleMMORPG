#pragma once
#include "Scene.h"

class CFramework
{
public:
	CFramework();
	~CFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);		    // ������ �ڵ� ���� �� ����, ������Ʈ �ʱ�ȭ
	void OnDestroy();										    // ������ �� ����, ������Ʈ �޸� ����

	void FrameAdvance();										// ������ ����

	void SetActive(bool bActive) { m_bActive = bActive; }		// �������α׷� Ȱ��ȭ ���� ����

private:
	HINSTANCE					m_hInstance = NULL;				// �������α׷� ������ ��ü �ڵ�
	HWND						m_hWnd = NULL;					// �������α׷� ���� ������ �ڵ�
	
	bool						m_bActive = true;				// �������α׷� Ȱ��ȭ ����
	
	HDC							m_hDCFrameBuffer = NULL;		// �ĸ���� �ڵ�
	HBITMAP						m_hBitmapFrameBuffer = NULL;	// �ĸ���� �ؽ��� �ڵ�
	
	unordered_map<int, CScene*> m_pScenes;						// �ĸ���ۿ� �������� ���
	int                         m_CurrentScene = TITLE_SCENE;   // ���� ����� � �������.
	int                         m_CurrentSceneRef = TITLE_SCENE; // SceneŬ������ ���� ���� ����� ��ȭ�� �����ϴ�. (Scene Ŭ���� ������)
	int                         m_PrevScene = TITLE_SCENE;      // ���� ����� � ����̾�����.

public:
	void BuildFrameBuffer();									// �ĸ���� ����
	void ClearFrameBuffer(DWORD dwColor);						// �ĸ���� �ʱ�ȭ
	void PresentFrameBuffer();									// �ĸ���ۿ� ������� ����

	void BuildObjects();										// ���α׷��� ���� ��ü �� ������ �ʱ�ȭ
	void ReleaseObjects();										// ���α׷��� ������ ��ü �� ������ �ʱ�ȭ
	void ProcessInput();										// �� �����ӿ��� ó���� �Է�ó��

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	  // ������ ���콺 �޼��� ó��
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	  // ������ Ű���� �޼��� ó��
	LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);  // ������ �޼��� ó��
};
