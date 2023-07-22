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

#ifdef PATCH_SWITCH_OBJ_NOISE
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// ノイズの 速度X 変化速度 を移動無し以外に設定している時、速度Y の値をもとに計算されるバグの修正
	inline class obj_Noise_t {
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "obj_noise";
	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			auto& cursor = GLOBAL::executable_memory_cursor;

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x04d8e7, 5);
			h.store_i8(0, '\xe8');
			h.store_i32(1, cursor - (GLOBAL::exedit_base + 0x04d8ec));
			/*
				1004d8e7 2bf0          sub     esi,eax
				1004d8e9 8b4a08        mov     ecx,dword ptr [edx+8]
				↓
				1004d8e7 e8xXxXxXxX    call    &executable_memory_cursor

				ecx,dword ptr [edx+8]を ecx,dword ptr [edx+ track_id*4]にする
			*/

			static constinit auto code_put = binstr_array(
				"2bf0"     // sub     esi, eax
				"8b4c244c" // mov     ecx, dword ptr[esp + 0x4c]
				"8b0c8a"   // mov     ecx, dword ptr[edx + ecx * 4]
				"c3"       // ret     exedit_base + 0x4d8ec
			);

			std::memcpy(cursor, code_put.data(), code_put.size());
			cursor += code_put.size();
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
	} Noise;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_NOISE
