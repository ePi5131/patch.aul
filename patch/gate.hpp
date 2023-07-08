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
#include <mutex>
#include <condition_variable>

class Gate {
	std::mutex mtx;
	std::condition_variable cond;
	bool flag;

public:
	Gate() :mtx{}, cond{}, flag{} {}

	Gate(bool opened) :mtx{}, cond{}, flag{opened} {}

	void open() {
		{
			std::lock_guard lock(mtx);
			flag = true;
		}

		cond.notify_one();
	}

	void wait() {
		std::unique_lock lock(mtx);
		cond.wait(lock, [this] { return flag; });
		flag = false;
	}
};
