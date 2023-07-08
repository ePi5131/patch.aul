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

#include "patch_setting_dialog_wndproc_override.hpp"

#include "patch_setting_dialog_move.hpp"
#include "patch_script_sort.hpp"

namespace patch {
	LRESULT CALLBACK setting_dialog_t::wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
		switch (message) {
		case WM_MOVE:
			#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
				setting_dialog_move(hwnd);
			#endif
			break;
		case WM_COMMAND:
			#ifdef PATCH_SWITCH_SCRIPT_SORT_PATCH
			if ((wparam & 0xffff) == 2079) {
				int ret = wndproc_orig(hwnd, message, wparam, lparam);
				patch_script_sort(hwnd);
				return ret;
			}
			#endif
			break;
		}
		return wndproc_orig(hwnd, message, wparam, lparam);
	}
}
