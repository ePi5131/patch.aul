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
#ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
#include "global.hpp"
#include "util_magic.hpp"
namespace patch {
	// init at exedit load
	// 動画ファイル と 音声ファイル で中間点を打っていないときでもファイルを再参照しても再生位置などの情報を変更しない
	inline void ignore_media_param_reset() {
		if (!PATCH_SWITCHER_MEMBER(PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET))return;

		OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::ignore_media_param_reset_mov, 2).store_i16(0, '\x90\xe9');
		OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::ignore_media_param_reset_aud, 1).store_i8(0, '\xeb');
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
