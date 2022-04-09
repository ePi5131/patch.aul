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

#include <aviutl.hpp>

#include "macro.h"

#include "cryptostring.hpp"

#include "global.hpp"
#include "offset_address.hpp"

#include "version.hpp"
#include "util.hpp"

#include "patch_console.hpp"

namespace patch {
	// init before filter plugin load
    // AviUtl のメインウィンドウのWndProcのフック
	inline class aviutl_wndproc_override_t {
		inline static HMENU aviutl_hmenu; // menu parent
		inline static HMENU aviutl_hmwnu_disp; // menu 表示

		//inline static HWND console_parent_hwnd_org;

		inline static WNDPROC aviutl_wndproc_orig;

		friend class console_t;

		static LRESULT CALLBACK wrap(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	public:
		inline void go() {
			
			auto hwnd = load_i32<AviUtl::EditHandle*>(GLOBAL::aviutl_base + OFS::AviUtl::edit_handle_ptr)->aviutl_window_info.main_window;

			//console_parent_hwnd_org = (HWND)SetWindowLongA(console.get_console_hwnd(), GWL_HWNDPARENT, (LONG)hwnd);
			aviutl_wndproc_orig = (WNDPROC)SetWindowLongA(hwnd, GWL_WNDPROC, (LONG)wrap);

#ifdef PATCH_SWITCH_CONSOLE
			if (!GLOBAL::config.console.visible) console.showWindow(SW_MINIMIZE);
#endif

			aviutl_hmenu = GetMenu(hwnd);
			aviutl_hmwnu_disp = GetSubMenu(aviutl_hmenu, 5);

		}

	} aviutl_wndproc_override;
} // namespace patch
