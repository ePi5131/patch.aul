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
#ifdef PATCH_SWITCH_SUSIE_LOAD

#include <exedit.hpp>

#include "util.hpp"
#include "global.hpp"
#include "offset_address.hpp"

#include "restorable_patch.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// Susieのプラグインで正しく対応拡張子情報を取得できない
	inline class susie_load_t {
		static void __cdecl LoadSpi(LPCSTR dir);

		inline static const char key[] = "susie_load";

		bool enabled = true;

		std::optional<restorable_patch_function> rpf;

	public:

		bool init() {
			rpf.emplace(GLOBAL::exedit_base + OFS::ExEdit::LoadSpi, &LoadSpi);
			rpf->switching(enabled);
		}

		void switching(bool flag) {
			rpf->switching(enabled = flag);
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

	}susie_load;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SUSIE_LOAD
