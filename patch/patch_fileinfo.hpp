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

#ifdef PATCH_SWITCH_FILEINFO

#include <exedit.hpp>
#include "config_rw.hpp"

#include "global.hpp"
#include "util.hpp"

namespace patch {

	// init at patch load
	// ファイルの情報を開くと落ちるのを修正
	inline class fileinfo_t {
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fileinfo";
	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;


			OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x01c47a, 6);
			h.store_i16(0, '\x90\xb8');
			h.store_i32(2, GLOBAL::aviutl_base + 0x0b87b0);
			/*
				0041c47a 8b800c040000    mov     eax,dword ptr[eax + 0x40c] ; editp+0x40c
				↓
				0041c47a 90              nop
				0041c47b b8XxXxXxXx      mov     eax,aviutl + 0xb87b0

					; eaxが本来はaviutl+0xb87b0でなければいけないのにプロジェクト保存時のアドレスを使っている
			*/
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
	} fileinfo;
} // namespace patch

#endif // ifdef PATCH_SWITCH_FILEINFO
