#pragma once
#include "stdafx.h"

#define ATTACK_TYPE_NON    0
#define ATTACK_TYPE_NORMAL 1
#define ATTACK_TYPE_POWER  2
#define ATTACK_TYPE_CLAW   3
#define ATTACK_FRAME_CHANGE_INTERVAL 10ms

#define CHAT_ALIVE_TIME 1000ms
#define CHAT_MAX_COUNT 5

class OBJECT
{
public:
	OBJECT() = default;
	OBJECT(unsigned char type, int X, int Y) : Type(type), x(X), y(Y) {}
	virtual ~OBJECT() {}
	unsigned char  Type; // Non, User, NPC, Monster, Tile, Obstacle, item, ..., etc.
	int            x, y;
};

class ITEM :public OBJECT
{
public:
	ITEM() = default;
	ITEM(unsigned char type, int X, int Y) : OBJECT(type, X, Y) {}
	ITEM(unsigned char type, int X, int Y, int how_many) : OBJECT(type, X, Y) { How_many = how_many; }
	virtual ~ITEM() {}

	int How_many;
};

struct INVENTORY_SLOT
{
	RECT AreaRect = { 0,0,0,0 };
	ITEM* Item = nullptr;
	friend void TransformSlot(int SlotNum, int& x, int& y);
	friend bool PointInSlot(int x, int y, RECT rect);
};

struct INVENTORY
{
	INVENTORY_SLOT Slot[MAX_ITEM_SLOT];
	char SelectedSlotNum = -1;
};

class MONSTER : public OBJECT
{
public:
	MONSTER() = default;
	MONSTER(unsigned char type, int X, int Y) : OBJECT(type, X, Y) {}
	virtual ~MONSTER() {}

	int   hp = 0;

	high_resolution_clock::time_point last_attack_time;
	char attack_type = ATTACK_TYPE_NON;
	char attack_frameNum = 0;
};

class USER :public OBJECT
{
public:
	USER() = default;
	USER(unsigned char type, int X, int Y) : OBJECT(type, X, Y) {}
	virtual ~USER() { ChatLog.clear(); }

	wchar_t       id_str[MAX_WSTR_LEN];
	unsigned char level = 0;
	unsigned int  exp = 0;
	unsigned int  hp = 0;
	unsigned int  mp = 0;
	int           damage = 0;
	int           defense = 0;
	QUEST         quest;
	INVENTORY     inventory;

	unsigned int max_hp = 0;
	unsigned int max_mp = 0;
	unsigned int max_exp = 0;

	list<array<wchar_t, MAX_WSTR_LEN>> ChatLog;

	high_resolution_clock::time_point last_chat_time;
	high_resolution_clock::time_point last_move_time;
	high_resolution_clock::time_point last_item_get_time;
	high_resolution_clock::time_point last_attack_time;
	high_resolution_clock::time_point last_item_use_time;
	char attack_type = ATTACK_TYPE_NON;
	char attack_frameNum = 0;

	bool CheckChatTopLineAliveOver();
};

class C_SIMPLE_MMORPG
{
public:
	C_SIMPLE_MMORPG();
	~C_SIMPLE_MMORPG();
	void Init();
	void LoadPNG(const WCHAR* filename);
	void LoadMap(const WCHAR* filename);

	void LoginOk(char* LoginByteStream);
	void SetmyCharacter(char* CharacterByteStream);

	void ProcessInput();

	void AddNewObject(char* ObjectByteStream);
	void ReleaseObject(char* ObjetByteStream);
	void ObjectPos(char* ObjetByteStream);
	void ObjectDoAttack(char* AttackInfoByteStream);
	void ObjectDoPowerAttack(char* AttackInfoByteStream);
	int  Find_ItemInWorld();
	char Find_PutAbleSlot(int itemID);
	void ItemGetOk(char* GetInfoByteStream);
	void ItemDropOk(char* DropInfoByteStream);
	void ItemBuyOk(char* BuyInfoByteStream);
	void ItemUseOk(char* UseInfoByteStream);
	void LevelUp(char* LevelUpInfoByteStream); 
	void ExpChange(char* ExpInfoByteStream);
	void HPChange(char* HPInfoByteStream);
	void MPChange(char* MPInfoByteStream);
	void MoneyChange(char* MoneyInfoByteStream);
	void QuestChange(char* QuestInfoByteStream);

	void SetChat(char* ChatByteStream);

	void SetFrameBuffer(HDC hDCFrameBuffer);
	void Draw(HWND hWnd, HDC hDCFrameBuffer);// 렌더링

	void myPlayerMoveBy(int dx, int dy);
	void myPlayerDoAttackAnimaition();
	void myPlayerDoPowerAttackAnimation();
	void myPlayerUseHP_Potion();
	void myPlayerUseMP_Potion();

	void StartMouseDrage(POINT* MousePos);
	void EndMouseDrage();

	bool myPlayerMoveDone();
	bool myPlayerAttackDone();
	bool myPlayerItemGetDone();
	bool myPlayerItemUseDone();

	int  Find_EmptyObject(int range_low, int range_high);
private:
	HDC                    m_hDCFrameBuffer;
	Graphics*              m_pScreen;
	vector<Bitmap*>        m_Images;
	vector<CachedBitmap*>  m_CachedImages; // 고속 렌더링용 리소스

	POINT*                 m_pMousePos = nullptr;
	ITEM*                  m_pDragItem = nullptr;

	char                   m_TileData[WORLD_SIZE][WORLD_SIZE];
	char                   m_Obstacles[WORLD_SIZE][WORLD_SIZE];
	map<int, OBJECT*>      m_pObjects;


	int                    m_myID;
	wchar_t                m_myID_Str[MAX_WSTR_LEN];

	void TransformScreen(int world_x, int world_y, int& screen_x, int& screen_y);
};