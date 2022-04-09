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

#pragma once
#include <thread>
#include <type_traits>
#include <vector>
#include <mutex>
#include <Windows.h>

inline class {
	inline static std::once_flag init_flag;
	inline static size_t thread_num;
	static void init() {
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		thread_num = info.dwNumberOfProcessors;
	}
public:
	template<class T, std::enable_if_t<std::is_invocable_v<T, size_t, size_t>, std::nullptr_t> = nullptr>
	inline void operator()(T func) const {
		std::call_once(init_flag, init);
		std::vector<std::thread> threads;
		for (size_t i = 0; i < thread_num; i++) threads.emplace_back(func, i, thread_num);
		for (auto&& t : threads) t.join();
	}
	auto get_thread_num() const {
		std::call_once(init_flag, init);
		return thread_num;
	}
} multi_threading;
