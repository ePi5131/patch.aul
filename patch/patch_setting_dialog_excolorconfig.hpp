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

#ifdef PATCH_SWITCH_SETTINGDIALOG_EXCOLORCONFIG
#include <exedit.hpp>
#include "global.hpp"
#include "util_magic.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 設定ダイアログで拡張色設定の下のフィルタにウィンドウが被る
	inline class excolorconfig_t {

		static void __cdecl efExColorConfig_setButtonStr_wrap_17083(ExEdit::Filter* efp);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "settingdialog_excolorconfig";

	public:
		inline static void(__cdecl* efExColorConfig_setButtonStr)(ExEdit::Filter* efp);

		void init() {
			enabled_i = enabled;
			if (!enabled_i) return;

			efExColorConfig_setButtonStr = reinterpret_cast<decltype(efExColorConfig_setButtonStr)>(GLOBAL::exedit_base + 0x016e90);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x017084, &efExColorConfig_setButtonStr_wrap_17083);

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

	} excolorconfig;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_EXCOLORCONFIG
