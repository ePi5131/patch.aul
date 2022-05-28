/*
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
