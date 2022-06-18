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

#include "config_rw.hpp"

namespace patch {
	inline class setting_dialog_move_t {
		inline static std::jthread th;
		inline static std::atomic_bool waiting;

		bool enabled = true;
        inline static const char key[] = "settingdialog_move";
public:
		void operator()(HWND) {
			if (!enabled)return;
			std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
		}
		
        void switching(bool flag) {
            enabled = flag;
        }

        bool is_enabled() { return enabled; }
        bool is_enabled_i() { return enabled; }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
            });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }
	} setting_dialog_move;
}
#endif
