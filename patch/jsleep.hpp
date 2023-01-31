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
#include <chrono>
#include <mutex>
#include <condition_variable>

class JSleep {
	bool signal;
	std::mutex mtx;
	std::condition_variable cv;
public:
	JSleep() : signal{ false }, mtx{}, cv{} {}

	void cancel() {
		std::lock_guard lock(mtx);
		signal = true;
		cv.notify_all();
	}

	template<class Rep, class Period>
	bool wait_for(const std::chrono::duration<Rep, Period>& rel_time) {
		std::unique_lock lock(mtx);
		cv.wait_for(lock, rel_time, [this]() { return signal; });
		return signal;
	}

	template<class Clock, class Duration>
	bool wait_until(const std::chrono::time_point<Clock, Duration>& abs_time) {
		std::unique_lock lock(mtx);
		cv.wait_until(lock, abs_time, [this]() { return signal; });
		return signal;
	}

	void reset() {
		std::lock_guard lock(mtx);
		signal = true;
	}
};
