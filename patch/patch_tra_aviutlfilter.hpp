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

#ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// 拡張編集以外のフィルタのトラックバーにトラックバー変化方法スクリプトを適用していると例外になる
	inline class tra_aviutlfilter_t {
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "tra_aviutlfilter";
	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			auto& cursor = GLOBAL::executable_memory_cursor;

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x06577a, 6);
			h.store_i16(0, '\x90\xe9');
			h.store_i32(2, cursor - (GLOBAL::exedit_base + 0x065780));
			/*
				1006577a 8b91d0000000  mov     edx,dword ptr [ecx+000000d0] ; filter_param_ptr->track_link
				10065780 8b89cc000000  mov     ecx,dword ptr [ecx+000000cc] ; filter_param_ptr->track_scale
				10065786 03c3          add     eax,ebx
				10065788 50            push    eax
				10065789 52            push    edx
				1006578a 8b1481        mov     edx,dword ptr [ecx+eax*4]
				1006578d 8b4514        mov     eax,dword ptr [ebp+14]


				1006577a 8b91d0000000  mov     edx,dword ptr [ecx+000000d0]
				↓
				1006577a 90e9XXXXXXXX  jmp     executable_memory_cursor

					; 拡張編集以外のフィルタの場合は
					; filter_param_ptr->track_link の部分を 0 に
					; filter_param_ptr->track_scale[eax] の部分を 1 に
			*/

			static constinit auto code_put = binstr_array(
				"03c3"                           // add     eax, ebx
				"50"                             // push    eax
				"8a5103"                         // mov     dl,[ecx + 03]
				"f6c204"                         // test    dl,04
				"7414"                           // jz      +20byte
				"8b91d0000000"                   // mov     edx, dword ptr[ecx + 000000d0]
				"8b89cc000000"                   // mov     ecx, dword ptr[ecx + 000000cc]
				"85c9"                           // test    ecx
				"0f85" PATCH_BINSTR_DUMMY_32(27) // jnz     exedit_base + 65789
				"33d2"                           // xor     edx
				"52"                             // push    edx
				"42"                             // inc     edx
				"e9" //PATCH_BINSTR_DUMMY_32(36) // jmp     exedit_base + 6578d
			);

			std::memcpy(cursor, code_put.data(), code_put.size());
			store_i32(cursor + 27, CalcNearJmp(reinterpret_cast<i32>(cursor + 27), GLOBAL::exedit_base + 0x065789));
			store_i32(cursor + 36, CalcNearJmp(reinterpret_cast<i32>(cursor + 36), GLOBAL::exedit_base + 0x06578d));
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
	} tra_aviutlfilter;
} // namespace patch

#endif // ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
