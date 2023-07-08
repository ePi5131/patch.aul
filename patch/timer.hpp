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
#include <unordered_map>
#include <chrono>
#include <memory>

#include "jsleep.hpp"

class Timer {
	std::unique_ptr<JSleep> sleep;
	std::jthread thread;

public:
	template<class Rep, class Period, class F>
	Timer(const std::chrono::duration<Rep, Period>& rel_time, F&& f) : sleep{ std::make_unique<JSleep>()}, thread{[f = std::forward<F>(f), rel_time, this](std::stop_token stoken) {
		while (!stoken.stop_requested()) {
			const auto now = std::chrono::system_clock::now();
			std::invoke(f);
			sleep->wait_until(now + rel_time);
		}
	} } {}

	~Timer() {
		kill();
	}

	void kill() {
		thread.request_stop();
		sleep->cancel();
	}
};
