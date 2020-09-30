#pragma once

constexpr int MAX_USER = 1000;
constexpr int MAX_STR_LEN = 50;

constexpr int NPC_ID_START = 1000;
constexpr int NUM_NPC = 200000;

#define WORLD_WIDTH		800
#define WORLD_HEIGHT	800

#define SERVER_PORT		3500

#define CS_UP		1
#define CS_DOWN		2
#define CS_LEFT		3
#define CS_RIGHT	4
#define CS_LOGIN_DUMMY           6

#define SC_LOGIN_OK			1
#define SC_PUT_PLAYER		2
#define SC_REMOVE_PLAYER	3
#define SC_POS				4
#define SC_CHAT				5

#pragma pack(push ,1)

struct sc_packet_pos {
	char size;
	char type;
	int id;
	int x, y;
};

struct sc_packet_remove_player {
	char size;
	char type;
	int id;
};

struct sc_packet_login_ok {
	char size;
	char type;
	int id;
};

struct sc_packet_put_player {
	char size;
	char type;
	int id;
	short x, y;
};

struct sc_packet_chat {
	char size;
	char type;
	int	id;
	wchar_t	message[MAX_STR_LEN];
};

struct cs_packet_login_dummy
{
	unsigned char size;
	unsigned char type;
};

struct cs_packet_up {
	char	size;
	char	type;
};

struct cs_packet_down {
	char	size;
	char	type;
};

struct cs_packet_left {
	char	size;
	char	type;
};

struct cs_packet_right {
	char	size;
	char	type;
};

#pragma pack (pop)