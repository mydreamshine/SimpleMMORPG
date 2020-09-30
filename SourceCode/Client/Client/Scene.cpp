#include "stdafx.h"
#include "Scene.h"

list<array<char,BUF_SIZE>> TempBuffer;

// Scene 외부에서 공유하거나, 공유되어지는 데이터를 지정 (with 공유이력)
void CScene::SetSharedData(BYTE* ByteStream, rsize_t ByteSize, bool isCopy, bool RefOnly)
{
	if (isCopy == true)
	{
		if (ByteStream == nullptr) return;
		if (m_SharedData != nullptr && m_SharedConfig == SHARED_CONFIG_IS_COPYED) delete m_SharedData;
		m_SharedData = new BYTE[ByteSize];
		m_SharedDataByteSize = ByteSize;
		memcpy_s(m_SharedData, ByteSize, ByteStream, ByteSize);
		m_SharedConfig = SHARED_CONFIG_IS_COPYED;
	}
	else if (RefOnly == true)
	{
		if (m_SharedData != nullptr && m_SharedConfig == SHARED_CONFIG_IS_COPYED) delete m_SharedData;
		m_SharedData = ByteStream;
		m_SharedDataByteSize = ByteSize;
		m_SharedConfig = SHARED_CONFIG_IS_REFERENCED;
	}
}

CScene::~CScene()
{
	if (m_SharedData != nullptr && m_SharedConfig == SHARED_CONFIG_IS_COPYED) delete m_SharedData;
}

ULONG_PTR g_GdiPlusTokenBoxData_s;
CTitleScene::CTitleScene(HWND hWnd, int* CurrentScne) : CScene(hWnd, CurrentScne)
{
	// GDI+ Start
	GdiplusStartupInput GdiSartupInput;
	GdiplusStartup(&g_GdiPlusTokenBoxData_s, &GdiSartupInput, NULL);
}

CTitleScene::~CTitleScene()
{
	CTitleScene::ReleaseObjects();

	if (m_pCachedImage) delete m_pCachedImage;
	if (m_pScreen) delete m_pScreen;
	// GDI+ Shutdown
	GdiplusShutdown(g_GdiPlusTokenBoxData_s);
}

// 오브젝트 생성
void CTitleScene::BuildObjects()
{
	// ID input Box
	m_hInputBox[0] = CreateControlBox(m_hWnd, (LPSTR)"EDIT", (LPSTR)"", (FRAME_WIDTH / 2) - 50, (FRAME_HEIGHT / 2) + 200, 150, 30);
	SetFocus(m_hInputBox[0]);
	// Password input Box
	m_hInputBox[1] = CreateControlBox(m_hWnd, (LPSTR)"EDIT", (LPSTR)"", (FRAME_WIDTH / 2) - 50, (FRAME_HEIGHT / 2) + 250, 150, 30);
	SendMessage(m_hInputBox[1], EM_SETPASSWORDCHAR, (WPARAM)'*', 0);
	// Button
	m_hButton = CreateControlBox(m_hWnd, (LPSTR)"BUTTON", (LPSTR)"GAME START", (FRAME_WIDTH / 2) - 20, (FRAME_HEIGHT / 2) + 300, 100, 20, (HMENU)IDOK);
	
	// IP Address Box
	m_hIPAddressControl = CreateControlBox(m_hWnd, (LPSTR)WC_IPADDRESS, (LPSTR)"", FRAME_WIDTH - 155, FRAME_HEIGHT - 30);
	
	DWORD Init_ip = 0x7f000001; // '127.0.0.1'
	SendMessage(m_hIPAddressControl, IPM_SETADDRESS, 0, (LPARAM)Init_ip);

	m_IPAddress = inet_addr(LoopBack_IP);
}

// 오브젝트 해제
void CTitleScene::ReleaseObjects()
{
	// 컨트롤 박스 해제
	if (m_hInputBox[0]) DestroyWindow(m_hInputBox[0]);
	if (m_hInputBox[1]) DestroyWindow(m_hInputBox[1]);
	if (m_hButton) DestroyWindow(m_hButton);
	if (m_hIPAddressControl) DestroyWindow(m_hIPAddressControl);

	if (m_pScreen != nullptr) delete m_pScreen;
	if (m_pCachedImage != nullptr) delete m_pCachedImage;

	m_pScreen = nullptr;
	m_pCachedImage = nullptr;
}

void CTitleScene::SetFrameBuffer(HDC hDCFrameBuffer)
{
	m_hDCFrameBuffer = hDCFrameBuffer;
	if (m_pScreen != nullptr) delete m_pScreen;
	m_pScreen = new Graphics(hDCFrameBuffer);

	if (m_pCachedImage) delete m_pCachedImage;
	Bitmap OriginBitmap(L"images/title.png");

	Bitmap ResizeBitmap(FRAME_WIDTH, FRAME_HEIGHT);
	Graphics ResizeScreen(&ResizeBitmap);
	ResizeScreen.DrawImage(&OriginBitmap, 0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	m_pCachedImage = new CachedBitmap(ResizeBitmap.Clone(0, 0, FRAME_WIDTH, FRAME_HEIGHT, PixelFormatDontCare), m_pScreen);
}

// 렌더링
void CTitleScene::Render(HWND hWnd, HDC hDCFrameBuffer)
{
	if (m_pScreen == nullptr) return;
	m_pScreen->DrawCachedBitmap(m_pCachedImage, 0, 0);

	PrintText(hDCFrameBuffer, 30, "굴림체", RGB(48, 48, 48), (FRAME_WIDTH / 2) - 100, (FRAME_HEIGHT / 2) + 200, L"ID: ");
	PrintText(hDCFrameBuffer, 30, "굴림체", RGB(48, 48, 48), (FRAME_WIDTH / 2) - 190, (FRAME_HEIGHT / 2) + 250, L"PASSWORD: ");
}

// 윈도우 메세지 처리
LRESULT CTitleScene::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam)) {
			ProcessSocket::closeProtocol();
			err_msgbox((char*)"Socket Error");
			break;
		}
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_READ:
			ProcessSocket::ReadPacket((SOCKET)wParam, this);
			break;
		case FD_CLOSE:
			ProcessSocket::closeProtocol();
			err_msgbox((char*)"Connect Close");
			break;
		}
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			wchar_t id[MAX_WSTR_LEN];
			wchar_t password[MAX_WSTR_LEN];
			ZeroMemory(id, sizeof(id));
			ZeroMemory(password, sizeof(password));
			GetWindowTextW(m_hInputBox[0], id, MAX_WSTR_LEN);
			GetWindowTextW(m_hInputBox[1], password, MAX_WSTR_LEN);
			SendMessage(m_hIPAddressControl, IPM_GETADDRESS, 0, (LPARAM)&m_IPAddress);

			//sc_packet_user_info packet;
			//packet.type = SC_USER_INFO;
			//packet.size = sizeof(sc_packet_user_info);
			//packet.obj_id = 0;
			//packet.obj_type = OBJECT_TYPE_USER;
			//packet.obj_x = WORLD_CENTER - 1;
			//packet.obj_y = WORLD_CENTER - 1;
			//wsprintfW(packet.user_id_str, id);
			//packet.user_level = 1;
			//packet.user_exp = 50;
			//packet.user_hp = 80;
			//packet.user_mp = 30;
			//packet.user_inventory[0].What = OBJECT_TYPE_ITEM_NORMAL_ARMOR;
			//packet.user_inventory[0].How_many = 1;
			//packet.user_inventory[1].What = OBJECT_TYPE_ITEM_SPECIAL_SWORD;
			//packet.user_inventory[1].How_many = 1;
			//packet.user_inventory[2].What = OBJECT_TYPE_ITEM_NORMAL_RING;
			//packet.user_inventory[2].How_many = 1;
			//packet.user_inventory[3].What = OBJECT_TYPE_ITEM_COIN;
			//packet.user_inventory[3].How_many = 200;
			//packet.user_inventory[4].What = OBJECT_TYPE_NON;
			//packet.user_inventory[4].How_many = 0;
			//packet.user_inventory[5].What = OBJECT_TYPE_ITEM_HP_POTION;
			//packet.user_inventory[5].How_many = 10;
			//packet.user_inventory[6].What = OBJECT_TYPE_NON;
			//packet.user_inventory[6].How_many = 0;
			//packet.user_inventory[7].What = OBJECT_TYPE_ITEM_MP_POTION;
			//packet.user_inventory[7].How_many = 5;
			//packet.user_inventory[8].What = OBJECT_TYPE_NON;
			//packet.user_inventory[8].How_many = 0;
			//packet.user_quest.Preceded_Quest = QUEST_TYPE::NON;
			//packet.user_quest.Type = QUEST_TYPE::ENCOUNTER_GOBLIN_KING;
			//packet.user_quest.Quest_Progress[0].What = OBJECT_TYPE_MONSTER_GOBLIN_WARRIOR;
			//packet.user_quest.Quest_Progress[0].How = Do_Action::KILL;
			//packet.user_quest.Quest_Progress[0].How_many = 50;
			//packet.user_quest.Quest_Progress[1].What = OBJECT_TYPE_MONSTER_GOBLIN_KING;
			//packet.user_quest.Quest_Progress[1].How = Do_Action::FIND;
			//packet.user_quest.Quest_Progress[1].How_many = 1;
			//CScene::SetSharedData(reinterpret_cast<BYTE*>(&packet), packet.size);// copy packet
			//if (m_pCurrentSceneIndex != nullptr) *m_pCurrentSceneIndex = PLAY_SCENE;

			if (ProcessSocket::InitProtocol(m_hWnd, htonl(m_IPAddress)) == true)
				ProcessSocket::SendPacket(CS_LOGIN, id, password);
			else
				err_msgbox((char*)"Error");
		}
		break;
	}
	return(0);
}

void CTitleScene::ProcessPacketFrom(char* BufferStream)
{
	switch (BufferStream[1])
	{
	case SC_LOGIN_OK:
	{
		sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(BufferStream);
		CScene::SetSharedData(reinterpret_cast<BYTE*>(packet), packet->size);// copy packet
		if (m_pCurrentSceneIndex != nullptr) *m_pCurrentSceneIndex = PLAY_SCENE;
		break;
	}
	case SC_USER_INFO:
	case SC_PUT_OBJECT:
		TempBuffer.emplace_back();
		memcpy_s(&TempBuffer.back()[0], BUF_SIZE, BufferStream, (ULONG)BufferStream[0]);
		break;
	case SC_LOGIN_FAIL:
		MessageBox(NULL, (LPCTSTR)"로그인에 실패했습니다.", (LPCSTR)"Login Error", MB_ICONERROR);
		break;
	}
}

CPlayScene::~CPlayScene()
{
	CPlayScene::ReleaseObjects();
}

void CPlayScene::BuildObjects()
{
	if (m_pCSIMPEMMORPG) delete m_pCSIMPEMMORPG;
	m_pCSIMPEMMORPG = new C_SIMPLE_MMORPG();

	m_pCSIMPEMMORPG->LoadMap(L"map_data/tile_data.txt");
	m_pCSIMPEMMORPG->LoadMap(L"map_data/obstacle_data.txt");
	//m_pCSIMPEMMORPG->LoadMap(L"map_data/npc_data.txt");
	m_pCSIMPEMMORPG->Init();

	if (m_SharedData != nullptr)
	{
		m_pCSIMPEMMORPG->LoginOk(reinterpret_cast<char*>(m_SharedData));
	}
	if (TempBuffer.size() != 0)
	{
		for (auto& buffer : TempBuffer)
		{
			CPlayScene::ProcessPacketFrom(&buffer[0]);
		}
		TempBuffer.clear();
	}

	// Button
	m_hButton[0] = CreateControlBox(m_hWnd, (LPSTR)"BUTTON", (LPSTR)"RETURN TITLE", (VIEW_SIZE - 3) * GDI_TILE_SIZE + (int)(GDI_TILE_SIZE * 1.5f) - 57, FRAME_HEIGHT - 200, 114, 20, (HMENU)IDOK);
	m_hButton[1] = CreateControlBox(m_hWnd, (LPSTR)"BUTTON", (LPSTR)"GAME EXIT", (VIEW_SIZE - 3) * GDI_TILE_SIZE + (int)(GDI_TILE_SIZE * 1.5f) - 45, FRAME_HEIGHT - 140, 90, 20, (HMENU)IDYES);
}

void CPlayScene::ReleaseObjects()
{
	if (m_hButton[0]) DestroyWindow(m_hButton[0]);
	if (m_hButton[1]) DestroyWindow(m_hButton[1]);

	if (m_pCSIMPEMMORPG) delete m_pCSIMPEMMORPG;
	m_pCSIMPEMMORPG = nullptr;
}

void CPlayScene::ProcessInput()
{
	m_pCSIMPEMMORPG->ProcessInput();

	if (m_pCSIMPEMMORPG->myPlayerMoveDone() == true)
	{
		if(m_keys['W'] == true || m_keys[VK_UP] == true)
		{
			m_pCSIMPEMMORPG->myPlayerMoveBy(0, -1);
			ProcessSocket::SendPacket(CS_MOVE_UP);
			if (GetAsyncKeyState('W') == 0) m_keys['W'] = false;
		}
		else if (m_keys['A'] == true || m_keys[VK_LEFT] == true)
		{
			m_pCSIMPEMMORPG->myPlayerMoveBy(-1, 0);
			ProcessSocket::SendPacket(CS_MOVE_LEFT);
			if (GetAsyncKeyState('A') == 0) m_keys['A'] = false;
		}
		else if (m_keys['S'] == true || m_keys[VK_DOWN] == true)
		{
			m_pCSIMPEMMORPG->myPlayerMoveBy(0, 1);
			ProcessSocket::SendPacket(CS_MOVE_DOWN);
			if (GetAsyncKeyState('S') == 0) m_keys['S'] = false;
		}
		else if (m_keys['D'] == true || m_keys[VK_RIGHT] == true)
		{
			m_pCSIMPEMMORPG->myPlayerMoveBy(1, 0);
			ProcessSocket::SendPacket(CS_MOVE_RIGHT);
			if (GetAsyncKeyState('D') == 0) m_keys['D'] = false;
		}
	}
	if (m_pCSIMPEMMORPG->myPlayerAttackDone() == true)
	{
		if (m_keys['J'] == true)
		{
			//m_pCSIMPEMMORPG->myPlayerDoAttackAnimaition();
			ProcessSocket::SendPacket(CS_ATTACK);
			if (GetAsyncKeyState('J') == 0) m_keys['J'] = false;
		}
		else if (m_keys['K'] == true)
		{
			//m_pCSIMPEMMORPG->myPlayerDoPowerAttackAnimation();
			ProcessSocket::SendPacket(CS_POWER_ATTACK);
			if (GetAsyncKeyState('K') == 0) m_keys['K'] = false;
		}
	}
	if (m_pCSIMPEMMORPG->myPlayerItemUseDone() == true)
	{
		if (m_keys['8'] == true)
		{
			m_pCSIMPEMMORPG->myPlayerUseHP_Potion();
			if (GetAsyncKeyState('8') == 0) m_keys['8'] = false;
		}
		else if (m_keys['9'] == true)
		{
			m_pCSIMPEMMORPG->myPlayerUseMP_Potion();
			if (GetAsyncKeyState('9') == 0) m_keys['9'] = false;
		}
	}
	if (m_pCSIMPEMMORPG->myPlayerItemGetDone() == true)
	{
		if (m_keys[VK_SPACE] == true)
		{
			/*int ItemID = m_pCSIMPEMMORPG->Find_ItemInWorld();
			char SlotID = m_pCSIMPEMMORPG->Find_PutAbleSlot(ItemID);
			if (ItemID != -1 && SlotID != -1)
			{
				sc_packet_get_item_ok packet;
				packet.type = SC_ITEM_GET_OK;
				packet.size = sizeof(sc_packet_get_item_ok);
				packet.obj_id = ItemID;
				packet.slot_num = (unsigned char)SlotID;

				m_pCSIMPEMMORPG->ItemGetOk(reinterpret_cast<char*>(&packet));
			}*/
			ProcessSocket::SendPacket(CS_ITEM_GET);
			if (GetAsyncKeyState(VK_SPACE) == 0) m_keys[VK_SPACE] = false;
		}
	}

	// 마우스 L 버튼 체크
	if (m_LButtonDown == TRUE && m_KeepOnLButtonDown == TRUE)
	{
		m_KeepOnLButtonDown = FALSE;
		m_pCSIMPEMMORPG->StartMouseDrage(&m_mousePos);
	}
	else if (m_LButtonDown == FALSE && m_KeepOnLButtonDown == FALSE)
	{
		m_KeepOnLButtonDown = TRUE;
		m_pCSIMPEMMORPG->EndMouseDrage();
	}
}

void CPlayScene::SetFrameBuffer(HDC hDCFrameBuffer)
{
	m_pCSIMPEMMORPG->SetFrameBuffer(hDCFrameBuffer);
	m_pCSIMPEMMORPG->LoadPNG(L"images/tiles.png");
	m_pCSIMPEMMORPG->LoadPNG(L"images/obstacles.png");
	m_pCSIMPEMMORPG->LoadPNG(L"images/characters.png");
	m_pCSIMPEMMORPG->LoadPNG(L"images/items.png");
	m_pCSIMPEMMORPG->LoadPNG(L"images/attacks.png");
	m_pCSIMPEMMORPG->LoadPNG(L"images/fog.png");
}

void CPlayScene::Render(HWND hWnd, HDC hDCFrameBuffer)
{
	m_pCSIMPEMMORPG->Draw(hWnd, hDCFrameBuffer);
}

void CPlayScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	//case WM_RBUTTONDOWN:
		if (m_KeepOnLButtonDown == TRUE) m_LButtonDown = TRUE;
		break;
	case WM_LBUTTONUP:
	//case WM_RBUTTONUP:
		if (m_LButtonDown == TRUE) m_LButtonDown = FALSE;
		break;
	case WM_MOUSEMOVE:
		m_mousePos.x = LOWORD(lParam);
		m_mousePos.y = HIWORD(lParam);
		break;
	}
}

void CPlayScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		m_keys[wParam] = true;
		if (GetAsyncKeyState('W')) m_keys['W'] = true;
		if (GetAsyncKeyState('A')) m_keys['A'] = true;
		if (GetAsyncKeyState('S')) m_keys['S'] = true;
		if (GetAsyncKeyState('D')) m_keys['D'] = true;
		if (GetAsyncKeyState('J')) m_keys['J'] = true;
		if (GetAsyncKeyState('K')) m_keys['K'] = true;
		if (GetAsyncKeyState('8')) m_keys['8'] = true;
		if (GetAsyncKeyState('9')) m_keys['9'] = true;
		break;
	case WM_KEYUP:
		m_keys[wParam] = false;
		if (GetAsyncKeyState('W') == 0) m_keys['W'] = false;
		if (GetAsyncKeyState('A') == 0) m_keys['A'] = false;
		if (GetAsyncKeyState('S') == 0) m_keys['S'] = false;
		if (GetAsyncKeyState('D') == 0) m_keys['D'] = false;
		if (GetAsyncKeyState('J') == 0) m_keys['J'] = false;
		if (GetAsyncKeyState('K') == 0) m_keys['K'] = false;
		if (GetAsyncKeyState('8') == 0) m_keys['8'] = false;
		if (GetAsyncKeyState('9') == 0) m_keys['9'] = false;
		break;
	}
}

LRESULT CPlayScene::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam)) {
			ProcessSocket::closeProtocol();
			err_quit((char*)"Socket Error");
			break;
		}
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_READ:
			ProcessSocket::ReadPacket((SOCKET)wParam, this);
			break;
		case FD_CLOSE:
			ProcessSocket::closeProtocol();
			err_quit((char*)"Connect Close");
			break;
		}
	}
	case WM_COMMAND:
		HWND temp = m_hWnd;
		switch (LOWORD(wParam))
		{
		case IDOK:
			ProcessSocket::closeProtocol();
			if (m_pCurrentSceneIndex != nullptr) *m_pCurrentSceneIndex = TITLE_SCENE;
			break;
		case IDYES:
			exit(1);
			break;
		}
		break;
	}
	return(0);
}

// SC_LOGIN_OK        1 -> Process TitleScene
// SC_LOGIN_FAIL      2 -> Process TitleScene
// SC_PUT_OBJECT      3
// SC_REMOVE_OBJECT   4
// SC_OBJECT_POS      5
// SC_ATTACK          6
// SC_POWER_ATTACK    7
// SC_ITEM_GET_OK     8
// SC_LEVEL_UP        9
// SC_EXP_CHANGE      10
// SC_HP_CHANGE       11
// SC_MP_CHANGE       12
// SC_MONEY_CHANGE    13
// SC_QUEST_CHANGE    14
// SC_CHAT            15


void CPlayScene::ProcessPacketFrom(char* BufferStream)
{
	switch (BufferStream[1])
	{
	case SC_USER_INFO:
		m_pCSIMPEMMORPG->SetmyCharacter(BufferStream);
		break;
	case SC_PUT_OBJECT:
		m_pCSIMPEMMORPG->AddNewObject(BufferStream);
		break;
	case SC_REMOVE_OBJECT:
		m_pCSIMPEMMORPG->ReleaseObject(BufferStream);
		break;
	case SC_OBJECT_POS:
		m_pCSIMPEMMORPG->ObjectPos(BufferStream);
		break;
	case SC_ATTACK:
		m_pCSIMPEMMORPG->ObjectDoAttack(BufferStream);
		break;
	case SC_POWER_ATTACK:
		m_pCSIMPEMMORPG->ObjectDoPowerAttack(BufferStream);
		break;
	case SC_ITEM_GET_OK:
		m_pCSIMPEMMORPG->ItemGetOk(BufferStream);
		break;
	case SC_ITEM_DROP_OK:
		m_pCSIMPEMMORPG->ItemDropOk(BufferStream);
		break;
	case SC_ITEM_BUY_OK:
		m_pCSIMPEMMORPG->ItemBuyOk(BufferStream);
		break;
	case SC_ITEM_USE_OK:
		m_pCSIMPEMMORPG->ItemUseOk(BufferStream);
		break;
	case SC_LEVEL_UP:
		m_pCSIMPEMMORPG->LevelUp(BufferStream);
		break;
	case SC_EXP_CHANGE:
		m_pCSIMPEMMORPG->ExpChange(BufferStream);
		break;
	case SC_HP_CHANGE:
		m_pCSIMPEMMORPG->HPChange(BufferStream);
		break;
	case SC_MP_CHANGE:
		m_pCSIMPEMMORPG->MPChange(BufferStream);
		break;
	case SC_MONEY_CHANGE:
		m_pCSIMPEMMORPG->MoneyChange(BufferStream);
		break;
	case SC_QUEST_CHANGE:
		m_pCSIMPEMMORPG->QuestChange(BufferStream);
		break;
	case SC_CHAT:
		m_pCSIMPEMMORPG->SetChat(BufferStream);
		break;
	}
}
