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
#include "macro.h"

#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
#include <atomic>
#include <thread>
#include <Windows.h>

namespace patch {
	inline class setting_dialog_move_t {
		inline static std::jthread th;
		inline static std::atomic_bool waiting;

		static bool enabled() {
			return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_SETTINGDIALOG_MOVE);
		}
public:
		void operator()(HWND hwnd) {
			if (!enabled())return;
			std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
		}
	} setting_dialog_move;
}
#endif
