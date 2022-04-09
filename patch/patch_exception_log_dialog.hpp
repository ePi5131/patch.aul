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
#ifdef PATCH_SWITCH_EXCEPTION_LOG

#include <string>
#include <unordered_map>
#include <thread>

#include <Windows.h>

#include <CommCtrl.h>
#pragma comment(linker,"/manifestdependency:\"type='win32' \
  name='Microsoft.Windows.Common-Controls' \
  version='6.0.0.0' \
  processorArchitecture='x86' \
  publicKeyToken='6595b64144ccf1df' \
  language='*'\"") 
#pragma comment(lib, "comctl32.lib")

#include <aviutl.hpp>

#include "resource.h"
#include "global.hpp"

namespace patch {
	class exception_log_dialog_window_t {
	public:
		struct Param {
			std::wstring link; // SysLink用のコード文字列
			std::wstring info_dir; // 表示する文字列
			std::wstring info_path; // 飛ばすパス
			std::string detail; // 例外の詳細
			EXCEPTION_POINTERS* pExp;

			Param& operator=(const Param& x) {
				this->link = x.link;
				this->info_dir = x.info_dir;
				this->info_path = x.info_path;
				this->detail = x.detail;
				this->pExp = x.pExp;

				return *this;
			}
		};

		// ウィンドウを生成する
		exception_log_dialog_window_t(const Param* param);

		// コモンコントロール初期化など
		static bool init() {
			INITCOMMONCONTROLSEX icc{
				.dwSize = sizeof(INITCOMMONCONTROLSEX),
				.dwICC = ICC_LINK_CLASS
			};
			return InitCommonControlsEx(&icc);
		}

	private:
		HWND hwnd;

		bool detail_showed;
		HWND hwnd_detail_text;
		HWND hwnd_save_text;
		HWND hwnd_link;
		HWND hwnd_button_ok;
		HWND hwnd_button_detail;
		HWND hwnd_check_stopmes;
		HWND hwnd_button_save_project;
		DWORD resize_ofs;
		const Param* param;

		void update_window_detail(HWND hwnd, bool flag) {
			ShowWindow(hwnd_detail_text, flag ? SW_SHOW : SW_HIDE);

			int ofs = flag ? static_cast<int>(resize_ofs) : -static_cast<int>(resize_ofs);

			RECT rect;
			POINT point;
			GetWindowRect(hwnd, &rect);
			SetWindowPos(hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top + ofs, SWP_NOMOVE | SWP_NOZORDER);

			GetWindowRect(hwnd_button_ok, &rect);
			point = { rect.left, static_cast<LONG>(rect.top + ofs) };
			ScreenToClient(hwnd, &point);
			SetWindowPos(hwnd_button_ok, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			GetWindowRect(hwnd_button_detail, &rect);
			point = { rect.left, static_cast<LONG>(rect.top + ofs) };
			ScreenToClient(hwnd, &point);
			SetWindowPos(hwnd_button_detail, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			GetWindowRect(hwnd_check_stopmes, &rect);
			point = { rect.left, static_cast<LONG>(rect.top + ofs) };
			ScreenToClient(hwnd, &point);
			SetWindowPos(hwnd_check_stopmes, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			GetWindowRect(hwnd_button_save_project, &rect);
			point = { rect.left, static_cast<LONG>(rect.top + ofs) };
			ScreenToClient(hwnd, &point);
			SetWindowPos(hwnd_button_save_project, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			GetWindowRect(hwnd_save_text, &rect);
			point = { rect.left, static_cast<LONG>(rect.top + ofs) };
			ScreenToClient(hwnd, &point);
			SetWindowPos(hwnd_save_text, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		};

	private:
		constexpr static UINT UPDATE_PARAM = WM_APP + 1;

		static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	};

} // namespace patch
#endif // ifdef PATCH_SWITCH_EXCEPTION_LOG
