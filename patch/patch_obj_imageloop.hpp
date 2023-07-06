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

#ifdef PATCH_SWITCH_OBJ_IMAGELOOP
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// 画像ループにて個別オブジェクトにして何かしら効果を付けた時に、ループごとに元の画像に戻さず効果を付与し続けていくのを修正

	inline class obj_ImageLoop_t {

		static void __cdecl save_current_image(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl obj_effect_noargs_wrap(ExEdit::ObjectFilterIndex ofi, ExEdit::FilterProcInfo* efpip, int flag);


		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "obj_imageloop";
	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;


			auto& cursor = GLOBAL::executable_memory_cursor;

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x05a80a, 6);
			h.store_i16(0, '\x90\xe8'); // nop, call
			h.replaceNearJmp(2, cursor);
			/*
				1005a80a 0f8e86010000    jle        1005a996
				; このjleで飛ぶことは無いはずなので置き換えてしまう
				↓

				1005a80a 90              nop
				1005a80b e8XxXxXxXx      call       executable_memory_cursor
				; ecxとedxは退避しなくても大丈夫そう
				; eaxは0に
			*/

			static const char code_put[] =
				"\x8b\x84\x24\xa4\x00\x00\x00"// mov     eax,dword ptr [esp+000000a4]
				"\x53"                        // push    ebx ; efpip
				"\x50"                        // push    eax ; efp
				"\xe8XXXX"                    // call    new_function
				"\x83\xc4\x08"                // add     esp,+08
				"\x33\xc0"                    // xor     eax,eax
				"\xc3"                        // ret
				;

			memcpy(cursor, code_put, sizeof(code_put) - 1);
			store_i32(cursor + 10, (int32_t)&save_current_image - (int32_t)cursor - 14);
			cursor += sizeof(code_put) - 1;



			ReplaceNearJmp(GLOBAL::exedit_base + 0x05a92c, &obj_effect_noargs_wrap);
			
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
	} ImageLoop;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_IMAGELOOP
