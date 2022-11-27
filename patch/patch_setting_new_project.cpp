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

#include "patch_setting_new_project.hpp"

#ifdef PATCH_SWITCH_SETTING_NEW_PROJECT
namespace patch {

	BOOL __cdecl setting_new_project_t::exedit_edit_open_wrap(int w, int h, int video_rate, int video_scale, int audio_rate, HWND hwnd, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp) {
		if (reinterpret_cast<BOOL(__cdecl*)(int, int, int, int, int, HWND, AviUtl::EditHandle*, AviUtl::FilterPlugin*)>(GLOBAL::exedit_base + OFS::ExEdit::exedit_edit_open)(w, h, video_rate, video_scale, audio_rate, hwnd, editp, fp)) {
			HWND* aviutl_hwnd = (HWND*)(GLOBAL::exedit_base + OFS::ExEdit::aviutl_hwnd);
			if (*(byte*)(GLOBAL::aviutl_base + 0x7ad58) & 1) { // クリッピング&リサイズが有効
				SendMessageA(*aviutl_hwnd, WM_COMMAND, 10008, 0); // フィルタ>クリッピング&リサイズ
			}
			SendMessageA(*aviutl_hwnd, WM_COMMAND, 12100, 0); // 設定>サイズの変更>なし
			SendMessageA(*aviutl_hwnd, WM_COMMAND, 12107, 0); // 設定>フレームレートの変更>なし
			return TRUE;
		}
		return FALSE;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTING_NEW_PROJECT