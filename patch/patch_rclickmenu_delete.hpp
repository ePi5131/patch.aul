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

#ifdef PATCH_SWITCH_RCLICKMENU_DELETE

#include <memory>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 右クリックメニューの削除でテキストの字間・行間が変わることがあるのを修正
	// 設定ダイアログの更新タイミングを変えることで修正
	inline class rclickmenu_delete_t {
		static void __cdecl disp_near_settingdialog_wrap3fef6();
		static void __cdecl DrawTimelineMask_wrap3ff68(int* to_draw);

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "r_click_menu_delete";

		inline static int last_id = -1;

	public:

		inline static int* settingdialog_object_idx_ptr;
		inline static int* selecting_obj_num_ptr;
		inline static int* selecting_obj_list;


		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			settingdialog_object_idx_ptr = reinterpret_cast<decltype(settingdialog_object_idx_ptr)>(GLOBAL::exedit_base + 0x177a10);
			selecting_obj_num_ptr = reinterpret_cast<decltype(selecting_obj_num_ptr)>(GLOBAL::exedit_base + 0x167d88);
			selecting_obj_list = reinterpret_cast<decltype(selecting_obj_list)>(GLOBAL::exedit_base + 0x179230);

			ReplaceNearJmp(GLOBAL::exedit_base + 0x03fef6, &disp_near_settingdialog_wrap3fef6);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x03ff68, &DrawTimelineMask_wrap3ff68);

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

	} rclickmenu_delete;
} // namespace patch
#endif // ifdef PATCH_SWITCH_RCLICKMENU_DELETE
