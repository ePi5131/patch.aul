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

#include "global.hpp"
#include "util_magic.hpp"

#include "patch_setting_dialog_move.hpp"

namespace patch {

	inline class setting_dialog_t {
		inline static WNDPROC wndproc_orig;
		static LRESULT wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
			switch (message) {
				case WM_MOVE:
					#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
						setting_dialog_move(hwnd);
					#endif
					break;
			}
			return wndproc_orig(hwnd, message, wparam, lparam);
		}

	public:
		void operator()() {
			wndproc_orig = reinterpret_cast<decltype(wndproc_orig)>(GLOBAL::exedit_base + 0x02cde0);
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x02e804, 4);
			h.store_i32(0, &wndproc);
		}
	} setting_dialog;
}
