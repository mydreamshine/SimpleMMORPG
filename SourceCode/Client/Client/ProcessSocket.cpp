#include "stdafx.h"
#include "ProcessSocket.h"
#include "Scene.h"

SOCKET server_connect_sock;
SOCKADDR_IN serveraddr;
WSABUF RecvWSABuffer;
WSABUF SendWSABuffer;
char RecvBufferStream[BUF_SIZE];
char SendBufferStream[BUF_SIZE];
char PacketBuffer[BUF_SIZE];
int  in_packet_size = 0;
int	 saved_packet_size = 0;

bool ProcessSocket::InitProtocol(HWND hWnd, DWORD ServerAddress)
{
	int retval;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	// Create Socket
	server_connect_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (server_connect_sock == INVALID_SOCKET)
		//err_quit((char*)"socket()");
		return false;

	
	// Setting Socket(Protocol, IPv4, PortNum) <- Server Information
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = ServerAddress;
	serveraddr.sin_port = htons(PORT_NUM);

	retval = connect(server_connect_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		//err_quit((char*)"connect()");
		return false;

	WSAAsyncSelect(server_connect_sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ);

	SendWSABuffer.buf = SendBufferStream;
	SendWSABuffer.len = BUF_SIZE;
	RecvWSABuffer.buf = RecvBufferStream;
	RecvWSABuffer.len = BUF_SIZE;
	ZeroMemory(SendBufferStream, sizeof(SendBufferStream));
	ZeroMemory(RecvBufferStream, sizeof(RecvBufferStream));
	ZeroMemory(PacketBuffer, sizeof(PacketBuffer));

	return true;
}

void ProcessSocket::ProcessPacketFrom(char* BufferStream, LPVOID arg)
{
	CScene* pScene = reinterpret_cast<CScene*>(arg);
	pScene->ProcessPacketFrom(BufferStream);
}

void ProcessSocket::SendPacket(char packet_type, ...)
{
	va_list arg_pointer;
	va_start(arg_pointer, packet_type);

	ZeroMemory(SendWSABuffer.buf, sizeof(SendWSABuffer.buf));
	switch (packet_type)
	{
	case CS_LOGIN:
	{
		cs_pacekt_login* packet = reinterpret_cast<cs_pacekt_login*>(SendWSABuffer.buf);
		packet->type = CS_LOGIN;
		packet->size = sizeof(cs_pacekt_login);
		wchar_t* id_str = va_arg(arg_pointer, wchar_t*);
		wchar_t* password_str = va_arg(arg_pointer, wchar_t*);
		wsprintfW(packet->id_str, L"%ws", id_str);
		wsprintfW(packet->password_str, L"%ws", password_str);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_LOGOUT:
	{
		cs_packet_logout* packet = reinterpret_cast<cs_packet_logout*>(SendWSABuffer.buf);
		packet->type = CS_LOGOUT;
		packet->size = sizeof(cs_packet_logout);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_MOVE_UP:
	{
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(SendWSABuffer.buf);
		packet->type = CS_MOVE_UP;
		packet->size = sizeof(cs_packet_move);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_MOVE_DOWN:
	{
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(SendWSABuffer.buf);
		packet->type = CS_MOVE_DOWN;
		packet->size = sizeof(cs_packet_move);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_MOVE_LEFT:
	{
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(SendWSABuffer.buf);
		packet->type = CS_MOVE_LEFT;
		packet->size = sizeof(cs_packet_move);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_MOVE_RIGHT:
	{
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(SendWSABuffer.buf);
		packet->type = CS_MOVE_RIGHT;
		packet->size = sizeof(cs_packet_move);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_ATTACK:
	{
		cs_packet_attack* packet = reinterpret_cast<cs_packet_attack*>(SendWSABuffer.buf);
		packet->type = CS_ATTACK;
		packet->size = sizeof(cs_packet_attack);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_POWER_ATTACK:
	{
		cs_packet_power_attack* packet = reinterpret_cast<cs_packet_power_attack*>(SendWSABuffer.buf);
		packet->type = CS_POWER_ATTACK;
		packet->size = sizeof(cs_packet_power_attack);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_ITEM_GET:
	{
		cs_packet_item_get* packet = reinterpret_cast<cs_packet_item_get*>(SendWSABuffer.buf);
		packet->type = CS_ITEM_GET;
		packet->size = sizeof(cs_packet_item_get);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_ITEM_DROP:
	{
		cs_packet_item_drop* packet = reinterpret_cast<cs_packet_item_drop*>(SendWSABuffer.buf);
		packet->type = CS_ITEM_DROP;
		packet->size = sizeof(cs_packet_item_drop);
		packet->slot_num = va_arg(arg_pointer, char);
		packet->how_many = va_arg(arg_pointer, short);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_ITEM_USE:
	{
		cs_packet_item_use* packet = reinterpret_cast<cs_packet_item_use*>(SendWSABuffer.buf);
		packet->type = CS_ITEM_USE;
		packet->size = sizeof(cs_packet_item_drop);
		packet->slot_num = va_arg(arg_pointer, char);
		packet->how_many = va_arg(arg_pointer, short);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_ITEM_SLOT_INFO_CHANGE:
	{
		cs_packet_change_item_slot* packet = reinterpret_cast<cs_packet_change_item_slot*>(SendWSABuffer.buf);
		packet->type = CS_ITEM_SLOT_INFO_CHANGE;
		packet->size = sizeof(cs_packet_change_item_slot);
		packet->from_slot = va_arg(arg_pointer, char);
		packet->new_slot = va_arg(arg_pointer, char);
		SendWSABuffer.len = packet->size;
		break;
	}
	case CS_CHAT:
	{
		cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(SendWSABuffer.buf);
		packet->type = CS_CHAT;
		packet->size = sizeof(cs_packet_chat);
		wchar_t* chat_str = va_arg(arg_pointer, wchar_t*);
		wcscpy_s(packet->message, MAX_WSTR_LEN, chat_str);
		SendWSABuffer.len = packet->size;
		break;
	}
	default:
		va_end(arg_pointer);    // 가변 인자 포인터를 NULL로 초기화
		return;
	}
	va_end(arg_pointer);    // 가변 인자 포인터를 NULL로 초기화

	DWORD iobyte;
	int retval = WSASend(server_connect_sock, &SendWSABuffer, 1, &iobyte, 0, NULL, NULL);
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - Fail WSASend(error_code :" << WSAGetLastError() << ")\n";
			//err_quit((char*)"WSASend");
		}
	}
}

void ProcessSocket::closeProtocol()
{
	//ProcessSocket::SendPacket(CS_LOGOUT);
	closesocket(server_connect_sock);
	// 윈속 종료
	WSACleanup();
}

void ProcessSocket::ReadPacket(SOCKET Socket, LPVOID arg)
{
	DWORD iobyte = 0, ioflag = 0;
	int retval = WSARecv(server_connect_sock, &RecvWSABuffer, 1, &iobyte, &ioflag, NULL, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			ProcessSocket::closeProtocol();
			err_quit((char*)"WSARecv()");
		}
	}

	char *ptr = RecvWSABuffer.buf;

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = (unsigned char)ptr[0];
		if ((int)iobyte + saved_packet_size >= in_packet_size) {
			memcpy(PacketBuffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);

			ProcessSocket::ProcessPacketFrom(PacketBuffer, arg);

			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(PacketBuffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}