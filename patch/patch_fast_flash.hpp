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
#ifdef PATCH_SWITCH_FAST_FLASH
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"

namespace patch::fast {
	// init at exedit load
	// 閃光の高速化
	inline class Flash_t {
		static BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

	public:

		struct efFlash_var { // 1a6b7c
			int r_intensity;
			int cx;
			int cy;
			int temp_w;
			int temp_h;
			short color_cb;
			short _padding1;
			int corrected_intensity;
			int pixel_range;
			short color_cr;
			short _padding2;
			int temp_x;
			int range;
			int temp_y;
			short color_y;
		};

		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_FAST_FLASH); }

		void operator()() {
			if (!enabled())return;
			store_i32(GLOBAL::exedit_base + OFS::ExEdit::efFlash_func_proc_ptr, &func_proc);

		}
	} Flash;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_FLASH
