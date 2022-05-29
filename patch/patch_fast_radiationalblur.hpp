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
#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 放射ブラーの高速化
	inline class RadiationalBlur_t {

		static BOOL media_mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL filter_mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL filter_mt_far_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.radiationalblur";
	public:
		struct efRadiationalBlur_var { // d75a8
			int blur_cx; // ブラーの中心位置
			int blur_cy;
			int w;
			int h;
			int pixel_range;
			int obj_cx;
			int range;
			int obj_cy;
		};

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;
			{   // Media object
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00b5c1, 6);
				h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
				h.replaceNearJmp(2, &media_mt_func);
			}

			{   // Filter object
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00bb39, 6);
				h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
				h.replaceNearJmp(2, &filter_mt_func);
			}
			{   // Filter object
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00bb4f, 6);
				h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
				h.replaceNearJmp(2, &filter_mt_far_func);
			}

			{   // SceneChange
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00c1ec, 6);
				h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
				h.replaceNearJmp(2, &filter_mt_func);
			}

		}

		void switching(bool flag) { enabled = flag; }

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }
		
        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
            });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }

	} RadiationalBlur;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
