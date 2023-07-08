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
#ifdef PATCH_SWITCH_FAST_DISPLACEMENTMAP

#include <aviutl.hpp>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// ディスプレイスメントマップの高速化
	inline class DisplacementMap_t {
		static BOOL mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		inline static const char* cl_func_name[3] = { "DisplacementMap_move","DisplacementMap_zoom","DisplacementMap_rot" };

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.displacementmap";

	public:

		struct efDisplacementMap_var { // 11effc
			int ox; // 11effc
			int oy; // 11f000
			void** mode_func; // 11f004
			int param1; // 11f008
			int param0; // 11f00c
		};

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::efDisplacementMap_mt_func_call, 6);
			h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
			h.replaceNearJmp(2, &mt_func);

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

	} DisplacementMap;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_DISPLACEMENTMAP
