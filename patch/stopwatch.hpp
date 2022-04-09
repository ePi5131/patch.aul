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
#include <iostream>

#include "debug_log.hpp"

class stopwatch {
#ifdef PATCH_STOPWATCH
	std::chrono::system_clock::time_point start;
public:
	stopwatch() noexcept : start(std::chrono::system_clock::now()) {}
	~stopwatch() {
		print();
	}
	long long now() const noexcept {
		auto end = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	}
	void print() const noexcept {
		debug_log("{} us", now());
	}
#else
public:
	long long now() const noexcept { return 0; }
	void print() const noexcept {}
#endif
};

class stopwatch_mem {
#ifdef PATCH_STOPWATCH
	std::chrono::system_clock::time_point start_tp;
	long long sum;
	long long cnt;

	long long now() const noexcept {
		auto end = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::microseconds>(end - start_tp).count();
	}
public:
	stopwatch_mem() noexcept : start_tp{}, sum{}, cnt{} {}
	~stopwatch_mem() {}
	void start() noexcept {
		start_tp = std::chrono::system_clock::now();
	}
	void stop() noexcept {
		auto time = now();
		cnt++;
		sum += time;
		debug_log("{} us  ave : {} us", time, sum / cnt);
	}
#else
public:
	void start() noexcept {}
	void stop() noexcept {}
#endif
};
