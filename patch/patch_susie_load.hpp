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
#ifdef PATCH_SWITCH_SUSIE_LOAD

#include <exedit.hpp>

#include "util.hpp"
#include "global.hpp"
#include "offset_address.hpp"

namespace patch {
	// init at exedit load
	// Susieのプラグインで正しく対応拡張子情報を取得できない
	inline class susie_load_t {
		static void __cdecl LoadSpi(LPCSTR dir);

		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_SUSIE_LOAD); }
	public:
		void operator()() const {
			if (!enabled())return;
			ReplaceFunction(GLOBAL::exedit_base + OFS::ExEdit::LoadSpi, &LoadSpi);
		}
	}susie_load;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SUSIE_LOAD
