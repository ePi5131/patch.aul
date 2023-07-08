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
#ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
#include "global.hpp"
#include "util_magic.hpp"

#include "restorable_patch.hpp"

namespace patch {
	// init at exedit load
	// 動画ファイル と 音声ファイル で中間点を打っていないときでもファイルを再参照しても再生位置などの情報を変更しない
	inline class ignore_media_param_reset_t {

		std::optional<restorable_patch_i16> rp1;
		std::optional<restorable_patch_i8> rp2;

		bool enabled = false;

		inline static const char key[] = "ignore_media_param_reset";

	public:
		void init() {
			rp1.emplace(GLOBAL::exedit_base + OFS::ExEdit::ignore_media_param_reset_mov, '\x90\xe9');
			rp2.emplace(GLOBAL::exedit_base + OFS::ExEdit::ignore_media_param_reset_aud, '\xeb');

			rp1->switching(enabled);
			rp2->switching(enabled);
		}

		void switching(bool flag) {
			enabled = flag;
			rp1->switching(enabled);
			rp2->switching(enabled);
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

	} ignore_media_param_reset;
} // namespace patch
#endif // ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
