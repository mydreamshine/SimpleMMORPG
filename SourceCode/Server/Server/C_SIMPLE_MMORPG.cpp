#include "C_SIMPLE_MMORPG.h"
#include <fstream>

void TransformSlot(int SlotNum, int & x, int & y)
{
	int DrawStartX = (VIEW_SIZE - 3) * GDI_TILE_SIZE - 12;
	int DrawStartY = 0;

	// Equipped
	if (SlotNum < 3)
	{
		x = DrawStartX + 3 * (SlotNum + 1) + GDI_ITEM_SIZE * SlotNum - 1;
		y = DrawStartY + 299;
	}
	else
	{
		SlotNum -= 3;
		x = DrawStartX + 3 * (SlotNum % 3 + 1) + GDI_ITEM_SIZE * (SlotNum % 3) - 1;
		y = DrawStartY + 399 + (SlotNum / 3) * SlotNum + GDI_ITEM_SIZE * (SlotNum / 3);
	}
}

bool PointInSlot(int x, int y, RECT rect)
{
	if (x < rect.left) return false;
	if (x > rect.right) return false;
	if (y < rect.top) return false;
	if (y > rect.bottom) return false;
	return true;
}

C_SIMPLE_MMORPG::~C_SIMPLE_MMORPG()
{
	C_SIMPLE_MMORPG::ReleaseObjects();
}

// Init Map, Objects, ...
void C_SIMPLE_MMORPG::init()
{
	for (int i = 0; i < WORLD_SIZE; ++i)
	{
		memset(m_TileData[i], OBJECT_TYPE_NON, WORLD_SIZE);
		memset(m_Obstacles[i], OBJECT_TYPE_NON, WORLD_SIZE);
	}
	ZeroMemory(m_pObjects, sizeof(m_pObjects));
	C_SIMPLE_MMORPG::LoadMap(L"map_data/tile_data.txt");
	C_SIMPLE_MMORPG::LoadMap(L"map_data/obstacle_data.txt");
	C_SIMPLE_MMORPG::LoadMap(L"map_data/npc_data.txt");
}

void C_SIMPLE_MMORPG::LoadMap(const WCHAR* filename)
{
	ifstream file(filename);
	string in_line;
	int x = 0, y = 0;
	if (file.is_open() != true) return;

	getline(file, in_line);
	while (getline(file, in_line))
	{
		x = 0;
		for (int i = 0; i < in_line.size(); ++i)
		{
			// Tile
			if (in_line[i] == 't')
			{
				i++;
				if (i >= in_line.size()) break;
				switch (in_line[i])
				{
				case '0':
					m_TileData[y][x] = OBJECT_TPYE_TILE_WATER;
					break;
				case '1':
					m_TileData[y][x] = OBJECT_TPYE_TILE_GRASS;
					break;
				case '2':
					m_TileData[y][x] = OBJECT_TPYE_TILE_SAND;
					break;
				}
				x++;
			}
			// Obstacle
			else if (in_line[i] == 'o')
			{
				i++;
				if (i >= in_line.size()) break;
				switch (in_line[i])
				{
				case '0':
					m_Obstacles[y][x] = OBJECT_TYPE_OBSTACLE_WALL;
					break;
				case '1':
					m_Obstacles[y][x] = OBJECT_TYPE_OBSTACLE_ROCK;
					break;
				case '2':
					m_Obstacles[y][x] = OBJECT_TYPE_OBSTACLE_TREE;
					break;
				}
				x++;
			}
			// Character
			else if (in_line[i] == 'c')
			{
				i++;
				if (i >= in_line.size()) break;
				switch (in_line[i])
				{
				case '0':
					//m_pObjects[NPC_START_INDEX + obj_id++] = new USER(OBJECT_TYPE_USER, x, y);
					break;
				case '1':
					C_SIMPLE_MMORPG::CreateNPC(OBJECT_TYPE_NPC_CHIEF, x, y);
					break;
				case '2':
					C_SIMPLE_MMORPG::CreateNPC(OBJECT_TYPE_NPC_BLACKSMITH, x, y);
					break;
				case '3':
					C_SIMPLE_MMORPG::CreateNPC(OBJECT_TYPE_NPC_POTION_MERCHANT, x, y);
					break;
				case '4':
					C_SIMPLE_MMORPG::CreateMonster(OBJECT_TYPE_MONSTER_SCORPION, x, y);
					break;
				case '5':
					C_SIMPLE_MMORPG::CreateMonster(OBJECT_TYPE_MONSTER_SCORPION_KING, x, y);
					break;
				case '6':
					C_SIMPLE_MMORPG::CreateMonster(OBJECT_TYPE_MONSTER_THEIF, x, y);
					break;
				case '7':
					C_SIMPLE_MMORPG::CreateMonster(OBJECT_TYPE_MONSTER_THEIF_BOSS, x, y);
					break;
				case '8':
					C_SIMPLE_MMORPG::CreateMonster(OBJECT_TYPE_MONSTER_WOLF, x, y);
					break;
				case '9':
					C_SIMPLE_MMORPG::CreateMonster(OBJECT_TYPE_MONSTER_WEREWOLF, x, y);
					break;
				case '10':
					C_SIMPLE_MMORPG::CreateMonster(OBJECT_TYPE_MONSTER_GOBLIN_WARRIOR, x, y);
					break;
				case '11':
					C_SIMPLE_MMORPG::CreateMonster(OBJECT_TYPE_MONSTER_GOBLIN_KING, x, y);
					break;
				}
				x++;
			}
		}
		y++;
	}
}
void C_SIMPLE_MMORPG::ReleaseObjects()
{
	for (int i = 0; i < MAX_CLIENTS + MAX_NPC; ++i)
	{
		if (m_pObjects[i] != nullptr)
		{
			delete m_pObjects[i];
			m_pObjects[i] = nullptr;
		}
	}

	// Clear Sector
	for (int i = 0; i < SECTOR_COUNT; ++i)
		m_Sector[i].clear();
}

int C_SIMPLE_MMORPG::CreateNPC(unsigned char type, int x, int y)
{
	// Set ID
	if (m_UseableNPC_ID == NPC_START_INDEX + MAX_NPC) return -1;
	int NewID = m_UseableNPC_ID++;

	// ID는 인덱스로서 활용
	// 새로운 ID로 NPC 생성
	NPC* npc = new NPC(type, x, y);
	m_pObjects[NewID] = npc;
	m_pObjects[NewID]->isAlive = true;
	m_pObjects[NewID]->isActive = true;

	// Set Sector
	m_Sector[C_SIMPLE_MMORPG::GetSectorIndex_By(x, y)][NewID] = npc;

	return NewID;
}

int C_SIMPLE_MMORPG::CreateMonster(unsigned char type, int x, int y)
{
	// Set ID
	if (m_UseableNPC_ID == NPC_START_INDEX + MAX_NPC) return -1;
	int NewID = m_UseableNPC_ID++;

	// ID는 인덱스로서 활용
	// 새로운 ID로 NPC 생성
	MONSTER* monster = new MONSTER(type, x, y);
	m_pObjects[NewID] = monster;
	if (OBJECT_TYPE_MONSTER_SCORPION_KING < type)
		monster->Aggro_Type = AGGRO_TYPE_BATTLE;
	else monster->Aggro_Type = AGGRO_TYPE_PEACE;
	monster->respawn_x = x;
	monster->respawn_y = y;
	monster->isAlive = true;
	monster->isActive = false;
	monster->Damage = 20;
	monster->Defense = 10;
	monster->HP = 100;
	monster->MoveDirection = rand() % (MONSTER_MOVE_RIGHT + 1);

	// Set Sector
	m_Sector[C_SIMPLE_MMORPG::GetSectorIndex_By(x, y)][NewID] = monster;

	return NewID;
}

int C_SIMPLE_MMORPG::ConnectUser()
{
	// Set ID
	int NewID;
	if (!GetUserID(NewID)) return -1;

	// ID는 인덱스로서 활용
	// 새로운 ID로 유저 생성
	USER* user;
	if (m_pObjects[NewID] == nullptr)
		m_pObjects[NewID] = user = new USER();
	else
		user = reinterpret_cast<USER*>(m_pObjects[NewID]);

	user->isConnect = true;

	return NewID;
}

void C_SIMPLE_MMORPG::LoginUser(int user_ID, wchar_t* user_name, char ConnectionType)
{
	USER* user = reinterpret_cast<USER*>(m_pObjects[user_ID]);

	if (user_name != NULL) memcpy_s(user->ID_str, sizeof(user->ID_str), user_name, sizeof(user->ID_str));
	else ZeroMemory(user->ID_str, MAX_WSTR_LEN);
	user->ConnectType = ConnectionType;
	user->isLogin = true;
}

bool C_SIMPLE_MMORPG::GameStart(int user_ID, queue<EVENT*>& GeneratedEvents, bool by_user_info, USER* user_info)
{
	USER* user = reinterpret_cast<USER*>(m_pObjects[user_ID]);

	// Set Object Type
	user->Type = OBJECT_TYPE_USER;

	// Set Player Pos
	user->x = WORLD_CENTER - 1;
	user->y = WORLD_CENTER - 1;

	// Set Stat
	user->Level = 1;
	user->Experience = 0;
	user->HP = 100;
	user->MP = 100;
	user->Damage = 20;
	user->Defense = 10;

	if (by_user_info == true)
	{
		user->x = user_info->x;
		user->y = user_info->y;
		user->Level = user_info->Level;
		user->Experience = user_info->Experience;
		user->HP = user_info->HP;
		user->MP = user_info->MP;
		// Set Quest
		memcpy_s(&user->Quest, sizeof(QUEST), &user_info->Quest, sizeof(QUEST));
		// Set Inventory
		memcpy_s(&user->Inventory, sizeof(INVENTORY), &user_info->Inventory, sizeof(INVENTORY));
	}
	user->isAlive = true;

	// Set Event
	GeneratedEvents.push(new TIME_EVENT(EVENT_TYPE::EVENT_AFTER_SOME_TIME, user_ID, user_ID, USER_HEAL, high_resolution_clock::now() + HEAL_COOLTIME));

	// Set View-List
	unordered_set<int> new_ViewList;
	
	RECT ViewRange{ user->x - USER_VIEW_RADIUS, user->y - USER_VIEW_RADIUS, user->x + USER_VIEW_RADIUS, user->y + USER_VIEW_RADIUS };
	RECT WakeupRange{ user->x - USER_VIEW_RADIUS_EX, user->y - USER_VIEW_RADIUS_EX, user->x + USER_VIEW_RADIUS_EX, user->y + USER_VIEW_RADIUS_EX };

	// USER의 시야가 2개이상의 Sector를 관찰하고 있는 경우에는 각 Sector별로 시야 범위 밖 오브젝트를 간추려내야 한다.
	unordered_set<int> Sector_indexs;
	// USER의 시야 크기는 21x21, Sector의 크기는 80x80이고
	// 월드의 구조가 그리드 구조이기 때문에
	// USER가 관찰 할 수 있는 (= USER의 시야 영역 안에 들어올 수 있는) Sector는 최대 4개이다.
	Sector_indexs.insert(C_SIMPLE_MMORPG::GetSectorIndex_By(WakeupRange.left, WakeupRange.top));
	Sector_indexs.insert(C_SIMPLE_MMORPG::GetSectorIndex_By(WakeupRange.left, WakeupRange.bottom));
	Sector_indexs.insert(C_SIMPLE_MMORPG::GetSectorIndex_By(WakeupRange.right, WakeupRange.top));
	Sector_indexs.insert(C_SIMPLE_MMORPG::GetSectorIndex_By(WakeupRange.right, WakeupRange.bottom));

	vector<unordered_map<int, OBJECT*>> SectorsByRange(Sector_indexs.size(), unordered_map<int, OBJECT*>());
	int SectorsByRange_index = 0;
	for (auto Sector_index : Sector_indexs)
	{
		// Sector에 대한 접근이 다른 스레드에서도 빈번히 일어난다. (현재 게임 구조는 오브젝트 생성, 삭제, 이동시 해당 오브젝트가 속해있는 Sector에 접근이 일어나는 구조이다.)
		Sector_other_Access.lock();
		SectorsByRange[SectorsByRange_index++] = m_Sector[Sector_index];
		Sector_other_Access.unlock();
	}

	list<int> ObjList;
	// list 구조여서 원소의 중복이 허용된다.
	// 그러나 서로 다른 Sector들 간에 동일한 오브젝트를 가지고 있는 경우는 없기 때문에 (Sector를 벗어날 때 확실히 erase를 해주기 때문.(+ Lock()-UnLock()과 같이.))
	// list 구조로 시야 범위 밖 오브젝트를 관리해도 문제없다.
	for (auto& Sector : SectorsByRange)
	{
		for (auto& Sector_element : Sector)
			ObjList.emplace_back(Sector_element.first); // 오브젝트 ID
	}

	for (auto i : ObjList)
	{
		// 새롭게 할당된 ID이거나 비활성 오브젝트는 영역 체크에서 제외
		if (i == user_ID) continue;
		if (m_pObjects[i] == nullptr) continue;
		if (m_pObjects[i]->isAlive == false) continue;

		// Monster 활성화 영역 체크
		if (OBJECT_TYPE_MONSTER_SCORPION <= m_pObjects[i]->Type && m_pObjects[i]->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING && m_pObjects[i]->isActive == false)// 해당 오브젝트가 Monster일 경우,
		{
			if (RectIn(WakeupRange.left, WakeupRange.top, WakeupRange.right, WakeupRange.bottom, m_pObjects[i]->x, m_pObjects[i]->y) == true)
			{
				m_pObjects[i]->isActive = true;
				GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, i, user_ID, USER_MOVED));
			}
		}

		// 시야 영역 체크
		if (RectIn(ViewRange.left, ViewRange.top, ViewRange.right, ViewRange.bottom, m_pObjects[i]->x, m_pObjects[i]->y) == true)
		{
			// 시야 영역 안에 해당 오브젝트 삽입
			new_ViewList.insert(i);
			// Set Event (SC_PUT_OBJECT)
			int how_many = 1;
			if (OBJECT_TYPE_ITEM_HP_POTION <= m_pObjects[i]->Type && m_pObjects[i]->Type <= OBJECT_TYPE_ITEM_SPECIAL_RING)
			{
				ITEM* droppedItem = reinterpret_cast<ITEM*>(m_pObjects[i]);
				how_many = droppedItem->How_many;
			}

			GeneratedEvents.push(new OBJECT_PUT_EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, i, SC_PUT_OBJECT, how_many));

			// 해당 오브젝트가 다른 User일 경우,
			if (i < MAX_CLIENTS)
			{
				// 해당 유저의 시야 영역 안에 새롭게 할당된 유저 삽입
				USER* otherUser = reinterpret_cast<USER*>(m_pObjects[i]);
				otherUser->viewlist_otherAccess.lock();
				otherUser->viewlist.insert(user_ID);
				otherUser->viewlist_otherAccess.unlock();

				GeneratedEvents.push(new OBJECT_PUT_EVENT(EVENT_TYPE::EVENT_DIRECT, i, user_ID, SC_PUT_OBJECT, 1));
				GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, i, SC_HP_CHANGE, (unsigned char)otherUser->Type, otherUser->HP));
				GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, i, user_ID, SC_HP_CHANGE, (unsigned char)user->Type, user->HP));
			}
			else if ((OBJECT_TYPE_MONSTER_SCORPION <= m_pObjects[i]->Type && m_pObjects[i]->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING))
			{
				MONSTER* otherMonster = reinterpret_cast<MONSTER*>(m_pObjects[i]);
				GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, i, SC_HP_CHANGE, (unsigned char)otherMonster->Type, otherMonster->HP));
			}

			if (m_pObjects[i]->Type == OBJECT_TYPE_MONSTER_GOBLIN_KING)
			{
				if (user->Quest.Type == QUEST_TYPE::ENCOUNTER_GOBLIN_KING && user->Quest.Quest_Progress[1].How_many == 0)
				{
					user->Quest.Quest_Progress[1].How_many = 1;
					GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, i, SC_QUEST_CHANGE));
				}
			}

		}
	}

	// 다른 스레드에서 접근할 만한 변수들에 대해서는 가급적이면 Lock()-UnLock()을 한번만 하고 한꺼번에 값을 변경한다.
	user->viewlist_otherAccess.lock();
	user->viewlist = new_ViewList;
	user->viewlist_otherAccess.unlock();

	Sector_other_Access.lock();
	m_Sector[C_SIMPLE_MMORPG::GetSectorIndex_By(user->x, user->y)][user_ID] = user; // Set Sector
	Sector_other_Access.unlock();

	return true;
}

void C_SIMPLE_MMORPG::ExitUser(int user_ID, queue<EVENT*>& GeneratedEvents)
{
	USER* user = reinterpret_cast<USER*>(m_pObjects[user_ID]);
	user->viewlist_otherAccess.lock();
	unordered_set<int> ViewList = user->viewlist;
	user->viewlist_otherAccess.unlock();

	// 해당 유저를 비활성화 하기 전에
	// 해당 유저가 지닌 시야 영역 내부의
	// 다른 유저의 시야 영역에서 해당 유저를 삭제 시켜줘야 한다.
	for (auto ID_in_View : ViewList)
	{
		if(ID_in_View < MAX_CLIENTS)
		{
			USER* other_user = reinterpret_cast<USER*>(m_pObjects[ID_in_View]);

			other_user->viewlist_otherAccess.lock();
			other_user->viewlist.erase(user_ID);
			other_user->viewlist_otherAccess.unlock();
			GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_View, user_ID, SC_REMOVE_OBJECT));
		}
		else if (OBJECT_TYPE_MONSTER_SCORPION <= m_pObjects[ID_in_View]->Type && m_pObjects[ID_in_View]->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING)// 해당 오브젝트가 Monster일 경우,
		{
			m_pObjects[ID_in_View]->isActive = false;// 비활성화(FALSE)로 해도 다른 유저의 ViewList에 남아있을 경우에는 다시 활성화(TRUE)가 되기 때문에 활성화 여부만 FALSE로 해준다.
		}
	}

	// 다른 스레드에서 접근할 만한 변수들에 대해서는 Lock()-UnLock()을 한번만 하고 한꺼번에 값을 변경한다.
	user->viewlist_otherAccess.lock();
	user->viewlist.clear();
	user->viewlist_otherAccess.unlock();
	user->isConnect = false;
	user->isLogin = false;
	user->isAlive = false;
	Sector_other_Access.lock();
	m_Sector[C_SIMPLE_MMORPG::GetSectorIndex_By(user->x, user->y)].erase(user_ID); // Set Sector
	Sector_other_Access.unlock();
}

void C_SIMPLE_MMORPG::SetUserViewList(int user_ID, queue<EVENT*>& GeneratedEvents)
{
	USER* user = reinterpret_cast<USER*>(m_pObjects[user_ID]);
	int old_SectorIndex = C_SIMPLE_MMORPG::GetSectorIndex_By(user->x, user->y);
	int px = user->x;
	int py = user->y;
	user->viewlist_otherAccess.lock();
	unordered_set<int> ViewList = user->viewlist;
	unordered_set<int> new_ViewList = ViewList;
	user->viewlist_otherAccess.unlock();

	// 해당 유저가 이동함에 따라 시야 영역에서 오브젝트를 제외.
	RECT ViewRange{ px - USER_VIEW_RADIUS, py - USER_VIEW_RADIUS, px + USER_VIEW_RADIUS, py + USER_VIEW_RADIUS };
	for (auto ID_in_View : ViewList)
	{
		// 시야 영역 체크 (시야 안 -> 시야 밖)
		if (RectIn(ViewRange.left, ViewRange.top, ViewRange.right, ViewRange.bottom, m_pObjects[ID_in_View]->x, m_pObjects[ID_in_View]->y) == false)
		{
			// 해당 오브젝트를 시야 영역에서 제외
			new_ViewList.erase(ID_in_View);

			// Set Event (SC_REMOVE_OBJECT)
			GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, ID_in_View, SC_REMOVE_OBJECT));

			// 해당 오브젝트가 다른 User일 경우,
			if (ID_in_View < MAX_CLIENTS)
			{
				// 해당 유저의 시야영역에서 이동한 유저를 제외
				USER* otherUser = reinterpret_cast<USER*>(m_pObjects[ID_in_View]);
				otherUser->viewlist_otherAccess.lock();
				otherUser->viewlist.erase(user_ID);
				otherUser->viewlist_otherAccess.unlock();

				// Set Event (SC_REMOVE_OBJECT)
				GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_View, user_ID, SC_REMOVE_OBJECT));
			}
		}
		else
		{
			// 시야 범위 내의 모든 오브젝트에게 움직였다라는 정보를 갱신
			if (ID_in_View < MAX_CLIENTS)
				GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_View, user_ID, SC_OBJECT_POS));
			else if (OBJECT_TYPE_MONSTER_SCORPION <= m_pObjects[ID_in_View]->Type && m_pObjects[ID_in_View]->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING && m_pObjects[ID_in_View]->isActive == false)// 해당 오브젝트가 Monster일 경우,
			{
				m_pObjects[ID_in_View]->isActive = true;
				// Set Event (PLAYER_MOVE)
				GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_View, user_ID, USER_MOVED));
			}
		}
	}


	RECT WakeupRange{ px - USER_VIEW_RADIUS_EX, py - USER_VIEW_RADIUS_EX, px + USER_VIEW_RADIUS_EX, py + USER_VIEW_RADIUS_EX };

	// 시야 범위 밖에 있는 오브젝트
	// USER의 시야가 2개이상의 Sector를 관찰하고 있는 경우에는 각 Sector별로 시야 범위 밖 오브젝트를 간추려내야 한다.
	unordered_set<int> Sector_indexs;
	// USER의 시야 크기는 21x21, Sector의 크기는 80x80이고
	// 월드의 구조가 그리드 구조이기 때문에
	// USER가 관찰 할 수 있는 (= USER의 시야 영역 안에 들어올 수 있는) Sector는 최대 4개이다.
	Sector_indexs.insert(C_SIMPLE_MMORPG::GetSectorIndex_By(WakeupRange.left, WakeupRange.top));
	Sector_indexs.insert(C_SIMPLE_MMORPG::GetSectorIndex_By(WakeupRange.left, WakeupRange.bottom));
	Sector_indexs.insert(C_SIMPLE_MMORPG::GetSectorIndex_By(WakeupRange.right, WakeupRange.top));
	Sector_indexs.insert(C_SIMPLE_MMORPG::GetSectorIndex_By(WakeupRange.right, WakeupRange.bottom));

	vector<unordered_map<int, OBJECT*>> SectorsByRange(Sector_indexs.size(), unordered_map<int, OBJECT*>());
	int SectorsByRange_index = 0;
	for (auto Sector_index : Sector_indexs)
	{
		// Sector에 대한 접근이 다른 스레드에서도 빈번히 일어난다. (현재 게임 구조는 오브젝트 생성, 삭제, 이동시 해당 오브젝트가 속해있는 Sector에 접근이 일어나는 구조이다.)
		Sector_other_Access.lock();
		SectorsByRange[SectorsByRange_index++] = m_Sector[Sector_index];
		Sector_other_Access.unlock();
	}

	list<int> OutSideList_View;
	// list 구조여서 원소의 중복이 허용된다.
	// 그러나 서로 다른 Sector들 간에 동일한 오브젝트를 가지고 있는 경우는 없기 때문에 (Sector를 벗어날 때 확실히 erase를 해주기 때문.(+ Lock()-UnLock()과 같이.))
	// list 구조로 시야 범위 밖 오브젝트를 관리해도 문제없다.
	for (auto& Sector : SectorsByRange)
	{
		for (auto& Sector_element : Sector)
			OutSideList_View.emplace_back(Sector_element.first); // 오브젝트 ID
	}

	// USER의 시야 영역 안에 들어올 수 있는 Sector들의 모든 오브젝트 중
	// USER의 시야 영역 안 오브젝트는 제외
	// 즉, USER의 시야 영역 밖 오브젝트만 참조하도록 구성 (이동을 하기전 시야영역 기준.)
	for (auto& ID_in_View : ViewList)
		OutSideList_View.remove(ID_in_View);

	for (auto ID_in_Outside : OutSideList_View)
	{
		// 입력처리를 한 유저의 ID이거나 비활성 오브젝트는 시야 영역 체크에서 제외
		if (ID_in_Outside == user_ID) continue;
		if (m_pObjects[ID_in_Outside] == nullptr) continue;
		if (m_pObjects[ID_in_Outside]->isAlive == false) continue;

		// Monster 활성화 영역 체크
		if (OBJECT_TYPE_MONSTER_SCORPION <= m_pObjects[ID_in_Outside]->Type && m_pObjects[ID_in_Outside]->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING && m_pObjects[ID_in_Outside]->isActive == false)// 해당 오브젝트가 Monster일 경우,
		{
			if (RectIn(WakeupRange.left, WakeupRange.top, WakeupRange.right, WakeupRange.bottom, m_pObjects[ID_in_Outside]->x, m_pObjects[ID_in_Outside]->y) == true)
			{
				m_pObjects[ID_in_Outside]->isActive = true;
				GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_Outside, user_ID, USER_MOVED));
			}
		}

		// 시야 영역 체크(시야 밖 -> 시야 안)
		if (RectIn(ViewRange.left, ViewRange.top, ViewRange.right, ViewRange.bottom, m_pObjects[ID_in_Outside]->x, m_pObjects[ID_in_Outside]->y) == true)
		{
			// 시야 영역 안에 해당 오브젝트 삽입
			new_ViewList.insert(ID_in_Outside);
			// Set Event (SC_PUT_OBJECT)
			int how_many = 1;
			if (OBJECT_TYPE_ITEM_HP_POTION <= m_pObjects[ID_in_Outside]->Type && m_pObjects[ID_in_Outside]->Type <= OBJECT_TYPE_ITEM_SPECIAL_RING)
			{
				ITEM* droppedItem = reinterpret_cast<ITEM*>(m_pObjects[ID_in_Outside]);
				how_many = droppedItem->How_many;
			}

			GeneratedEvents.push(new OBJECT_PUT_EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, ID_in_Outside, SC_PUT_OBJECT, how_many));

			if (ID_in_Outside < MAX_CLIENTS)
			{
				// 해당 유저의 시야 영역 안에 새롭게 할당된 유저 삽입
				USER* otherUser = reinterpret_cast<USER*>(m_pObjects[ID_in_Outside]);
				otherUser->viewlist_otherAccess.lock();
				otherUser->viewlist.insert(user_ID);
				otherUser->viewlist_otherAccess.unlock();

				GeneratedEvents.push(new OBJECT_PUT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_Outside, user_ID, SC_PUT_OBJECT, 1));
				GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, ID_in_Outside, SC_HP_CHANGE, (unsigned char)otherUser->Type, otherUser->HP));
				GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_Outside, user_ID, SC_HP_CHANGE, (unsigned char)user->Type, user->HP));
			}
			else if ((OBJECT_TYPE_MONSTER_SCORPION <= m_pObjects[ID_in_Outside]->Type && m_pObjects[ID_in_Outside]->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING))
			{
				MONSTER* otherMonster = reinterpret_cast<MONSTER*>(m_pObjects[ID_in_Outside]);
				GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, ID_in_Outside, SC_HP_CHANGE, (unsigned char)otherMonster->Type, otherMonster->HP));
			}

			if (m_pObjects[ID_in_Outside]->Type == OBJECT_TYPE_MONSTER_GOBLIN_KING)
			{
				if (user->Quest.Type == QUEST_TYPE::ENCOUNTER_GOBLIN_KING && user->Quest.Quest_Progress[1].How_many == 0)
				{
					user->Quest.Quest_Progress[1].How_many = 1;
					GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, ID_in_Outside, SC_QUEST_CHANGE));
				}
			}
		}
	}

	user->viewlist_otherAccess.lock();
	user->viewlist = new_ViewList;
	user->viewlist_otherAccess.unlock();

	int new_SectorIndex = C_SIMPLE_MMORPG::GetSectorIndex_By(user->x, user->y);
	if (old_SectorIndex != new_SectorIndex) // Sector 인덱스가 변경되었다 -> 오브젝트가 속해있어야 할 Sector를 변경해줘야 한다.
	{
		Sector_other_Access.lock();
		m_Sector[old_SectorIndex].erase(user_ID); // 기존에 속해있던 Sector에서는 지워주고
		m_Sector[new_SectorIndex][user_ID] = user; // 새로운 Sector에 할당
		Sector_other_Access.unlock();
	}
}

void C_SIMPLE_MMORPG::SetUserViewListFromMonster(int user_ID, int monster_ID, queue<EVENT*>& GeneratedEvents)
{
	//int EVENT_FROM_USER_ID = Event.Ref_Object;
	MONSTER* monster = reinterpret_cast<MONSTER*>(m_pObjects[monster_ID]);
	USER* user = reinterpret_cast<USER*>(m_pObjects[user_ID]);
	int old_SectorIndex = C_SIMPLE_MMORPG::GetSectorIndex_By(monster->x, monster->y);
	int px = monster->x;
	int py = monster->y;
	int user_x = user->x;
	int user_y = user->y;
	user->viewlist_otherAccess.lock();
	unordered_set<int> new_viewlist = user->viewlist;
	user->viewlist_otherAccess.unlock();
	unordered_set<int> otherUserList;
	for (auto ID_in_View : otherUserList)
	{
		if (m_pObjects[ID_in_View]->Type == OBJECT_TYPE_USER)
			otherUserList.insert(ID_in_View);
	}

	// 시야 영역 체크
	RECT ViewRange = { user_x - USER_VIEW_RADIUS, user_y - USER_VIEW_RADIUS, user_x + USER_VIEW_RADIUS, user_y + USER_VIEW_RADIUS };
	if (RectIn(ViewRange.left, ViewRange.top, ViewRange.right, ViewRange.bottom, px, py) == false)
	{
		if (new_viewlist.count(monster_ID) != 0) // 시야 안 -> 시야 밖
		{
			new_viewlist.erase(monster_ID);
			// 유저한테 Monster가 시야영역을 벗어났음을 알림
			GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, monster_ID, SC_REMOVE_OBJECT));

			for (auto otherUserID : otherUserList)
			{
				USER* otherUser = reinterpret_cast<USER*>(m_pObjects[otherUserID]);
				otherUser->viewlist_otherAccess.lock();
				if (otherUser->viewlist.count(monster_ID) != 0)
				{
					otherUser->viewlist.erase(monster_ID);
					GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, monster_ID, SC_REMOVE_OBJECT));
					otherUser->viewlist_otherAccess.unlock();
				}
				else otherUser->viewlist_otherAccess.lock();
			}
		}

		// 활성화 영역 체크
		RECT WakeupRange = { user_x - USER_VIEW_RADIUS_EX, user_y - USER_VIEW_RADIUS_EX, user_x + USER_VIEW_RADIUS_EX, user_y + USER_VIEW_RADIUS_EX };
		if (RectIn(WakeupRange.left, WakeupRange.top, WakeupRange.right, WakeupRange.bottom, px, py) == true && monster->isActive == false)
		{
			monster->isActive = true;
			GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, monster_ID, user_ID, USER_MOVED));
		}
	}
	else
	{
		if (new_viewlist.count(monster_ID) == 0) // 시야 밖 -> 시야 안
		{
			new_viewlist.insert(monster_ID);
			GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, monster_ID, SC_PUT_OBJECT));
			GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, monster_ID, SC_HP_CHANGE, (unsigned char)monster->Type, monster->HP));

			for (auto otherUserID : otherUserList)
			{
				USER* otherUser = reinterpret_cast<USER*>(m_pObjects[otherUserID]);
				otherUser->viewlist_otherAccess.lock();
				if (otherUser->viewlist.count(monster_ID) == 0)
				{
					otherUser->viewlist.insert(monster_ID);
					GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, monster_ID, SC_PUT_OBJECT));
					GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, monster_ID, SC_HP_CHANGE, (unsigned char)monster->Type, monster->HP));
					otherUser->viewlist_otherAccess.unlock();
				}
				else otherUser->viewlist_otherAccess.lock();
			}
		}
		else // 기존에 시야 안에 있었던 NPC
		{
			GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, user_ID, monster_ID, SC_OBJECT_POS));

			for (auto otherUserID : otherUserList)
			{
				USER* otherUser = reinterpret_cast<USER*>(m_pObjects[otherUserID]);
				otherUser->viewlist_otherAccess.lock();
				if (otherUser->viewlist.count(monster_ID) != 0)
				{
					otherUser->viewlist.insert(monster_ID);
					GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, monster_ID, SC_OBJECT_POS));
					otherUser->viewlist_otherAccess.unlock();
				}
				else otherUser->viewlist_otherAccess.lock();
			}
		}
	}

	user->viewlist_otherAccess.lock();
	user->viewlist = new_viewlist;
	user->viewlist_otherAccess.unlock();

	// 다른 스레드에서 접근할 만한 변수들에 대해서는 Lock()-UnLock()을 한번만 하고 한꺼번에 값을 변경한다.
	int new_SectorIndex = C_SIMPLE_MMORPG::GetSectorIndex_By(monster->x, monster->y);
	if (old_SectorIndex != new_SectorIndex) // Sector 인덱스가 변경되었다 -> 오브젝트가 속해있어야 할 Sector를 변경해줘야 한다.
	{
		Sector_other_Access.lock();
		m_Sector[old_SectorIndex].erase(monster_ID); // 기존에 속해있던 Sector에서는 지워주고
		m_Sector[new_SectorIndex][monster_ID] = monster; // 새로운 Sector에 할당
		Sector_other_Access.unlock();
	}
}

void C_SIMPLE_MMORPG::ProcessInputOfUser(int ID, char input, queue<EVENT*>& GeneratedEvents)
{
	USER* user = reinterpret_cast<USER*>(m_pObjects[ID]);
	if (user->isAlive == false) return;
	int px = user->x;
	int py = user->y;
	int mp = user->MP;
	user->viewlist_otherAccess.lock();
	unordered_set<int> ViewList = user->viewlist;
	user->viewlist_otherAccess.unlock();

	unordered_set<int> ohterUserList;
	for (auto ID_in_View : ViewList)
	{
		if (m_pObjects[ID_in_View]->Type == OBJECT_TYPE_USER)
			ohterUserList.insert(ID_in_View);
	}


	if (input == CS_ATTACK || input == CS_POWER_ATTACK)
	{
		if (input == CS_POWER_ATTACK) mp = Clamp(0, mp - 10, 999);
		for (auto ID_in_View : ViewList)
		{
			if (input == CS_POWER_ATTACK && user->MP == 0) break;
			if (OBJECT_TYPE_MONSTER_SCORPION <= m_pObjects[ID_in_View]->Type && m_pObjects[ID_in_View]->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING)
			{
				bool DamageAble = false;
				if (m_pObjects[ID_in_View]->x == px && m_pObjects[ID_in_View]->y == py) DamageAble = true;
				else if (m_pObjects[ID_in_View]->x == px && m_pObjects[ID_in_View]->y == py - 1) DamageAble = true;
				else if (m_pObjects[ID_in_View]->x == px && m_pObjects[ID_in_View]->y == py + 1) DamageAble = true;
				else if (m_pObjects[ID_in_View]->x == px - 1 && m_pObjects[ID_in_View]->y == py) DamageAble = true;
				else if (m_pObjects[ID_in_View]->x == px + 1 && m_pObjects[ID_in_View]->y == py) DamageAble = true;

				if (DamageAble == true)
				{
					MONSTER* monster = reinterpret_cast<MONSTER*>(m_pObjects[ID_in_View]);
					if (monster->isAlive == true)
					{
						int damage = (input == CS_ATTACK) ? user->Damage : user->Damage * 2;
						monster->HP = Clamp(0, monster->HP + (monster->Defense - damage), 9999);
						{
							wstring message;
							ObjecttypeToString(monster->Type, message);
							message += L"에게 ";
							message += to_wstring(abs(monster->Defense - damage));
							message += L"의 피해를 입혔습니다.";
							GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
						}

						if (monster->HP == 0)
						{
							monster->isAlive = false;
							{
								wstring message;
								C_SIMPLE_MMORPG::ObjecttypeToString(monster->Type, message);
								message += L"(을)를 처치하였습니다.";
								GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID_in_View, SC_CHAT, message.c_str()));
							}
							GeneratedEvents.push(new TIME_EVENT(EVENT_TYPE::EVENT_AFTER_SOME_TIME, ID_in_View, ID, MONSTER_RESPAWN, high_resolution_clock::now() + MOSTER_RESPAWN_COOL_TIME));

							user->viewlist_otherAccess.lock();
							user->viewlist.erase(ID_in_View);
							user->viewlist_otherAccess.unlock();
							GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID_in_View, SC_REMOVE_OBJECT));

							int newItemID = C_SIMPLE_MMORPG::CreatableItemID();
							ITEM* droppedItem;
							if (m_pObjects[newItemID] == nullptr)
							{
								droppedItem = new ITEM(OBJECT_TYPE_ITEM_COIN, monster->x, monster->y, 1, newItemID);
								m_pObjects[newItemID] = droppedItem;
							}
							else
							{
								droppedItem = reinterpret_cast<ITEM*>(m_pObjects[newItemID]);
								droppedItem->ID = newItemID;
								droppedItem->Type = OBJECT_TYPE_ITEM_COIN;
								droppedItem->isActive = true;
								droppedItem->isAlive = true;
								droppedItem->x = monster->x;
								droppedItem->y = monster->y;
								droppedItem->How_many = 1;
							}

							Sector_other_Access.lock();
							m_Sector[C_SIMPLE_MMORPG::GetSectorIndex_By(monster->x, monster->y)][newItemID] = droppedItem;
							Sector_other_Access.unlock();

							user->viewlist_otherAccess.lock();
							user->viewlist.insert(newItemID);
							user->viewlist_otherAccess.unlock();

							GeneratedEvents.push(new OBJECT_PUT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, newItemID, SC_PUT_OBJECT, droppedItem->How_many));

							// 시야 내 다른 유저들에게 알림
							int monsterID = ID_in_View;
							for (auto otherUserID : ohterUserList)
							{
								USER* otherUser = reinterpret_cast<USER*>(m_pObjects[otherUserID]);
								otherUser->viewlist_otherAccess.lock();
								if (otherUser->viewlist.count(monsterID) != 0)
								{
									otherUser->viewlist.erase(monsterID);
									otherUser->viewlist.insert(newItemID);
									otherUser->viewlist_otherAccess.unlock();

									GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, monsterID, SC_REMOVE_OBJECT));
									GeneratedEvents.push(new OBJECT_PUT_EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, newItemID, SC_PUT_OBJECT, droppedItem->How_many));
								}
								else otherUser->viewlist_otherAccess.unlock();
							}

							user->Experience += 10;
							if ((int)user->Experience >= user->GetMaxEXP())
							{
								user->Experience -= user->GetMaxEXP();
								user->Level += 1;
								GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, L"레벌업!"));
								GeneratedEvents.push(new LEVEL_UP_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID_in_View, SC_LEVEL_UP, user->Level, user->Experience));

								user->HP = user->GetMaxHP();
								GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID_in_View, SC_LEVEL_UP, (unsigned char)user->Type, user->HP));
								user->MP = user->GetMaxHP();
								GeneratedEvents.push(new MP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID_in_View, SC_LEVEL_UP, user->MP));
								GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, L"HP와 MP가 모두 회복되었습니다."));
							}
							else
							{
								GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, L"경험치가 10 증가하였습니다.!"));
								GeneratedEvents.push(new EXP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID_in_View, SC_EXP_CHANGE, user->Experience));
							}

							switch (user->Quest.Type)
							{
							case QUEST_TYPE::TEST_ABILITY:
								if (monster->Type == OBJECT_TYPE_MONSTER_SCORPION) user->Quest.Quest_Progress[0].How_many += 1;
								if (monster->Type == OBJECT_TYPE_MONSTER_SCORPION_KING) user->Quest.Quest_Progress[1].How_many += 1;
								break;
							case QUEST_TYPE::SUBJUGATE_BANDIT:
								if (monster->Type == OBJECT_TYPE_MONSTER_THEIF) user->Quest.Quest_Progress[0].How_many += 1;
								if (monster->Type == OBJECT_TYPE_MONSTER_THEIF_BOSS) user->Quest.Quest_Progress[1].How_many += 1;
								break;
							case QUEST_TYPE::ENCOUNTER_GOBLIN_KING:
								if (OBJECT_TYPE_MONSTER_WOLF <= monster->Type && monster->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING) user->Quest.Quest_Progress[0].How_many += 1;
								break;
							}
							GeneratedEvents.push(new QUEST_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID_in_View, SC_QUEST_CHANGE, &user->Quest));
							continue;
						}

						GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID_in_View, SC_HP_CHANGE, (unsigned char)monster->Type, monster->HP));

						// 시야 내 다른 유저들에게 알림
						int monsterID = ID_in_View;
						for (auto otherUserID : ohterUserList)
						{
							USER* otherUser = reinterpret_cast<USER*>(m_pObjects[otherUserID]);
							otherUser->viewlist_otherAccess.lock();
							if (otherUser->viewlist.count(monsterID) != 0)
							{
								otherUser->viewlist_otherAccess.unlock();
								GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, monsterID, SC_HP_CHANGE, (unsigned char)monster->Type, monster->HP));
							}
							else otherUser->viewlist_otherAccess.unlock();
						}
					}
				}
			}
		}
		for (auto otherUserID : ohterUserList)
		{
			USER* otherUser = reinterpret_cast<USER*>(m_pObjects[otherUserID]);
			if (input == CS_POWER_ATTACK && user->MP > 0)
				GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, ID, SC_POWER_ATTACK));
			else
				GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, ID, SC_ATTACK));
		}
		if (input == CS_POWER_ATTACK && user->MP > 0)
		{
			user->MP = mp;
			{
				wstring message(L"스킬사용(강한공격)!, 유저의 MP가 10만큼 감소합니다.");
				GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
			}
			GeneratedEvents.push(new MP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_MP_CHANGE, user->MP));
			GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_POWER_ATTACK));
		}
		else if(input == CS_ATTACK) GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_ATTACK));
		return;
	}

	// Move Position
	if      (input == CS_MOVE_UP)    py = Clamp(WORLD_MIN, --py, WORLD_MAX);
	else if (input == CS_MOVE_DOWN)  py = Clamp(WORLD_MIN, ++py, WORLD_MAX);
	else if (input == CS_MOVE_LEFT)  px = Clamp(WORLD_MIN, --px, WORLD_MAX);
	else if (input == CS_MOVE_RIGHT) px = Clamp(WORLD_MIN, ++px, WORLD_MAX);

	if (m_TileData[py][px] != OBJECT_TPYE_TILE_WATER
		&& m_Obstacles[py][px] == OBJECT_TYPE_NON)
	{
		user->x = px;
		user->y = py;
	}

	C_SIMPLE_MMORPG::SetUserViewList(ID, GeneratedEvents);

	GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_OBJECT_POS));
}

void C_SIMPLE_MMORPG::ProcessItemEventOfUser(int ID, char* buf, queue<EVENT*>& GeneratedEvents)
{
	USER* user = reinterpret_cast<USER*>(m_pObjects[ID]);
	if (user->isAlive == false) return;
	int px = user->x;
	int py = user->y;
	user->viewlist_otherAccess.lock();
	unordered_set<int> ViewList = user->viewlist;
	user->viewlist_otherAccess.unlock();
	unordered_set<int> new_ViewList = ViewList;

	switch (buf[1])
	{
	case CS_ITEM_GET:
	{
		int world_out_itemID = -1;
		char obj_type = OBJECT_TYPE_NON;
		char SlotNum = -1;
		for (auto ID_in_View : ViewList)
		{
			obj_type = m_pObjects[ID_in_View]->Type;
			if (OBJECT_TYPE_ITEM_HP_POTION <= obj_type && obj_type <= OBJECT_TYPE_ITEM_SPECIAL_RING)
			{
				if (m_pObjects[ID_in_View]->x == px && m_pObjects[ID_in_View]->y == py)
				{
					SlotNum = C_SIMPLE_MMORPG::Find_PutAbleSlot(ID_in_View, ID);
					if (SlotNum != -1)
					{
						ITEM* get_item = reinterpret_cast<ITEM*> (m_pObjects[ID_in_View]);
						ITEM* slot_item = reinterpret_cast<ITEM*> (user->Inventory.Slot[SlotNum].Item);

						if (slot_item == nullptr)
						{
							int x, y;
							TransformSlot(SlotNum, x, y);
							slot_item = new ITEM(obj_type, x, y, get_item->How_many, ID_in_View);
							slot_item->isAlive = true;
							user->Inventory.Slot[SlotNum].Item = slot_item;
						}
						else if (slot_item->isAlive == false)
						{
							slot_item->Type = get_item->Type;
							slot_item->How_many = get_item->How_many;
							slot_item->ID = ID_in_View;
							slot_item->isAlive = true;
						}
						else if (slot_item->Type == get_item->Type)
						{
							slot_item->How_many += get_item->How_many;							
						}
						world_out_itemID = get_item->ID;

						Sector_other_Access.lock();
						m_Sector[C_SIMPLE_MMORPG::GetSectorIndex_By(px, py)].erase(ID_in_View);
						Sector_other_Access.unlock();

						new_ViewList.erase(ID_in_View);
						user->viewlist_otherAccess.lock();
						user->viewlist = new_ViewList;
						user->viewlist_otherAccess.unlock();

						{
							wstring message(L"아이템(");
							ObjecttypeToString(get_item->Type, message);
							message += L")을";
							message += to_wstring(get_item->How_many);
							message += L"만큼 획득하였습니다.";
							GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
						}
						GeneratedEvents.push(new ITEM_GET_OK_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, world_out_itemID, SC_ITEM_GET_OK, get_item->Type, SlotNum, get_item->How_many));

						Sector_other_Access.lock();
						m_Sector->erase(ID_in_View);
						Sector_other_Access.unlock();

						get_item->Type = OBJECT_TYPE_NON;
						get_item->isAlive = false;
						get_item->How_many = 0;
						break;
					}
				}
			}
		}

		if (world_out_itemID != -1)
		{
			for (auto ID_in_View : new_ViewList)
			{
				if (ID_in_View < MAX_CLIENTS)
				{
					USER* user = reinterpret_cast<USER*>(m_pObjects[ID_in_View]);
					user->viewlist_otherAccess.lock();
					if (user->viewlist.count(world_out_itemID) != 0)
					{
						user->viewlist.erase(world_out_itemID);
						user->viewlist_otherAccess.unlock();
						GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_View, world_out_itemID, SC_REMOVE_OBJECT));
					}
					else user->viewlist_otherAccess.unlock();
				}
			}
		}

		break;
	}
	case CS_ITEM_DROP:
	{
		cs_packet_item_drop* droppedItemInfo = reinterpret_cast<cs_packet_item_drop*>(buf);
		char SlotNum = (char)droppedItemInfo->slot_num;
		ITEM* slot_item = reinterpret_cast<ITEM*> (user->Inventory.Slot[SlotNum].Item);
		int newItemID = C_SIMPLE_MMORPG::CreatableItemID();
		ITEM* item_inWorld = reinterpret_cast<ITEM*> (m_pObjects[newItemID]);

		if (item_inWorld == nullptr)
		{
			item_inWorld = new ITEM(slot_item->Type, px, py, slot_item->How_many, newItemID);
			m_pObjects[newItemID] = item_inWorld;
		}
		else if (item_inWorld->isAlive == false)
		{
			item_inWorld->Type = slot_item->Type;
			item_inWorld->isAlive = true;
			item_inWorld->x = px;
			item_inWorld->y = py;
			item_inWorld->ID = newItemID;
			item_inWorld->How_many = slot_item->How_many;
		}
		ITEM* dropped_Item = item_inWorld;

		Sector_other_Access.lock();
		m_Sector[C_SIMPLE_MMORPG::GetSectorIndex_By(px, py)][newItemID] = dropped_Item;
		Sector_other_Access.unlock();

		slot_item->Type = OBJECT_TYPE_NON;
		slot_item->isAlive = false;
		slot_item->How_many = 0;
		user->viewlist_otherAccess.lock();
		user->viewlist.insert(dropped_Item->ID);
		user->viewlist_otherAccess.unlock();
		{
			wstring message(L"아이템(");
			ObjecttypeToString(dropped_Item->Type, message);
			message += L")을";
			message += to_wstring(dropped_Item->How_many);
			message += L"만큼 버렸습니다.";
			GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
		}
		GeneratedEvents.push(new ITEM_DROP_OK_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, dropped_Item->ID, SC_ITEM_DROP_OK, (int)SlotNum, dropped_Item->How_many));
		

		for (auto ID_in_View : ViewList)
		{
			if (ID_in_View < MAX_CLIENTS)
			{
				USER* user = reinterpret_cast<USER*>(m_pObjects[ID_in_View]);
				user->viewlist_otherAccess.lock();
				user->viewlist.insert(dropped_Item->ID);
				user->viewlist_otherAccess.unlock();
				GeneratedEvents.push(new OBJECT_PUT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_View, dropped_Item->ID, SC_PUT_OBJECT, dropped_Item->How_many));
			}
		}
		break;
	}
	case CS_ITEM_USE:
	{
		cs_packet_item_use* itemuseinfo = reinterpret_cast<cs_packet_item_use*>(buf);
		char SlotNum = (char)itemuseinfo->slot_num;
		ITEM* slot_item = reinterpret_cast<ITEM*> (user->Inventory.Slot[SlotNum].Item);
		if (slot_item == nullptr) return;
		if (slot_item->Type == OBJECT_TYPE_NON) return;

		int useCount = (slot_item->How_many - itemuseinfo->how_many > 0) ? itemuseinfo->how_many : slot_item->How_many;
		for (int i = useCount; i > 0; --i)
		{
			if (slot_item->Type == OBJECT_TYPE_ITEM_HP_POTION) user->HP = Clamp(0, user->HP + 30, user->GetMaxHP());
			else if (slot_item->Type == OBJECT_TYPE_ITEM_MP_POTION)	user->MP = Clamp(0, user->MP + 20, user->GetMaxMP());
		}

		{
			wstring message(L"아이템(");
			ObjecttypeToString(slot_item->Type, message);
			message += L")을";
			message += to_wstring(useCount);
			message += L"만큼 사용하였습니다.";
			GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
		}

		if (slot_item->Type == OBJECT_TYPE_ITEM_HP_POTION)
		{
			{
				wstring message(L"HP가 30만큼 회복하였습니다.");
				GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
			}

			GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_HP_CHANGE, (unsigned char)user->Type, user->HP));
		}
		else if (slot_item->Type == OBJECT_TYPE_ITEM_MP_POTION)
		{
			{
				wstring message(L"MP가 20만큼 회복하였습니다.");
				GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
			}

			GeneratedEvents.push(new MP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_MP_CHANGE, user->MP));
		}

		if (slot_item->How_many - itemuseinfo->how_many <= 0)
		{
			slot_item->Type = OBJECT_TYPE_NON;
			slot_item->isAlive = false;
			slot_item->How_many = 0;
		}
		else
		{
			slot_item->How_many -= itemuseinfo->how_many;
		}

		GeneratedEvents.push(new ITEM_USE_OK_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, slot_item->ID, SC_ITEM_USE_OK, SlotNum, slot_item->How_many));
		break;
	}
	case CS_ITEM_BUY:
	{
		cs_packet_item_buy* item_buyinfo = reinterpret_cast<cs_packet_item_buy*>(buf);
		int total_Price;
		switch (item_buyinfo->item_type)
		{
		case OBJECT_TYPE_ITEM_HP_POTION:    total_Price = HP_POTION_PRICE    * item_buyinfo->how_many;
		case OBJECT_TYPE_ITEM_MP_POTION:    total_Price = MP_POTION_PRICE    * item_buyinfo->how_many;
		case OBJECT_TYPE_ITEM_NORMAL_SWORD: total_Price = NORMAL_SWORD_PRICE * item_buyinfo->how_many;
		case OBJECT_TYPE_ITEM_NORMAL_ARMOR: total_Price = NORMAL_ARMOR_PRICE * item_buyinfo->how_many;
		case OBJECT_TYPE_ITEM_NORMAL_RING:  total_Price = NORMAL_RING_PRICE  * item_buyinfo->how_many;
		}

		char CoinSlotNum = -1;
		for (int i = 0; i < MAX_ITEM_SLOT; ++i)
		{
			ITEM* slot_item = reinterpret_cast<ITEM*> (user->Inventory.Slot[i].Item);
			if (slot_item == nullptr) continue;
			if (slot_item->Type == OBJECT_TYPE_ITEM_COIN) CoinSlotNum = i;
		}
		if (CoinSlotNum != -1)
		{
			char FutableSlot = -1;
			for (int i = MAX_ITEM_SLOT - 1; i >= 0; --i)
			{
				ITEM* slot_item = reinterpret_cast<ITEM*> (user->Inventory.Slot[i].Item);
				if (slot_item == nullptr) FutableSlot = i;
				if (slot_item->isAlive == false) FutableSlot = i;
				if (slot_item->Type == item_buyinfo->item_type)
				{
					ITEM* Coin = reinterpret_cast<ITEM*>(user->Inventory.Slot[CoinSlotNum].Item);
					if (Coin->How_many >= total_Price)
					{
						Coin->How_many -= total_Price;
						if (Coin->How_many == 0)
						{
							Coin->Type = OBJECT_TYPE_NON;
							Coin->isAlive = false;
						}
						{
							wstring message(L"코인을");
							message += to_wstring(total_Price);
							message += L"만큼 사용하였습니다.";
							GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
						}
						GeneratedEvents.push(new MONEY_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, Coin->ID, SC_MONEY_CHANGE, Coin->How_many, CoinSlotNum));

						{
							wstring message(L"아이템(");
							ObjecttypeToString(slot_item->Type, message);
							message += L")을 ";
							message += to_wstring(slot_item->How_many);
							message += L"만큼 구매하였습니다.";
							GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
						}
						slot_item->How_many += item_buyinfo->how_many;
						GeneratedEvents.push(new ITEM_BUY_OK_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, slot_item->ID, SC_ITEM_BUY_OK, slot_item->Type, i, slot_item->How_many));
						return;
					}
				}
			}
			if (FutableSlot != -1)
			{
				ITEM* slot_item = reinterpret_cast<ITEM*> (user->Inventory.Slot[FutableSlot].Item);
				ITEM* Coin = reinterpret_cast<ITEM*>(user->Inventory.Slot[CoinSlotNum].Item);
				int newItemID = C_SIMPLE_MMORPG::CreatableItemID();

				if (Coin->How_many >= total_Price )
				{
					Coin->How_many -= total_Price;
					if (Coin->How_many == 0)
					{
						Coin->Type = OBJECT_TYPE_NON;
						Coin->isAlive = false;
					}
					{
						wstring message(L"코인을");
						message += to_wstring(total_Price);
						message += L"만큼 사용하였습니다.";
						GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
					}
					GeneratedEvents.push(new MONEY_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, Coin->ID, SC_MONEY_CHANGE, Coin->How_many, CoinSlotNum));

					if (slot_item == nullptr)
					{
						int x, y;
						TransformSlot(FutableSlot, x, y);
						slot_item = new ITEM(item_buyinfo->item_type, x, y, item_buyinfo->how_many, newItemID);
						user->Inventory.Slot[FutableSlot].Item = slot_item;
					}
					else
					{
						slot_item->Type = item_buyinfo->item_type;
						slot_item->isAlive = true;
						slot_item->How_many = item_buyinfo->how_many;
						slot_item->ID = newItemID;
					}

					{
						wstring message(L"아이템(");
						ObjecttypeToString(slot_item->Type, message);
						message += L")을 ";
						message += to_wstring(slot_item->How_many);
						message += L"만큼 구매하였습니다.";
						GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_CHAT, message.c_str()));
					}
					GeneratedEvents.push(new ITEM_BUY_OK_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, slot_item->ID, SC_ITEM_BUY_OK, slot_item->Type, FutableSlot, slot_item->How_many));
				}
			}
		}
		break;
	}
	case CS_ITEM_SLOT_INFO_CHANGE:
	{
		cs_packet_change_item_slot* item_slotchange_info = reinterpret_cast<cs_packet_change_item_slot*>(buf);
		ITEM* temp = user->Inventory.Slot[item_slotchange_info->new_slot].Item;
		user->Inventory.Slot[item_slotchange_info->new_slot].Item = user->Inventory.Slot[item_slotchange_info->from_slot].Item;
		user->Inventory.Slot[item_slotchange_info->from_slot].Item = temp;
		break;
	}
	}
}


void C_SIMPLE_MMORPG::ProcessMonsterEvent(EVENT& Event, queue<EVENT*>& GeneratedEvents)
{
	int MONSTER_ID = Event.Do_Object;
	int USER_ID = Event.Ref_Object;
	//int EVENT_FROM_USER_ID = Event.Ref_Object;
	MONSTER* monster = reinterpret_cast<MONSTER*>(m_pObjects[MONSTER_ID]);
	if (monster->isAlive == false) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[USER_ID]);
	user->viewlist_otherAccess.lock();
	unordered_set<int> userViewList = user->viewlist;
	user->viewlist_otherAccess.unlock();
	int px = monster->x;
	int py = monster->y;
	int user_x = user->x;
	int user_y = user->y;
	char moveDirection = monster->MoveDirection;
	unordered_set<int> otherUserList;

	for (auto ID_in_View : userViewList)
	{
		if (m_pObjects[ID_in_View]->Type == OBJECT_TYPE_USER)
			otherUserList.insert(ID_in_View);
	}

	if (Event.Command == MONSTER_ATTACK)
	{
		bool DamageAble = false;
		if (user_x == px && user_y == py) DamageAble = true;
		else if (user_x == px && user_y == py - 1) DamageAble = true;
		else if (user_x == px && user_y == py + 1) DamageAble = true;
		else if (user_x == px - 1 && user_y == py) DamageAble = true;
		else if (user_x == px + 1 && user_y == py) DamageAble = true;

		if (DamageAble == true && user->isAlive == true)
		{
			int damage = user->Defense - monster->Damage;
			user->HP = Clamp(0, user->HP + damage, 9999);

			{
				wstring message(L"몬스터(");
				ObjecttypeToString(monster->Type, message);
				message += L"로부터 ";
				message += to_wstring(abs(damage));
				message += L"만큼 피해를 입었습니다.";
				GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, USER_ID, USER_ID, SC_CHAT, message.c_str()));
			}

			GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, USER_ID, USER_ID, SC_HP_CHANGE, (unsigned char)user->Type, user->HP));

			// 다른 유저에게 알림
			for (auto otherUserID : otherUserList)
				GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, USER_ID, SC_HP_CHANGE, (unsigned char)user->Type, user->HP));

			if (user->HP == 0)
			{
				{
					wstring message(L"체력이 다해 죽었습니다...");
					GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, USER_ID, USER_ID, SC_CHAT, message.c_str()));
				}
				user->isAlive = false;
				GeneratedEvents.push(new TIME_EVENT(EVENT_TYPE::EVENT_AFTER_SOME_TIME, USER_ID, MONSTER_ID, USER_RESPAWN, high_resolution_clock::now() + RESPAWN_COOLTIME));
			}
		}
		GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, USER_ID, MONSTER_ID, SC_ATTACK));

		// 다른 유저에게 알림
		for (auto otherUserID : otherUserList)
			GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, otherUserID, MONSTER_ID, SC_ATTACK));

		// 공격을 하고 난 후 이벤트 결정 (Find Attack_Target)
		C_SIMPLE_MMORPG::CheckAndSetMonster_Event(Event, GeneratedEvents);
		return;
	}
	// Move Position
	if      (moveDirection == MONSTER_MOVE_UP)     py = Clamp(WORLD_MIN, --py, WORLD_MAX);
	else if (moveDirection == MONSTER_MOVE_DOWN)   py = Clamp(WORLD_MIN, ++py, WORLD_MAX);
	else if (moveDirection == MONSTER_MOVE_LEFT)   px = Clamp(WORLD_MIN, --px, WORLD_MAX);
	else if (moveDirection == MONSTER_MOVE_RIGHT)  px = Clamp(WORLD_MIN, ++px, WORLD_MAX);

	if (m_TileData[py][px] != OBJECT_TPYE_TILE_WATER
		&& m_Obstacles[py][px] == OBJECT_TYPE_NON)
	{
		monster->x = px;
		monster->y = py;
	}

	C_SIMPLE_MMORPG::SetUserViewListFromMonster(USER_ID, MONSTER_ID, GeneratedEvents);

	// 이동을 하고 난 후 이벤트 결정 (몬스터 반경 user가 존재하면 해당 user following(from moveDirection with A*)
	// AGGRO_TYPE이 BATTLE_TYPE이거나 battle_target이 존재할 경우, 상하좌우 위치에 해당 타켓이 존재하면 공격
	C_SIMPLE_MMORPG::CheckAndSetMonster_Event(Event, GeneratedEvents);
}

void C_SIMPLE_MMORPG::HealToUser(int ID, queue<EVENT*>& GeneratedEvents)
{
	USER* user = reinterpret_cast<USER*>(m_pObjects[ID]);
	user->viewlist_otherAccess.lock();
	unordered_set<int> userViewList = user->viewlist;
	user->viewlist_otherAccess.unlock();
	int maxHP = user->GetMaxHP();
	float heal_amount = maxHP * 0.01f;
	int newHP = user->HP;
	newHP = Clamp(0, user->HP + (unsigned int)heal_amount, maxHP);
	GeneratedEvents.push(new TIME_EVENT(EVENT_TYPE::EVENT_AFTER_SOME_TIME, ID, ID, USER_HEAL, high_resolution_clock::now() + HEAL_COOLTIME));
	if (abs((int)user->HP - newHP) == 0) return;
	user->HP = newHP;
	if (user->isAlive == false) return;
	GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID, ID, SC_HP_CHANGE, (unsigned char)user->Type, (int)user->HP));

	for (auto ID_in_View : userViewList)
	{
		if (m_pObjects[ID_in_View]->Type == OBJECT_TYPE_USER)
			GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, ID_in_View, ID, SC_HP_CHANGE, (unsigned char)user->Type, (int)user->HP));
	}
}

void C_SIMPLE_MMORPG::Respawn(EVENT& Event, queue<EVENT*>& GeneratedEvents)
{
	if (Event.Command == USER_RESPAWN)
	{
		USER* user = reinterpret_cast<USER*>(m_pObjects[Event.Do_Object]);
		user->HP = user->GetMaxHP();
		user->MP = user->GetMaxMP();
		user->Experience = (unsigned int) (user->Experience / 2.0f);
		user->isAlive = true;
		user->x = WORLD_CENTER - 1;
		user->y = WORLD_CENTER - 1;
		GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, Event.Do_Object, Event.Do_Object, SC_OBJECT_POS));
		GeneratedEvents.push(new HP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, Event.Do_Object, Event.Do_Object, SC_HP_CHANGE, user->Type, user->HP));
		GeneratedEvents.push(new MP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, Event.Do_Object, Event.Do_Object, SC_MP_CHANGE, user->MP));
		GeneratedEvents.push(new EXP_CHANGE_EVENT(EVENT_TYPE::EVENT_DIRECT, Event.Do_Object, Event.Do_Object, SC_EXP_CHANGE, user->Experience));
		GeneratedEvents.push(new EVENT(EVENT_TYPE::EVENT_DIRECT, Event.Do_Object, Event.Do_Object, SC_OBJECT_POS));
		{
			wstring message(L"마을로 부활되었습니다.");
			GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, Event.Do_Object, Event.Do_Object, SC_CHAT, message.c_str()));
			message.clear();
			message += L"체력과 마나가 모두 회복되었습니다.";
			GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, Event.Do_Object, Event.Do_Object, SC_CHAT, message.c_str()));
			message.clear();
			message += L"경험치가 절반으로 줄었습니다.";
			GeneratedEvents.push(new CHAT_EVENT(EVENT_TYPE::EVENT_DIRECT, Event.Do_Object, Event.Do_Object, SC_CHAT, message.c_str()));
		}
		C_SIMPLE_MMORPG::SetUserViewList(Event.Do_Object, GeneratedEvents);
	}
	else if (Event.Command == MONSTER_RESPAWN)
	{
		MONSTER* monster = reinterpret_cast<MONSTER*>(m_pObjects[Event.Do_Object]);
		monster->HP = 100;
		monster->isAlive = true;
		monster->x = monster->respawn_x;
		monster->y = monster->respawn_y;
		monster->Attack_Target_x = -1;
		monster->Attack_Target_y = -1;
		C_SIMPLE_MMORPG::SetUserViewListFromMonster(Event.Ref_Object, Event.Do_Object, GeneratedEvents);
	}
}

void C_SIMPLE_MMORPG::CheckAndSetMonster_Event(EVENT& Event, queue<EVENT*>& GeneratedEvents)
{
	int MONSTER_ID = Event.Do_Object;
	int USER_ID = Event.Ref_Object;
	MONSTER* monster = reinterpret_cast<MONSTER*>(m_pObjects[MONSTER_ID]);
	if (monster->isAlive == false) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[USER_ID]);
	int px = monster->x;
	int py = monster->y;
	int user_x = user->x;
	int user_y = user->y;
	bool monster_active = monster->isActive;
	bool monster_alive = monster->isAlive;
	char moveDirection = monster->MoveDirection;
	//unordered_set<int> GeEvents;

	// Monster 시야 영역 체크
	RECT ViewRange = { px - MONSTER_VIEW_RADIUS, py - MONSTER_VIEW_RADIUS, px + MONSTER_VIEW_RADIUS, py + MONSTER_VIEW_RADIUS };
	if (RectIn(ViewRange.left, ViewRange.top, ViewRange.right, ViewRange.bottom, user_x, user_y) == true && monster_active == true && monster_alive == true)
	{
		if (rand() % 2)
		{
			monster->MoveDirection = rand() % (MONSTER_MOVE_RIGHT + 1);
			monster->GeneratedEventLock.lock();
			GeneratedEvents.push(new TIME_EVENT(EVENT_TYPE::EVENT_AFTER_SOME_TIME, MONSTER_ID, USER_ID, MONSTER_MOVE, high_resolution_clock::now() + MONSTER_MOVE_COOL_TIME));
			monster->GeneratedEventLock.unlock();
		}
		else
		{
			monster->GeneratedEventLock.lock();
			GeneratedEvents.push(new TIME_EVENT(EVENT_TYPE::EVENT_AFTER_SOME_TIME, MONSTER_ID, USER_ID, MONSTER_ATTACK, high_resolution_clock::now() + MONSTER_ATTACK_COOL_TIME));
			monster->GeneratedEventLock.unlock();
		}
		return;
	}

	// 활성화 영역 체크
	RECT WakeupRange = { user_x - USER_VIEW_RADIUS_EX, user_y - USER_VIEW_RADIUS_EX, user_x + USER_VIEW_RADIUS_EX, user_y + USER_VIEW_RADIUS_EX };
	if (RectIn(WakeupRange.left, WakeupRange.top, WakeupRange.right, WakeupRange.bottom, px, py) == true && monster_active == true && monster_alive == true)
	{
		monster->MoveDirection = rand() % (MONSTER_MOVE_RIGHT + 1);
		monster->GeneratedEventLock.lock();
		GeneratedEvents.push(new TIME_EVENT(EVENT_TYPE::EVENT_AFTER_SOME_TIME, MONSTER_ID, USER_ID, MONSTER_MOVE, high_resolution_clock::now() + MONSTER_MOVE_COOL_TIME));
		monster->GeneratedEventLock.unlock();
	}
	else monster->isActive = false;

}

int C_SIMPLE_MMORPG::Find_ItemInUserPos(int userID)
{
	USER* user = reinterpret_cast<USER*>(m_pObjects[userID]);
	user->viewlist_otherAccess.lock();
	unordered_set<int> ViewList = user->viewlist;
	user->viewlist_otherAccess.unlock();

	for (auto& ID_in_View : ViewList)
	{
		if (OBJECT_TYPE_ITEM_HP_POTION <= m_pObjects[ID_in_View]->Type && m_pObjects[ID_in_View]->Type <= OBJECT_TYPE_ITEM_SPECIAL_RING)
		{
			if (user->x == m_pObjects[ID_in_View]->x && user->y == m_pObjects[ID_in_View]->y) return ID_in_View;
		}
	}
	return -1;
}

char C_SIMPLE_MMORPG::Find_PutAbleSlot(int itemID, int userID)
{
	if ((itemID < DROP_ITEM_START_INDEX) || ((DROP_ITEM_START_INDEX + MAX_DROP_ITEM) <= itemID)) return -1;
	USER* user = reinterpret_cast<USER*>(m_pObjects[userID]);
	char SlotNum = -1;
	unsigned char ItemType = m_pObjects[itemID]->Type;
	for (int i = MAX_ITEM_SLOT - 1; i >= 0; --i)
	{
		if (OBJECT_TYPE_ITEM_HP_POTION <= ItemType && ItemType <= OBJECT_TYPE_ITEM_COIN && (0 <= i && i <= 2)) break;

		if (user->Inventory.Slot[i].Item == nullptr)
		{
			SlotNum = i;
		}
		else if (user->Inventory.Slot[i].Item->isAlive == false)
		{
			SlotNum = i;
		}
		else if (i > 2 && user->Inventory.Slot[i].Item->Type == m_pObjects[itemID]->Type)
		{
			SlotNum = i;
			break;
		}
	}
	return SlotNum;
}

int C_SIMPLE_MMORPG::CreatableItemID()
{
	for (int i = DROP_ITEM_START_INDEX; i < DROP_ITEM_START_INDEX + MAX_DROP_ITEM; ++i)
	{
		if (m_pObjects[i] == nullptr) return i;
		if (m_pObjects[i]->isAlive == false) return i;
	}
	return -1;
}


void C_SIMPLE_MMORPG::ObjecttypeToString(unsigned char obj_type, wstring & wstr)
{
	switch (obj_type)
	{
	case OBJECT_TYPE_NON                    : wstr += L""; break;
	case OBJECT_TYPE_USER                   : wstr += L"유저"; break;
	case OBJECT_TYPE_NPC_CHIEF              : wstr += L"촌장"; break;
	case OBJECT_TYPE_NPC_BLACKSMITH         : wstr += L"대장장이"; break;
	case OBJECT_TYPE_NPC_POTION_MERCHANT    : wstr += L"포션상인"; break;
	case OBJECT_TYPE_MONSTER_SCORPION       : wstr += L"전갈"; break;
	case OBJECT_TYPE_MONSTER_SCORPION_KING  : wstr += L"전갈킹"; break;
	case OBJECT_TYPE_MONSTER_THEIF          : wstr += L"도적"; break;
	case OBJECT_TYPE_MONSTER_THEIF_BOSS     : wstr += L"도적두목"; break;
	case OBJECT_TYPE_MONSTER_WOLF           : wstr += L"늑대"; break;
	case OBJECT_TYPE_MONSTER_WEREWOLF       : wstr += L"늑대인간"; break;
	case OBJECT_TYPE_MONSTER_GOBLIN_WARRIOR : wstr += L"고블린전사"; break;
	case OBJECT_TYPE_MONSTER_GOBLIN_KING    : wstr += L"고블린킹"; break;
	case OBJECT_TPYE_TILE_WATER             : wstr += L"물"; break;
	case OBJECT_TPYE_TILE_GRASS             : wstr += L"잔디"; break;
	case OBJECT_TPYE_TILE_SAND              : wstr += L"모래"; break;
	case OBJECT_TYPE_OBSTACLE_WALL          : wstr += L"벽"; break;
	case OBJECT_TYPE_OBSTACLE_ROCK          : wstr += L"바위"; break;
	case OBJECT_TYPE_OBSTACLE_TREE          : wstr += L"나무"; break;
	case OBJECT_TYPE_ITEM_HP_POTION         : wstr += L"HP 포션"; break;
	case OBJECT_TYPE_ITEM_MP_POTION         : wstr += L"MP 포션"; break;
	case OBJECT_TYPE_ITEM_COIN              : wstr += L"코인(돈)"; break;  
	case OBJECT_TYPE_ITEM_NORMAL_ARMOR      : wstr += L"평범한 갑옷"; break;
	case OBJECT_TYPE_ITEM_NORMAL_SWORD      : wstr += L"평범한 검"; break;
	case OBJECT_TYPE_ITEM_NORMAL_RING       : wstr += L"평범한 반지"; break;
	case OBJECT_TYPE_ITEM_SPECIAL_ARMOR     : wstr += L"특별한 갑옷"; break;
	case OBJECT_TYPE_ITEM_SPECIAL_SWORD     : wstr += L"특별한 검"; break;
	case OBJECT_TYPE_ITEM_SPECIAL_RING      : wstr += L"특별한 반지"; break;
	}
}

// ID: 0 ~ MAX_CLIENTS까지의 숫자 (m_Players의 인덱스로 활용)
bool C_SIMPLE_MMORPG::GetUserID(int& ID)
{
	USER* user = nullptr;
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (m_pObjects[i] == nullptr)
		{
			ID = i;
			return true;
		}
		user = reinterpret_cast<USER*>(m_pObjects[i]);
		if (user->isConnect == false)
		{
			ID = i;
			return true;
		}
	}

	ID = -1;
	return false;
}

bool C_SIMPLE_MMORPG::GetRandomPos(int & x, int & y)
{
	x = rand() % WORLD_SIZE;
	y = rand() % WORLD_SIZE;
	return true;
}

int C_SIMPLE_MMORPG::GetSectorIndex_By(int x, int y)
{
	x = Clamp(0, x, WORLD_SIZE - 1);
	y = Clamp(0, y, WORLD_SIZE - 1);
	return (x / SECTOR_SIZE) + (y / SECTOR_SIZE) * (WORLD_SIZE / SECTOR_SIZE);
}
