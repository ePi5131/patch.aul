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

#ifdef PATCH_SWITCH_RCLICKMENU_SPLIT

#include <memory>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 右クリックメニューの分割で設定ダイアログの更新が行われないのを修正
	inline class rclickmenu_split_t {
		static int __cdecl filter_sendmessage_wrap3fd46(int object_idx, int wparam, int flag);
		static void __cdecl splitted_object_new_group_belong_wrap3fd5c();

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "r_click_menu_split";

		inline static int last_id = -1;

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			ReplaceNearJmp(GLOBAL::exedit_base + 0x03fd46, &filter_sendmessage_wrap3fd46);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x03fd5c, &splitted_object_new_group_belong_wrap3fd5c);

		}
		void switching(bool flag) {
			enabled = flag;
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}

	} rclickmenu_split;
} // namespace patch
#endif // ifdef PATCH_SWITCH_RCLICKMENU_SPLIT
