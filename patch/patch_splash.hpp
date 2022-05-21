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
#ifdef PATCH_SWITCH_SPLASH

#include <memory>
#include <atomic>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <Windows.h>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include "global.hpp"
#include "util.hpp"
#include "debug_log.hpp"
#include "config_rw.hpp"

namespace patch {

	void __cdecl InitEnd();

	// init at dllload
	// スプラッシュウィンドウ
	inline class splash_t {
		
		inline static std::thread thread;
		inline static std::mutex mutex;

		inline static struct {
			std::wstring phase;
			std::wstring detail;
		} state;

		__declspec(align(4)) inline static std::atomic<bool> load_finished{false};
		__declspec(align(4)) inline static std::atomic<bool> update_state{false};

		inline static const char classname[] = "patchaul_splash";

		static LRESULT CALLBACK SplashWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		inline static const char key[] = "splash";

		bool enabled = false;
		bool enabled_i;

	public:
		void init() {
			enabled_i = enabled;

			WNDCLASSA winc{
				.style = 0,
				.lpfnWndProc = SplashWndProc,
				.cbClsExtra = 0,
				.cbWndExtra = 0,
				.hInstance = GLOBAL::patchaul_hinst,
				.hIcon = LoadIconA(NULL, IDI_APPLICATION),
				.hCursor = LoadCursorA(NULL, IDC_ARROW),
				.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
				.lpszMenuName = NULL,
				.lpszClassName = classname
			};
			RegisterClassA(&winc);

			{
				// 0x02e70 の Init の ret にスプラッシュウィンドウを閉じる処理を仕込む

				OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x041af, 0x21);
				store_i16(GLOBAL::aviutl_base + 0x041af, '\xeb\x11'); // jmp near +0x11

				store_i16(GLOBAL::aviutl_base + 0x041c2, '\x50\xb9'); // push eax | mov ecx, (i32)
				store_i32(GLOBAL::aviutl_base + 0x041c4, &InitEnd);

				store_i16(GLOBAL::aviutl_base + 0x041c8, '\xff\xd1'); // call ecx

				store_i32(GLOBAL::aviutl_base + 0x041ca, '\x58\x8b\xe5\x5d'); // pop eax | mov esp, ebp | pop ebp
				store_i8(GLOBAL::aviutl_base + 0x041ce, '\xc3'); // ret
			}
		}
		~splash_t() {
			finish();
			
			UnregisterClassA(classname, GLOBAL::patchaul_hinst);
		}


		void switching(bool flag) {
			enabled = flag;
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled; }
		
		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}

		void start() {
			finish();

			thread = std::thread([] {
				//WS_EX_LAYERED
				auto hwnd = CreateWindowExA(0, classname, "", WS_VISIBLE, 0, 0, 100, 100, NULL, NULL, NULL, NULL);

				MSG message;
				while (true) {
					if (PeekMessageA(&message, NULL, 0, 0, PM_NOREMOVE)) {
						if (message.message == WM_QUIT)break;
						LRESULT result = GetMessageA(&message, NULL, 0, 0);
						if (result <= 0)break;
						DispatchMessageA(&message);
					}
					else {
						Sleep(50);
					}
					if (update_state.load()) {
						update_state.store(false);
						InvalidateRect(hwnd, NULL, FALSE);
						auto updated = UpdateWindow(hwnd);
						//debug_log(updated);
					}
					if (load_finished.load())
						DestroyWindow(hwnd);
				}
			});
		}
		void finish() {
			if (thread.joinable()) {
				load_finished.store(true);
				thread.join();
				load_finished.store(false);
			}
		}
		void set_phase(std::wstring_view phase, std::wstring_view detail) {
			{
				std::lock_guard lock(mutex);
				state.phase = phase;
				state.detail = detail;
			}
			update_state.store(true);
		}
		void set_detail(std::wstring_view detail) {
			{
				std::lock_guard lock(mutex);
				state.detail = detail;
			}
			update_state.store(true);
		}

	} splash;
	
} // namespace patch
#endif // ifdef PATCH_SWITCH_SPLASH
