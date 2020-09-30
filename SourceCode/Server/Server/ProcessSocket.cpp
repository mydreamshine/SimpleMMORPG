#include "ProcessSocket.h"
#include "CODBC.h"

HANDLE g_worker_iocp;
SOCKET listen_sock;

CODBC g_ODBC;
C_SIMPLE_MMORPG SimpleMMORPG;

class Aescending_Time
{
public:
	bool operator() (const TIME_EVENT lhs, const TIME_EVENT rhs) const
	{
		return (lhs.ExecutionTime > rhs.ExecutionTime);
	}
};
priority_queue<TIME_EVENT, vector<TIME_EVENT>, Aescending_Time> Timer_Queue;
mutex Timer_other_Access;
mutex Cout_other_Access;

void send_packet(int key, unsigned char* packet)
{
	SOCKETINFO* Socket_Info = &SimpleMMORPG.GetUserData(key)->Socket_info;
	SOCKET client_s = Socket_Info->Socket;

	WSAOVERLAPPED_EX_IO* over = new WSAOVERLAPPED_EX_IO();

	over->dataBuffer.len = (ULONG)packet[0];
	over->dataBuffer.buf = over->messageBuffer;
	memcpy_s(over->messageBuffer, BUF_SIZE, packet, (ULONG)packet[0]);
	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
	over->event_state = IS_SENDED;
	if (WSASend(client_s, &over->dataBuffer, 1, NULL, 0, &(over->overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			Cout_other_Access.lock();
			err_display((char*)"SEND_ERROR");
			Cout_other_Access.unlock();
		}
	}
}

void recv_packet(int key)
{
	DWORD flags = 0;

	SOCKETINFO* Socket_Info = &SimpleMMORPG.GetUserData(key)->Socket_info;

	SOCKET client_s = Socket_Info->Socket;

	WSAOVERLAPPED_EX_IO* over = &Socket_Info->Overlapped_Ex_IO;

	over->dataBuffer.len = BUF_SIZE;
	over->dataBuffer.buf = over->messageBuffer;
	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
	if (WSARecv(client_s, &over->dataBuffer, 1, NULL, &flags, &(over->overlapped), NULL) == SOCKET_ERROR)
	{
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
		{
			Cout_other_Access.lock();
			err_display((char*)"RECV_ERROR");
			Cout_other_Access.unlock();
		}
	}
}

void send_login_ok_packet(int to, wchar_t* id_str)
{
	sc_packet_login_ok packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;
	packet.id = to;
	if(id_str != NULL) wcsncpy_s(packet.user_id_str, id_str, MAX_WSTR_LEN);
	send_packet(to, reinterpret_cast<unsigned char*>(&packet));
}

void send_put_object_packet(int to, int obj, int how_many)
{
	sc_packet_put_object packet;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_OBJECT;
	OBJECT* Obj = SimpleMMORPG.GetObjectData(obj);
	packet.obj_id = obj;
	packet.obj_type = Obj->Type;
	packet.obj_x = Obj->x;
	packet.obj_y = Obj->y;
	packet.how_many = how_many;
	send_packet(to, reinterpret_cast<unsigned char*>(&packet));
}

void send_object_pos_packet(int to, int obj)
{
	sc_packet_pos packet;
	packet.size = sizeof(packet);
	packet.type = SC_OBJECT_POS;
	OBJECT* Obj = SimpleMMORPG.GetObjectData(obj);
	packet.obj_id = obj;
	packet.obj_x = Obj->x;
	packet.obj_y = Obj->y;
	send_packet(to, reinterpret_cast<unsigned char*>(&packet));
}

void send_remove_object_packet(int to, int obj)
{
	sc_packet_remove_object packet;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_OBJECT;
	packet.obj_id = obj;
	send_packet(to, reinterpret_cast<unsigned char*>(&packet));
}

void send_chat_packet(int to, int from, wchar_t *message)
{
	sc_packet_chat packet;
	packet.size = sizeof(packet);
	packet.type = SC_CHAT;
	packet.user_id = from;
	wcsncpy_s(packet.message, message, MAX_WSTR_LEN);
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_user_info_packet(int to)
{
	sc_packet_user_info packet;
	int packetsize = sizeof(packet);
	packet.size = (unsigned char)sizeof(packet);
	packet.type = SC_USER_INFO;
	USER* user = SimpleMMORPG.GetUserData(to);
	packet.obj_id = to;
	packet.obj_type = user->Type;
	packet.obj_x = user->x;
	packet.obj_y = user->y;
	packet.user_level = user->Level;
	packet.user_exp = user->Experience;
	packet.user_hp = user->HP;
	packet.user_mp = user->MP;
	memcpy_s(&packet.user_quest, sizeof(QUEST), &user->Quest, sizeof(QUEST));
	for (int i = 0; i < MAX_ITEM_SLOT; ++i)
	{
		ITEM* Item = user->Inventory.Slot[i].Item;
		if (Item == nullptr)
		{
			packet.user_inventory[i].What = OBJECT_TYPE_NON;
			packet.user_inventory[i].How_many = -1;
			packet.user_inventory[i].ID = -1;
			continue;
		}

		packet.user_inventory[i].What = Item->Type;
		packet.user_inventory[i].How_many = Item->How_many;
		packet.user_inventory[i].ID = Item->ID;
	}

	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_login_fail_packet(int to)
{
	sc_packet_login_fail packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_FAIL;
	packet.id = to;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_attack_packet(int to, int target)
{
	sc_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = SC_ATTACK;
	packet.from = to;
	packet.target = target;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_power_attack_packet(int to, int target)
{
	sc_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = SC_POWER_ATTACK;
	packet.from = to;
	packet.target = target;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_item_get_ok(int to, int itemID, unsigned char Item_type, int SlotID, int how_many)
{
	sc_packet_get_item_ok packet;
	packet.size = sizeof(packet);
	packet.type = SC_ITEM_GET_OK;
	packet.obj_id = itemID;
	packet.obj_type = Item_type;
	packet.slot_num = (unsigned char)SlotID;
	packet.how_many = how_many;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_item_drop_ok(int to, int itemID, int SlotID, int how_many)
{
	sc_packet_drop_item_ok packet;
	packet.size = sizeof(packet);
	packet.type = SC_ITEM_DROP_OK;
	packet.obj_id = itemID;
	packet.slot_num = (unsigned char)SlotID;
	packet.how_many = how_many;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_item_use_ok(int to, int itemID, int SlotID, int how_many)
{
	sc_packet_use_item_ok packet;
	packet.size = sizeof(packet);
	packet.type = SC_ITEM_USE_OK;
	packet.slot_num = (unsigned char)SlotID;
	packet.how_many = how_many;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_item_buy_ok(int to, int itemID, unsigned char Item_type, int SlotID, int how_many)
{
	sc_packet_buy_item_ok packet;
	packet.size = sizeof(packet);
	packet.type = SC_ITEM_BUY_OK;
	packet.obj_id = itemID;
	packet.obj_type = Item_type;
	packet.slot_num = (unsigned char)SlotID;
	packet.how_many = how_many;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_level_up(int to, unsigned char level, unsigned int exp)
{
	sc_packet_levelUp packet;
	packet.size = sizeof(packet);
	packet.type = SC_LEVEL_UP;
	packet.user_level = level;
	packet.user_exp = exp;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_exp_change(int to, unsigned int exp)
{
	sc_packet_exp_change packet;
	packet.size = sizeof(packet);
	packet.type = SC_EXP_CHANGE;
	packet.user_exp = exp;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_hp_change(int to, int obj, unsigned char obj_type, unsigned int hp)
{
	sc_packet_hp_change packet;
	packet.size = sizeof(packet);
	packet.type = SC_HP_CHANGE;
	packet.obj_id = obj;
	packet.obj_hp = hp;
	packet.obj_type = obj_type;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_mp_change(int to, unsigned int mp)
{
	sc_packet_mp_change packet;
	packet.size = sizeof(packet);
	packet.type = SC_MP_CHANGE;
	packet.user_mp = mp;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_money_change(int to, int money, char money_slot)
{
	sc_packet_money_change packet;
	packet.size = sizeof(packet);
	packet.type = SC_MONEY_CHANGE;
	packet.slot_num = (unsigned char)money_slot;
	packet.user_money = money;
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

void send_quest_change(int to, QUEST* quest)
{
	sc_packet_quest_change packet;
	packet.size = sizeof(packet);
	packet.type = SC_QUEST_CHANGE;
	memcpy_s(&packet.quest_info, sizeof(QUEST), quest, sizeof(QUEST));
	send_packet(to, reinterpret_cast<unsigned char *>(&packet));
}

// Ÿ�� �̺�Ʈ �ο�
void set_time_event(TIME_EVENT* TimeEvent)
{
	Timer_other_Access.lock();
	Timer_Queue.push(*TimeEvent);
	Timer_other_Access.unlock();
}


void process_event_by_send_packet(EVENT& Event)
{
	switch (Event.Command)
	{
	case SC_PUT_OBJECT:
	{
		OBJECT_PUT_EVENT* event_object_put = reinterpret_cast<OBJECT_PUT_EVENT*>(&Event);
		send_put_object_packet(event_object_put->Do_Object, event_object_put->Ref_Object, event_object_put->How_many);
		break;
	}
	case SC_OBJECT_POS:
		send_object_pos_packet(Event.Do_Object, Event.Ref_Object);
		break;
	case SC_REMOVE_OBJECT:
		send_remove_object_packet(Event.Do_Object, Event.Ref_Object);
		break;
	case SC_CHAT:
	{
		CHAT_EVENT* event_chat = reinterpret_cast<CHAT_EVENT*>(&Event);
		send_chat_packet(event_chat->Ref_Object, event_chat->Do_Object, event_chat->chat_message);
		break;
	}
	case SC_LOGIN_OK:
	{
		USER_LOGIN_OK_EVENT* event_user_login_ok = reinterpret_cast<USER_LOGIN_OK_EVENT*>(&Event);
		send_login_ok_packet(event_user_login_ok->Do_Object, event_user_login_ok->user_id_str);
		break;
	}
	case SC_USER_INFO:
	{
		send_user_info_packet(Event.Do_Object);
		break;
	}
	case SC_LOGIN_FAIL:
		send_login_fail_packet(Event.Do_Object);
		break;
	case SC_ATTACK:
		send_attack_packet(Event.Do_Object, Event.Ref_Object);
		break;
	case SC_POWER_ATTACK:
		send_power_attack_packet(Event.Do_Object, Event.Ref_Object);
		break;
	case SC_ITEM_GET_OK:
	{
		ITEM_GET_OK_EVENT* event_item_get_ok = reinterpret_cast<ITEM_GET_OK_EVENT*>(&Event);
		send_item_get_ok(event_item_get_ok->Do_Object, event_item_get_ok->Ref_Object, event_item_get_ok->Item_type, event_item_get_ok->slotID, event_item_get_ok->How_many);
		break;
	}
	case SC_ITEM_DROP_OK:
	{
		ITEM_DROP_OK_EVENT* event_item_drop_ok = reinterpret_cast<ITEM_DROP_OK_EVENT*>(&Event);
		send_item_drop_ok(event_item_drop_ok->Do_Object, event_item_drop_ok->Ref_Object, event_item_drop_ok->slotID, event_item_drop_ok->How_many);
		break;
	}
	case SC_ITEM_USE_OK:
	{
		ITEM_USE_OK_EVENT* event_item_use_ok = reinterpret_cast<ITEM_USE_OK_EVENT*>(&Event);
		send_item_use_ok(event_item_use_ok->Do_Object, event_item_use_ok->Ref_Object, event_item_use_ok->slotID, event_item_use_ok->How_many);
		break;
	}
	case SC_ITEM_BUY_OK:
	{
		ITEM_BUY_OK_EVENT* event_item_buy_ok = reinterpret_cast<ITEM_BUY_OK_EVENT*>(&Event);
		send_item_buy_ok(event_item_buy_ok->Do_Object, event_item_buy_ok->Ref_Object, event_item_buy_ok->Item_type, event_item_buy_ok->slotID, event_item_buy_ok->How_many);
		break;
	}
	case SC_LEVEL_UP:
	{
		LEVEL_UP_EVENT* event_level_up = reinterpret_cast<LEVEL_UP_EVENT*>(&Event);
		send_level_up(event_level_up->Do_Object, event_level_up->Level, event_level_up->Experience);
		break;
	}
	case SC_EXP_CHANGE:
	{
		EXP_CHANGE_EVENT* event_exp_change = reinterpret_cast<EXP_CHANGE_EVENT*>(&Event);
		send_exp_change(event_exp_change->Do_Object, event_exp_change->Experience);
		break;
	}
	case SC_HP_CHANGE:
	{
		HP_CHANGE_EVENT* event_hp_change = reinterpret_cast<HP_CHANGE_EVENT*>(&Event);
		send_hp_change(event_hp_change->Do_Object, event_hp_change->Ref_Object, event_hp_change->Obj_type, event_hp_change->HP);
		break;
	}
	case SC_MP_CHANGE:
	{
		MP_CHANGE_EVENT* event_mp_change = reinterpret_cast<MP_CHANGE_EVENT*>(&Event);
		send_mp_change(event_mp_change->Do_Object, event_mp_change->MP);
		break;
	}
	case SC_MONEY_CHANGE:
	{
		MONEY_CHANGE_EVENT* event_money_change = reinterpret_cast<MONEY_CHANGE_EVENT*>(&Event);
		send_money_change(event_money_change->Do_Object, event_money_change->Money, event_money_change->MoneySlot);
		break;
	}
	case SC_QUEST_CHANGE:
	{
		QUEST_CHANGE_EVENT* event_quest_change = reinterpret_cast<QUEST_CHANGE_EVENT*>(&Event);
		send_quest_change(event_quest_change->Do_Object, &event_quest_change->Quest);
		break;
	}
	}
}

void process_event(EVENT& Event)
{
	switch (Event.Command)
	{
	case MONSTER_MOVE:
	case MONSTER_ATTACK:
	case USER_MOVED:
	case USER_HEAL:
	case USER_RESPAWN:
	case MONSTER_RESPAWN:
	{
		queue<EVENT*> GeneratedEvents;

		if (Event.Command == MONSTER_MOVE || Event.Command == MONSTER_ATTACK)
			SimpleMMORPG.ProcessMonsterEvent(Event, GeneratedEvents);
		else if (Event.Command == USER_MOVED)
			SimpleMMORPG.CheckAndSetMonster_Event(Event, GeneratedEvents);
		else if (Event.Command == USER_HEAL)
			SimpleMMORPG.HealToUser(Event.Do_Object, GeneratedEvents);
		else if (Event.Command == USER_RESPAWN || Event.Command == MONSTER_RESPAWN)
			SimpleMMORPG.Respawn(Event, GeneratedEvents);		

		EVENT* GeneratedEvent = nullptr;
		while (GeneratedEvents.size() != 0)
		{
			GeneratedEvent = GeneratedEvents.front();
			GeneratedEvents.pop();
			if (GeneratedEvent->Type == EVENT_TYPE::EVENT_AFTER_SOME_TIME)
				set_time_event(reinterpret_cast<TIME_EVENT*>(GeneratedEvent));
			else
				process_event_by_send_packet(*GeneratedEvent);
			delete GeneratedEvent;
		}
		break;
	}
	default:
		process_event_by_send_packet(Event);
		break;
	}
}

void process_events(queue<EVENT*>& Events)
{
	EVENT* Event = nullptr;
	while (Events.size() != 0)
	{
		Event = Events.front();
		Events.pop();
		process_event(*Event);
		delete Event;
	}
}

// �ش� Event�� ó���ؾ� ���� GetQueuedCompletionStatus(g_iocp)�� ȣ���ϴ� Worker�����忡�� �˸�
void set_event_to_worker(TIME_EVENT* Event)
{
	// PostQueuedCompletionStatus() : IOCP ��ü�� ������ �ϷḦ �뺸
	// ���ڷ� ���޵� ���� �̿��� ������ �Ϸ� ��Ŷ�� �ϳ� ����� IOCP �Ϸ� ť�� �����ִ� �Լ�.
	// ��, GetQueuedCompletionStatus()�� ����� �Լ�
	// ���� �ǵ���� �̹� �Ϸ�� ����(Eventó��)�� GetQueuedCompletionStatus()�� �뺸�ϴ� �뵵�� ���̴� �� �Ϲ���������,
	// timer_thread�� ���ϸ� ���̱� ���� (worker_thread���� �����ϱ� ����)
	// pop�� Event�� ���� ó���� worker_thread���� �����Ѵ�.
	WSAOVERLAPPED_EX* over = new WSAOVERLAPPED_EX();
	over->event_state = Event->Command;
	over->event_target = Event->Ref_Object;
	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
	PostQueuedCompletionStatus(g_worker_iocp, 1, static_cast<int>(Event->Do_Object), &(over->overlapped));
}

bool InitProtocol()
{
	// ODBC Init
	cout << "[TCP ����] ";
	g_ODBC.AllocateHandles();
	cout << "[TCP ����] ";
	g_ODBC.ConnectDataSource((SQLWCHAR*)DSN, NULL, NULL);

	// Game Init
	cout << "[TCP ����] ���� ���� ������...";
	SimpleMMORPG.init();
	cout << "\n[TCP ����] ���� ���� ����Ϸ�!" << endl;

	int retval;
	// Winsock Start - windock.dll �ε�
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file" << endl;
		return false;
	}

	// ��� ���� ����
	listen_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_sock == INVALID_SOCKET) err_quit((char*)"WSASocket()");

	// ��� ������ ��� ����(Protocol, IPv4, PortNum)
	{
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));

		serveraddr.sin_family = PF_INET;
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		/*
		INADDR_ANY :
		������ ip�ּҸ� ������ ���� ��� Ȥ�� �ֱ������� ip�ּҰ� �ٲ� ��츦 ����ؼ�
		������ �ش��ϴ� ��� ip�ּҷ� ������ �����ϵ��� ����ϱ� ����.
		*/
		serveraddr.sin_port = htons(PORT_NUM);
		retval = ::bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR)
		{
			// ���� �޸� ����(������ ������)
			closesocket(listen_sock);
			// ���� ����
			WSACleanup();
			err_quit((char*)"bind()");
		}
	}

	// Ŭ���̾�Ʈ�κ��� ��û ���(��� ������ ����.)
	retval = listen(listen_sock, SOMAXCONN);
	/*
	backlog : ���� ���� �� ť�� ��Ƴ��� �� �ִ� �ִ� ���ġ
	�������̸� SOMAXCONN���� �Ѵ�.
	Why?
	������ �ϵ��� �ٲ� ��츦 ����ؼ�.
	*/
	if (retval == SOCKET_ERROR)
	{
		// ���� �޸� ����(������ ������)
		closesocket(listen_sock);
		// ���� ����
		WSACleanup();
		err_quit((char*)"listen()");
	}

	cout << "[TCP ����] ���� ��� �غ� �Ϸ�" << endl << "[TCP ����] Ŭ���̾�Ʈ ��û �����..." << endl;
	return true;
}

void do_accept()
{
	// ������ ��ſ� ����� ����
	SOCKET client_sock = NULL;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	ZeroMemory(&clientaddr, addrlen);

	while (true)
	{
		// ������ ������ Ŭ���̾�Ʈ Ȯ�� (��� ������ ���� Ȯ��.)
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);

		/*
		Ŭ���̾�Ʈ�κ��� ���� ��û�� ������
		��Ʒ� ��� �����
		(accept()�Լ� ���ο��� ���� �ݺ��ؼ� Ŭ���̾�Ʈ�� ������ ����Ѵ�.)
		*/
		if (client_sock == INVALID_SOCKET)
		{
			err_display((char*)"accept()");
			continue;
		}

		cout << "[TCP ����] <- [TCP Ŭ���̾�Ʈ] [IP �ּ�=" << inet_ntoa(clientaddr.sin_addr) << ", ��Ʈ��ȣ=" << ntohs(clientaddr.sin_port);
		int newID = SimpleMMORPG.ConnectUser();
		if (newID == -1)
		{
			cout << "]\n[TCP ����] �����ο�(" << MAX_CLIENTS << ")���� ���� �����Դϴ�. �ش� Ŭ���̾�Ʈ�� ������ �����մϴ�."
				<< " [IP �ּ�=" << inet_ntoa(clientaddr.sin_addr) << ", ��Ʈ��ȣ=" << ntohs(clientaddr.sin_port) << "]" << endl;
			closesocket(client_sock);
			continue;
		}
		cout << ", ID=" << (int)newID << "]"
			<< " Ŭ���̾�Ʈ�� �����Ͽ����ϴ�." << endl;

		// ���� ������ ����.
		SOCKETINFO* Socket_info = &SimpleMMORPG.GetUserData(newID)->Socket_info;
		ZeroMemory(Socket_info, sizeof(SOCKETINFO));
		Socket_info->Socket = client_sock;
		Socket_info->Overlapped_Ex_IO.dataBuffer.len = BUF_SIZE;
		Socket_info->Overlapped_Ex_IO.dataBuffer.buf = Socket_info->Overlapped_Ex_IO.messageBuffer;
		Socket_info->Overlapped_Ex_IO.event_state = IS_RECVED;

		/*
		HANDLE CreateIoCompletionPort (
		  HANDLE FileHandle, // handle to file
		  HANDLE ExistingCompletionPort, // handle to I/O completion port
		  ULONG_PTR CompletionKey, // completion key
		  DWORD NumberOfConcurrentThreads // number of threads to execute concurrently);

		���ڸ� ����, hFile�� IOCP�� ����ϴ� I/O �ڵ�( ����, ���� �� )�̴�.
		�̷��� �����ν� IOCP�� hFile���� �Ͼ�� I/O�� �����ϸ鼭 �۾��� ������ ��, �˷��� �� �ְ� �Ǵ� ���̴�.
		hExistingPort�� ���ο� ��Ʈ�� �����Ϸ��� NULL, ������ �ִ� IOCP�� ������ �Ϸ��� CreateIoCompletionPort()�Լ��� ������ ��ȯ�ߴ� IOCP �ڵ��� �Ѱ��ش�.
		�� ��° CompletionKey�� ������ ���ߴ� Ű�� ��Ÿ����.
		���߿� �ڵ鿡���� �۾��� ������ IOCP�� � �ڵ鿡���� �۾����� �˷��� �� �ڵ� �� ��ü�� �Ѱ��ִ� ���� �ƴ϶�, �� Ű ���� �˷��ش�.
		����, ���� ���� �ڵ��� ����ߴٸ�, ����� �� �� Ű ���� �����ϰ� ���ֹǷν� ������ ������ �� �ֵ��� �Ѵ�.
		������ NumberOfConcurrentThreads�� IOCP�� ����� �۾��� �� �� �󸶳� ���� �����带 ����Ͽ� �۾��� �� �� �����ϴ� ������
		Ư���� ��Ȯ�ϰ� �󸶳� �����ؾ� �� �� �� ���� 0�� ������ �����ϸ� ���� ����ȭ�� ������� �����带 �����Ͽ� ����Ѵ�.
		������ �����ϸ� IOCP�ڵ��� ��ȯ�ϰ� ���� �ϸ� NULL���� ��ȯ�Ѵ�.
		Tip dwCompletionKey���� ���� ������ ����� �� �ִ�. Ÿ���� DWORD�̹Ƿ� � ������ �����ͷ� ����ϴ� �͵� �����ϰ� �����νᵵ ����� �� �ִ�.
		*/
		// IOCP�� ���
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), g_worker_iocp, newID, 0);

		// Ŭ���̾�Ʈ�κ��� Recv�� �����Ѵ�.
		recv_packet(newID);
	}

	// ���� �޸� ����(������ ������)
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
}

void disconnect(int id)
{
	USER* user = SimpleMMORPG.GetUserData(id);
	bool bLogin = user->isLogin;
	char ConnectType = user->ConnectType;
	if (ConnectType == CS_LOGIN && bLogin == true)
	{
		USER save_info;
		wmemcpy_s(save_info.ID_str, MAX_WSTR_LEN, user->ID_str, MAX_WSTR_LEN);
		save_info.x = user->x;
		save_info.y = user->y;
		save_info.Level = user->Level;
		save_info.Experience = user->Experience;
		save_info.HP = user->HP;
		save_info.MP = user->MP;
		memcpy_s(&save_info.Inventory, sizeof(INVENTORY), &user->Inventory, sizeof(INVENTORY));
		memcpy_s(&save_info.Quest, sizeof(QUEST), &user->Quest, sizeof(QUEST));

		// ���� ����
		wstring query(L"EXEC save_user_info @id = ");
		query += save_info.ID_str;
		query += L", @x = ";
		query += to_wstring(save_info.x);
		query += L", @y = ";
		query += to_wstring(save_info.y);
		query += L", @level = ";
		query += to_wstring((int)save_info.Level);
		query += L", @exp = ";
		query += to_wstring((int)save_info.Experience);
		query += L", @hp = ";
		query += to_wstring((int)save_info.HP);
		query += L", @mp = ";
		query += to_wstring((int)save_info.MP);
		for (int i = 0; i < MAX_ITEM_SLOT; ++i)
		{
			query += L", @slot";
			query += to_wstring(i);
			query += L" = ";
			ITEM* Item = save_info.Inventory.Slot[i].Item;
			if (Item == nullptr)
			{
				query += L"non";
				continue;
			}
			switch (Item->Type)
			{
			case OBJECT_TYPE_NON:                query += L"non"; break;
			case OBJECT_TYPE_ITEM_COIN:          query += L"coi"; break;
			case OBJECT_TYPE_ITEM_HP_POTION:     query += L"hpp"; break;
			case OBJECT_TYPE_ITEM_MP_POTION:     query += L"mpp"; break;
			case OBJECT_TYPE_ITEM_NORMAL_SWORD:  query += L"nos"; break;
			case OBJECT_TYPE_ITEM_NORMAL_ARMOR:  query += L"noa"; break;
			case OBJECT_TYPE_ITEM_NORMAL_RING:   query += L"nor"; break;
			case OBJECT_TYPE_ITEM_SPECIAL_SWORD: query += L"sps"; break;
			case OBJECT_TYPE_ITEM_SPECIAL_ARMOR: query += L"spa"; break;
			case OBJECT_TYPE_ITEM_SPECIAL_RING:  query += L"spr"; break;
			}
			query += to_wstring(Item->How_many);
		}
		query += L", @quest_type = ";
		switch (save_info.Quest.Type)
		{
		case QUEST_TYPE::NON:                   query += to_wstring(0); break;
		case QUEST_TYPE::TEST_ABILITY:          query += to_wstring(1); break;
		case QUEST_TYPE::SUBJUGATE_BANDIT:      query += to_wstring(2); break;
		case QUEST_TYPE::ENCOUNTER_GOBLIN_KING: query += to_wstring(3); break;
		}
		query += L", @quest_con0 = ";
		query += to_wstring(save_info.Quest.Quest_Progress[0].How_many);
		query += L", @quest_con1 = ";
		query += to_wstring(save_info.Quest.Quest_Progress[1].How_many);
		

		// ���� ����
		cout << "[TCP ����] ";
		g_ODBC.ExecuteStatementDirect((SQLWCHAR*)(query.data()));
	}

	queue<EVENT*> GeneratedEvents;
	SimpleMMORPG.ExitUser(id, GeneratedEvents);

	// ������ ������ �����Ͽ� �߻��� �̺�Ʈ���� ó���Ѵ�.
	process_events(GeneratedEvents);

	SOCKET socket = user->Socket_info.Socket;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(socket, (SOCKADDR*)&clientaddr, &addrlen);
	Cout_other_Access.lock();
	cout << "[TCP ����] <- [TCP Ŭ���̾�Ʈ] [IP �ּ�=" << inet_ntoa(clientaddr.sin_addr) << ", ��Ʈ��ȣ=" << ntohs(clientaddr.sin_port) << ", ID=" << (int)id << "]"
		<< " Ŭ���̾�Ʈ���� ������ ���������ϴ�." << endl;
	Cout_other_Access.unlock();
	closesocket(socket);
}

void process_packet(int id, char* buf)
{
	SOCKET socket = SimpleMMORPG.GetUserData(id)->Socket_info.Socket;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(socket, (SOCKADDR*)&clientaddr, &addrlen);

	queue<EVENT*> GeneratedEvents;

	switch (buf[1])
	{
	case CS_LOGIN:
	{
		cs_pacekt_login* login_packet = reinterpret_cast<cs_pacekt_login*>(buf);

		// ���� ����
		wstring query(L"EXEC find_user @id = ");
		query += login_packet->id_str;
		query += L", @password = ";
		query += login_packet->password_str;

		// ���� ����
		cout << "[TCP ����] ";
		g_ODBC.ExecuteStatementDirect((SQLWCHAR*)(query.data()));

		// ���� ���� ��� ��������
		USER user_info;
		char slot_info[MAX_ITEM_SLOT][MAX_ITEM_DIGIT + 1]; // + item_type
		int  quest_type;
		int  quest_condition[2];
		g_ODBC.SetBufferLength(11);
		if (g_ODBC.RetrieveResult((char*)"iiiiiisssssssssiii",
			&user_info.x, &user_info.y, &user_info.Level, &user_info.Experience, &user_info.HP, &user_info.MP,
			&slot_info[0], &slot_info[1], &slot_info[2], &slot_info[3],
			&slot_info[4], &slot_info[5], &slot_info[6], &slot_info[7], &slot_info[8],
			&quest_type, &quest_condition[0], &quest_condition[1]) == true)
		{
			SimpleMMORPG.LoginUser(id, login_packet->id_str, CS_LOGIN);

			for (int i = 0; i < MAX_ITEM_SLOT; ++i)
			{
				int new_itemID = SimpleMMORPG.CreatableItemID();
				if (new_itemID == -1) break;
				/*
				non
				coi: coin
				hpp: hp potion
				mpp: mp potion
				nos: normal sword
				noa: normal armor
				nor: normal ring
				sps: special sword
				spa: special armor
				spr: special ring
				*/
				string type;
				string num_str;
				for (int j = 0; j < MAX_ITEM_DIGIT + 1; ++j)
				{
					if (slot_info[i][j] == '\0' || slot_info[i][j] == ' ') break;
					if (j < 3) type.push_back(slot_info[i][j]);
					else num_str.push_back(slot_info[i][j]);
				}

				if (type.compare("non") == 0) continue;
				unsigned char item_type = OBJECT_TYPE_NON;
				if (type.compare("coi") == 0) item_type = OBJECT_TYPE_ITEM_COIN;
				if (type.compare("hpp") == 0) item_type = OBJECT_TYPE_ITEM_HP_POTION;
				if (type.compare("mpp") == 0) item_type = OBJECT_TYPE_ITEM_MP_POTION;
				if (type.compare("nos") == 0) item_type = OBJECT_TYPE_ITEM_NORMAL_SWORD;
				if (type.compare("noa") == 0) item_type = OBJECT_TYPE_ITEM_NORMAL_ARMOR;
				if (type.compare("nor") == 0) item_type = OBJECT_TYPE_ITEM_NORMAL_RING;
				if (type.compare("sps") == 0) item_type = OBJECT_TYPE_ITEM_SPECIAL_SWORD;
				if (type.compare("spa") == 0) item_type = OBJECT_TYPE_ITEM_SPECIAL_ARMOR;
				if (type.compare("spr") == 0) item_type = OBJECT_TYPE_ITEM_SPECIAL_RING;

				int item_num = stoi(num_str);

				int x, y;
				TransformSlot(i, x, y);

				user_info.Inventory.Slot[i].Item = new ITEM(item_type, x, y, item_num, new_itemID);
			}

			switch (quest_type)
			{
			case 0:
				user_info.Quest.Type = QUEST_TYPE::NON;
				user_info.Quest.Quest_Progress[0] = Action_Condition{ OBJECT_TYPE_NON, Do_Action::NOTHING, 0 };
				user_info.Quest.Quest_Progress[1] = Action_Condition{ OBJECT_TYPE_NON, Do_Action::NOTHING, 0 };
				break;
			case 1:
				user_info.Quest.Type = QUEST_TYPE::TEST_ABILITY;
				user_info.Quest.Quest_Progress[0] = Action_Condition{ OBJECT_TYPE_MONSTER_SCORPION, Do_Action::KILL, (short)quest_condition[0] };
				user_info.Quest.Quest_Progress[1] = Action_Condition{ OBJECT_TYPE_MONSTER_SCORPION_KING, Do_Action::KILL, (short)quest_condition[1] };
				break;
			case 2:
				user_info.Quest.Type = QUEST_TYPE::SUBJUGATE_BANDIT;
				user_info.Quest.Quest_Progress[0] = Action_Condition{ OBJECT_TYPE_MONSTER_THEIF, Do_Action::KILL, (short)quest_condition[0] };
				user_info.Quest.Quest_Progress[1] = Action_Condition{ OBJECT_TYPE_MONSTER_THEIF_BOSS, Do_Action::KILL, (short)quest_condition[1] };
				break;
			case 3:
				user_info.Quest.Type = QUEST_TYPE::ENCOUNTER_GOBLIN_KING;
				user_info.Quest.Quest_Progress[0] = Action_Condition{ OBJECT_TYPE_MONSTER_GOBLIN_WARRIOR, Do_Action::KILL, (short)quest_condition[0] };
				user_info.Quest.Quest_Progress[1] = Action_Condition{ OBJECT_TYPE_MONSTER_GOBLIN_KING, Do_Action::FIND, (short)quest_condition[1] };
				break;
			}

			SimpleMMORPG.GameStart(id, GeneratedEvents, true, &user_info);

			// �ش� Ŭ���̾�Ʈ���� ������ �Ϸ�Ǿ����� �˸���.
			send_login_ok_packet(id, login_packet->id_str);
			// �ش� Ŭ���̾�Ʈ���� ���� ������ �����Ѵ�.
			send_user_info_packet(id);
		}
		else
		{
			Cout_other_Access.lock();
			cout << "[TCP ����] DB�� " << login_packet->id_str << "�� �������� �ʰų�, ��й�ȣ�� �߸��Ǿ����ϴ�. �ش� Ŭ���̾�Ʈ�� ������ �����մϴ�."
				<< " [IP �ּ�=" << inet_ntoa(clientaddr.sin_addr) << ", ��Ʈ��ȣ=" << ntohs(clientaddr.sin_port) << "]" << endl;
			Cout_other_Access.unlock();
			disconnect(id);
			return;
		}
		break;
	}
	case CS_LOGIN_DUMMY:
		SimpleMMORPG.LoginUser(id, NULL, CS_LOGIN_DUMMY);
		SimpleMMORPG.GameStart(id, GeneratedEvents);
		// �ش� Ŭ���̾�Ʈ���� ������ �Ϸ�Ǿ����� �˸���.
		send_login_ok_packet(id, NULL);
		break;
	case CS_MOVE_UP:
	case CS_MOVE_DOWN:
	case CS_MOVE_LEFT:
	case CS_MOVE_RIGHT:
	case CS_ATTACK:
	case CS_POWER_ATTACK:
		SimpleMMORPG.ProcessInputOfUser(id, buf[1], GeneratedEvents);
		break;
	case CS_ITEM_GET:
	case CS_ITEM_DROP:
	case CS_ITEM_USE:
	case CS_ITEM_BUY:
	case CS_ITEM_SLOT_INFO_CHANGE:
		SimpleMMORPG.ProcessItemEventOfUser(id, buf, GeneratedEvents);
		break;
	case CS_CHAT:
		break;
	case CS_LOGOUT:
		disconnect(id);
		break;
	}

	// ��Ŷ ó�� �� �߻��� �̺�Ʈ���� ó���Ѵ�.
	process_events(GeneratedEvents);
}

void CreateIOCP()
{
	g_worker_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

void CloseIOCP()
{
	CloseHandle(g_worker_iocp);
	// ODBC Handle Close
	g_ODBC.DisconnectDataSource();
}

bool worker_thread()
{
	DWORD io_byte;
#ifdef _WIN32
	#ifdef _WIN64
		ULONG64 key;
	#else
		ULONG key;
	#endif // _WIN64
#endif // _WIN32

	WSAOVERLAPPED_EX* lpover_ex;
	BOOL is_error;
	while (true)
	{
		// ���� �����尡 �ش� Ŭ���̾�Ʈ�� ���� ó���Ѵ�.
		is_error = GetQueuedCompletionStatus(g_worker_iocp, &io_byte, &key, reinterpret_cast<LPOVERLAPPED*>(&lpover_ex), INFINITE);
		// �̶� GetQueuedCompletionStatus()�� ���� ����� ������ ���� IOCP�� �����Ѵ�.
		/*
		���� GQCS�Լ��� ���� �Ǿ� ���ư��� �ִ� ������ ����
		�������� Concurrent Thread ���� �Ѿ��
		�� �Լ��� ť�� ������ �ִ��� ������ ���� �ʴ´�.
		�� �� ������ Thread Switching�� ȿ�������� �����Ѵ�.
		�� Concurrent Thread ���� �Ѿ�� �ʵ��� IOCP�� �����Ͽ��� �����带 �����.
		������ �̷��ٰ� �Ͽ��� ���ư��� �ִ� ������ ���� Concurrent Thread ������ �׻� ������ �ʴ�.
		�̺��� Ŭ ���� �ִ�. �װ��� IOCP�� Smart�ϰ� ó���ȴٴ� �����̴�.
		*/
		int id = (int)key;
		if (io_byte == 0)
		{
			disconnect(id);
			if (lpover_ex->event_state == IS_SENDED)
				delete reinterpret_cast<WSAOVERLAPPED_EX_IO*>(lpover_ex);
			
			continue;
		}
		else if (is_error == FALSE)
		{
			int err_no = WSAGetLastError();
			if (err_no != 64)
			{
				err_display((char*)"GetQueuedCompletionStatus");
				//continue;
			}
			else
			{
				disconnect(id);
				if (lpover_ex->event_state == IS_SENDED)
					delete reinterpret_cast<WSAOVERLAPPED_EX_IO*>(lpover_ex);

				continue;
			}
		}

		if (lpover_ex->event_state == IS_RECVED)
		{
			WSAOVERLAPPED_EX_IO* lpover_ex_io = reinterpret_cast<WSAOVERLAPPED_EX_IO*>(lpover_ex);
			SOCKETINFO* Socket_info = &SimpleMMORPG.GetUserData(id)->Socket_info;

			int rest_size = io_byte;
			char* ptr = lpover_ex_io->messageBuffer;
			unsigned char packet_size = 0;
			if (Socket_info->prevSize > 0) packet_size = Socket_info->PacketBuffer[0];
			while (rest_size > 0)
			{
				if (packet_size == 0) packet_size = ptr[0];
				int required = packet_size - Socket_info->prevSize;
				if (rest_size >= required)
				{
					memcpy(Socket_info->PacketBuffer + Socket_info->prevSize, ptr, required);
					process_packet(id, Socket_info->PacketBuffer);
					rest_size -= required;
					ptr += required;
					packet_size = 0;
				}
				else
				{
					// prevSize�� ���� ���� �ٲ�°�?...
					memcpy(Socket_info->PacketBuffer + Socket_info->prevSize, ptr, rest_size);
					rest_size = 0;
				}
			}
			recv_packet(id);
		}
		else if (lpover_ex->event_state == IS_SENDED)// IS_SENDED
		{
			delete lpover_ex;
		}
		else if (lpover_ex->event_state == MONSTER_MOVE)
		{
			EVENT Event(EVENT_TYPE::EVENT_DIRECT, static_cast<int>(key), lpover_ex->event_target, MONSTER_MOVE);
			process_event(Event);
			delete lpover_ex;
		}
		else if (lpover_ex->event_state == USER_MOVED)
		{
			EVENT Event(EVENT_TYPE::EVENT_DIRECT, static_cast<int>(key), lpover_ex->event_target, USER_MOVED);
			process_event(Event);
			delete lpover_ex;
		}
		else if (lpover_ex->event_state == MONSTER_ATTACK)
		{
			EVENT Event(EVENT_TYPE::EVENT_DIRECT, static_cast<int>(key), lpover_ex->event_target, MONSTER_ATTACK);
			process_event(Event);
			delete lpover_ex;
		}
		else if (lpover_ex->event_state == USER_HEAL)
		{
			EVENT Event(EVENT_TYPE::EVENT_DIRECT, static_cast<int>(key), lpover_ex->event_target, USER_HEAL);
			process_event(Event);
			delete lpover_ex;
		}
		else if (lpover_ex->event_state == USER_RESPAWN)
		{
			EVENT Event(EVENT_TYPE::EVENT_DIRECT, static_cast<int>(key), lpover_ex->event_target, USER_RESPAWN);
			process_event(Event);
			delete lpover_ex;
		}
		else if (lpover_ex->event_state == MONSTER_RESPAWN)
		{
			EVENT Event(EVENT_TYPE::EVENT_DIRECT, static_cast<int>(key), lpover_ex->event_target, MONSTER_RESPAWN);
			process_event(Event);
			delete lpover_ex;
		}
	}
}

bool timer_thread()
{
	TIME_EVENT Event;
	do
	{
		this_thread::sleep_for(10ms);
		do
		{
			Timer_other_Access.lock();
			if (Timer_Queue.size() == 0)
			{
				Timer_other_Access.unlock();
				break;
			}
			else Timer_other_Access.unlock();

			Timer_other_Access.lock();
			Event = Timer_Queue.top();
			Timer_other_Access.unlock();
			if (Event.ExecutionTime >= high_resolution_clock::now()) break;

			Timer_other_Access.lock();
			Timer_Queue.pop();
			Timer_other_Access.unlock();

			set_event_to_worker(&Event);
		} while (true);
	} while (true);
}
