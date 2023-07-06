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

#include "patch_rendering.hpp"

#ifdef PATCH_SWITCH_RENDERING
namespace patch {


	int __stdcall rendering_t::mid_render() {
		int fast_process = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::fast_process);
		if (fast_process) {
			int* rendering_data = (int*)(GLOBAL::exedit_base + OFS::ExEdit::rendering_data);
			int j = 10;
			for (int i = 0; i < 4096; i++) {
				rendering_data[j] = 0;
				rendering_data[j + 1] = 0;
				j += 17;
			}
		}
		int* rendering_data_count = (int*)(GLOBAL::exedit_base + OFS::ExEdit::rendering_data_count);
		*rendering_data_count = 4096;
		reinterpret_cast<void(__cdecl*)(int, BOOL)>(GLOBAL::exedit_base + OFS::ExEdit::do_multi_thread_func)(GLOBAL::exedit_base + OFS::ExEdit::rendering_mt_func, TRUE);
		return 0; // rendering_data_countをいくつにするか
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_RENDERING
