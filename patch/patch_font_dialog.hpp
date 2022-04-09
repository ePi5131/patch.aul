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
#ifdef PATCH_SWITCH_FONT_DIALOG

#include <Windows.h>
#include "global.hpp"

namespace patch {
	// init at exedit load
	// フォント選択ダイアログを表示
	// コンボボックスの右クリで適当なダイアログを召喚する
	inline class font_dialog_t {

		//inline static constexpr LPCWSTR class_name = L"PATCH_FONTDLG";

	public:
		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_FONT_DIALOG); }

		void operator()() {
			if (!enabled())return;
			/*WNDCLASSW wc{
				.style = 0,
				.lpfnWndProc = nullptr,
				.cbClsExtra = 0,
				.cbWndExtra = 0,
				.hInstance = GLOBAL::patchaul_hinst,
				.hIcon = NULL,
				.hCursor = NULL,
				.hbrBackground = HBRUSH(COLOR_BACKGROUND + 1),
				.lpszMenuName = NULL,
				.lpszClassName = class_name,
			};
			RegisterClassW(&wc);
			*/
			


		}

		~font_dialog_t() {
			//UnregisterClassW(class_name, GLOBAL::patchaul_hinst);
		}

	} font_dialog;
	
} // namespace patch
#endif // ifdef PATCH_SWITCH_FONT_DIALOG
