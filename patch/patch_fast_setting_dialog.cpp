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

#include "patch_fast_setting_dialog.hpp"
#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG

#include <exedit.hpp>

//#define PATCH_STOPWATCH
#include "stopwatch.hpp"

namespace patch {

	static stopwatch_mem sw;

	void __cdecl fast_setting_dialog_t::FUN_10030500_Wrap() {
		sw.start();
		auto hwnd = load_i32<HWND>(GLOBAL::exedit_base + 0x1539c8);
		SendMessageA(hwnd, WM_SETREDRAW, FALSE, NULL);
		reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + 0x030500)();
		SendMessageA(hwnd, WM_SETREDRAW, TRUE, NULL);
		UpdateWindow(hwnd);
		sw.stop();
	}

	BOOL __cdecl fast_setting_dialog_t::FUN_100305e0_Wrap(int param1) {
		sw.start();
		auto hwnd = load_i32<HWND>(GLOBAL::exedit_base + 0x1539c8);
		SendMessageA(hwnd, WM_SETREDRAW, FALSE, NULL);
		auto ret = reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + 0x0305e0)(param1);
		SendMessageA(hwnd, WM_SETREDRAW, TRUE, NULL);
		UpdateWindow(hwnd);
		sw.stop();
		return ret;
	}


	void __cdecl fast_setting_dialog_t::FUN_10030500_Wrap2() {
		sw.start();
		auto hwnd = load_i32<HWND>(GLOBAL::exedit_base + 0x1539c8);
		SendMessageA(hwnd, WM_SETREDRAW, FALSE, NULL);
		reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + 0x030500)();
	}

	BOOL __cdecl fast_setting_dialog_t::FUN_100305e0_Wrap2(int param1) {
		auto hwnd = load_i32<HWND>(GLOBAL::exedit_base + 0x1539c8);
		auto ret = reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + 0x0305e0)(param1);
		SendMessageA(hwnd, WM_SETREDRAW, TRUE, NULL);
		UpdateWindow(hwnd);
		sw.stop();
		return ret;
	}

	void __cdecl fast_setting_dialog_t::FUN_10030500_Wrap3() {
		reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + 0x030500)();
	}

	void __cdecl fast_setting_dialog_t::FUN_1002bf10_Wrap(HDC hDC) {
		if (load_i32<ExEdit::Object*>(GLOBAL::exedit_base + 0x1e0fa4) == nullptr) return;
		if (load_i32<int>(GLOBAL::exedit_base + 0x177a10) == -1)return;
		reinterpret_cast<void(__cdecl*)(HDC)>(GLOBAL::exedit_base + 0x02bf10)(hDC);
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
