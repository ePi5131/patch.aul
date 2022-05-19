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

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "font_dialog";

	public:
		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;
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


		void switching(bool flag) {
			enabled = flag;
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}
	} font_dialog;
	
} // namespace patch
#endif // ifdef PATCH_SWITCH_FONT_DIALOG
