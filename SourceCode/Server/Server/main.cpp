#include "ProcessSocket.h"

int main(int argc, char argv[])
{
	// 에러 메세지 한글로 출력하기 위함
	setlocale(LC_ALL, "korean");
	wcout.imbue(locale("korean"));
	srand((unsigned)time(NULL));

	vector <thread> worker_threads;

	CreateIOCP();
	if (!InitProtocol())
		return 0;
	thread accept_thread{ do_accept };

	for (int i = 0; i < CORE_NUM; ++i)
		worker_threads.emplace_back(thread{ worker_thread });
	thread timer{ timer_thread };

	timer.join();
	for (auto& th : worker_threads)
		th.join();
	accept_thread.join();

	CloseIOCP();

	system("pause");
}