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

			auto clipping_and_resize_fp = (AviUtl::FilterPlugin*)(GLOBAL::aviutl_base + OFS::AviUtl::filter_clipping_and_resize_ptr);
			if (*(int8_t*)&clipping_and_resize_fp->flag & (int8_t)AviUtl::FilterPlugin::Flag::Active) { // クリッピング&リサイズが有効
				SendMessageA(*aviutl_hwnd, WM_COMMAND, 10008, 0); // フィルタ>クリッピング&リサイズ
			}

			/*
			以下2つに関して参考として
			track_array[0] : 選択されているもの
			track_array[1] : メニューの開始ID
			track_array[2] : メニューの最終ID
			*/
			auto change_size_fp = (AviUtl::FilterPlugin*)(GLOBAL::aviutl_base + OFS::AviUtl::filter_change_size_ptr);
			if (change_size_fp->track_array[0] != 0) { // なし以外
				SendMessageA(*aviutl_hwnd, WM_COMMAND, change_size_fp->track_array[1], 0);
			}
			
			auto change_framerate_fp = (AviUtl::FilterPlugin*)(GLOBAL::aviutl_base + OFS::AviUtl::filter_change_framerate_ptr);
			if (change_framerate_fp->track_array[0] != 0) { // なし以外
				SendMessageA(*aviutl_hwnd, WM_COMMAND, change_framerate_fp->track_array[1], 0);
			}
			return TRUE;
		}
		return FALSE;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTING_NEW_PROJECT
