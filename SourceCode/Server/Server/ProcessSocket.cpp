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

// 타임 이벤트 부여
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

// 해당 Event를 처리해야 함을 GetQueuedCompletionStatus(g_iocp)를 호출하는 Worker스레드에게 알림
void set_event_to_worker(TIME_EVENT* Event)
{
	// PostQueuedCompletionStatus() : IOCP 객체에 가상의 완료를 통보
	// 인자로 전달된 값을 이용해 가상의 완료 패킷을 하나 만들어 IOCP 완료 큐에 보내주는 함수.
	// 즉, GetQueuedCompletionStatus()를 깨우는 함수
	// 원래 의도라면 이미 완료된 상태(Event처리)를 GetQueuedCompletionStatus()에 통보하는 용도로 쓰이는 게 일반적이지만,
	// timer_thread의 부하를 줄이기 위해 (worker_thread에서 전담하기 위해)
	// pop한 Event에 대한 처리를 worker_thread에게 위임한다.
	WSAOVERLAPPED_EX* over = new WSAOVERLAPPED_EX();
	over->event_state = Event->Command;
	over->event_target = Event->Ref_Object;
	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
	PostQueuedCompletionStatus(g_worker_iocp, 1, static_cast<int>(Event->Do_Object), &(over->overlapped));
}

bool InitProtocol()
{
	// ODBC Init
	cout << "[TCP 서버] ";
	g_ODBC.AllocateHandles();
	cout << "[TCP 서버] ";
	g_ODBC.ConnectDataSource((SQLWCHAR*)DSN, NULL, NULL);

	// Game Init
	cout << "[TCP 서버] 게임 월드 빌드중...";
	SimpleMMORPG.init();
	cout << "\n[TCP 서버] 게임 월드 빌드완료!" << endl;

	int retval;
	// Winsock Start - windock.dll 로드
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file" << endl;
		return false;
	}

	// 대기 소켓 생성
	listen_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_sock == INVALID_SOCKET) err_quit((char*)"WSASocket()");

	// 대기 소켓의 통신 설정(Protocol, IPv4, PortNum)
	{
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));

		serveraddr.sin_family = PF_INET;
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		/*
		INADDR_ANY :
		서버가 ip주소를 여러개 갖을 경우 혹은 주기적으로 ip주소가 바뀔 경우를 대비해서
		서버에 해당하는 모든 ip주소로 접근이 가능하도록 허용하기 위함.
		*/
		serveraddr.sin_port = htons(PORT_NUM);
		retval = ::bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR)
		{
			// 소켓 메모리 해제(서버의 대기소켓)
			closesocket(listen_sock);
			// 윈속 종료
			WSACleanup();
			err_quit((char*)"bind()");
		}
	}

	// 클라이언트로부터 요청 대기(대기 소켓을 통해.)
	retval = listen(listen_sock, SOMAXCONN);
	/*
	backlog : 동시 접속 시 큐에 담아넣을 수 있는 최대 허용치
	가급적이면 SOMAXCONN으로 한다.
	Why?
	서버의 하드웨어가 바뀔 경우를 대비해서.
	*/
	if (retval == SOCKET_ERROR)
	{
		// 소켓 메모리 해제(서버의 대기소켓)
		closesocket(listen_sock);
		// 윈속 종료
		WSACleanup();
		err_quit((char*)"listen()");
	}

	cout << "[TCP 서버] 소켓 통신 준비 완료" << endl << "[TCP 서버] 클라이언트 요청 대기중..." << endl;
	return true;
}

void do_accept()
{
	// 데이터 통신에 사용할 변수
	SOCKET client_sock = NULL;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	ZeroMemory(&clientaddr, addrlen);

	while (true)
	{
		// 서버에 접속한 클라이언트 확인 (대기 소켓을 통해 확인.)
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);

		/*
		클라이언트로부터 받은 요청이 있으면
		↓아래 명령 실행↓
		(accept()함수 내부에서 무한 반복해서 클라이언트의 접속을 대기한다.)
		*/
		if (client_sock == INVALID_SOCKET)
		{
			err_display((char*)"accept()");
			continue;
		}

		cout << "[TCP 서버] <- [TCP 클라이언트] [IP 주소=" << inet_ntoa(clientaddr.sin_addr) << ", 포트번호=" << ntohs(clientaddr.sin_port);
		int newID = SimpleMMORPG.ConnectUser();
		if (newID == -1)
		{
			cout << "]\n[TCP 서버] 수용인원(" << MAX_CLIENTS << ")보다 많은 접속입니다. 해당 클라이언트의 접속을 해제합니다."
				<< " [IP 주소=" << inet_ntoa(clientaddr.sin_addr) << ", 포트번호=" << ntohs(clientaddr.sin_port) << "]" << endl;
			closesocket(client_sock);
			continue;
		}
		cout << ", ID=" << (int)newID << "]"
			<< " 클라이언트가 접속하였습니다." << endl;

		// 소켓 정보를 지정.
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

		인자를 보면, hFile은 IOCP에 등록하는 I/O 핸들( 소켓, 파일 등 )이다.
		이렇게 함으로써 IOCP는 hFile에서 일어나는 I/O를 감시하면서 작업이 끝났을 때, 알려줄 수 있게 되는 것이다.
		hExistingPort는 새로운 포트를 생성하려면 NULL, 기존에 있는 IOCP에 연결을 하려면 CreateIoCompletionPort()함수가 이전에 반환했던 IOCP 핸들을 넘겨준다.
		세 번째 CompletionKey는 위에서 말했던 키를 나타낸다.
		나중에 핸들에서의 작업이 끝나면 IOCP는 어떤 핸들에서의 작업인지 알려줄 때 핸들 값 자체를 넘겨주는 것이 아니라, 이 키 값을 알려준다.
		따라서, 여러 개의 핸들을 등록했다면, 등록할 때 이 키 값을 고유하게 해주므로써 각각을 구분할 수 있도록 한다.
		마지막 NumberOfConcurrentThreads는 IOCP가 입출력 작업을 할 때 얼마나 많은 쓰레드를 사용하여 작업을 할 지 설정하는 것으로
		특별히 정확하게 얼마나 설정해야 할 지 모를 때는 0의 값으로 설정하면 가장 최적화된 방법으로 스레드를 생성하여 사용한다.
		생성이 성공하면 IOCP핸들을 반환하고 실패 하면 NULL값을 반환한다.
		Tip dwCompletionKey값은 여러 가지로 사용할 수 있다. 타입이 DWORD이므로 어떤 형식의 포인터로 사용하는 것도 가능하고 정수로써도 사용할 수 있다.
		*/
		// IOCP에 등록
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), g_worker_iocp, newID, 0);

		// 클라이언트로부터 Recv를 시작한다.
		recv_packet(newID);
	}

	// 소켓 메모리 해제(서버의 대기소켓)
	closesocket(listen_sock);

	// 윈속 종료
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

		// 쿼리 정의
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
		

		// 쿼리 실행
		cout << "[TCP 서버] ";
		g_ODBC.ExecuteStatementDirect((SQLWCHAR*)(query.data()));
	}

	queue<EVENT*> GeneratedEvents;
	SimpleMMORPG.ExitUser(id, GeneratedEvents);

	// 유저가 게임을 종료하여 발생한 이벤트들을 처리한다.
	process_events(GeneratedEvents);

	SOCKET socket = user->Socket_info.Socket;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(socket, (SOCKADDR*)&clientaddr, &addrlen);
	Cout_other_Access.lock();
	cout << "[TCP 서버] <- [TCP 클라이언트] [IP 주소=" << inet_ntoa(clientaddr.sin_addr) << ", 포트번호=" << ntohs(clientaddr.sin_port) << ", ID=" << (int)id << "]"
		<< " 클라이언트와의 접속이 끊어졌습니다." << endl;
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

		// 쿼리 정의
		wstring query(L"EXEC find_user @id = ");
		query += login_packet->id_str;
		query += L", @password = ";
		query += login_packet->password_str;

		// 쿼리 실행
		cout << "[TCP 서버] ";
		g_ODBC.ExecuteStatementDirect((SQLWCHAR*)(query.data()));

		// 쿼리 실행 결과 가져오기
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

			// 해당 클라이언트에게 접속이 완료되었음을 알린다.
			send_login_ok_packet(id, login_packet->id_str);
			// 해당 클라이언트에게 유저 정보를 전송한다.
			send_user_info_packet(id);
		}
		else
		{
			Cout_other_Access.lock();
			cout << "[TCP 서버] DB에 " << login_packet->id_str << "가 존재하지 않거나, 비밀번호가 잘못되었습니다. 해당 클라이언트의 접속을 해제합니다."
				<< " [IP 주소=" << inet_ntoa(clientaddr.sin_addr) << ", 포트번호=" << ntohs(clientaddr.sin_port) << "]" << endl;
			Cout_other_Access.unlock();
			disconnect(id);
			return;
		}
		break;
	}
	case CS_LOGIN_DUMMY:
		SimpleMMORPG.LoginUser(id, NULL, CS_LOGIN_DUMMY);
		SimpleMMORPG.GameStart(id, GeneratedEvents);
		// 해당 클라이언트에게 접속이 완료되었음을 알린다.
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

	// 패킷 처리 후 발생한 이벤트들을 처리한다.
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
		// 여러 스레드가 해당 클라이언트에 대해 처리한다.
		is_error = GetQueuedCompletionStatus(g_worker_iocp, &io_byte, &key, reinterpret_cast<LPOVERLAPPED*>(&lpover_ex), INFINITE);
		// 이때 GetQueuedCompletionStatus()를 통해 깨우는 스레드 수는 IOCP가 결정한다.
		/*
		현재 GQCS함수가 리턴 되어 돌아가고 있는 쓰레드 수가
		지정해준 Concurrent Thread 수를 넘어서면
		이 함수는 큐에 내용이 있더라도 리턴이 되질 않는다.
		이 점 때문에 Thread Switching을 효율적으로 관리한다.
		즉 Concurrent Thread 수를 넘어서지 않도록 IOCP가 조절하여서 쓰레드를 깨운다.
		하지만 이렇다고 하여서 돌아가고 있는 쓰레드 수가 Concurrent Thread 수보다 항상 작지는 않다.
		이보다 클 수도 있다. 그것이 IOCP가 Smart하게 처리된다는 장점이다.
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
					// prevSize의 값은 언제 바뀌는가?...
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
