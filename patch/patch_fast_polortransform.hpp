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
#ifdef PATCH_SWITCH_FAST_POLORTRANSFORM

#include <aviutl.hpp>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"

namespace patch::fast {
	// init at exedit load
	// 極座標変換の高速化
	inline class PolorTransform_t {
		static BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

	public:
		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_FAST_POLORTRANSFORM); }

		void operator()() {
			if (!enabled())return;
			store_i32(GLOBAL::exedit_base + OFS::ExEdit::efPolorTransform_func_proc_ptr, &func_proc);
		}
	} PolorTransform;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_POLORTRANSFORM
