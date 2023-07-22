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

#ifdef PATCH_SWITCH_TRA_SPECIFIED_SPEED

#include <memory>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// トラック変化方式の「移動量指定」で計算が足りていなかった（主に時間制御との組み合わせでバグる）のを修正
	inline class tra_specified_speed_t {

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "tra_specified_speed";

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			auto& cursor = GLOBAL::executable_memory_cursor;

			/*
				1006821c 8b8cb7f8000000     mov     ecx,dword ptr [edi+esi*4+000000f8] ;ecx = obj[object_idx].track_value_left[track_begin]
				10068223 0faf442458         imul    eax,dword ptr [esp+58] ;eax *= obj_frame
				↓
				1006821c e8XxXxXxXx         call    bin_data
				10068221 8b8cb7f8000000     mov     ecx,dword ptr [edi+esi*4+000000f8] ;ecx = obj[object_idx].track_value_left[track_begin]


				; arg3_subframeが0以外の時はobj_frameが100倍されてarg3_subframeが加算されている
				; 移動量指定では100で割るコードを忘れている
			*/


			OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x06821c, 12);
			h.store_i8(0, '\xe8');
			h.replaceNearJmp(1, cursor);
			h.store_i32(5, '\x8b\x8c\xb7\xf8');
			h.store_i32(8, '\xf8\x00\x00\x00'); // \xf8は範囲ダブらせてstore_i32 * 2 で行っています

			static constinit auto code_put = binstr_array(
				"0faf44245c" // imul    eax,dword ptr [esp+5c] ;eax *= obj_frame
				"8b4d10"     // mov     ecx,dword ptr [ebp+10] ;ecx = arg3_subframe
				"85c9"       // test    ecx,ecx
				"7408"       // jz      skip 8 byte ;if(ecx == 0)return
				"b964000000" // mov     ecx,00000064 ;ecx = 100
				"99"         // cdq
				"f7f9"       // idiv    ecx ;edx = eax % ecx, eax /= ecx
				"c3"         // ret      ;return
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

	} tra_specified_speed;
} // namespace patch
#endif // ifdef PATCH_SWITCH_TRA_SPECIFIED_SPEED
