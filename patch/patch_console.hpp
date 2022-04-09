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

namespace patch {
	// init at dllload
	// コンソール関連
	inline class console_t {
		inline static HANDLE h_stdout;
		inline static HWND console_hwnd;
		inline static bool is_valid;
	public:
		friend class override_debugstring_t;
		void exit() {
			writeConsole("bye");
			FreeConsole();
		}

		void operator()() {
			if (!PATCH_SWITCHER_MEMBER(PATCH_SWITCH_CONSOLE))return;
			if (AllocConsole() != TRUE) {
				patch_resource_message_w(PATCH_RS_PATCH_FAILED_TO_INIT_CONSOLE, MB_TASKMODAL | MB_ICONERROR);
				is_valid = false;
				return;
			}
			console_hwnd = GetConsoleWindow();

			if (auto rect = GLOBAL::config.console.rect; rect) {
				SetWindowPos(console_hwnd, NULL, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			update_showwindow();

			SetConsoleTitleA("Console Window");

			h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

			if (GLOBAL::config.switcher.console_escape) {
				DWORD consoleMode;
				GetConsoleMode(h_stdout, &consoleMode);
				SetConsoleMode(h_stdout, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
			}

			FILE* fp;
			if (GLOBAL::config.switcher.console_input) {
				freopen_s(&fp, "CONIN$", "r", stdin);
			}
			freopen_s(&fp, "CONOUT$", "w", stderr);
			freopen_s(&fp, "CONOUT$", "w", stdout);

			setConsoleTextAttribute(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
			
			writeConsole("patch.aul\n");
			setConsoleTextAttribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

			is_valid = true;

		}
		HWND get_console_hwnd() const { return console_hwnd; }

		bool valid() const {
			return is_valid;
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
			SetWindowPos(console_hwnd, HWND_NOTOPMOST, NULL, NULL, NULL, NULL, SWP_NOSIZE | SWP_NOMOVE);
		}

		void update_visible_state(HMENU aviutl_hmwnu_disp) const {
			modify_menuitem_check(aviutl_hmwnu_disp, 20002, FALSE, [](bool state) -> bool {
				if (GLOBAL::config.switcher.console_input) {
					GLOBAL::config.console.visible = !IsIconic(console_hwnd);
				}
				return GLOBAL::config.console.visible;
			});
		}

		void update_showwindow() {
			bool& console_flag = GLOBAL::config.console.visible;
			if (GLOBAL::config.switcher.console_input) {
				if (console_flag) {
					showWindow(SW_RESTORE);
				}
				else {
					console_get_setting_rect(GLOBAL::config.console.rect);
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

		// コンソールの表示 メニューが押されたときに呼ばれる関数
		void menu_console(HMENU aviutl_hmwnu_disp) {
			if (!GLOBAL::config.switcher.console) {
				patch_resource_message_w(PATCH_RS_PATCH_CONSOLE_IS_DIABLED, MB_TOPMOST | MB_ICONINFORMATION);
				return;
			}
			bool& console_flag = GLOBAL::config.console.visible;
			if (GetKeyState(VK_LCONTROL) & 0x8000 | GetKeyState(VK_RCONTROL) & 0x8000) {
				console_flag = true;
			}
			else {
				console_flag = !console_flag;
			}

			modify_menuitem_check(aviutl_hmwnu_disp, 20002, FALSE, [console_flag](bool state) {
				return console_flag;
			});

			update_showwindow();
		}
		
	} console;
} // namespace patch


#endif // ifdef PATCH_SWITCH_CONSOLE
