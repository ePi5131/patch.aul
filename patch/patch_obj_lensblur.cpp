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

#include "patch_obj_lensblur.hpp"

#ifdef PATCH_SWITCH_OBJ_LENSBLUR
namespace patch {
	void* __cdecl obj_LensBlur_t::efLensBlur_resize_709a0_wrap_12809(void* pix_edit, int w0, int h0, int w1, int h1, void* pix_temp) {
		if (w0 && h0 && w1 && h1) {
			return reinterpret_cast<void* (__cdecl*)(void*, int, int, int, int, void*)>(GLOBAL::exedit_base + 0x0709a0)(pix_edit, w0, h0, w1, h1, pix_temp);
		}
		return pix_edit;
	}

	void* __cdecl obj_LensBlur_t::efLensBlur_resize_71420_wrap_126a6(void* pix_edit, int w0, int h0, int w1, int h1, void* pix_temp) {
		if (w0 && h0 && w1 && h1) {
			return reinterpret_cast<void* (__cdecl*)(void*, int, int, int, int, void*)>(GLOBAL::exedit_base + 0x071420)(pix_edit, w0, h0, w1, h1, pix_temp);
		}
		return pix_edit;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_LENSBLUR
