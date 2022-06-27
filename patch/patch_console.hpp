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
#ifdef PATCH_SWITCH_CONSOLE

#include <cstdio>
#include <iostream>

#include <Windows.h>
#include <oleacc.h>

#include "global.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at dllload
	// コンソール関連
	inline class console_t {
		HANDLE h_stdout;
		HWND console_hwnd;
		bool valid;

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "console";

		inline static const char key_escape[] = "console.escape";
		inline static const char key_input[] = "console.input";
		inline static const char key_debug_string[] = "console.debug_string";
		inline static const char key_debug_string_time[] = "console.debug_string.time";

		bool escape = true;
		bool input = false;
		bool debug_string = true;
		bool debug_string_time = true;

		inline static const char key_visible[] = "visible";
		inline static const char key_rect[] = "rect";

		bool visible = false;
		std::optional<RECT> rect;

		static void __stdcall debug_print_override(LPCSTR lpOutputString);
		static void __stdcall exedit_lua_error_override(LPCSTR lpOutputString);

		inline static constexpr auto* debug_print_override_ptr = &debug_print_override;
		inline static constexpr auto* exedit_lua_error_override_ptr = &exedit_lua_error_override;


	public:
		friend class aviutl_wndproc_override_t;
		void exit() {
			writeConsole("bye");
			FreeConsole();
		}

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			if (AllocConsole() != TRUE) {
				patch_resource_message_w(PATCH_RS_PATCH_FAILED_TO_INIT_CONSOLE, MB_TASKMODAL | MB_ICONERROR);
				valid = false;
				return;
			}
			console_hwnd = GetConsoleWindow();

			if (rect) {
				SetWindowPos(console_hwnd, NULL, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			update_showwindow();

			SetConsoleTitleA("Console Window");

			h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

			if (escape) {
				DWORD consoleMode;
				GetConsoleMode(h_stdout, &consoleMode);
				SetConsoleMode(h_stdout, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
			}

			FILE* fp;
			if (input) {
				freopen_s(&fp, "CONIN$", "r", stdin);
			}
			freopen_s(&fp, "CONOUT$", "w", stderr);
			freopen_s(&fp, "CONOUT$", "w", stdout);

			setConsoleTextAttribute(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
			
			writeConsole("patch.aul\n");
			setConsoleTextAttribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

			valid = true;
		}

		void init_at_exedit_init() {
#ifdef PATCH_SWITCH_DEBUGSTRING
			if (debug_string) {
				OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::OutputDebugString_calling_err1, 4).store_i32(0, &exedit_lua_error_override_ptr);
				OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::OutputDebugString_calling_err2, 4).store_i32(0, &exedit_lua_error_override_ptr);
				OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::OutputDebugString_calling_err3, 4).store_i32(0, &exedit_lua_error_override_ptr);
				OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::OutputDebugString_calling_dbg, 4).store_i32(0, &debug_print_override_ptr);
			}
#endif
		}

		void switching(bool flag) {
			enabled = flag;
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }

		void config_load(ConfigReader& cr) {
			cr.regist(key_visible, [this](json_value_s* value) { ConfigReader::load_variable(value, visible); });
			cr.regist(key_rect, [this](json_value_s* value) { ConfigReader::load_variable(value, rect); });
		}

		void config_store(ConfigWriter& cw) {
			cw.append(key_visible, visible);
			
			if (!IsIconic(console_hwnd)) {
				RECT rect;
				if (GetWindowRect(console_hwnd, &rect) == TRUE) {
					cw.append(key_rect, rect);
				}
			}
		}

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
			cr.regist(key_escape, [this](json_value_s* value) {
				ConfigReader::load_variable(value, escape);
			});
			cr.regist(key_input, [this](json_value_s* value) {
				ConfigReader::load_variable(value, input);
			});
			cr.regist(key_debug_string, [this](json_value_s* value) {
				ConfigReader::load_variable(value, debug_string);
			});
			cr.regist(key_debug_string_time, [this](json_value_s* value) {
				ConfigReader::load_variable(value, debug_string_time);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
			cw.append(key_escape, escape);
			cw.append(key_input, input);
			cw.append(key_debug_string, debug_string);
			cw.append(key_debug_string_time, debug_string_time);
		}


		bool is_valid() const {
			return valid;
		}

		// コンソールの表示 メニューが押されたときに呼ばれる関数
		void menu_console(HMENU aviutl_hmwnu_disp) {
			if (!enabled_i) {
				patch_resource_message_w(PATCH_RS_PATCH_CONSOLE_IS_DIABLED, MB_TOPMOST | MB_ICONINFORMATION);
				return;
			}
			bool& console_flag = visible;
			if ((GetKeyState(VK_LCONTROL) & 0x8000) | (GetKeyState(VK_RCONTROL) & 0x8000)) {
				console_flag = true;
			}
			else {
				console_flag = !console_flag;
			}

			modify_menuitem_check(aviutl_hmwnu_disp, 20002, FALSE, [console_flag](bool) {
				return console_flag;
			});

			update_showwindow();
		}

		DWORD writeConsole(std::string_view str) {
			DWORD written;
			WriteConsoleA(h_stdout, str.data(), str.length(), &written, NULL);
			return written;
		}

		void setConsoleTextAttribute(WORD wAttributes) {
			SetConsoleTextAttribute(h_stdout, wAttributes);
		}

		void showWindow(int nCmdShow) {
			ShowWindow(console_hwnd, nCmdShow);
		}

		// コンソールウィンドウを前面に移動させる
		void front() const {
			SetWindowPos(console_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}

		void update_visible_state(HMENU aviutl_hmwnu_disp) {
			modify_menuitem_check(aviutl_hmwnu_disp, 20002, FALSE, [this](bool) -> bool {
				if (input) {
					visible = !IsIconic(console_hwnd);
				}
				return visible;
			});
		}

		void console_get_setting_rect(std::optional<RECT>& rect) const {
			if (!IsIconic(console_hwnd)) {
				rect = RECT{};
				if (GetWindowRect(console_hwnd, &rect.value()) == FALSE) rect = std::nullopt;
			}
		}

		void update_showwindow() {
			bool& console_flag = visible;
			if (input) {
				if (console_flag) {
					showWindow(SW_RESTORE);
				}
				else {
					console_get_setting_rect(rect);
					showWindow(SW_MINIMIZE);
				}
			}
			else {
				if (console_flag) {
					showWindow(SW_RESTORE);
					showWindow(SW_SHOW);
				}
				else {
					showWindow(SW_HIDE);
				}
			}
		}

	} console;
} // namespace patch


#endif // ifdef PATCH_SWITCH_CONSOLE
