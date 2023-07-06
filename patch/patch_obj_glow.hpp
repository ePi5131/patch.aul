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

#ifdef PATCH_SWITCH_OBJ_GLOW

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// グローのバグ修正

	/* オフセットアドレス exedit + 55625 の修正
		曲線移動などで しきい値 が負の数になった時にエラーが出る
	*/

	/* 小さいオブジェクトに効果が無いのを修正
		スレッド数より小さいオブジェクトに効果が乗らない
	*/


	inline class obj_Glow_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "obj_glow";
	public:


		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			{ // オフセットアドレス exedit + 55625 の修正

				auto& cursor = GLOBAL::executable_memory_cursor;

				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x054ed5, 6);
				h.store_i16(0, '\x90\xe8');
				h.replaceNearJmp(2, cursor);
				/*
					10054ed5 8b4908             mov     ecx,dword ptr [ecx+08]
					10054ed8 c1e10c             shl     ecx,0c
					↓
					10054ed5 90                 nop
					10054ed6 e8xXxXxXxX         call    &executable_memory_cursor

					; しきい値 track[2] が0未満の時に0にする
				*/

				static const char code_put[] =
					"\x8b\x49\x08"             // mov     ecx,dword ptr [ecx+08]
					"\x85\xc9"                 // test    ecx,ecx
					"\x7c\x04"                 // jl      skip,4
					"\xc1\xe1\x0c"             // shl     ecx,0c
					"\xc3"                     // ret
					"\x33\xc9"                 // xor     ecx,ecx
					"\xc3"                     // ret
					;

				memcpy(cursor, code_put, sizeof(code_put) - 1);
				cursor += sizeof(code_put) - 1;
			}

			
			{ // 小さいオブジェクトに効果が無いのを修正
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x055572, 2);
					h.store_i16(0, '\xeb\x0a');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0556c0, 2);
					h.store_i16(0, '\xeb\x0a');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0557d0, 2);
					h.store_i16(0, '\xeb\x0a');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x055910, 2);
					h.store_i16(0, '\xeb\x0a');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x055a54, 1);
					h.store_i8(0, '\xeb');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x055cb6, 1);
					h.store_i8(0, '\xeb');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x055f0e, 2);
					h.store_i16(0, '\xeb\x0a');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x056258, 1);
					h.store_i8(0, '\xeb');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x056a18, 1);
					h.store_i8(0, '\xeb');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x05710c, 1);
					h.store_i8(0, '\xeb');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x05776d, 1);
					h.store_i8(0, '\xeb');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x057dcc, 1);
					h.store_i8(0, '\xeb');
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x05846c, 1);
					h.store_i8(0, '\xeb');
				}
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
	} Glow;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_GLOW
