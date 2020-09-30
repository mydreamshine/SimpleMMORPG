#pragma once
#include "stdafx.h"

////////////////////////////////////// Network Option /////////////////////////////////////
struct WSAOVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;
	char          event_state;
	int           event_target;
};
struct WSAOVERLAPPED_EX_IO : WSAOVERLAPPED_EX
{
	WSABUF dataBuffer;
	char   messageBuffer[BUF_SIZE];
};
struct SOCKETINFO
{
	SOCKET              Socket;
	WSAOVERLAPPED_EX_IO Overlapped_Ex_IO;

	char PacketBuffer[BUF_SIZE]; // 재조립용 (패킷으로부터.)
	int  prevSize;
};
///////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////// Game Event //////////////////////////////////////
// Event Type
enum EVENT_TYPE
{
	EVENT_DIRECT,          // 즉시 실행
	EVENT_AFTER_SOME_TIME  // 일정 시간 후 실행
};
// Type, Do_Object, Ref_Object, Command
// 어떤 유형으로, 누가, 무엇을(무엇으로부터), 어떻게 하는가
struct EVENT
{
	EVENT_TYPE    Type; // 즉시 수행해야 할 이벤트(DIRECT)인가, 특정 시간 후 수행해야 할 이벤트(TIME)인가
	int           Do_Object; // 이벤트를 수행할 오브젝트    (Who, 누가)
	int           Ref_Object; // 이벤트에 참조되는 오브젝트 (What, 무엇을) 또는 (From, 무엇으로부터)
	char          Command; // 수행해야 할 이벤트는 무엇인가 (How Do, 어떻게 하는 가)

	EVENT() = default;
	EVENT(EVENT_TYPE type, int do_object, int ref_object, char command) :
		Type(type), Do_Object(do_object), Ref_Object(ref_object), Command(command) {}
};
// Type, Do_Object, Ref_Object, Command, ExecutionTime
// 어떤 유형으로, 누가, 무엇을(무엇으로부터), 어떻게, 언제 하는가
struct TIME_EVENT : EVENT
{
	high_resolution_clock::time_point ExecutionTime; // 이벤트를 수행할 시간

	TIME_EVENT() = default;
	TIME_EVENT(
		EVENT_TYPE type, int do_object, int ref_object, char command,
		high_resolution_clock::time_point ex_time) :
		EVENT(type, do_object, ref_object, command), ExecutionTime(ex_time) {}
};
struct OBJECT_PUT_EVENT : EVENT
{
	int How_many;
	OBJECT_PUT_EVENT() = default;
	OBJECT_PUT_EVENT(
		EVENT_TYPE type, int do_object, int ref_object, char command, int how_many) :
		EVENT(type, do_object, ref_object, command), How_many(how_many) {}
};
struct CHAT_EVENT : EVENT
{
	wchar_t chat_message[MAX_WSTR_LEN];

	CHAT_EVENT() = default;
	CHAT_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, const wchar_t* wmessage) :
		EVENT(type, do_object, ref_object, command)
	{
		ZeroMemory(chat_message, sizeof(chat_message));
		wcscpy_s(chat_message, wmessage);
	}
};
struct USER_LOGIN_OK_EVENT : EVENT
{
	wchar_t user_id_str[MAX_WSTR_LEN];
	USER_LOGIN_OK_EVENT() = default;
	USER_LOGIN_OK_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, wchar_t* id_str) :
		EVENT(type, do_object, ref_object, command)
	{
		ZeroMemory(user_id_str, sizeof(user_id_str));
		wcscpy_s(user_id_str, id_str);
	}
};
struct USER_INFO_EVENT : EVENT
{
	USER_INFO_EVENT() = default;
	USER_INFO_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command) :
		EVENT(type, do_object, ref_object, command) {}
};
struct ITEM_GET_OK_EVENT : EVENT
{
	unsigned char Item_type;
	unsigned char slotID;
	int           How_many;
	ITEM_GET_OK_EVENT() = default;
	ITEM_GET_OK_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, unsigned char item_type, unsigned char slot_id, int how_many) :
		EVENT(type, do_object, ref_object, command), Item_type(item_type), slotID(slot_id), How_many(how_many) {}
};
struct ITEM_DROP_OK_EVENT : EVENT
{
	int slotID;
	int How_many;
	ITEM_DROP_OK_EVENT() = default;
	ITEM_DROP_OK_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, int slot_id, int how_many) :
		EVENT(type, do_object, ref_object, command), slotID(slot_id), How_many(how_many) {}
};
struct ITEM_USE_OK_EVENT : EVENT
{
	int slotID;
	int How_many;
	ITEM_USE_OK_EVENT() = default;
	ITEM_USE_OK_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, int slot_id, int how_many) :
		EVENT(type, do_object, ref_object, command), slotID(slot_id), How_many(how_many) {}
};
struct ITEM_BUY_OK_EVENT : EVENT
{
	unsigned char Item_type;
	unsigned char slotID;
	int           How_many;
	ITEM_BUY_OK_EVENT() = default;
	ITEM_BUY_OK_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, unsigned char item_type, unsigned char slot_id, int how_many) :
		EVENT(type, do_object, ref_object, command), Item_type(item_type), slotID(slot_id), How_many(how_many) {}
};
struct LEVEL_UP_EVENT : EVENT
{
	unsigned char Level;
	unsigned int  Experience;
	LEVEL_UP_EVENT() = default;
	LEVEL_UP_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, unsigned char level, unsigned int exp) :
		EVENT(type, do_object, ref_object, command), Level(level), Experience(exp) {}
};
struct EXP_CHANGE_EVENT : EVENT
{
	unsigned int  Experience;
	EXP_CHANGE_EVENT() = default;
	EXP_CHANGE_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, unsigned int exp) :
		EVENT(type, do_object, ref_object, command), Experience(exp) {}
};
struct HP_CHANGE_EVENT : EVENT
{
	unsigned char Obj_type;
	unsigned int  HP;
	HP_CHANGE_EVENT() = default;
	HP_CHANGE_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, unsigned char obj_type, unsigned int hp) :
		EVENT(type, do_object, ref_object, command), Obj_type(obj_type), HP(hp) {}
};
struct MP_CHANGE_EVENT : EVENT
{
	unsigned int  MP;
	MP_CHANGE_EVENT() = default;
	MP_CHANGE_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, unsigned int mp) :
		EVENT(type, do_object, ref_object, command), MP(mp) {}
};
struct MONEY_CHANGE_EVENT : EVENT
{
	int  Money;
	char MoneySlot;
	MONEY_CHANGE_EVENT() = default;
	MONEY_CHANGE_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, int money, char money_slot) :
		EVENT(type, do_object, ref_object, command), Money(money), MoneySlot(money_slot) {}
};
struct QUEST_CHANGE_EVENT : EVENT
{
	QUEST Quest;
	QUEST_CHANGE_EVENT() = default;
	QUEST_CHANGE_EVENT(EVENT_TYPE type, int do_object, int ref_object, char command, QUEST* quest) :
		EVENT(type, do_object, ref_object, command) { memcpy_s(&Quest, sizeof(QUEST), quest, sizeof(QUEST));	}
};
///////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////// Define Object //////////////////////////////////////
class OBJECT
{
public:
	OBJECT() = default;
	OBJECT(unsigned char newType, int newX, int newY) : Type(newType), x(newX), y(newY) { isAlive = true; }

	char  Type; // Non, User, NPC, Monster, Tile, Obstacle, item, ..., etc.
	int   x, y;

	bool isAlive = true; // freelist로 활용하기 위함
	bool isActive = false; // 오브젝트 범용적 활성화
	virtual ~OBJECT() {}
};

#define MAX_ITEM_DIGIT 10 // Item 개수 최대 자리수
class ITEM :public OBJECT
{
public:
	ITEM() = default;
	ITEM(unsigned char type, int X, int Y) : OBJECT(type, X, Y) {}
	ITEM(unsigned char type, int X, int Y, int how_many, int id) : OBJECT(type, X, Y) { How_many = how_many; ID = id; }
	virtual ~ITEM() {}

	int How_many;
	int ID;
};

void TransformSlot(int SlotNum, int & x, int & y);
bool PointInSlot(int x, int y, RECT rect);
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

class NPC : public OBJECT
{
public:
	NPC() = default;
	NPC(unsigned char type, int X, int Y) : OBJECT(type, X, Y) {}
	virtual ~NPC() {}
};

#define MONSTER_VIEW_RADIUS 3
#define AGGRO_TYPE_PEACE  0
#define AGGRO_TYPE_BATTLE 1
#define MONSTER_MOVE_UP    0
#define MONSTER_MOVE_DOWN  1
#define MONSTER_MOVE_LEFT  2
#define MONSTER_MOVE_RIGHT 3
#define MOSTER_RESPAWN_COOL_TIME 3000ms
#define MONSTER_MOVE_COOL_TIME 1000ms
#define MONSTER_ATTACK_COOL_TIME 1000ms
class MONSTER : public NPC
{
public:
	MONSTER() = default;
	MONSTER(unsigned char type, int X, int Y) : NPC(type, X, Y) {}

	int respawn_x = 0;
	int respawn_y = 0;

	char Aggro_Type = AGGRO_TYPE_PEACE; // PEACE, BATTLE
	int  Attack_Target_x = -1;
	int  Attack_Target_y = -1;

	unsigned int HP      = 0;
	int          Damage  = 0;
	int          Defense = 0;

	char MoveDirection = 0;

	high_resolution_clock::time_point last_move_time;
	high_resolution_clock::time_point last_attack_time;
	
	mutex GeneratedEventLock;

	bool isMoveCoolTimeDone()
	{
		if (high_resolution_clock::now() >= last_move_time + MONSTER_MOVE_COOL_TIME)
		{
			last_move_time = last_move_time + MONSTER_MOVE_COOL_TIME;
			return true;
		}
		return false;
	}
	bool isAttackCoolTimeDone()
	{
		if (high_resolution_clock::now() >= last_attack_time + MONSTER_ATTACK_COOL_TIME)
		{
			last_attack_time = last_attack_time + MONSTER_ATTACK_COOL_TIME;
			return true;
		}
		return false;
	}

	void Do_pathFind(bool moveAble, char* TileData[WORLD_SIZE], char* ObstacleData[WORLD_SIZE])
	{
		if (Attack_Target_x == -1 || Attack_Target_y == -1) return;
		if (moveAble == false) return;
	}

};

#define USER_VIEW_RADIUS 7
#define USER_VIEW_RADIUS_EX 12
class USER : public OBJECT
{
public:
	USER() = default;
	USER(unsigned char type, int X, int Y) : OBJECT(type, X, Y) { ZeroMemory(ID_str, MAX_WSTR_LEN); }
	virtual ~USER() { viewlist.clear(); }

	SOCKETINFO Socket_info;
	bool       isLogin  = false;
	bool       isConnect = false;
	char       ConnectType = CS_LOGIN;

	unordered_set<int> viewlist;
	mutex              viewlist_otherAccess;

	// Game Data
	wchar_t       ID_str[MAX_WSTR_LEN];
	unsigned char Level;
	unsigned int  Experience;
	unsigned int  HP;
	unsigned int  MP;
	int           Damage;
	int           Defense;
	QUEST         Quest;
	INVENTORY     Inventory;

	int GetMaxHP() { return (100 + (Level - 1) * 30); }
	int GetMaxMP() { return (100 + (Level - 1) * 20); }
	int GetMaxEXP() { return (100 + (Level - 1) * 100); }
};
///////////////////////////////////////////////////////////////////////////////////////////



class C_SIMPLE_MMORPG
{
public:
	~C_SIMPLE_MMORPG();

	void init();
	void LoadMap(const WCHAR* filename);
	void ReleaseObjects();

	int  CreateNPC(unsigned char type, int x, int y); // return ID
	int  CreateMonster(unsigned char type, int x, int y);

	int  ConnectUser(); // Only Accept, Not Playing, return ID
	void LoginUser(int user_ID, wchar_t* user_name, char ConnectionType);
	bool GameStart(int user_ID, queue<EVENT*>& GeneratedEvents, bool by_user_info = false, USER* user_info = NULL);

	void ExitUser(int user_ID, queue<EVENT*>& GeneratedEvents);

	void SetUserViewList(int user_ID, queue<EVENT*>& GeneratedEvents);
	void SetUserViewListFromMonster(int user_ID, int monster_ID, queue<EVENT*>& GeneratedEvents);

	void ProcessInputOfUser(int ID, char input, queue<EVENT*>& GeneratedEvents);
	void ProcessItemEventOfUser(int ID, char* buf, queue<EVENT*>& GeneratedEvents);
	void ProcessMonsterEvent(EVENT& Event, queue<EVENT*>& GeneratedEvents);
	void HealToUser(int ID, queue<EVENT*>& GeneratedEvents);
	void Respawn(EVENT& Event, queue<EVENT*>& GeneratedEvents);

	void CheckAndSetMonster_Event(EVENT& Event, queue<EVENT*>& GeneratedEvents);

	OBJECT* GetObjectData(int ID) { return m_pObjects[ID]; }
	USER*   GetUserData(int ID) { return reinterpret_cast<USER*>(m_pObjects[ID]); }

	int  Find_ItemInUserPos(int userID);
	char Find_PutAbleSlot(int itemID, int userID);
	int  CreatableItemID();

	void ObjecttypeToString(unsigned char obj_type, wstring& wstr);

private:
	char                        m_TileData[WORLD_SIZE][WORLD_SIZE];
	char                        m_Obstacles[WORLD_SIZE][WORLD_SIZE];
	OBJECT*                     m_pObjects[MAX_OBJECT]; // 단순 오브젝트 배열
	unordered_map<int, OBJECT*> m_Sector[SECTOR_COUNT]; // 오브젝트들을 관리 (오브젝트 배열의 원소를 참조하는 형식)
	mutex                       Sector_other_Access;

	int                         m_UseableNPC_ID = NPC_START_INDEX;

	bool GetUserID(int& ID);
	bool GetRandomPos(int& x, int& y);
	int  GetSectorIndex_By(int x, int y);
};