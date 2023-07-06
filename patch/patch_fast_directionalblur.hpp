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
#ifdef PATCH_SWITCH_FAST_DIRECTIONALBLUR
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 方向ブラーの高速化
	inline class DirectionalBlur_t {
		static BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl filter_mt_wrap00cae8(AviUtl::MultiThreadFunc func, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.directionalblur";
	public:

		struct efDirectionalBlur_var { // d75cc
			int x_step; 
			int y_step;
			int x_end;
			int y_end;
			int x_begin;
			int range;
			int y_begin;
		};


		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			store_i32(GLOBAL::exedit_base + OFS::ExEdit::efDirectionalBlur_func_proc_ptr, &func_proc);

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::efDirectionalBlur_Filter_mt_func_call, 6);
			h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
			h.replaceNearJmp(2, &filter_mt_wrap00cae8);


		}

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}

	} DirectionalBlur;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_DIRECTIONALBLUR
