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
#ifdef PATCH_SWITCH_OBJ_SPECIALCOLORCONV

#include <memory>

#include "util.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 特定色域変換で変換前色に黒色を指定すると0除算が起こることがあるのを修正

	/*
		問題のコード
		if (pix_y < before_y) {
			after_y = (after_y * pix_y) / before_y;
		}

		今回行った修正は以下を追加 (Yが負の値を取らない前提で作られていたようなので)
		if (pix_y < 0) {
			pix_y = 0;
		}

	*/

	inline class obj_specialcolorconv_t {

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "obj_specialcolorconv";


	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			auto& cursor = GLOBAL::executable_memory_cursor;
			{
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x015cca, 7);
					h.store_i32(0, '\x90\x90\xe8\x00');
					h.replaceNearJmp(3, cursor);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x015e25, 7);
					h.store_i32(0, '\x90\x90\xe8\x00');
					h.replaceNearJmp(3, cursor);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x015f77, 7);
					h.store_i32(0, '\x90\x90\xe8\x00');
					h.replaceNearJmp(3, cursor);
				}

				{
					/*
						10015f77 0fbf31             movsx   esi,dword ptr [ecx]
						10015f7a 0fbf5d00           movsx   ebx,dword ptr [ebp+00]

						↓

						10015f77 9090               nop
						10015f79 e8XxXxXxXx         call    executable_memory_cursor

							; esi(pix->y)が0未満の場合、0にする
					*/
					static constinit auto code_put = binstr_array(
						"0fbf31"   // movsx   esi,dword ptr [ecx]
						"0fbf5d00" // movsx   ebx,dword ptr [ebp+00]
						"85f6"     // test    esi,esi
						"7f02"     // jg      skip,+02
						"33f6"     // xor     esi,esi
						"c3"       // ret
					);

					std::memcpy(cursor, code_put.data(), code_put.size());
					cursor += code_put.size();
				}
			}

			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0156f5, 6);
				h.store_i16(0, '\x90\xe8');
				h.replaceNearJmp(2, cursor);
				/*
					100156f5 0fbf1f             movsx   ebx,dword ptr [edi]
					100156f8 0fbf2e             movsx   ebp,dword ptr [esi]

					↓

					100156f5 90                 nop
					100156f6 e8XxXxXxXx         call    executable_memory_cursor

						; ebx(pix->y)が0未満の場合、0にする
				*/
				static constinit auto code_put = binstr_array(
					"0fbf1f" // movsx   ebx,dword ptr [edi]
					"0fbf2e" // movsx   ebp,dword ptr [esi]
					"85db"   // test    ebx,ebx
					"7f02"   // jg      skip,+02
					"33db"   // xor     ebx,ebx
					"c3"     // ret
				);

				std::memcpy(cursor, code_put.data(), code_put.size());
				cursor += code_put.size();
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
	} obj_specialcolorconv;
} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_SPECIALCOLORCONV
