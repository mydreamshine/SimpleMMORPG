#pragma once

namespace ProcessSocket
{
	bool InitProtocol(HWND hWnd, DWORD ServerAddress);
	void ProcessPacketFrom(char* BufferStream, LPVOID arg);
	void ReadPacket(SOCKET Socket, LPVOID arg);
	/*
	CS_LOGIN:        Need Parameter (wcahr_t id_str[MAX_WSTR_LEN], wchar_t password_str[MAX_WSTR_LEN])
	CS_LOGOUT:       Parameter nothing
	CS_MOVE_UP:      Parameter nothing
	CS_MOVE_DOWN:    Parameter nothing
	CS_MOVE_LEFT:    Parameter nothing
	CS_MOVE_RIGHT:   Parameter nothing
	CS_ATTACK:       Parameter nothing
	CS_POWER_ATTACK: Parameter nothing
	CS_ITEM_GET:     Parameter nothing
	CS_ITEM_DROP:    Need Parameter (char slot_num, short how_many)
	CS_CHAT:         Need Parameter (wcahr_t chat_str[MAX_WSTR_LEN])
	*/
	void SendPacket(char packet_type, ...);
	void closeProtocol();
}
