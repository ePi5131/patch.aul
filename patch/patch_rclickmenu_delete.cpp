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

#include "patch_rclickmenu_delete.hpp"

#ifdef PATCH_SWITCH_RCLICKMENU_DELETE
namespace patch {

	void __cdecl rclickmenu_delete_t::disp_near_settingdialog_wrap3fef6() {
		if (0 <= *settingdialog_object_idx_ptr) {
			int object_idx = *settingdialog_object_idx_ptr;
			if (*selecting_obj_num_ptr) {
				object_idx = selecting_obj_list[0];
			}
			last_id = reinterpret_cast<int(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::get_near_object_idx)(object_idx, 1);
		}
	}
	void __cdecl rclickmenu_delete_t::DrawTimelineMask_wrap3ff68(int* to_draw) {
		if (0 <= last_id) {
			reinterpret_cast<void(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::disp_settingdialog)(last_id);
		}
		last_id = -1;
		reinterpret_cast<void(__cdecl*)(int*)>(GLOBAL::exedit_base + OFS::ExEdit::DrawTimelineMask)(to_draw);
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_RCLICKMENU_DELETE