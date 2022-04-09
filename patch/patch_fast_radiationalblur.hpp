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
#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"

namespace patch::fast {
	// init at exedit load
	// 放射ブラーの高速化
	inline class RadiationalBlur_t {
		static BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

	public:
		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_FAST_RADIATIONALBLUR); }

		void operator()() {
			if (!enabled())return;
			store_i32(GLOBAL::exedit_base + OFS::ExEdit::efRadiationalBlur_func_proc_ptr, &func_proc);
		}
	} RadiationalBlur;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
