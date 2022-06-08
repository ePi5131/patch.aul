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
#ifdef PATCH_SWITCH_FAST_BORDER

#include <aviutl.hpp>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 縁取りのバグ修正＆ちょっとだけ高速化
	inline class Border_t {
		static BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.border";

	public:

		struct efBorder_var { // 1b1e30
			unsigned short* ExEditMemory;
			int add_size;
			int inv_range;
			int shift_x;
			int shift_y;
			int _undefined;
			char _exdata_def[260]; // 1b1e48 - 1b1f4b
			short color_cb; // 1b1f4c
			short color_cr;
			short color_y;
			short _alpha_shift;// short _padding;
			int alpha;
		};


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			store_i32(GLOBAL::exedit_base + OFS::ExEdit::efBorder_func_proc_var_ptr, &func_proc);
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

	} Border;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_BORDER
