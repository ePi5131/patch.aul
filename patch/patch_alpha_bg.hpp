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
#ifdef PATCH_SWITCH_ALPHA_BG

#include <Windows.h>
#include <aviutl.hpp>

namespace patch {

	// アルファチャンネル部分に背景を表示する
	// 凍結: 普段からアルファチャンネル付きでのレンダリングをしなきゃならない 容易に操作できるスイッチを用意しなければならない
	inline class alpha_bg_t {

	public:
		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_ALPHA_BG); }

		static BOOL func_proc(AviUtl::FilterPlugin* fp, AviUtl::FilterProcInfo* fpip) {
			if (!enabled())return;

			const auto rect_size = 16;

			const auto pattern_size = rect_size * 2;
			if (!fp->exfunc->is_saving(fpip->editp)) {
				const auto w = fpip->w;
				const auto h = fpip->h;

				const auto ox = pattern_size - (w / 2) % pattern_size;
				const auto oy = pattern_size - (h / 2) % pattern_size;

				for (int y = 0; y < h; y++) {
					for (int x = 0; x < w; x++) {
						auto& p = *reinterpret_cast<AviUtl::PixelYC*>(reinterpret_cast<uintptr_t>(fpip->ycp_edit) + x * fpip->yc_size + y * fpip->line_size);
						if ((
							((x + ox) % pattern_size < rect_size) ^
							((y + oy) % pattern_size < rect_size)
							) == 1) {
							p = { 3584,0,0 };
						}
						else {
							p = { 4096,0,0 };
						}
					}
				}
			}
			return TRUE;
		}

	} alpha_bg;
} // namespace patch
#endif // ifdef PATCH_SWITCH_ALPHA_BG
