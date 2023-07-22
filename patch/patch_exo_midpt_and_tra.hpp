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
#ifdef PATCH_SWITCH_EXO_MIDPT_AND_TRA
#include <exedit.hpp>
#include "config_rw.hpp"
#include "util_magic.hpp"

namespace patch {
	// init at exedit load
	/* 中間点有りで.traスクリプト変化方式のあるオブジェクトがあると正常に読み込めないことがあるのを修正
	発生条件：
	・aviutl.exeを開いた後、一度も使用していない.traスクリプト変化方式が含まれること
	・中間点有りオブジェクトであること
	*/
	inline class exo_midpt_and_tra_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "exo_midpt_and_tra";


	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			auto& cursor = GLOBAL::executable_memory_cursor;

			// 100345d9 e8d2030000         call    exedit+349b0

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0345da, 4);
			h.replaceNearJmp(0, cursor);

			static constinit auto code_put = binstr_array(
				"8b842440010000"                 // mov     eax,dword ptr [esp+140] ; flag
				"85c0"                           // test    eax,eax
				"0f84" PATCH_BINSTR_DUMMY_32(11) // jz      exedit+349b0
				"c3"                             // ret
			);

			std::memcpy(cursor, code_put.data(), code_put.size());
			store_i32(cursor + 11, CalcNearJmp(reinterpret_cast<i32>(cursor + 11), GLOBAL::exedit_base + 0x0349b0));
			
			cursor += code_put.size();

		}

		void switching(bool flag) { enabled = flag; }

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

	} exo_midpt_and_tra;
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_MIDPT_AND_TRA
