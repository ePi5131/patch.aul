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

				static const char code_put_write[] =
					"\x8b\xc8"                 // mov     ecx,eax
					"\x50"                     // push    eax
					"\x83\xe1\x07"             // and     ecx,07
					"\xc1\xe9\x01"             // shr     ecx,01
					"\x83\xf1\x02"             // xor     ecx,02
					"\x85\xc9"                 // test    ecx,ecx
					"\x74\x0e"                 // jz      skip 0e
					"\x51"                     // push    ecx
					"\x68XXXX"                 // push    "_fold_gui=%d\r\n"
					"\x55"                     // push    ebp
					"\xff\xd3"                 // call    ebx
					"\x83\xc4\x0c"             // add     esp,0c
					"\x03\xe8"                 // add     ebp,eax
					"\x58"                     // pop     eax
					"\x24\x01"                 // and     al,01
					"\x84\xc0"                 // test    al,al
					"\x0f\x84XXXX"             // jz      exedit + 0x288b7
					"\xe9"// & XXXX            // jmp     exedit + 0x288c6
					;

				memcpy(cursor, code_put_write, sizeof(code_put_write) - 1);
				store_i32(cursor + 18, &fold_gui_write);
				cursor += sizeof(code_put_write) - 1 + 4;
				store_i32(cursor - 9, GLOBAL::exedit_base + 0x0288b7 - ((int)cursor - 5));
				store_i32(cursor - 4, GLOBAL::exedit_base + 0x0288c6 - (int)cursor);
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

				static const char code_put_read[] =
					"\x8b\x54\x24\x14"         // mov     edx,dword ptr [esp+14]
					"\x52"                     // push    edx
					"\x68XXXX"                 // push    "_fold_gui"
					"\xff\xd6"                 // call    esi
					"\x85\xc0"                 // test    eax,eax
					"\x75\x1f"                 // jnz     skip,1f
					"\x53"                     // push    ebx
					"\xe8XXXX"                 // call    100918ab
					"\x83\xc4\x04"             // add     esp,04
					"\x85\xc0"                 // test    eax,eax
					"\x74\x12"                 // jz      skip,12
					"\x83\xe0\x03"             // and     eax,03
					"\xc1\xe0\x01"             // shl     eax,01
					"\x8b\x4c\x24\x48"         // mov     ecx,dword ptr [esp+48]
					"\x03\xcd"                 // add     ecx,ebp
					"\x83\xc1\x72"             // add     ecx,+72
					"\x30\x41\x72"             // xor     byte ptr [ecx+72],al
					"\x8b\x54\x24\x14"         // mov     edx,dword ptr [esp+14]
					"\x52"                     // push    edx
					"\xe9"// & XXXX            // jmp     exedit + 0x29b26
					;

				memcpy(cursor, code_put_read, sizeof(code_put_read) - 1);
				store_i32(cursor + 6, &fold_gui_read);
				store_i32(cursor + 18, GLOBAL::exedit_base + 0x0918ab - ((int)cursor + 22));
				cursor += sizeof(code_put_read) - 1 + 4;
				store_i32(cursor - 4, GLOBAL::exedit_base + 0x029b26 - (int)cursor);
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
