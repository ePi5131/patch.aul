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
#ifdef PATCH_SWITCH_EXO_SCENEIDX
#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "restorable_patch.hpp"

namespace patch {
	// init at exedit load
	// オブジェクトファイルにシーン・シーン(音声)のシーン番号が正しく入出力されない
	inline class exo_sceneidx_t {

		bool enabled = true;
		inline static const char key[] = "exo_sceneidx";

		std::optional<restorable_patch_i16> rp1;
		std::optional<restorable_patch_i32> rp2;
		std::optional<restorable_patch_i32> rp3;

		inline static const char scene_name[] = "scene";
	public:
		void init() {

			rp1.emplace(GLOBAL::exedit_base + OFS::ExEdit::efSceneAudio_exdatause_idx_type, (i16)1);
			rp2.emplace(GLOBAL::exedit_base + OFS::ExEdit::efSceneAudio_exdatause_idx_name, &scene_name);
			rp3.emplace(GLOBAL::exedit_base + OFS::ExEdit::efScene_exdatause_idx_name, &scene_name);

			rp1->switching(enabled);
			rp2->switching(enabled);
			rp3->switching(enabled);
		}

		void switching(bool flag) {
			enabled = flag;
			rp1->switching(enabled);
			rp2->switching(enabled);
			rp3->switching(enabled);
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
	} exo_sceneidx;
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_SCENEIDX
