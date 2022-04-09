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
#ifdef PATCH_SWITCH_FAST_GETPUTPIXELDATA

#include <intrin.h>

#include "global.hpp"
#include "offset_address.hpp"
#include "mylua.hpp"
#include "util_magic.hpp"

namespace patch::fast {
	// init at ExEdit init
	// obj.getpixeldata/putpixeldata 他の動作を高速化
	// AVXを使用
	inline class getputpixeldata_t {
		static int avx_getpixeldata(lua_State* L);

		static int avx_putpixeldata(lua_State* L);

		static BOOL BufCpyBGRA2YCA(void* dst, void* src, int w, int h, int line);
		static BOOL BufCpyYCA2BGRA(void* dst, void* src, int w, int h, int line);

		union m128_256 {
			__m256 m256;
			__m128 m128;
		};

	public:
		void operator()() {
			OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::getpixeldata, 4).store_i32(0, &avx_getpixeldata);
		}
	} getputpixeldata;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_GETPUTPIXELDATA
