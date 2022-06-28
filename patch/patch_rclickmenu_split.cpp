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

#include "patch_rclickmenu_split.hpp"

#ifdef PATCH_SWITCH_RCLICKMENU_SPLIT
namespace patch {

	int __cdecl rclickmenu_split_t::filter_sendmessage_wrap3fd46(int object_idx, int wparam, int flag) {
		if (last_id == -1) {
			last_id = object_idx;
		}
		return reinterpret_cast<int(__cdecl*)(int,int,int)>(GLOBAL::exedit_base + OFS::ExEdit::filter_sendmessage)(object_idx, wparam, flag);
	}

	void __cdecl rclickmenu_split_t::splitted_object_new_group_belong_wrap3fd5c() {
		if (last_id != -1) {
			reinterpret_cast<void(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::disp_settingdialog)(last_id);
			last_id = -1;
		}
		reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::splitted_object_new_group_belong)();

	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_RCLICKMENU_SPLIT