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
#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV

#include "global.hpp"
#include "offset_address.hpp"
#include "util_magic.hpp"

namespace patch {
	// init at exedit load
	// 特定色域変換の拡張データ領域にstatusが2つある問題
	inline class exo_specialcolorconv_t {
		inline static const char status2_name[] = "status2";

		bool enabled = true;
		inline static const char key[] = "exo_specialcolorconv";

		std::optional<restorable_patch_i32> rp;

	public:
		void init() {
			rp.emplace(GLOBAL::exedit_base + OFS::ExEdit::specialcolorconv_status2, &status2_name);

			rp->switching(enabled);
		}

		void switching(bool flag) {
			rp->switching(enabled = flag);
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled; }

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
		   });
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}
	} exo_specialcolorconv;
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
