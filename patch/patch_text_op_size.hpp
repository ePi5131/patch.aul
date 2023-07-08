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
#ifdef PATCH_SWITCH_TEXT_OP_SIZE

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

namespace patch {
	// init at exedit load
	// 制御文字のサイズのみを変えるとフォントの情報が壊れる
	inline class text_op_size_t {
		std::optional<restorable_patch_i8> rp;

		bool enabled = true;
		inline static const char key[] = "text_op_size";
	public:
		void init() {
			rp.emplace(GLOBAL::exedit_base + OFS::ExEdit::text_op_logfont_size, sizeof(LOGFONTW));

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
	} text_op_size;
} // namespace patch
#endif // ifdef PATCH_SWITCH_TEXT_OP_SIZE
