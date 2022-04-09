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

#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

namespace patch {
	// init at exedit load
	// オブジェクトファイルからの入力で，(-1,0)の値の正負が逆転する
	inline class exo_trackminusval_t {
		static int __cdecl exo_trackminusval_insteadfunc(char** strp, ExEdit::Filter* efp, int track_idx);
	public:
		void operator()() {
			if (!PATCH_SWITCHER_MEMBER(PATCH_SWITCH_EXO_TRACK_MINUSVAL)) return;
			ReplaceFunction(GLOBAL::exedit_base + OFS::ExEdit::exo_readtrack, exo_trackminusval_insteadfunc);
		}
	} exo_trackminusval;
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
