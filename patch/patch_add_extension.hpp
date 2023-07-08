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

#ifdef PATCH_SWITCH_ADD_EXTENSION

#include <memory>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 動画、音声ファイル参照の時の拡張子にexedit.iniにあるものを追加する機能
	inline class add_extension_t {

		static int __cdecl wsprintfA_wrap2701a(void* ebp, LPSTR dst, LPCSTR src);

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "add_extension";


	public:


		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			{
				/*
					0042701a ff1584f34600            call    dword ptr [USER32.wsprintfA]
					00427020 83c408                  add     esp,+08

					↓

					0042701a 55                      push    ebp
					0042701b e8XxXxXxXx              call    wsprintfA_wrap2701a(void* ebp, LPSTR dst, LPCSTR src)
					00427020 83c40c                  add     esp,+0c



					00427034 899df8faffff            mov     dword ptr [ebp+fffffaf8],ebx
					↓
					00427034 eb04                    jmp     skip,+04

				*/

				OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x02701a, 28);
				h.store_i16(0, '\x55\xe8');
				h.replaceNearJmp(2, &wsprintfA_wrap2701a);
				h.store_i8(8, '\x0c');
				h.store_i16(26, '\xeb\x04');

			}


		}
		void switching(bool flag) {
			enabled = flag;
		}

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

	} add_extension;
} // namespace patch
#endif // ifdef PATCH_SWITCH_ADD_EXTENSION
