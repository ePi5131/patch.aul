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
#ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
#include <Windows.h>

#include "global.hpp"
#include "util_magic.hpp"

namespace patch {
	// call at v_func_WncProc WM_SIZING
	// 拡張編集ウィンドウの上部を正常にドラッグできるようにする
	inline BOOL exeditwindow_sizing(WPARAM wparam, LPARAM lparam) {
		if (!PATCH_SWITCHER_MEMBER(PATCH_SWITCH_EXEDITWINDOW_SIZING)) return -1;

		auto LayerHeightState = load_i32<int>(GLOBAL::exedit_base + 0x0a3e20);
		auto DAT_101a530c = load_i32<int>(GLOBAL::exedit_base + 0x1a530c);

		auto rect = (RECT*)lparam;
		if (rect->right < rect->left + 128) rect->right = rect->left + 128;

		auto top = rect->top;
		auto layer_count = (rect->bottom - rect->top - DAT_101a530c + LayerHeightState / 2) / LayerHeightState;

		bool is_top;
		switch (wparam) {
		case WMSZ_TOP:
		case WMSZ_TOPLEFT:
		case WMSZ_TOPRIGHT:
			is_top = true;
			break;
		default:
			is_top = false;
		}

		if (layer_count < 1) layer_count = 1;
		else if (100 < layer_count) layer_count = 100;

		if (is_top) {
			rect->top = rect->bottom - (DAT_101a530c + LayerHeightState * layer_count);
		}
		else {
			rect->bottom = rect->top + DAT_101a530c + LayerHeightState * layer_count;
		}
		return 0;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
