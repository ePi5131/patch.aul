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

            static const char code_put[] =
                "\x03\xc3"          // add     eax, ebx
                "\x50"              // push    eax
                "\x8a\x51\x03"      // mov     dl,[ecx + 03]
                "\xf6\xc2\x04"      // test    dl,04
                "\x74\x14"          // jz      +20byte
                "\x8b\x91\xd0"
                "\x00\x00\x00"      // mov     edx, dword ptr[ecx + 000000d0]
                "\x8b\x89\xcc"
                "\x00\x00\x00"      // mov     ecx, dword ptr[ecx + 000000cc]
                "\x85\xc9"          // test    ecx
                "\x0f\x85XXXX"      // jnz     exedit_base + 65789
                "\x33\xd2"          // xor     edx
                "\x52"              // push    edx
                "\x42"              // inc     edx
                "\xe9XXXX"          // jmp     exedit_base + 6578d
                ;

            memcpy(cursor, code_put, sizeof(code_put) - 1);

            store_i32(cursor + 27, GLOBAL::exedit_base + 0x065789 - (uint32_t)(cursor + 31));

            cursor += sizeof(code_put) - 1;
            store_i32(cursor - 4, GLOBAL::exedit_base + 0x06578d - (uint32_t)cursor);
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
