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

#include "patch_failed_longer_path.hpp"
#ifdef PATCH_SWITCH_FAILED_LONGER_PATH


namespace patch {
	BOOL __cdecl failed_longer_path_t::dlg_get_load_name_wrap(AviUtl::ExFunc* a_exfunc, LPSTR name, LPSTR filter, LPSTR def) {
		char temp[264];
		if (!a_exfunc->dlg_get_load_name(temp, filter, def))return FALSE;
		if (258 < lstrlenA(temp)) {
			HWND* settingdialog_hwnd = (HWND*)(GLOBAL::exedit_base + OFS::ExEdit::settingdialog_hwnd);
			MessageBoxA(*settingdialog_hwnd, str_new_longer_msg, temp, MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);
			lstrcpyA(name, def);
			return FALSE;
		}
		lstrcpyA(name, temp);
		return TRUE;
	}

	UINT __stdcall failed_longer_path_t::DragQueryFileA_exedit_wrap(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch) {
		UINT path_len = DragQueryFileA(hDrop, iFile, lpszFile, cch);
		if (258 < path_len) {
			auto exedit_hwnd = (HWND*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd);
			MessageBoxA(*exedit_hwnd, str_new_longer_msg, lpszFile, MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);
			return 0;
		}
		return path_len;
	}

	UINT __stdcall failed_longer_path_t::DragQueryFileA_settingdialog_wrap(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch) {
		UINT path_len = DragQueryFileA(hDrop, iFile, lpszFile, cch);
		if (258 < path_len) {
			auto settingdialog_hwnd = (HWND*)(GLOBAL::exedit_base + OFS::ExEdit::settingdialog_hwnd);
			MessageBoxA(*settingdialog_hwnd, str_new_longer_msg, lpszFile, MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);
			return 0;
		}
		return path_len;
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_FAILED_LONGER_PATH
