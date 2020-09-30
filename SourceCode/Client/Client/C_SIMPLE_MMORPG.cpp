#include "stdafx.h"
#include "C_SIMPLE_MMORPG.h"
#include "ProcessSocket.h"

void TransformSlot(int SlotNum, int & x, int & y)
{
	Rect DrawRect;
	DrawRect.X = (VIEW_SIZE - 3) * GDI_TILE_SIZE - 12;
	DrawRect.Y = 0;

	// Equipped
	if (SlotNum < 3)
	{
		x = DrawRect.X + 3 * (SlotNum + 1) + GDI_ITEM_SIZE * SlotNum - 1;
		y = DrawRect.Y + 299;
	}
	else
	{
		SlotNum -= 3;
		x = DrawRect.X + 3 * (SlotNum % 3 + 1) + GDI_ITEM_SIZE * (SlotNum % 3) - 1;
		y = DrawRect.Y + 399 + (SlotNum / 3) * SlotNum + GDI_ITEM_SIZE * (SlotNum / 3);
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

bool USER::CheckChatTopLineAliveOver()
{
	if (high_resolution_clock::now() >= last_chat_time + CHAT_ALIVE_TIME)
	{
		last_chat_time = high_resolution_clock::now() + CHAT_ALIVE_TIME;
		return true;
	}
	return false;
}

ULONG_PTR g_GdiPlusTokenBoxData;

C_SIMPLE_MMORPG::C_SIMPLE_MMORPG()
{
	// GDI+ Start
	GdiplusStartupInput GdiSartupInput;
	GdiplusStartup(&g_GdiPlusTokenBoxData, &GdiSartupInput, NULL);
	for (int i = 0; i < WORLD_SIZE; ++i)
	{
		memset(m_TileData[i], OBJECT_TYPE_NON, WORLD_SIZE);
		memset(m_Obstacles[i], OBJECT_TYPE_NON, WORLD_SIZE);
	}
}

C_SIMPLE_MMORPG::~C_SIMPLE_MMORPG()
{
	for (auto& object : m_pObjects)
	{
		if (object.second != nullptr) delete object.second;
	}
	m_pObjects.clear();

	for (auto image : m_Images)
		delete image;
	for (auto cachedimage : m_CachedImages)
		delete cachedimage;
	if (m_pScreen) delete m_pScreen;
	// GDI+ Shutdown
	GdiplusShutdown(g_GdiPlusTokenBoxData);
}

void C_SIMPLE_MMORPG::Init()
{
}

void C_SIMPLE_MMORPG::LoadPNG(const WCHAR * filename)
{
	Bitmap OriginBitmap(filename);
	if (m_pScreen == nullptr) return;

	INT IMAGE_ROW, IMAGE_COLUMN, UNIT_SIZE;

	if (wcscmp(filename, L"images/tiles.png") == 0)
	{
		IMAGE_ROW = TILE_IMAGE_ROW;
		IMAGE_COLUMN = TILE_IMAGE_COLUMN;
		UNIT_SIZE = GDI_TILE_SIZE;
	}
	else if (wcscmp(filename, L"images/obstacles.png") == 0)
	{
		IMAGE_ROW = OBSTACLE_IMAGE_ROW;
		IMAGE_COLUMN = OBSTACLE_IMAGE_COLUMN;
		UNIT_SIZE = GDI_OBSTACLE_SIZE;
	}
	else if (wcscmp(filename, L"images/characters.png") == 0)
	{
		IMAGE_ROW = CHARACTER_IMAGE_ROW;
		IMAGE_COLUMN = CHARACTER_IMAGE_COLUMN;
		UNIT_SIZE = GDI_CHARACTER_SIZE;
	}
	else if (wcscmp(filename, L"images/items.png") == 0)
	{
		IMAGE_ROW = ITEM_IMAGE_ROW;
		IMAGE_COLUMN = ITEM_IMAGE_COLUMN;
		UNIT_SIZE = GDI_ITEM_SIZE;
	}
	else if (wcscmp(filename, L"images/attacks.png") == 0)
	{
		IMAGE_ROW = EFFECT_ATTACK_IMAGE_ROW;
		IMAGE_COLUMN = EFFECT_ATTACK_IMAGE_COLUMN;
		UNIT_SIZE = GDI_EFFECT_ATTACK_SIZE;
	}
	else if (wcscmp(filename, L"images/fog.png") == 0)
	{
		Bitmap ResizeBitmap(VIEWPORT_SIZE, VIEWPORT_SIZE);
		Graphics ResizeScreen(&ResizeBitmap);
		ResizeScreen.DrawImage(&OriginBitmap, 0, 0, VIEWPORT_SIZE, VIEWPORT_SIZE);
		m_Images.emplace_back(ResizeBitmap.Clone(0, 0, VIEWPORT_SIZE, VIEWPORT_SIZE, PixelFormatDontCare));
		m_CachedImages.emplace_back(new CachedBitmap(m_Images.back(), m_pScreen));
		return;
	}
	else
	{
		cout << "허용되지 않은 리소스입니다." << endl;
		return;
	}

	INT Clip_Width = OriginBitmap.GetWidth() / IMAGE_COLUMN;
	INT Clip_Height = OriginBitmap.GetHeight() / IMAGE_ROW;

	for (int row = 0; row < IMAGE_ROW; ++row)
	{
		for (int column = 0; column < IMAGE_COLUMN; ++column)
		{
			Bitmap ResizeBitmap(UNIT_SIZE, UNIT_SIZE);
			Graphics ResizeScreen(&ResizeBitmap);
			Rect sourceRect(column*Clip_Width, row*Clip_Height, Clip_Width, Clip_Height);
			Bitmap* ClipedBitmap = OriginBitmap.Clone(sourceRect, PixelFormatDontCare);

			ResizeScreen.DrawImage(ClipedBitmap, 0, 0, UNIT_SIZE, UNIT_SIZE);
			delete ClipedBitmap;

			m_Images.emplace_back(ResizeBitmap.Clone(0, 0, UNIT_SIZE, UNIT_SIZE, PixelFormatDontCare));
			m_CachedImages.emplace_back(new CachedBitmap(m_Images.back(), m_pScreen));
		}
	}
}

void C_SIMPLE_MMORPG::LoadMap(const WCHAR* filename)
{
	ifstream file(filename);
	string in_line;
	int x = 0, y = 0;
	int obj_id = 0;
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
					m_pObjects[NPC_START_INDEX + obj_id++] = new USER(OBJECT_TYPE_USER, x, y);
					break;
				case '1':
					m_pObjects[NPC_START_INDEX + obj_id++] = new OBJECT(OBJECT_TYPE_NPC_CHIEF, x, y);
					break;
				case '2':
					m_pObjects[NPC_START_INDEX + obj_id++] = new OBJECT(OBJECT_TYPE_NPC_BLACKSMITH, x, y);
					break;
				case '3':
					m_pObjects[NPC_START_INDEX + obj_id++] = new OBJECT(OBJECT_TYPE_NPC_POTION_MERCHANT, x, y);
					break;
				case '4':
					m_pObjects[NPC_START_INDEX + obj_id++] = new MONSTER(OBJECT_TYPE_MONSTER_SCORPION, x, y);
					break;
				case '5':
					m_pObjects[NPC_START_INDEX + obj_id++] = new MONSTER(OBJECT_TYPE_MONSTER_SCORPION_KING, x, y);
					break;
				case '6':
					m_pObjects[NPC_START_INDEX + obj_id++] = new MONSTER(OBJECT_TYPE_MONSTER_THEIF, x, y);
					break;
				case '7':
					m_pObjects[NPC_START_INDEX + obj_id++] = new MONSTER(OBJECT_TYPE_MONSTER_THEIF_BOSS, x, y);
					break;
				case '8':
					m_pObjects[NPC_START_INDEX + obj_id++] = new MONSTER(OBJECT_TYPE_MONSTER_WOLF, x, y);
					break;
				case '9':
					m_pObjects[NPC_START_INDEX + obj_id++] = new MONSTER(OBJECT_TYPE_MONSTER_WEREWOLF, x, y);
					break;
				case '10':
					m_pObjects[NPC_START_INDEX + obj_id++] = new MONSTER(OBJECT_TYPE_MONSTER_GOBLIN_WARRIOR, x, y);
					break;
				case '11':
					m_pObjects[NPC_START_INDEX + obj_id++] = new MONSTER(OBJECT_TYPE_MONSTER_GOBLIN_KING, x, y);
					break;
				}
				x++;
			}
		}
		y++;
	}
}

void C_SIMPLE_MMORPG::LoginOk(char* LoginByteStream)
{
	sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(LoginByteStream);
	m_myID = packet->id;
	wcsncpy_s(m_myID_Str, packet->user_id_str, MAX_WSTR_LEN);
}

void C_SIMPLE_MMORPG::SetmyCharacter(char* CharacterByteStream)
{
	if (CharacterByteStream == nullptr) return;
	sc_packet_user_info* packet = reinterpret_cast<sc_packet_user_info*>(CharacterByteStream);
	if (packet->type != SC_USER_INFO) return;

	if (m_pObjects.count(packet->obj_id) == 0)
		m_pObjects[packet->obj_id] = new USER(packet->obj_type, packet->obj_x, packet->obj_y);
	m_myID = packet->obj_id;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	wsprintfW(user->id_str, L"%ws", m_myID_Str);
	//wcsncpy_s(user->id_str, m_myID_Str, MAX_WSTR_LEN);
	user->level = packet->user_level;
	user->exp = packet->user_exp;
	user->hp = packet->user_hp;
	user->mp = packet->user_mp;
	memcpy_s(&user->quest, sizeof(user->quest), &packet->user_quest, sizeof(packet->user_quest));

	user->max_hp = 100 + (user->level - 1)* 30;
	user->max_mp = 100 + (user->level - 1)* 20;
	user->max_exp = 100 + (user->level - 1) * 100;

	Rect DrawRect;
	// Set EquipmentSlot Rect
	int ui_start_x = (VIEW_SIZE - 3) * GDI_TILE_SIZE - 12;
	for (int i = 1; i <= 3; ++i)
	{
		DrawRect = { ui_start_x + 3 * i + GDI_ITEM_SIZE * (i - 1) - 1, 299, GDI_ITEM_SIZE + 1, GDI_ITEM_SIZE + 1 };
		user->inventory.Slot[i - 1].AreaRect = { DrawRect.X , DrawRect.Y, DrawRect.X + DrawRect.Width, DrawRect.Y + DrawRect.Height };
	}

	// Set ItemSlot Rect
	for (int i = 1; i <= 2; ++i)
	{
		for (int j = 1; j <= 3; ++j)
		{
			DrawRect = { ui_start_x + 3 * j + GDI_ITEM_SIZE * (j - 1) - 1, 399 + 3 * (i - 1) + GDI_ITEM_SIZE * (i - 1), GDI_ITEM_SIZE + 1, GDI_ITEM_SIZE + 1 };
			user->inventory.Slot[i * 3 + (j - 1)].AreaRect = { DrawRect.X , DrawRect.Y, DrawRect.X + DrawRect.Width, DrawRect.Y + DrawRect.Height };
		}
	}

	unsigned char obj_type = OBJECT_TYPE_NON;
	for (int i = 0; i < MAX_ITEM_SLOT; ++i)
	{
		if (packet->user_inventory[i].What == OBJECT_TYPE_NON) continue;
		int x, y;
		TransformSlot(i, x, y);
		user->inventory.Slot[i].Item = new ITEM(packet->user_inventory[i].What, x, y, packet->user_inventory[i].How_many);
	}
}

void C_SIMPLE_MMORPG::ProcessInput()
{
	if (m_pDragItem != nullptr)
	{
		if (m_pMousePos != nullptr)
		{
			m_pDragItem->x = m_pMousePos->x - GDI_ITEM_SIZE / 2;
			m_pDragItem->y = m_pMousePos->y - GDI_ITEM_SIZE / 2;
		}
		else
		{
			if (m_pObjects.count(m_myID) == 0) return;
			USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
			user->inventory.Slot[user->inventory.SelectedSlotNum].Item = m_pDragItem;
			TransformSlot(user->inventory.SelectedSlotNum, m_pDragItem->x, m_pDragItem->y);
			user->inventory.SelectedSlotNum = -1;
			m_pDragItem = nullptr;
		}
	}
}

void C_SIMPLE_MMORPG::AddNewObject(char* ObjectByteStream)
{
	sc_packet_put_object* newObject = reinterpret_cast<sc_packet_put_object*>(ObjectByteStream);
	if (m_pObjects.count(newObject->obj_id) != 0)
	{
		m_pObjects[newObject->obj_id]->Type = newObject->obj_type;
		m_pObjects[newObject->obj_id]->x = newObject->obj_x;
		m_pObjects[newObject->obj_id]->y = newObject->obj_y;
		if (OBJECT_TYPE_ITEM_HP_POTION <= newObject->obj_type && newObject->obj_type <= OBJECT_TYPE_ITEM_SPECIAL_RING)
		{
			ITEM* item = reinterpret_cast<ITEM*>(m_pObjects[newObject->obj_id]);
			item->How_many = newObject->how_many;
		}
	}
	else
	{
		if (OBJECT_TYPE_ITEM_HP_POTION <= newObject->obj_type && newObject->obj_type <= OBJECT_TYPE_ITEM_SPECIAL_RING)
		{
			m_pObjects[newObject->obj_id] = new ITEM(newObject->obj_type, newObject->obj_x, newObject->obj_y, newObject->how_many);
		}
		else if (OBJECT_TYPE_MONSTER_SCORPION <= newObject->obj_type && newObject->obj_type <= OBJECT_TYPE_MONSTER_GOBLIN_KING)
		{
			m_pObjects[newObject->obj_id] = new MONSTER(newObject->obj_type, newObject->obj_x, newObject->obj_y);
		}
		else if (OBJECT_TYPE_USER == newObject->obj_type)
		{
			m_pObjects[newObject->obj_id] = new USER(newObject->obj_type, newObject->obj_x, newObject->obj_y);
		}
		else
		{
			m_pObjects[newObject->obj_id] = new OBJECT(newObject->obj_type, newObject->obj_x, newObject->obj_y);
		}
	}
}

void C_SIMPLE_MMORPG::ObjectPos(char* ObjectByteStream)
{
	sc_packet_pos* ObjectPos = reinterpret_cast<sc_packet_pos*>(ObjectByteStream);
	if (m_pObjects.count(ObjectPos->obj_id) == 0) return;
	m_pObjects[ObjectPos->obj_id]->x = ObjectPos->obj_x;
	m_pObjects[ObjectPos->obj_id]->y = ObjectPos->obj_y;
}

void C_SIMPLE_MMORPG::ObjectDoAttack(char* AttackInfoByteStream)
{
	sc_packet_attack* AttackInfo = reinterpret_cast<sc_packet_attack*>(AttackInfoByteStream);
	if (m_pObjects.count(AttackInfo->target) == 0) return;
	if (m_pObjects[AttackInfo->target]->Type != OBJECT_TYPE_USER
		&& (m_pObjects[AttackInfo->target]->Type < OBJECT_TYPE_MONSTER_SCORPION
		|| m_pObjects[AttackInfo->target]->Type > OBJECT_TYPE_MONSTER_GOBLIN_KING)) return;

	if (m_pObjects[AttackInfo->target]->Type == OBJECT_TYPE_USER)
	{
		USER* user = reinterpret_cast<USER*>(m_pObjects[AttackInfo->target]);
		user->attack_type = ATTACK_TYPE_NORMAL;
		user->last_attack_time = high_resolution_clock::now();
		user->attack_frameNum = 0;
	}
	else
	{
		MONSTER* monster = reinterpret_cast<MONSTER*>(m_pObjects[AttackInfo->target]);
		monster->attack_type = ATTACK_TYPE_CLAW;
		monster->last_attack_time = high_resolution_clock::now();
	}
}

void C_SIMPLE_MMORPG::ObjectDoPowerAttack(char* AttackInfoByteStream)
{
	sc_packet_power_attack* PowerAttackInfo = reinterpret_cast<sc_packet_power_attack*>(AttackInfoByteStream);
	if (m_pObjects.count(PowerAttackInfo->target) == 0) return;
	if (m_pObjects[PowerAttackInfo->target]->Type != OBJECT_TYPE_USER
		&& (m_pObjects[PowerAttackInfo->target]->Type < OBJECT_TYPE_MONSTER_SCORPION
		|| m_pObjects[PowerAttackInfo->target]->Type > OBJECT_TYPE_MONSTER_GOBLIN_KING)) return;

	if (m_pObjects[PowerAttackInfo->target]->Type == OBJECT_TYPE_USER)
	{
		USER* user = reinterpret_cast<USER*>(m_pObjects[PowerAttackInfo->target]);
		user->attack_type = ATTACK_TYPE_POWER;
		user->last_attack_time = high_resolution_clock::now();
	}
	else
	{
		MONSTER* monster = reinterpret_cast<MONSTER*>(m_pObjects[PowerAttackInfo->target]);
		monster->attack_type = ATTACK_TYPE_CLAW;
		monster->last_attack_time = high_resolution_clock::now();
	}
}

int C_SIMPLE_MMORPG::Find_ItemInWorld()
{
	if (m_pObjects.count(m_myID) == 0) return -1;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	for (auto& object : m_pObjects)
	{
		if (object.second == nullptr) continue;
		if (OBJECT_TYPE_ITEM_HP_POTION <= object.second->Type && object.second->Type <= OBJECT_TYPE_ITEM_SPECIAL_RING)
		{
			if (user->x == object.second->x && user->y == object.second->y)
				return object.first;
		}
	}
	return -1;
}

char C_SIMPLE_MMORPG::Find_PutAbleSlot(int itemID)
{
	if ((itemID < DROP_ITEM_START_INDEX) || ((DROP_ITEM_START_INDEX + MAX_DROP_ITEM) <= itemID)) return -1;
	if (m_pObjects.count(m_myID) == 0) return -1;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	char SlotNum = -1;
	unsigned char ItemType = m_pObjects[itemID]->Type;
	for (int i = MAX_ITEM_SLOT - 1; i >= 0; --i)
	{
		if (OBJECT_TYPE_ITEM_HP_POTION <= ItemType && ItemType <= OBJECT_TYPE_ITEM_COIN && (0 <= i && i <= 2)) break;

		if (user->inventory.Slot[i].Item == nullptr)
		{
			SlotNum = i;
		}
		else if (i > 2 && user->inventory.Slot[i].Item->Type == m_pObjects[itemID]->Type)
		{
			SlotNum = i;
			break;
		}
	}
	return SlotNum;
}

// Item Type
// OBJECT_TYPE_ITEM_HP_POTION         19
// OBJECT_TYPE_ITEM_MP_POTION         20
// OBJECT_TYPE_ITEM_COIN              21  
// OBJECT_TYPE_ITEM_NORMAL_ARMOR      22
// OBJECT_TYPE_ITEM_NORMAL_SWORD      23
// OBJECT_TYPE_ITEM_NORMAL_RING       24
// OBJECT_TYPE_ITEM_SPECIAL_ARMOR     25
// OBJECT_TYPE_ITEM_SPECIAL_SWORD     26
// OBJECT_TYPE_ITEM_SPECIAL_RING      27
void C_SIMPLE_MMORPG::ItemGetOk(char* GetInfoByteStream)
{
	sc_packet_get_item_ok* ItemGetInfo = reinterpret_cast<sc_packet_get_item_ok*>(GetInfoByteStream);
	if (m_pObjects.count(ItemGetInfo->obj_id) == 0) return;
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	INVENTORY* inventory = &user->inventory;
	ITEM* SlotItem = inventory->Slot[ItemGetInfo->slot_num].Item;
	if (m_pObjects.count(ItemGetInfo->obj_id) == 0)
	{
		int x, y;
		TransformSlot(ItemGetInfo->slot_num, x, y);
		m_pObjects[ItemGetInfo->obj_id] = new ITEM(ItemGetInfo->obj_type, x, y, ItemGetInfo->how_many);
	}
	ITEM* DroppedItem = reinterpret_cast<ITEM*>(m_pObjects[ItemGetInfo->obj_id]);

	if (SlotItem == nullptr)
	{
		TransformSlot(ItemGetInfo->slot_num, DroppedItem->x, DroppedItem->y);
		SlotItem = inventory->Slot[ItemGetInfo->slot_num].Item = DroppedItem;
		m_pObjects[ItemGetInfo->obj_id] = nullptr;
		m_pObjects.erase(ItemGetInfo->obj_id);
		return;
	}
	else if (SlotItem->Type != DroppedItem->Type)
	{
		delete SlotItem;
		TransformSlot(ItemGetInfo->slot_num, DroppedItem->x, DroppedItem->y);
		SlotItem = inventory->Slot[ItemGetInfo->slot_num].Item = DroppedItem;
		m_pObjects[ItemGetInfo->obj_id] = nullptr;
		m_pObjects.erase(ItemGetInfo->obj_id);
		return;
	}

	SlotItem->How_many += DroppedItem->How_many;
	delete DroppedItem;
	m_pObjects.erase(ItemGetInfo->obj_id);
}

void C_SIMPLE_MMORPG::ItemDropOk(char* DropInfoByteStream)
{
	sc_packet_drop_item_ok* DropInfo = reinterpret_cast<sc_packet_drop_item_ok*>(DropInfoByteStream);
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	m_pDragItem->x = user->x;
	m_pDragItem->y = user->y;
	m_pObjects[C_SIMPLE_MMORPG::Find_EmptyObject(DROP_ITEM_START_INDEX, DROP_ITEM_START_INDEX + MAX_DROP_ITEM - 1)] = m_pDragItem;
	m_pDragItem = nullptr;
	user->inventory.SelectedSlotNum = -1;
}

void C_SIMPLE_MMORPG::ItemBuyOk(char* BuyInfoByteStream)
{
	sc_packet_buy_item_ok* packet = reinterpret_cast<sc_packet_buy_item_ok*>(BuyInfoByteStream);
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	ITEM* SlotItem = user->inventory.Slot[packet->slot_num].Item;
	if (SlotItem == nullptr)
	{
		int x, y;
		TransformSlot(packet->slot_num, x, y);
		user->inventory.Slot[packet->slot_num].Item = new ITEM(packet->obj_type, x, y, packet->how_many);
	}
	else
	{
		SlotItem->Type = packet->obj_type;
		SlotItem->How_many = packet->how_many;
	}
}

void C_SIMPLE_MMORPG::ItemUseOk(char* UseInfoByteStream)
{
	sc_packet_use_item_ok* packet = reinterpret_cast<sc_packet_use_item_ok*>(UseInfoByteStream);
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	ITEM* SlotItem = user->inventory.Slot[packet->slot_num].Item;
	if (packet->how_many == 0 && SlotItem != nullptr)
	{
		delete SlotItem;
		user->inventory.Slot[packet->slot_num].Item = nullptr;
	}
	else SlotItem->How_many = packet->how_many;
}

void C_SIMPLE_MMORPG::LevelUp(char * LevelUpInfoByteStream)
{
	sc_packet_levelUp* LevelUpInfo = reinterpret_cast<sc_packet_levelUp*>(LevelUpInfoByteStream);
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	user->level = LevelUpInfo->user_level;
	user->exp = LevelUpInfo->user_exp;
	user->hp = user->max_hp = 100 + (user->level - 1) * 30;
	user->mp = user->max_mp = 100 + (user->level - 1) * 20;
	user->max_exp = 100 + (user->level - 1) * 100;
}

void C_SIMPLE_MMORPG::ExpChange(char* ExpInfoByteStream)
{
	sc_packet_exp_change* ExpInfo = reinterpret_cast<sc_packet_exp_change*>(ExpInfoByteStream);
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	user->exp = ExpInfo->user_exp;
}

void C_SIMPLE_MMORPG::HPChange(char* HPInfoByteStream)
{
	sc_packet_hp_change* HpInfo = reinterpret_cast<sc_packet_hp_change*>(HPInfoByteStream);
	if (m_pObjects.count(HpInfo->obj_id) == 0)
	{
		if (HpInfo->obj_type == OBJECT_TYPE_USER) m_pObjects[HpInfo->obj_id] = new USER(OBJECT_TYPE_USER, 0, 0);
		else if (OBJECT_TYPE_MONSTER_SCORPION <= HpInfo->obj_type && HpInfo->obj_type <= OBJECT_TYPE_MONSTER_GOBLIN_KING) m_pObjects[HpInfo->obj_id] = new MONSTER(HpInfo->obj_type, 0, 0);
	}
	if (m_pObjects[HpInfo->obj_id]->Type == OBJECT_TYPE_USER)
	{
		USER* user = reinterpret_cast<USER*>(m_pObjects[HpInfo->obj_id]);
		user->hp = HpInfo->obj_hp;
	}
	else if (OBJECT_TYPE_MONSTER_SCORPION <= m_pObjects[HpInfo->obj_id]->Type && m_pObjects[HpInfo->obj_id]->Type <= OBJECT_TYPE_MONSTER_GOBLIN_KING)
	{
		MONSTER* monster = reinterpret_cast<MONSTER*>(m_pObjects[HpInfo->obj_id]);
		monster->hp = HpInfo->obj_hp;
	}
}

void C_SIMPLE_MMORPG::MPChange(char* MPInfoByteStream)
{
	sc_packet_mp_change* MpInfo = reinterpret_cast<sc_packet_mp_change*>(MPInfoByteStream);
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	user->mp = MpInfo->user_mp;
}

void C_SIMPLE_MMORPG::MoneyChange(char* MoneyInfoByteStream)
{
	sc_packet_money_change* MoneyInfo = reinterpret_cast<sc_packet_money_change*>(MoneyInfoByteStream);
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	ITEM* slot_item = user->inventory.Slot[MoneyInfo->slot_num].Item;
	if (slot_item == nullptr) return;
	if (slot_item->Type != OBJECT_TYPE_ITEM_COIN) return;
	slot_item->How_many = MoneyInfo->user_money;
}

void C_SIMPLE_MMORPG::QuestChange(char* QuestInfoByteStream)
{
	sc_packet_quest_change* Quest = reinterpret_cast<sc_packet_quest_change*>(QuestInfoByteStream);
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	memcpy_s(&user->quest, sizeof(QUEST), &Quest->quest_info, sizeof(QUEST));
}

void C_SIMPLE_MMORPG::ReleaseObject(char* ObjectByteStream)
{
	sc_packet_remove_object* releaseObject = reinterpret_cast<sc_packet_remove_object*>(ObjectByteStream);
	if (m_pObjects.count(releaseObject->obj_id) == 0) return;
	delete m_pObjects[releaseObject->obj_id];
	m_pObjects.erase(releaseObject->obj_id);
}

void C_SIMPLE_MMORPG::SetChat(char* ChatByteStream)
{
	sc_packet_chat* ChatObj = reinterpret_cast<sc_packet_chat*>(ChatByteStream);
	array<wchar_t, MAX_WSTR_LEN> WChat;
	copy(begin(ChatObj->message), end(ChatObj->message), begin(WChat));
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	user->ChatLog.emplace_front(WChat);
	if (user->ChatLog.size() > CHAT_MAX_COUNT) user->ChatLog.erase(prev(user->ChatLog.end()));
	user->last_chat_time = high_resolution_clock::now();
}

void C_SIMPLE_MMORPG::SetFrameBuffer(HDC hDCFrameBuffer)
{
	m_hDCFrameBuffer = hDCFrameBuffer;
	if (m_pScreen != nullptr) delete m_pScreen;
	m_pScreen = new Graphics(hDCFrameBuffer);
}

void C_SIMPLE_MMORPG::Draw(HWND hWnd, HDC hDCFrameBuffer)
{
	if (m_pScreen == nullptr) return;
	if (m_pObjects.count(m_myID) == 0) return;
	if (m_pObjects[m_myID] == nullptr) return;

	RECT Map_draw_Range;
	Map_draw_Range.left   = max(WORLD_MIN, m_pObjects[m_myID]->x - VIEW_CENTER);
	Map_draw_Range.top    = max(WORLD_MIN, m_pObjects[m_myID]->y - VIEW_CENTER);
	Map_draw_Range.right  = min(WORLD_MAX, m_pObjects[m_myID]->x + VIEW_CENTER);
	Map_draw_Range.bottom = min(WORLD_MAX, m_pObjects[m_myID]->y + VIEW_CENTER);
	Rect DrawRect(0, 0, GDI_TILE_SIZE, GDI_TILE_SIZE);
	int DrawType = TILE_WATER;

	for (int row = Map_draw_Range.top; row <= Map_draw_Range.bottom; ++row)
	{
		for (int col = Map_draw_Range.left; col <= Map_draw_Range.right; ++col)
		{
			TransformScreen(col, row, DrawRect.X, DrawRect.Y);
			DrawRect.X -= GDI_TILE_SIZE / 2;
			DrawRect.Y -= GDI_TILE_SIZE / 2;

			// Tile Draw
			bool is_nothing = false;
			switch (m_TileData[row][col])
			{
			case OBJECT_TPYE_TILE_WATER:
				DrawType = TILE_WATER;
				break;
			case OBJECT_TPYE_TILE_GRASS:
				DrawType = TILE_GRASS;
				break;
			case OBJECT_TPYE_TILE_SAND:
				DrawType = TILE_SAND;
				break;
			default:
				is_nothing = true;
				break;
			}
			if (is_nothing != true) m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X, DrawRect.Y);

			// Obstacle Draw
			is_nothing = false;
			switch (m_Obstacles[row][col])
			{
			case OBJECT_TYPE_OBSTACLE_WALL:
				DrawType = OBSTACLE_WALL;
				break;
			case OBJECT_TYPE_OBSTACLE_ROCK:
				DrawType = OBSTACLE_ROCK;
				break;
			case OBJECT_TYPE_OBSTACLE_TREE:
				DrawType = OBSTACLE_TREE;
				break;
			default:
				is_nothing = true;
				break;
			}
			if(is_nothing != true) m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X, DrawRect.Y);
		}
	}

	// Draw Object
	DrawRect = { 0, 0, GDI_CHARACTER_SIZE, GDI_CHARACTER_SIZE };
	DrawType = CHARACTER_WARRIOR;
	//int i = 0;
	for (auto& object : m_pObjects)
	{
		if (object.first == m_myID) continue;
		TransformScreen(object.second->x, object.second->y, DrawRect.X, DrawRect.Y);
		DrawRect.X -= DrawRect.Width / 2;
		DrawRect.Y -= DrawRect.Height / 2;

		switch (object.second->Type)
		{
		case OBJECT_TYPE_USER:
			DrawType = CHARACTER_WARRIOR;
			break;
		case OBJECT_TYPE_NPC_CHIEF:
			DrawType = CHARACTER_CHIEF;
			break;
		case OBJECT_TYPE_NPC_BLACKSMITH:
			DrawType = CHARACTER_BLACKSMITH;
			break;
		case OBJECT_TYPE_NPC_POTION_MERCHANT:
			DrawType = CHARACTER_POTION_MERCHANT;
			break;
		case OBJECT_TYPE_MONSTER_SCORPION:
			DrawType = CHARACTER_SCORPION;
			break;
		case OBJECT_TYPE_MONSTER_SCORPION_KING:
			DrawType = CHARACTER_SCORPION_KING;
			break;
		case OBJECT_TYPE_MONSTER_THEIF:
			DrawType = CHARACTER_THEIF;
			break;
		case OBJECT_TYPE_MONSTER_THEIF_BOSS:
			DrawType = CHARACTER_THEIF_BOSS;
			break;
		case OBJECT_TYPE_MONSTER_WOLF:
			DrawType = CHARACTER_WOLF;
			break;
		case OBJECT_TYPE_MONSTER_WEREWOLF:
			DrawType = CHARACTER_WEREWOLF;
			break;
		case OBJECT_TYPE_MONSTER_GOBLIN_WARRIOR:
			DrawType = CHARACTER_GOBLIN_WARRIOR;
			break;
		case OBJECT_TYPE_MONSTER_GOBLIN_KING:
			DrawType = CHARACTER_GOBLIN_KING;
			break;
		case OBJECT_TYPE_ITEM_COIN:
			DrawType = ITEM_COIN;
			break;
		case OBJECT_TYPE_ITEM_HP_POTION:
			DrawType = ITEM_HP_POTION;
			break;
		case OBJECT_TYPE_ITEM_MP_POTION:
			DrawType = ITEM_MP_POTION;
			break;
		case OBJECT_TYPE_ITEM_NORMAL_ARMOR:
			DrawType = ITEM_NORMAL_ARMOR;
			break;
		case OBJECT_TYPE_ITEM_NORMAL_SWORD:
			DrawType = ITEM_NORMAL_SWORD;
			break;
		case OBJECT_TYPE_ITEM_NORMAL_RING:
			DrawType = ITEM_NORMAL_RING;
			break;
		case OBJECT_TYPE_ITEM_SPECIAL_ARMOR:
			DrawType = ITEM_SPECIAL_ARMOR;
			break;
		case OBJECT_TYPE_ITEM_SPECIAL_SWORD:
			DrawType = ITEM_SPECIAL_SWORD;
			break;
		case OBJECT_TYPE_ITEM_SPECIAL_RING:
			break;
			DrawType = ITEM_SPECIAL_RING;
		}
		m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X, DrawRect.Y);
		if (ITEM_HP_POTION <= DrawType && DrawType <= ITEM_SPECIAL_RING)
		{
			ITEM* Item = reinterpret_cast<ITEM*>(object.second);
			if (Item->How_many > 1)
			{
				wchar_t text[256];
				wsprintfW(text, L"%d", Item->How_many);
				SIZE text_size;
				GetTextExtentExPointW(hDCFrameBuffer, text, (int)wcsnlen_s(text, 256), 0, NULL, NULL, &text_size);
				PrintText(hDCFrameBuffer, 12, "굴림체", RGB(46, 46, 46), DrawRect.X + 3 + GDI_ITEM_SIZE - (int)(text_size.cx / 1.3f), DrawRect.Y + GDI_ITEM_SIZE - (int)(text_size.cy / 1.6f), text);
			}
		}
	}

	// 유저 캐릭터가 다른 오브젝트보다 항상 위에 그려지도록 함.
	DrawRect = { 0, 0, GDI_CHARACTER_SIZE, GDI_CHARACTER_SIZE };
	DrawType = CHARACTER_WARRIOR;
	TransformScreen(m_pObjects[m_myID]->x, m_pObjects[m_myID]->y, DrawRect.X, DrawRect.Y);
	DrawRect.X -= DrawRect.Width / 2;
	DrawRect.Y -= DrawRect.Height / 2;
	m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X, DrawRect.Y);

	// Effect Attack Draw
	DrawRect = { 0, 0, GDI_EFFECT_ATTACK_SIZE, GDI_EFFECT_ATTACK_SIZE };
	high_resolution_clock::time_point last_attack_time;
	char attack_type = ATTACK_TYPE_NON;
	char attack_frameNum = 0;
	for (auto& object : m_pObjects)
	{
		if (object.second->Type != OBJECT_TYPE_USER
			&& (object.second->Type < OBJECT_TYPE_MONSTER_SCORPION
			|| object.second->Type > OBJECT_TYPE_MONSTER_GOBLIN_KING)) continue;

		if (object.second->Type == OBJECT_TYPE_USER)
		{
			USER* user = reinterpret_cast<USER*>(object.second);
			attack_type = user->attack_type;
			last_attack_time = user->last_attack_time;
			attack_frameNum = user->attack_frameNum;
		}
		else
		{
			MONSTER* monster = reinterpret_cast<MONSTER*>(object.second);
			attack_type = monster->attack_type;
			last_attack_time = monster->last_attack_time;
			attack_frameNum = monster->attack_frameNum;
		}
		if (attack_type != ATTACK_TYPE_NON)
		{
			switch (attack_type)
			{
			case ATTACK_TYPE_NORMAL:
				DrawType = EFFECT_ATTACK_1_1;
				break;
			case ATTACK_TYPE_POWER:
				DrawType = EFFECT_ATTACK_2_1;
				break;
			case ATTACK_TYPE_CLAW:
				DrawType = EFFECT_ATTACK_3_1;
				break;
			}

			// Update Effect Frame
			if (attack_frameNum < (EFFECT_ATTACK_IMAGE_COLUMN - 1))
			{
				if (attack_frameNum < 0) attack_frameNum = 0;
				if (high_resolution_clock::now() >= last_attack_time + ATTACK_FRAME_CHANGE_INTERVAL)
				{
					if (object.second->Type == OBJECT_TYPE_USER)
					{
						USER* user = reinterpret_cast<USER*>(object.second);
						user->attack_frameNum = Clamp(0, user->attack_frameNum + 1, EFFECT_ATTACK_IMAGE_COLUMN);
						user->last_attack_time = high_resolution_clock::now() + ATTACK_FRAME_CHANGE_INTERVAL;
					}
					else
					{
						MONSTER* monster = reinterpret_cast<MONSTER*>(object.second);
						monster->attack_frameNum = Clamp(0, monster->attack_frameNum + 1, EFFECT_ATTACK_IMAGE_COLUMN);
						monster->last_attack_time = high_resolution_clock::now() + ATTACK_FRAME_CHANGE_INTERVAL;
					}
				}
			}
			else
			{
				if (object.second->Type == OBJECT_TYPE_USER)
				{
					USER* user = reinterpret_cast<USER*>(object.second);
					user->attack_frameNum = 0;
					user->attack_type = ATTACK_TYPE_NON;
				}
				else
				{
					MONSTER* monster = reinterpret_cast<MONSTER*>(object.second);
					monster->attack_frameNum = 0;
					monster->attack_type = ATTACK_TYPE_NON;
				}
				attack_frameNum = 0;
			}
			
			DrawType += attack_frameNum;

			// 상
			TransformScreen(object.second->x, object.second->y - 1, DrawRect.X, DrawRect.Y);
			DrawRect.X -= DrawRect.Width / 2;
			DrawRect.Y -= DrawRect.Height / 2;
			m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X, DrawRect.Y);

			// 하
			TransformScreen(object.second->x, object.second->y + 1, DrawRect.X, DrawRect.Y);
			DrawRect.X -= DrawRect.Width / 2;
			DrawRect.Y -= DrawRect.Height / 2;
			m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X, DrawRect.Y);

			// 좌
			TransformScreen(object.second->x - 1, object.second->y, DrawRect.X, DrawRect.Y);
			DrawRect.X -= DrawRect.Width / 2;
			DrawRect.Y -= DrawRect.Height / 2;
			m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X, DrawRect.Y);

			// 우
			TransformScreen(object.second->x + 1, object.second->y, DrawRect.X, DrawRect.Y);
			DrawRect.X -= DrawRect.Width / 2;
			DrawRect.Y -= DrawRect.Height / 2;
			m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X, DrawRect.Y);			
		}
	}

	// Draw HP
	unsigned int hp = 0;
	for (auto& object : m_pObjects)
	{
		if (object.first == m_myID) continue;
		if (object.second->Type != OBJECT_TYPE_USER
			&& (object.second->Type < OBJECT_TYPE_MONSTER_SCORPION
			|| object.second->Type > OBJECT_TYPE_MONSTER_GOBLIN_KING)) continue;

		if (object.second->Type == OBJECT_TYPE_USER)
		{
			USER* user = reinterpret_cast<USER*>(object.second);
			hp = user->hp;
		}
		else 
		{
			MONSTER* monster = reinterpret_cast<MONSTER*>(object.second);
			hp = monster->hp;
		}


		TransformScreen(object.second->x, object.second->y, DrawRect.X, DrawRect.Y);
		SolidBrush Brush1(Color::LightGray);
		m_pScreen->FillRectangle(&Brush1, DrawRect.X - GDI_PROGRESS_BAR_WIDTH / 2, DrawRect.Y - GDI_CHARACTER_SIZE, GDI_PROGRESS_BAR_WIDTH, GDI_PROGRESS_BAR_HEIGHT);
		SolidBrush Brush2(Color::Red);
		m_pScreen->FillRectangle(&Brush2, DrawRect.X - GDI_PROGRESS_BAR_WIDTH / 2, DrawRect.Y - GDI_CHARACTER_SIZE, (int)(GDI_PROGRESS_BAR_WIDTH * ((float)hp / GDI_PROGRESS_BAR_WIDTH)), GDI_PROGRESS_BAR_HEIGHT);

		wchar_t text[256];
		wsprintfW(text, L"HP:%d", hp);
		SIZE text_size;
		GetTextExtentExPointW(hDCFrameBuffer, text, (int)wcsnlen_s(text, 256), 0, NULL, NULL, &text_size);
		PrintText(hDCFrameBuffer, 20, "굴림체", RGB(13, 13, 13), DrawRect.X - text_size.cx / 2, DrawRect.Y - GDI_CHARACTER_SIZE, text);
	}

	// Fog Draw
	m_pScreen->DrawCachedBitmap(m_CachedImages[UI_FOG], 0, 0);

	// Draw Pos Text
	{
		wchar_t text[256];
		wsprintfW(text, L"MY POSITION (%2d, %2d)", m_pObjects[m_myID]->x, m_pObjects[m_myID]->y);
		PrintText(hDCFrameBuffer, 30, "굴림체", RGB(114, 114, 114), 10, VIEWPORT_SIZE - 38, text);
	}

	// Draw Chatting Message
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	if (user->ChatLog.size() != 0 && user->CheckChatTopLineAliveOver())
		user->ChatLog.erase(prev(user->ChatLog.end()));
	int chat_count = 1;
	for (auto& chat : user->ChatLog)
	{
		PrintText(hDCFrameBuffer, 24, "굴림체", RGB(chat_count * 32, chat_count * 32, 196), 10, VIEWPORT_SIZE - 45 - chat_count * 24, &chat[0]);
		chat_count++;
	}

	// Draw Stat Window
	DrawRect.X = (VIEW_SIZE - 3) * GDI_TILE_SIZE - 12;
	DrawRect.Y = 0;
	DrawRect.Width = 3 * GDI_TILE_SIZE + 12;
	DrawRect.Height = VIEWPORT_SIZE;

	// Stat Window Background
	SolidBrush Brush(Color::SlateGray);
	m_pScreen->FillRectangle(&Brush, DrawRect);

	// ID draw
	{
		wchar_t text[256];
		wsprintfW(text, L"ID:%ws", user->id_str);
		PrintText(hDCFrameBuffer, 22, "굴림체", RGB(255, 255, 255), DrawRect.X + 8, DrawRect.Y + 20, text);
	}

	// User Chracter Draw in UI
	{
		DrawType = CHARACTER_WARRIOR;
		m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], DrawRect.X + 16, DrawRect.Y + 70);
	}

	// Level draw
	{
		wchar_t text[256];
		wsprintfW(text, L"Lev.%d", (int)user->level);
		PrintText(hDCFrameBuffer, 22, "굴림체", RGB(255, 255, 255), DrawRect.X + 66, DrawRect.Y + 80, text);
	}

	// Exp draw
	{
		SolidBrush Brush1(Color::LightGray);
		m_pScreen->FillRectangle(&Brush1, DrawRect.X + 19, DrawRect.Y + 140, GDI_PROGRESS_BAR_WIDTH, GDI_PROGRESS_BAR_HEIGHT);
		SolidBrush Brush2(Color(202, 253, 153));
		m_pScreen->FillRectangle(&Brush2, DrawRect.X + 19, DrawRect.Y + 140, (int)(GDI_PROGRESS_BAR_WIDTH * (user->exp / (float)user->max_exp)), GDI_PROGRESS_BAR_HEIGHT);
		wchar_t text[256];
		wsprintfW(text, L"Exp:%d/%d", user->exp, user->max_exp);
		SIZE text_size;
		GetTextExtentExPointW(hDCFrameBuffer, text, (int)wcsnlen_s(text, 256), 0, NULL, NULL, &text_size);
		PrintText(hDCFrameBuffer, 18, "굴림체", RGB(13, 13, 13), DrawRect.X + 6 + (int)(GDI_TILE_SIZE * 1.5f) - (int)(text_size.cx / 1.6f), DrawRect.Y + 141, text);
	}

	// HP draw
	{
		SolidBrush Brush1(Color::LightGray);
		m_pScreen->FillRectangle(&Brush1, DrawRect.X + 19, DrawRect.Y + 180, GDI_PROGRESS_BAR_WIDTH, GDI_PROGRESS_BAR_HEIGHT);
		SolidBrush Brush2(Color(221, 97, 97));
		m_pScreen->FillRectangle(&Brush2, DrawRect.X + 19, DrawRect.Y + 180, (int)(GDI_PROGRESS_BAR_WIDTH * (user->hp / (float)user->max_hp)), GDI_PROGRESS_BAR_HEIGHT);
		wchar_t text[256];
		wsprintfW(text, L"HP:%d/%d", user->hp, user->max_hp);
		SIZE text_size;
		GetTextExtentExPointW(hDCFrameBuffer, text, (int)wcsnlen_s(text, 256), 0, NULL, NULL, &text_size);
		PrintText(hDCFrameBuffer, 18, "굴림체", RGB(13, 13, 13), DrawRect.X + 6 + (int)(GDI_TILE_SIZE * 1.5f) - (int)(text_size.cx / 1.7f), DrawRect.Y + 181, text);
	}

	// MP draw
	{
		SolidBrush Brush1(Color::LightGray);
		m_pScreen->FillRectangle(&Brush1, DrawRect.X + 19, DrawRect.Y + 220, GDI_PROGRESS_BAR_WIDTH, GDI_PROGRESS_BAR_HEIGHT);
		SolidBrush Brush2(Color(81, 128, 208));
		m_pScreen->FillRectangle(&Brush2, DrawRect.X + 19, DrawRect.Y + 220, (int)(GDI_PROGRESS_BAR_WIDTH * (user->mp / (float)user->max_mp)), GDI_PROGRESS_BAR_HEIGHT);
		wchar_t text[256];
		wsprintfW(text, L"MP:%d/%d", user->mp, user->max_mp);
		SIZE text_size;
		GetTextExtentExPointW(hDCFrameBuffer, text, (int)wcsnlen_s(text, 256), 0, NULL, NULL, &text_size);
		PrintText(hDCFrameBuffer, 18, "굴림체", RGB(13, 13, 13), DrawRect.X + 6 + (int)(GDI_TILE_SIZE * 1.5f) - (int)(text_size.cx / 1.7f), DrawRect.Y + 221, text);
	}

	// Equipment slot draw
	{
		wchar_t text[256];
		wsprintfW(text, L"Equipped");
		PrintText(hDCFrameBuffer, 18, "굴림체", RGB(255, 255, 255), DrawRect.X + 6, DrawRect.Y + 280, text);

		SolidBrush Brush(Color::DarkGray);
		RECT SlotRect;
		for (int i = 0; i < 3; ++i)
		{
			SlotRect = user->inventory.Slot[i].AreaRect;
				m_pScreen->FillRectangle(&Brush, SlotRect.left, SlotRect.top, SlotRect.right - SlotRect.left, SlotRect.bottom - SlotRect.top);
		}

		for (int i = 0; i < 3; ++i)
		{
			if (user->inventory.Slot[i].Item == nullptr) continue;
			ITEM* SlotItem = user->inventory.Slot[i].Item;
			switch (SlotItem->Type)
			{
			case OBJECT_TYPE_ITEM_NORMAL_ARMOR:
				DrawType = ITEM_NORMAL_ARMOR;
				break;
			case OBJECT_TYPE_ITEM_NORMAL_SWORD:
				DrawType = ITEM_NORMAL_SWORD;
				break;
			case OBJECT_TYPE_ITEM_NORMAL_RING:
				DrawType = ITEM_NORMAL_RING;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_ARMOR:
				DrawType = ITEM_SPECIAL_ARMOR;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_SWORD:
				DrawType = ITEM_SPECIAL_SWORD;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_RING:
				break;
				DrawType = ITEM_SPECIAL_RING;
			default:
				continue;
			}
			m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], SlotItem->x, SlotItem->y);
		}
	}

	// Item slot draw
	{
		wchar_t text[256];
		wsprintfW(text, L"Item");
		PrintText(hDCFrameBuffer, 18, "굴림체", RGB(255, 255, 255), DrawRect.X + 6, DrawRect.Y + 380, text);

		SolidBrush Brush(Color::DarkGray);
		RECT SlotRect;
		for (int i = 1; i <= 2; ++i)
		{
			for (int j = 1; j <= 3; ++j)
			{
				SlotRect = user->inventory.Slot[i * 3 + (j - 1)].AreaRect;
				m_pScreen->FillRectangle(&Brush, SlotRect.left, SlotRect.top, SlotRect.right - SlotRect.left, SlotRect.bottom - SlotRect.top);
			}
		}

		for (int i = 0; i < 6; ++i)
		{
			if (user->inventory.Slot[i + 3].Item == nullptr) continue;
			ITEM* SlotItem = user->inventory.Slot[i + 3].Item;
			switch (SlotItem->Type)
			{
			case OBJECT_TYPE_ITEM_COIN:
				DrawType = ITEM_COIN;
				break;
			case OBJECT_TYPE_ITEM_HP_POTION:
				DrawType = ITEM_HP_POTION;
				break;
			case OBJECT_TYPE_ITEM_MP_POTION:
				DrawType = ITEM_MP_POTION;
				break;
			case OBJECT_TYPE_ITEM_NORMAL_ARMOR:
				DrawType = ITEM_NORMAL_ARMOR;
				break;
			case OBJECT_TYPE_ITEM_NORMAL_SWORD:
				DrawType = ITEM_NORMAL_SWORD;
				break;
			case OBJECT_TYPE_ITEM_NORMAL_RING:
				DrawType = ITEM_NORMAL_RING;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_ARMOR:
				DrawType = ITEM_SPECIAL_ARMOR;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_SWORD:
				DrawType = ITEM_SPECIAL_SWORD;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_RING:
				break;
				DrawType = ITEM_SPECIAL_RING;
			default:
				continue;
			}
			m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], SlotItem->x, SlotItem->y);

			if (OBJECT_TYPE_ITEM_HP_POTION <= SlotItem->Type && SlotItem->Type <= OBJECT_TYPE_ITEM_COIN && SlotItem->How_many > 0)
			{
				wchar_t text[256];
				wsprintfW(text, L"%d", SlotItem->How_many);
				SIZE text_size;
				GetTextExtentExPointW(hDCFrameBuffer, text, (int)wcsnlen_s(text, 256), 0, NULL, NULL, &text_size);
				PrintText(hDCFrameBuffer, 12, "굴림체", RGB(46, 46, 46), DrawRect.X + 3 * (i % 3 + 1) + GDI_ITEM_SIZE * (i % 3 + 1) - (int)(text_size.cx / 1.3f), DrawRect.Y + 400 + (i / 3) * i + GDI_ITEM_SIZE * (i / 3 + 1) - (int)(text_size.cy / 1.6f), text);
			}
		}
	}

	// Quest info draw
	{
		wchar_t text[256];
		wsprintfW(text, L"Quest");
		PrintText(hDCFrameBuffer, 18, "굴림체", RGB(255, 255, 255), DrawRect.X + 6, DrawRect.Y + 530, text);

		switch (user->quest.Type)
		{
		case NON:
		{
			wchar_t text[256];
			wsprintfW(text, L"진행중인 퀘스트가 없습");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 550, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"니다.");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 563, text);
			break;
		}
		case TEST_ABILITY: // 능력 테스트 (Kill 전갈 10 And Kill 전갈킹 1)
		{
			wchar_t text[256];
			wsprintfW(text, L"전갈무리를 사냥하십시");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 550, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"오.");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 563, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"전갈:%d/%d", user->quest.Quest_Progress[0].How_many, 10);
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 583, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"전갈킹:%d/%d", user->quest.Quest_Progress[1].How_many, 1);
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 603, text);
			break;
		}
		case SUBJUGATE_BANDIT: // 산적 토벌 (Kill 산적 30 And Kill 산적두목 2)
		{
			wchar_t text[256];
			wsprintfW(text, L"도적무리를 토벌하십시");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 550, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"오.");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 563, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"도적:%d/%d", user->quest.Quest_Progress[0].How_many, 30);
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 583, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"도적두목:%d/%d", user->quest.Quest_Progress[1].How_many, 2);
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 603, text);
			break;
		}
		case ENCOUNTER_GOBLIN_KING: // 고블린 킹과의 조우 (Kill 깊은 숲 몬스터 100 And Encounter 고블린 킹)
		{
			wchar_t text[256];
			wsprintfW(text, L"깊은 숲 몬스터를 처치");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 550, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"하고, 고블린 킹을 발견");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 563, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"하십시오.");
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 576, text);

			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"깊은 숲 몬스터:%d/%d", user->quest.Quest_Progress[0].How_many, 100);
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 596, text);
			ZeroMemory(text, sizeof(text));
			wsprintfW(text, L"고블린킹:%d/%d", user->quest.Quest_Progress[1].How_many, 2);
			PrintText(hDCFrameBuffer, 12, "굴림체", RGB(255, 255, 255), DrawRect.X + 3, DrawRect.Y + 616, text);
			break;
		}
		}
	}

	// Draged Item Draw
	{
		if (m_pDragItem != nullptr)
		{
			switch (m_pDragItem->Type)
			{
			case OBJECT_TYPE_ITEM_COIN:
				DrawType = ITEM_COIN;
				break;
			case OBJECT_TYPE_ITEM_HP_POTION:
				DrawType = ITEM_HP_POTION;
				break;
			case OBJECT_TYPE_ITEM_MP_POTION:
				DrawType = ITEM_MP_POTION;
				break;
			case OBJECT_TYPE_ITEM_NORMAL_ARMOR:
				DrawType = ITEM_NORMAL_ARMOR;
				break;
			case OBJECT_TYPE_ITEM_NORMAL_SWORD:
				DrawType = ITEM_NORMAL_SWORD;
				break;
			case OBJECT_TYPE_ITEM_NORMAL_RING:
				DrawType = ITEM_NORMAL_RING;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_ARMOR:
				DrawType = ITEM_SPECIAL_ARMOR;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_SWORD:
				DrawType = ITEM_SPECIAL_SWORD;
				break;
			case OBJECT_TYPE_ITEM_SPECIAL_RING:
				break;
				DrawType = ITEM_SPECIAL_RING;
			}

			m_pScreen->DrawCachedBitmap(m_CachedImages[DrawType], m_pDragItem->x, m_pDragItem->y);
			if (OBJECT_TYPE_ITEM_HP_POTION <= m_pDragItem->Type && m_pDragItem->Type <= OBJECT_TYPE_ITEM_COIN && m_pDragItem->How_many > 0)
			{
				wchar_t text[256];
				wsprintfW(text, L"%d", m_pDragItem->How_many);
				SIZE text_size;
				GetTextExtentExPointW(hDCFrameBuffer, text, (int)wcsnlen_s(text, 256), 0, NULL, NULL, &text_size);
				PrintText(hDCFrameBuffer, 12, "굴림체", RGB(46, 46, 46), m_pDragItem->x + GDI_ITEM_SIZE - (int)(text_size.cx / 1.3f), m_pDragItem->y + GDI_ITEM_SIZE - (int)(text_size.cy / 1.6f), text);
			}
		}
	}
}

void C_SIMPLE_MMORPG::myPlayerMoveBy(int dx, int dy)
{
	if (m_pObjects.count(m_myID) == 0) return;
	int new_x = Clamp(WORLD_MIN, m_pObjects[m_myID]->x + dx, WORLD_MAX);
	int new_y = Clamp(WORLD_MIN, m_pObjects[m_myID]->y + dy, WORLD_MAX);

	if (m_TileData[new_y][new_x] == OBJECT_TPYE_TILE_WATER) return;
	if (m_Obstacles[new_y][new_x] != OBJECT_TYPE_NON) return;

	m_pObjects[m_myID]->x = new_x;
	m_pObjects[m_myID]->y = new_y;
}

void C_SIMPLE_MMORPG::myPlayerDoAttackAnimaition()
{
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	user->attack_type = ATTACK_TYPE_NORMAL;
	user->last_attack_time = high_resolution_clock::now();
}

void C_SIMPLE_MMORPG::myPlayerDoPowerAttackAnimation()
{
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	if (user->mp == 0) return;
	//user->mp -= 10;
	user->attack_type = ATTACK_TYPE_POWER;
	user->last_attack_time = high_resolution_clock::now();
}

void C_SIMPLE_MMORPG::myPlayerUseHP_Potion()
{
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	ITEM* HP_Potion = nullptr;
	char SlotNum = -1;
	for (int i = 0; i < MAX_ITEM_SLOT; ++i)
	{
		if (user->inventory.Slot[i].Item == nullptr) continue;
		if (user->inventory.Slot[i].Item->Type != OBJECT_TYPE_ITEM_HP_POTION) continue;
		HP_Potion = user->inventory.Slot[i].Item;
		SlotNum = (char)i;
		break;
	}
	if (HP_Potion != nullptr) ProcessSocket::SendPacket(CS_ITEM_USE, SlotNum, 1);
}

void C_SIMPLE_MMORPG::myPlayerUseMP_Potion()
{
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	ITEM* MP_Potion = nullptr;
	char SlotNum = -1;
	for (int i = 0; i < MAX_ITEM_SLOT; ++i)
	{
		if (user->inventory.Slot[i].Item == nullptr) continue;
		if (user->inventory.Slot[i].Item->Type != OBJECT_TYPE_ITEM_MP_POTION) continue;
		MP_Potion = user->inventory.Slot[i].Item;
		SlotNum = (char)i;
		break;
	}
	if (MP_Potion != nullptr) ProcessSocket::SendPacket(CS_ITEM_USE, SlotNum, 1);
}

void C_SIMPLE_MMORPG::StartMouseDrage(POINT* MousePos)
{
	m_pMousePos = MousePos;

	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	for (int i = 0; i < MAX_ITEM_SLOT; ++i)
	{
		if (user->inventory.Slot[i].Item == nullptr) continue;
		if (PointInSlot(m_pMousePos->x, m_pMousePos->y, user->inventory.Slot[i].AreaRect) == true)
		{
			m_pDragItem = user->inventory.Slot[i].Item;
			user->inventory.Slot[i].Item = nullptr;
			user->inventory.SelectedSlotNum = (char)i;
			m_pDragItem->x = m_pMousePos->x - GDI_ITEM_SIZE / 2;
			m_pDragItem->y = m_pMousePos->y - GDI_ITEM_SIZE / 2;
			break;
		}
	}
}

void C_SIMPLE_MMORPG::EndMouseDrage()
{
	if (m_pDragItem == nullptr)
	{
		m_pMousePos = nullptr;
		return;
	}
	if (m_pObjects.count(m_myID) == 0) return;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);

	bool SlotItemChange = false;

	// 아이템을 드래그 한 상태에서 다른 슬롯에 놓을 경우.
	for (int i = 0; i < MAX_ITEM_SLOT; ++i)
	{
		if (PointInSlot(m_pMousePos->x, m_pMousePos->y, user->inventory.Slot[i].AreaRect) == true)
		{
			// 해당 슬롯에 다른 아이템이 이미 존재하는 경우
			if (user->inventory.Slot[i].Item != nullptr)
			{
				// 드래그 되었던 아이템의 이전 슬롯 위치로 해당 슬롯 아이템을 옮기고,
				// 해당 슬롯은 드래그 되었던 아이템이 위치하게 한다.
				// 만약, 해당 슬롯이 장비 슬롯이고 드래그 되었던 아이템이 장비 아이템이 아닐 경우에는
				// 드래그 되었던 아이템이 드래그 되기 전의 슬롯 위치로 되돌아 간다.
				if ((0 <= i && i <= 2) && (OBJECT_TYPE_ITEM_HP_POTION <= m_pDragItem->Type && m_pDragItem->Type <= OBJECT_TYPE_ITEM_COIN))
				{
					TransformSlot(user->inventory.SelectedSlotNum, m_pDragItem->x, m_pDragItem->y);
					user->inventory.Slot[user->inventory.SelectedSlotNum].Item = m_pDragItem;
				}
				else
				{
					TransformSlot(user->inventory.SelectedSlotNum, user->inventory.Slot[i].Item->x, user->inventory.Slot[i].Item->y);
					user->inventory.Slot[user->inventory.SelectedSlotNum].Item = user->inventory.Slot[i].Item;
					TransformSlot(i, m_pDragItem->x, m_pDragItem->y);
					user->inventory.Slot[i].Item = m_pDragItem;
					ProcessSocket::SendPacket(CS_ITEM_SLOT_INFO_CHANGE, (unsigned char)user->inventory.SelectedSlotNum, (unsigned char)i);
				}
				
				m_pDragItem = nullptr;
				user->inventory.SelectedSlotNum = -1;
				SlotItemChange = true;
			}
			else // 해당 슬롯이 비어있을 경우
			{
				if ((0 <= i && i <= 2) && (OBJECT_TYPE_ITEM_HP_POTION <= m_pDragItem->Type && m_pDragItem->Type <= OBJECT_TYPE_ITEM_COIN))
				{
					TransformSlot(user->inventory.SelectedSlotNum, m_pDragItem->x, m_pDragItem->y);
					user->inventory.Slot[user->inventory.SelectedSlotNum].Item = m_pDragItem;
				}
				else
				{
					TransformSlot(i, m_pDragItem->x, m_pDragItem->y);
					user->inventory.Slot[i].Item = m_pDragItem;
					ProcessSocket::SendPacket(CS_ITEM_SLOT_INFO_CHANGE, (unsigned char)user->inventory.SelectedSlotNum, (unsigned char)i);
				}
				m_pDragItem = nullptr;
				user->inventory.SelectedSlotNum = -1;
				SlotItemChange = true;
			}
			break;
		}
	}

	RECT Stat_UI_RECT = { (VIEW_SIZE - 3) * GDI_TILE_SIZE - 12, 0, VIEWPORT_SIZE, VIEWPORT_SIZE };
	if (SlotItemChange != true)
	{
		// 드래그 되었던 아이템을 슬롯이 아닌 유저 상태 창 내부의 다른 영역에 놓았을 경우
		if (PointInSlot(m_pMousePos->x, m_pMousePos->y, Stat_UI_RECT) == true)
		{
			TransformSlot(user->inventory.SelectedSlotNum, m_pDragItem->x, m_pDragItem->y);
			user->inventory.Slot[user->inventory.SelectedSlotNum].Item = m_pDragItem;
			m_pDragItem = nullptr;
			user->inventory.SelectedSlotNum = -1;
			SlotItemChange = true;
		}
		else // 드래그 되었던 아이템을 월드에 놓았을 경우
		{
			ProcessSocket::SendPacket(CS_ITEM_DROP, user->inventory.SelectedSlotNum, m_pDragItem->How_many);
			/*m_pDragItem->x = user->x;
			m_pDragItem->y = user->y;
			m_pObjects[C_SIMPLE_MMORPG::Find_EmptyObject(DROP_ITEM_START_INDEX, DROP_ITEM_START_INDEX + MAX_DROP_ITEM - 1)] = m_pDragItem;
			m_pDragItem = nullptr;
			user->inventory.SelectedSlotNum = -1;
			SlotItemChange = true;*/
		}
	}
	m_pMousePos = nullptr;
}

bool C_SIMPLE_MMORPG::myPlayerMoveDone()
{
	if (m_pObjects.count(m_myID) == 0) return false;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	if (user == nullptr) return false;
	if (high_resolution_clock::now() >= user->last_move_time + MOVE_COOLTIME)
	{
		user->last_move_time = high_resolution_clock::now() + MOVE_COOLTIME;
		return true;
	}
	return false;
}

bool C_SIMPLE_MMORPG::myPlayerAttackDone()
{
	if (m_pObjects.count(m_myID) == 0) return false;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	if (user == nullptr) return false;
	if (high_resolution_clock::now() >= user->last_attack_time + ATTACK_COOLTIME)
	{
		user->last_attack_time = high_resolution_clock::now() + ATTACK_COOLTIME;
		return true;
	}
	return false;
}

bool C_SIMPLE_MMORPG::myPlayerItemGetDone()
{
	if (m_pObjects.count(m_myID) == 0) return false;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	if (user == nullptr) return false;
	if (high_resolution_clock::now() >= user->last_item_get_time + GET_ITEM_COOLTIME)
	{
		user->last_item_get_time = high_resolution_clock::now() + GET_ITEM_COOLTIME;
		return true;
	}
	return false;
}

bool C_SIMPLE_MMORPG::myPlayerItemUseDone()
{
	if (m_pObjects.count(m_myID) == 0) return false;
	USER* user = reinterpret_cast<USER*>(m_pObjects[m_myID]);
	if (user == nullptr) return false;
	if (high_resolution_clock::now() >= user->last_item_use_time + USE_ITEM_COOLTIME)
	{
		user->last_item_use_time = high_resolution_clock::now() + USE_ITEM_COOLTIME;
		return true;
	}
	return false;
}

int C_SIMPLE_MMORPG::Find_EmptyObject(int range_low, int range_high)
{
	if (range_low < 0) return -1;
	if (range_high >= MAX_OBJECT) return -1;

	for (int i = range_low; i < range_high; ++i)
	{
		if (m_pObjects.count(i) == 0) return i;
	}
	return -1;
}

void C_SIMPLE_MMORPG::TransformScreen(int world_x, int world_y, int& screen_x, int& screen_y)
{
	if (m_pObjects.count(m_myID) == 0) return;
	screen_x = VIEPORT_CENTER + (world_x - m_pObjects[m_myID]->x) * GDI_TILE_SIZE;
	screen_y = VIEPORT_CENTER + (world_y - m_pObjects[m_myID]->y) * GDI_TILE_SIZE;
}
