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
#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV

#include "global.hpp"
#include "offset_address.hpp"
#include "util_magic.hpp"

namespace patch {
	// init at exedit load
	// 特定色域変換の拡張データ領域にstatusが2つある問題
	inline class exo_specialcolorconv_t {
		inline static const char status2_name[] = "status2";

	public:
		void operator()() {
			if (!PATCH_SWITCHER_MEMBER(PATCH_SWITCH_EXO_SPECIALCOLORCONV)) return;
			OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::specialcolorconv_status2, 4).store_i32(0, &status2_name);
		}
	} exo_specialcolorconv;
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
