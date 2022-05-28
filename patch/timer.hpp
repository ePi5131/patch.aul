#include <functional>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>

#include <Windows.h>

class Timer;
extern Timer timer;

inline class Timer {
	using timer_func = std::function<void()>;

	std::unordered_map<UINT_PTR, timer_func> list;

	static VOID CALLBACK timerproc(HWND, UINT, UINT_PTR nIDEvent, DWORD) {
		const auto& list = timer.list;
		if (auto func = list.find(nIDEvent); func != list.end())
			func->second();
	}

public:
	~Timer() {
		for (auto [id, _] : list) {
			KillTimer(NULL, id);
		}
	}

	void set(timer_func f, UINT elapse) {
		UINT_PTR try_id = 1;
		UINT_PTR timer_id;
		while ((timer_id = SetTimer(NULL, try_id, elapse, timerproc)) == 0) {
			try_id++;
		}
		list.try_emplace(timer_id, f);
	}
} timer;
