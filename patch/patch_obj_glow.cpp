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

#include "patch_obj_glow.hpp"

#ifdef PATCH_SWITCH_OBJ_GLOW
namespace patch {

	void __cdecl obj_Glow_t::lower_right_convolution1_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		thread_num *= 2;
		int func_ptr = GLOBAL::exedit_base + 0x570d0;
		reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_id, thread_num, efp, efpip);
		reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_num - thread_id - 1, thread_num, efp, efpip);
	}
	void __cdecl obj_Glow_t::lower_right_convolution2_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		thread_num *= 2;
		int func_ptr = GLOBAL::exedit_base + 0x57730;
		reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_id, thread_num, efp, efpip);
		reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_num - thread_id - 1, thread_num, efp, efpip);
	}
	void __cdecl obj_Glow_t::lower_left_convolution1_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		thread_num *= 2;
		int func_ptr = GLOBAL::exedit_base + 0x57d90;
		reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_id, thread_num, efp, efpip);
		reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_num - thread_id - 1, thread_num, efp, efpip);
	}
	void __cdecl obj_Glow_t::lower_left_convolution2_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		thread_num *= 2;
		int func_ptr = GLOBAL::exedit_base + 0x58430;
		reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_id, thread_num, efp, efpip);
		reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_num - thread_id - 1, thread_num, efp, efpip);
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_GLOW
