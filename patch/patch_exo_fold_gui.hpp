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

#ifdef PATCH_SWITCH_EXO_FOLD_GUI
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"
#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// exa,exo生成時、設定ダイアログの▼（折り畳み）とマウスマーク（GUI表示）の情報を追加する
	inline class exo_fold_gui_t {
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "exa_fold_gui";

		inline static const char fold_gui_write[] = "_fold_gui=%d\r\n";
		inline static const char fold_gui_read[] = "_fold_gui";

	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			auto& cursor = GLOBAL::executable_memory_cursor;

			{ // exo_write
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0288b1, 5);
				h.store_i8(0, '\xe9');
				h.replaceNearJmp(1, cursor);
				/*
					100288b1 2401               and     al,01
					100288b3 84c0               test    al,al
					100288b5 750f               jnz     100288c6
					↓
					100288b1 e9XxXxXxXx         jmp     executable_memory_cursor

				*/

				static constinit auto code_put_write = binstr_array(
					"8bc8"                           // mov     ecx,eax
					"50"                             // push    eax
					"83e107"                         // and     ecx,07
					"c1e901"                         // shr     ecx,01
					"83f102"                         // xor     ecx,02
					"85c9"                           // test    ecx,ecx
					"740e"                           // jz      skip 0e
					"51"                             // push    ecx
					"68" PATCH_BINSTR_DUMMY_32(18)   // push    "_fold_gui=%d\r\n"
					"55"                             // push    ebp
					"ffd3"                           // call    ebx
					"83c40c"                         // add     esp,0c
					"03e8"                           // add     ebp,eax
					"58"                             // pop     eax
					"2401"                           // and     al,01
					"84c0"                           // test    al,al
					"0f84" PATCH_BINSTR_DUMMY_32(37) // jz      exedit + 0x288b7
					"e9" //PATCH_BINSTR_DUMMY_32(42) // jmp     exedit + 0x288c6
				);

				std::memcpy(cursor, code_put_write.data(), code_put_write.size());
				store_i32(cursor + 18, &fold_gui_write);
				store_i32(cursor + 37, CalcNearJmp(reinterpret_cast<i32>(cursor + 37), GLOBAL::exedit_base + 0x0288b7));
				store_i32(cursor + 42, CalcNearJmp(reinterpret_cast<i32>(cursor + 42), GLOBAL::exedit_base + 0x0288c6));
				cursor += code_put_write.size() + 4;
			}

			{ // exo_read
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x029b21, 5);
				h.store_i8(0, '\xe9');
				h.replaceNearJmp(1, cursor);
				/*
					10029b21 8b542414           mov     edx,dword ptr [esp+14]
					10029b25 52                 push    edx
					↓
					100288b1 e9XxXxXxXx         jmp     executable_memory_cursor

				*/

				static constinit auto code_put_read = binstr_array(
					"8b542414"                       // mov     edx,dword ptr [esp+14]
					"52"                             // push    edx
					"68" PATCH_BINSTR_DUMMY_32(6)    // push    "_fold_gui"
					"ffd6"                           // call    esi
					"85c0"                           // test    eax,eax
					"751f"                           // jnz     skip,1f
					"53"                             // push    ebx
					"e8" PATCH_BINSTR_DUMMY_32(18)   // call    100918ab
					"83c404"                         // add     esp,04
					"85c0"                           // test    eax,eax
					"7412"                           // jz      skip,12
					"83e003"                         // and     eax,03
					"c1e001"                         // shl     eax,01
					"8b4c2448"                       // mov     ecx,dword ptr [esp+48]
					"03cd"                           // add     ecx,ebp
					"83c172"                         // add     ecx,+72
					"304172"                         // xor     byte ptr [ecx+72],al
					"8b542414"                       // mov     edx,dword ptr [esp+14]
					"52"                             // push    edx
					"e9" //PATCH_BINSTR_DUMMY_32(53) // jmp     exedit + 0x29b26
				);

				std::memcpy(cursor, code_put_read.data(), code_put_read.size());
				store_i32(cursor + 6, &fold_gui_read);
				store_i32(cursor + 18, CalcNearJmp(reinterpret_cast<i32>(cursor + 18), GLOBAL::exedit_base + 0x0918ab));
				store_i32(cursor + 53, CalcNearJmp(reinterpret_cast<i32>(cursor + 53), GLOBAL::exedit_base + 0x029b26));
				cursor += code_put_read.size() + 4;
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
	} exo_fold_gui;
} // namespace patch

#endif // ifdef PATCH_SWITCH_EXO_FOLD_GUI
