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

#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
#include "global.hpp"
#include "util_magic.hpp"
#include "global.hpp"

namespace patch {
	// init at exedit load
	// 設定ダイアログの高速化
	inline class fast_setting_dialog_t {
		static void __cdecl FUN_10030500_Wrap();
		static BOOL __cdecl FUN_100305e0_Wrap(int param1);

		static void __cdecl FUN_10030500_Wrap2();
		static BOOL __cdecl FUN_100305e0_Wrap2(int param1);

		static void __cdecl FUN_10030500_Wrap3();

		static void __cdecl FUN_1002bf10_Wrap(HDC hDC);
	public:

		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_FAST_SETTINGDIALOG); }

		void operator()() {
			if (!enabled()) return;

			// 単体呼び出し
			ReplaceNearJmp(GLOBAL::exedit_base + 0x0417c8, &FUN_10030500_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x04194a, &FUN_10030500_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x0419eb, &FUN_10030500_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x041b4f, &FUN_10030500_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x041c65, &FUN_10030500_Wrap);

			ReplaceNearJmp(GLOBAL::exedit_base + 0x02cd04, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x02da5d, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x03957d, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x0417d3, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x041956, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x0419f7, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x041b5a, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x041c71, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x041f52, &FUN_100305e0_Wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x08d75b, &FUN_100305e0_Wrap);
			
			// 30500の直後に305e0を呼ぶとき
			ReplaceNearJmp(GLOBAL::exedit_base + 0x041ade, &FUN_10030500_Wrap2);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x041ae9, &FUN_100305e0_Wrap2);

			// 305e0中での30500の呼び出し
			//ReplaceNearJmp(GLOBAL::exedit_base + 0x0306ac, &FUN_10030500_Wrap3);

			// 2bf2c 例外対策
			ReplaceNearJmp(GLOBAL::exedit_base + 0x02ceb7, &FUN_1002bf10_Wrap);
			// テキスト の行間等のコントロールの描画がおかしくなる
			//OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x02e881, 4).store_i32(0, WS_EX_TOOLWINDOW | WS_EX_COMPOSITED);
		}

	} fast_setting_dialog;
} // namespace patch
#endif // ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
