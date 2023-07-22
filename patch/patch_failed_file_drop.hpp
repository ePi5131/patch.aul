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

#ifdef PATCH_SWITCH_FAILED_LONGER_PATH

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// ドロップ処理で最終的に何も行われなかったときにメッセージを出力する

	inline class failed_file_drop_t {

		inline static const char str_failed_drop_msg[] = "拡張編集へドロップされたファイルの種類が判別できませんでした\nexedit.iniを確認してください";



		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "failed_file_drop";
	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;



			auto& cursor = GLOBAL::executable_memory_cursor;

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x03c454, 4);
			h.replaceNearJmp(0, cursor);
			/*
				 1003c452 0f84f4760000        jz      10043b4c
				 ↓
				 1003c452 0f84XxXxXxXx        jz      executable_memory_cursor



			*/

			static constinit auto code_put = binstr_array(
				"8d8c24d0000000"                 //  lea     ecx,dword ptr [esp+000000d0]
				"51"                             //  push    ecx
				"e8" PATCH_BINSTR_DUMMY_32(9)    //  call    1004e1d0 ; ExtractExtension
				"83c404"                         //  add     esp,+04
				"6830200400"                     //  push    0x42030
				"50"                             //  push    eax
				"68" PATCH_BINSTR_DUMMY_32(23)   //  push    i32(&str_failed_drop_msg)
				"a1" PATCH_BINSTR_DUMMY_32(28)   //  mov     eax, [i32(exedit+exedit_hwnd)]
				"50"                             //  push    eax
				"ff15" PATCH_BINSTR_DUMMY_32(35) //  call    dword ptr [exedit+MessageBoxA]
				"e9"                             //  jmp     10043b4c
			);

			std::memcpy(cursor, code_put.data(), code_put.size());
			store_i32(cursor + 9, CalcNearJmp(reinterpret_cast<i32>(cursor + 13), GLOBAL::exedit_base + 0x04e1d0));
			store_i32(cursor + 23, &str_failed_drop_msg);
			store_i32(cursor + 28, GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd);
			store_i32(cursor + 35, GLOBAL::exedit_base + 0x09a320);
			store_i32(cursor + 40, CalcNearJmp(reinterpret_cast<i32>(cursor + 40), GLOBAL::exedit_base + 0x043b4c));
			cursor += code_put.size() + 4;
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
	} failed_file_drop;
} // namespace patch

#endif // ifdef PATCH_SWITCH_FAILED_FILE_DROP
