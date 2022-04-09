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
#ifdef PATCH_SWITCH_EXO_SCENEIDX
#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

namespace patch {
	// init at exedit load
	// オブジェクトファイルにシーン・シーン(音声)のシーン番号が正しく入出力されない
	inline void exo_sceneidx() {
		if (!PATCH_SWITCHER_MEMBER(PATCH_SWITCH_EXO_SCENEIDX))return;
		static const char name[] = "scene";
		store_i16(GLOBAL::exedit_base + OFS::ExEdit::efSceneAudio_exdatause_idx_type, 1);
		store_i32(GLOBAL::exedit_base + OFS::ExEdit::efSceneAudio_exdatause_idx_name, &name);
		store_i32(GLOBAL::exedit_base + OFS::ExEdit::efScene_exdatause_idx_name, &name);
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_SCENEIDX
