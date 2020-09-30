#pragma once
#include <chrono>
using namespace std;
using namespace chrono;
// 열거형 상수, 매직넘버, 매크로 등은 한 곳에서만 정의하고 관리한다.



/////////////////////////////////////// Object Type ///////////////////////////////////////
#define OBJECT_TYPE_NON                    0

#define OBJECT_TYPE_USER                   1

// NPC Type
#define OBJECT_TYPE_NPC_CHIEF              2
#define OBJECT_TYPE_NPC_BLACKSMITH         3
#define OBJECT_TYPE_NPC_POTION_MERCHANT    4

// Monster Type
#define OBJECT_TYPE_MONSTER_SCORPION       5
#define OBJECT_TYPE_MONSTER_SCORPION_KING  6
#define OBJECT_TYPE_MONSTER_THEIF          7
#define OBJECT_TYPE_MONSTER_THEIF_BOSS     8
#define OBJECT_TYPE_MONSTER_WOLF           9
#define OBJECT_TYPE_MONSTER_WEREWOLF       10
#define OBJECT_TYPE_MONSTER_GOBLIN_WARRIOR 11
#define OBJECT_TYPE_MONSTER_GOBLIN_KING    12

// Tile Type
#define OBJECT_TPYE_TILE_WATER             13
#define OBJECT_TPYE_TILE_GRASS             14
#define OBJECT_TPYE_TILE_SAND              15

// Obstacle(장애물) Type
#define OBJECT_TYPE_OBSTACLE_WALL          16
#define OBJECT_TYPE_OBSTACLE_ROCK          17
#define OBJECT_TYPE_OBSTACLE_TREE          18

// Item Type
#define OBJECT_TYPE_ITEM_HP_POTION         19
#define OBJECT_TYPE_ITEM_MP_POTION         20
#define OBJECT_TYPE_ITEM_COIN              21  
#define OBJECT_TYPE_ITEM_NORMAL_ARMOR      22
#define OBJECT_TYPE_ITEM_NORMAL_SWORD      23
#define OBJECT_TYPE_ITEM_NORMAL_RING       24
#define OBJECT_TYPE_ITEM_SPECIAL_ARMOR     25
#define OBJECT_TYPE_ITEM_SPECIAL_SWORD     26
#define OBJECT_TYPE_ITEM_SPECIAL_RING      27
///////////////////////////////////////////////////////////////////////////////////////////





///////////////////////////////////// Render Elements /////////////////////////////////////
// Graphic Resource Type
#define GDI_GRAPHIC_RESOURCE_TILES         0
#define GDI_GRAPHIC_RESOURCE_OBSTACLES     1
#define GDI_GRAPHIC_RESOURCE_CHARACTERS    2
#define GDI_GRAPHIC_RESOURCE_ITEMS         3
#define GDI_GRAPHIC_RESOURCE_EFFECT_ATTACK 4
#define GDI_GRAPHIC_RESOURCE_UI_FOG        5

// Cached Graphic Resource Index Num
#define TILE_WATER                0
#define TILE_GRASS                1
#define TILE_SAND                 2
#define OBSTACLE_WALL             3
#define OBSTACLE_ROCK             4
#define OBSTACLE_TREE             5
#define CHARACTER_WARRIOR         6
#define CHARACTER_CHIEF           7
#define CHARACTER_BLACKSMITH      8
#define CHARACTER_POTION_MERCHANT 9
#define CHARACTER_SCORPION        10
#define CHARACTER_SCORPION_KING   11
#define CHARACTER_THEIF           12
#define CHARACTER_THEIF_BOSS      13
#define CHARACTER_WOLF            14
#define CHARACTER_WEREWOLF        15
#define CHARACTER_GOBLIN_WARRIOR  16
#define CHARACTER_GOBLIN_KING     17
#define ITEM_HP_POTION            18
#define ITEM_MP_POTION            19
#define ITEM_COIN                 20
#define ITEM_NORMAL_ARMOR         21
#define ITEM_NORMAL_SWORD         22
#define ITEM_NORMAL_RING          23
#define ITEM_SPECIAL_ARMOR        24
#define ITEM_SPECIAL_SWORD        25
#define ITEM_SPECIAL_RING         26
#define EFFECT_ATTACK_1_1         27
#define EFFECT_ATTACK_1_2         28
#define EFFECT_ATTACK_1_3         29
#define EFFECT_ATTACK_1_4         30
#define EFFECT_ATTACK_1_5         31
#define EFFECT_ATTACK_1_6         32
#define EFFECT_ATTACK_1_7         33
#define EFFECT_ATTACK_1_8         34
#define EFFECT_ATTACK_2_1         35
#define EFFECT_ATTACK_2_2         36
#define EFFECT_ATTACK_2_3         37
#define EFFECT_ATTACK_2_4         38
#define EFFECT_ATTACK_2_5         39
#define EFFECT_ATTACK_2_6         40
#define EFFECT_ATTACK_2_7         41
#define EFFECT_ATTACK_2_8         42
#define EFFECT_ATTACK_3_1         43
#define EFFECT_ATTACK_3_2         44
#define EFFECT_ATTACK_3_3         45
#define EFFECT_ATTACK_3_4         46
#define EFFECT_ATTACK_3_5         47
#define EFFECT_ATTACK_3_6         48
#define EFFECT_ATTACK_3_7         49
#define EFFECT_ATTACK_3_8         50
#define UI_FOG                    51

// Graphic Resource Count
#define TILE_IMAGE_ROW              1
#define TILE_IMAGE_COLUMN           3
#define OBSTACLE_IMAGE_ROW          1
#define OBSTACLE_IMAGE_COLUMN       3
#define CHARACTER_IMAGE_ROW         3
#define CHARACTER_IMAGE_COLUMN      4
#define ITEM_IMAGE_ROW              3
#define ITEM_IMAGE_COLUMN           3
#define EFFECT_ATTACK_IMAGE_ROW     3
#define EFFECT_ATTACK_IMAGE_COLUMN  8

// Graphic Resource Unit
#define GDI_GRAPHIC_UNIT       56
#define GDI_EFFECT_ATTACK_UNIT 192

// Graphic Resource Rendering Unit
#define GDI_TILE_SIZE          42
#define GDI_OBSTACLE_SIZE      42
#define GDI_CHARACTER_SIZE     42
#define GDI_ITEM_SIZE          42
#define GDI_EFFECT_ATTACK_SIZE 84

#define GDI_PROGRESS_BAR_WIDTH  100
#define GDI_PROGRESS_BAR_HEIGHT 20

#define VIEW_SIZE 21 // number of tile
#define VIEW_CENTER (VIEW_SIZE / 2)
#define VIEWPORT_SIZE (VIEW_SIZE * GDI_TILE_SIZE)
#define VIEPORT_CENTER (VIEWPORT_SIZE / 2)
///////////////////////////////////////////////////////////////////////////////////////////





///////////////////////////////////// World Elements //////////////////////////////////////
#define WORLD_SIZE 300
#define WORLD_CENTER (WORLD_SIZE / 2)
#define WORLD_MIN 0
#define WORLD_MAX (WORLD_SIZE - 1)
#define WORLD_BOUNDARY_MIN (WORLD_MIN + VIEW_CENTER)
#define WORLD_BOUNDARY_MAX (WORLD_MAX - VIEW_CENTER)
#define SECTOR_SIZE 60
#define SECTOR_COUNT (WORLD_SIZE / SECTOR_SIZE) * (WORLD_SIZE / SECTOR_SIZE)
///////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////// Map State ////////////////////////////////////////
// Map State 고정형 (이동불가능한.)
#define UNMOVEABLE_OVER_TILES 0
// Map State 이동형 (해당위치로 이동가능한.)
#define MOVEABLE_OVER_TILES   1
///////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////// Game Elements //////////////////////////////////////
#define TITLE_SCENE 0
#define PLAY_SCENE  1

#define RESPAWN_COOLTIME 3000ms
#define HEAL_COOLTIME 1000ms
#define MOVE_COOLTIME 100ms
#define FAST_MOVE_COOLTIME 50ms
#define ATTACK_COOLTIME 200ms
#define GET_ITEM_COOLTIME 100ms
#define USE_ITEM_COOLTIME 300ms

#define HP_POTION_PRICE 5
#define MP_POTION_PRICE 10
#define NORMAL_SWORD_PRICE 15
#define NORMAL_ARMOR_PRICE 30
#define NORMAL_RING_PRICE  40

#define MAX_ITEM_SLOT 9
// What, How_many
// 무엇을, 얼마만큼
struct INVENTORY_ITEM_INFO
{
	unsigned char What;
	short         How_many;
	short         ID;
};

// (KILL, FIND, BUY, ATTACK, FOLLOW, CONVERSATION)
enum Do_Action
{
	NOTHING, KILL, FIND, BUY, GET, DROP, ATTACK, POWER_ATTACK, FOLLOW, CONVERSATION
};

// What, How, How_many
// 무엇을, 어떻게, 얼마만큼
struct Action_Condition
{
	short     What = -1;                 // 무엇을 (Object ID 또는 Object Type)
	Do_Action How = Do_Action::NOTHING;  // 어떻게
	short     How_many = -1;             // 얼마만큼
};


enum QUEST_TYPE
{
	NON,
	TEST_ABILITY, // 능력 테스트 (Kill 전갈 10 And Kill 전갈킹 1)
	SUBJUGATE_BANDIT, // 산적 토벌 (Kill 산적 30 And Kill 산적두목 2)
	ENCOUNTER_GOBLIN_KING // 고블린 킹과의 조우 (Kill 깊은 숲 몬스터 100 And Encounter 고블린 킹)
};

#define MAX_QUEST_CONDITION 2
// Type, Preceded_Quest, Complete_Condition (퀘스트 종류, 선행해야 할 퀘스트, 퀘스트 완료 조건)
struct QUEST
{
	QUEST_TYPE Type = QUEST_TYPE::NON; // 퀘스트 종류
	QUEST_TYPE Preceded_Quest = QUEST_TYPE::NON; // 선행해야 할 퀘스트
	Action_Condition Quest_Progress[MAX_QUEST_CONDITION]; // 퀘스트 진행 사항
};
///////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////// Network Option /////////////////////////////////////
#define PORT_NUM 3500
#define LoopBack_IP "127.0.0.1"
#define BUF_SIZE 512
#define MAX_CLIENTS   1000
#define MAX_NPC       3000
#define MAX_DROP_ITEM 1000
#define MAX_OBJECT    (MAX_CLIENTS + MAX_NPC + MAX_DROP_ITEM)
#define NPC_START_INDEX MAX_CLIENTS
#define DROP_ITEM_START_INDEX (MAX_CLIENTS + MAX_NPC)
#define MAX_WSTR_LEN 50
#define NAME_LEN MAX_WSTR_LEN
///////////////////////////////////////////////////////////////////////////////////////////





//////////////////// 통신 데이터 타입 + 이벤트 타입(혹은 State Machine) ///////////////////
// Client -> Server
#define CS_MOVE_UP               1
#define CS_MOVE_DOWN             2
#define CS_MOVE_LEFT             3
#define CS_MOVE_RIGHT            4
#define CS_LOGIN                 5
#define CS_LOGIN_DUMMY           6
#define CS_ATTACK                7
#define CS_POWER_ATTACK          8
#define CS_ITEM_GET              9
#define CS_ITEM_DROP             10
#define CS_ITEM_USE              11
#define CS_ITEM_BUY              12
#define CS_ITEM_SLOT_INFO_CHANGE 13
#define CS_CHAT                  14
#define CS_LOGOUT                15

// Server -> Client (or Event)
#define SC_LOGIN_OK        1
#define SC_PUT_OBJECT      2
#define SC_REMOVE_OBJECT   3
#define SC_OBJECT_POS      4
#define SC_CHAT            5
#define SC_USER_INFO       6
#define SC_LOGIN_FAIL      7
#define SC_ATTACK          8
#define SC_POWER_ATTACK    9
#define SC_ITEM_GET_OK     10
#define SC_ITEM_DROP_OK    11
#define SC_ITEM_USE_OK     12
#define SC_ITEM_BUY_OK     13
#define SC_LEVEL_UP        14
#define SC_EXP_CHANGE      15
#define SC_HP_CHANGE       16
#define SC_MP_CHANGE       17
#define SC_MONEY_CHANGE    18
#define SC_QUEST_CHANGE    19

// IOCP Event
#define IS_RECVED          20
#define IS_SENDED          21
#define MONSTER_MOVE       22
#define USER_MOVED         23
#define USER_HEAL          24
#define MONSTER_ATTACK     25
#define USER_RESPAWN       26
#define MONSTER_RESPAWN    27
///////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////// 통신 패킷 타입 /////////////////////////////////////
#pragma pack(push, 1)
// Client -> Server
struct cs_packet_login_dummy
{
	unsigned char size;
	unsigned char type;
};
struct cs_pacekt_login
{
	unsigned char size;
	unsigned char type;

	// Game Data
	wchar_t       id_str[MAX_WSTR_LEN];
	wchar_t       password_str[MAX_WSTR_LEN];
};
struct cs_packet_move
{
	unsigned char size;
	unsigned char type;
};
struct cs_packet_attack
{
	unsigned char size;
	unsigned char type;
};
struct cs_packet_power_attack
{
	unsigned char size;
	unsigned char type;
};
struct cs_packet_item_get
{
	unsigned char size;
	unsigned char type;
};
struct cs_packet_item_drop
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned char slot_num;
	short         how_many;
};
struct cs_packet_item_use
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned char slot_num;
	short         how_many;
};
struct cs_packet_chat
{
	unsigned char size;
	unsigned char type;

	// Game Data
	wchar_t       message[MAX_WSTR_LEN];
};
struct cs_packet_logout
{
	unsigned char size;
	unsigned char type;
};
struct cs_packet_change_item_slot
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned char from_slot;
	unsigned char new_slot;
};
struct cs_packet_item_buy
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned char item_type;
	int           how_many;
};

struct cs_packet_doAction
{
	unsigned char size;
	unsigned char type;

	// Game Data
	Action_Condition action_condition; // What, How, How_many (무엇을, 어떻게, 얼마만큼)
};


// Server -> Client
struct sc_packet_login_ok
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           id;
	wchar_t       user_id_str[MAX_WSTR_LEN];
};
struct sc_packet_user_info
{
	unsigned char size;
	unsigned char type;

	// Game Data
	short               obj_id;
	unsigned char       obj_type;
	short               obj_x, obj_y;
	unsigned short      user_hp;
	unsigned short      user_mp;
	unsigned char       user_level;
	unsigned short      user_exp;
	INVENTORY_ITEM_INFO user_inventory[MAX_ITEM_SLOT];
	QUEST               user_quest;
};
struct sc_packet_login_fail
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           id;
};
struct sc_packet_put_object
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           obj_id;
	unsigned char obj_type;
	int           obj_x, obj_y;
	int           how_many;
};
struct sc_packet_remove_object
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           obj_id;
};
struct sc_packet_pos
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           obj_id;
	int           obj_x, obj_y;
};
struct sc_packet_attack
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           from;
	int           target;
};
struct sc_packet_power_attack
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           from;
	int           target;
};
struct sc_packet_get_item_ok
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           obj_id;
	unsigned char obj_type;
	unsigned char slot_num;
	int           how_many;
};
struct sc_packet_drop_item_ok
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           obj_id;
	unsigned char slot_num;
	int           how_many;
};
struct sc_packet_use_item_ok
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned char slot_num;
	int           how_many;
};
struct sc_packet_buy_item_ok
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           obj_id;
	unsigned char obj_type;
	unsigned char slot_num;
	int           how_many;
};
struct sc_packet_levelUp
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned char user_level; // 레벨업을 했을 경우에만. (user exp >= max_exp)
	unsigned int  user_exp; // 죽거나, 퀘스트를 완료하거나, 몬스터를 처치했을 경우에만.
};
struct sc_packet_exp_change
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned int  user_exp; // 죽거나, 퀘스트를 완료하거나, 몬스터를 처치했을 경우에만.
};
struct sc_packet_hp_change
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           obj_id; // 유저의 HP뿐만 아니라 다른 몬스터나 다른 유저의 HP를 보여줘야 할 필요가 있다.
	unsigned char obj_type;
	unsigned int  obj_hp;
};
struct sc_packet_mp_change
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned int  user_mp; // 죽거나, 퀘스트를 완료하거나, 몬스터를 처치했을 경우에만.
};
struct sc_packet_money_change
{
	unsigned char size;
	unsigned char type;

	// Game Data
	unsigned char slot_num;
	unsigned int  user_money; // 유저가 코인을 얻거나, 아이템을 구매했을 때만.
};
struct sc_packet_quest_change
{
	unsigned char size;
	unsigned char type;

	// Game Data
	QUEST         quest_info;
};
struct sc_packet_chat
{
	unsigned char size;
	unsigned char type;

	// Game Data
	int           user_id;
	wchar_t       message[MAX_WSTR_LEN];
};
#pragma pack(pop)
///////////////////////////////////////////////////////////////////////////////////////////