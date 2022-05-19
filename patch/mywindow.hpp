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
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "util_resource.hpp"

#include "patch_exo_trackminusval.hpp"

// playground
inline class mywindow_t {
	HMENU hmenu;
	HWND hwnd;

	inline static LRESULT CALLBACK wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
		if (message != WM_COMMAND) return DefWindowProc(hwnd, message, wparam, lparam);
		switch (LOWORD(wparam)) {
		case 10000: {
			patch::exo_trackminusval.switching(false);
			return FALSE;
		}
		case 10001: {
			patch::exo_trackminusval.switching(true);
			return FALSE;
		}
		}
		return DefWindowProc(hwnd, message, wparam, lparam);
	}

public:
#if _DEBUG
	~mywindow_t() {
		DestroyWindow(hwnd);
		DestroyMenu(hmenu);
	}

	void init() {
		hmenu = CreateMenu();
		InsertMenuA(hmenu, -1, MF_BYPOSITION | MF_STRING, 10000, "switch");
		InsertMenuA(hmenu, -1, MF_BYPOSITION | MF_STRING, 10001, "switch2");

		hwnd = CreateWindowA("STATIC", "patchhhhhhhh", 0, 0, 0, 1000, 500, NULL, hmenu, GLOBAL::patchaul_hinst, NULL);

		SetWindowLongA(hwnd, GWL_WNDPROC, (LONG)&wndproc);

		ShowWindow(hwnd, SW_SHOWNORMAL);
	}
#else
	void init() {}
#endif
}mywindow;
