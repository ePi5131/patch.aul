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

#include "patch_change_disp_scene.hpp"

#ifdef PATCH_SWITCH_CHANGE_DISP_SCENE
namespace patch {

	void update_title() {
		char title[64];
		int frame = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::frame_cursor);
		int frame_n = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::frame_n);
		frame = std::clamp(frame, 0, frame_n - 1);

		double framerate = *(double*)(GLOBAL::exedit_base + OFS::ExEdit::double_framerate);
		double framerate_scale = *(double*)(GLOBAL::exedit_base + OFS::ExEdit::double_framerate_scale);
		int centisecond = (int)(100.0 / framerate * (double)frame * framerate_scale);
		char* tl_title = (char*)(GLOBAL::exedit_base + OFS::ExEdit::tl_title);
		wsprintfA(title, tl_title, centisecond / 360000, (centisecond % 360000) / 6000, (centisecond % 6000) / 100, centisecond % 100, frame + 1, frame_n);

		SetWindowTextA(*(HWND*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd), title);
	}

	int __cdecl change_disp_scene_t::set_frame_wrap(AviUtl::EditHandle* editp, int frame) {

		int* ini_shiftselect = (int*)(GLOBAL::aviutl_base + OFS::AviUtl::ini_shiftselect);
		int tmp_shiftselect = *ini_shiftselect;
		*ini_shiftselect = 0;

		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
		int ret = a_exfunc->set_frame(editp, frame);

		update_title();

		*ini_shiftselect = tmp_shiftselect;

		return ret;
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_CHANGE_DISP_SCENE
