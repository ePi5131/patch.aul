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
#ifdef PATCH_SWITCH_KEYCONFIG
#include <exedit.hpp>
#include "util_magic.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at patch load
	// 言語リソースプラグインなどでメニューの「編集」の部分が変わるとaviutl.keyのプラグイン設定部分が正しく読み込めなくなるのを修正
	inline class KeyConfig_t {

		static int __stdcall lstrcmpA_wrap(char* menu_data_str, char* keyfile_data_str);


		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "keyconfig";


	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{   // 音声ファイルの再生速度トラックの最大値2000.0のはずが800.0となってしまう処理があるのを修正
				/*
                    004331c2 ff1550f24600       call    lstrcmpA

					↓

					004331c2 90                 nop
					004331c3 e8XxXxXxXx         call    new_function
				*/

				OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x331c2, 6);
				h.store_i16(0, '\x90\xe8');
				h.replaceNearJmp(2, &lstrcmpA_wrap);
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

	} KeyConfig;
} // namespace patch
#endif // ifdef PATCH_SWITCH_KEYCONFIG
