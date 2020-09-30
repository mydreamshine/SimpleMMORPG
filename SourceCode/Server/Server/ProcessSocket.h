#pragma once
#include "C_SIMPLE_MMORPG.h"
#define CORE_NUM 4
#define DSN L"SimpleMMORPG_Server"

bool InitProtocol();
void do_accept();
//void SendPacketBy(multimap<char, EVENT_CASE>& EventTargets);

void process_packet(int id, char* buf);
void disconnect(int id);

void CreateIOCP();
void CloseIOCP();

// Åë½Å + ÄÁÅÙÃ÷
bool worker_thread();
// ÄÁÅÙÃ÷
bool timer_thread();