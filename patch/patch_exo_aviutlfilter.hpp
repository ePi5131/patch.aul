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

#ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "restorable_patch.hpp"
#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// 拡張編集以外のフィルタを使ったフィルタオブジェクトを正常にオブジェクトファイルに出力できない
	inline class exo_aviutlfilter_t {

		bool enabled = true;
		inline static const char key[] = "exo_aviutlfilter";

		std::optional<restorable_patch> rp1;
		std::optional<restorable_patch> rp2;
		std::optional<restorable_patch> rp3;

	public:
		void init() {
			
			if (!enabled)return;

			auto& cursor = GLOBAL::executable_memory_cursor;

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_RangeBegin, 30);
			/*
				10028a80 CALL EBX ; wsprintfA
				10028a82 ADD EBP, EAX
					  84 JMP [mycode]
				         ADD ESP 0CH

						 MOV AL, BYTE PTR [ESI + 3H]
						 TEST AL, 04H
						 JZ ASSIGN_ZERO

				         MOV EAX, DWORD PTR [ESI + CCH] ; filter_param_ptr->track_scale
				         TEST EAX, EAX
						 JZ ASSIGN_ZERO

						 MOV ECX, DWORD PTR [ESP + 2CH] ; count
						 MOV EAX, DWORD PTR [EAX + ECX * 4] ; track_scale[count]
						 JMP [original]
						 
						 ASSIGN_ZERO:
						 XOR EAX, EAX
						 JMP [original]

				10028a9c 
			*/

			auto apply = [&cursor](uint32_t ofs, uint32_t esp_add, std::optional<restorable_patch>& rp) {
				static const char code_put[] =
					"\x83\xc4\x0c" // ADD ESP 0CH
					"\x8a\x46\x03" // MOV AL, BYTE PTR [ESI + 3H]
					"\xa8\x04" // TEST AL, 04H
					"\x74\x17" // JZ ASSIGN_ZERO
					"\x8b\x86\xcc\x00\x00\x00" // MOV EAX, DWORD PTR [ESI + CCH] ; filter_param_ptr->track_scale
					"\x85\xc0" // TEST EAX, EAX
					"\x74\x0d" // JZ ASSIGN_ZERO
					"\x8b\x4c\x24\x2c" // MOV ECX, DWORD PTR [ESP + 2CH] ; count
					"\x8b\x04\x88" // MOV EAX, DWORD PTR [EAX + ECX * 4] ; track_scale[count]
					"\xe9XXXX" // JMP rel32
					// ASSIGN_ZERO:
					"\x31\xc0" // XOR EAX, EAX
					"\xe9XXXX" // JMP rel32
					;

				char injection[6];
				injection[0] = '\xe9'; // jmp rel32
				store_i32(injection + 1, CalcNearJmp(ofs + 1, reinterpret_cast<i32>(cursor)));
				injection[5] = '\x90'; // nop
				rp.emplace(ofs, injection, sizeof(injection));

				memcpy(cursor, code_put, sizeof(code_put) - 1);
				auto ret = ofs + 0x18;
				store_i8(cursor + 2, esp_add);
				store_i32(cursor + 0x1c, CalcNearJmp(reinterpret_cast<i32>(cursor + 0x1c), ret));
				store_i32(cursor + 0x23, CalcNearJmp(reinterpret_cast<i32>(cursor + 0x23), ret));
				cursor += sizeof(code_put) - 1;
			};

			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite1, 0xc, rp1);
			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite2, 0xc, rp2);
			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite3, 0x18, rp3);

			rp1->switching(enabled);
			rp2->switching(enabled);
			rp3->switching(enabled);
		}


		void switching(bool flag) {
			enabled = flag;
			rp1->switching(enabled);
			rp2->switching(enabled);
			rp3->switching(enabled);
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled; }


		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}


	} exo_aviutlfilter;
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
