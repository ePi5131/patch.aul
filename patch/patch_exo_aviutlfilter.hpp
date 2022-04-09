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

namespace patch {

	// init at exedit load
	// 拡張編集以外のフィルタを使ったフィルタオブジェクトを正常にオブジェクトファイルに出力できない
	inline class exo_aviutlfilter_t {
	private:

		inline static uint32_t jmp1;
		inline static uint32_t jmp2;
		inline static uint32_t jmp3;

		inline static uint32_t ret1;
		inline static uint32_t ret2;
		inline static uint32_t ret3;

	public:
		void operator()() {
			if (!PATCH_SWITCHER_MEMBER(PATCH_SWITCH_EXO_AVIUTL_FILTER))return;

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

			auto apply = [&cursor](uint32_t ofs, uint32_t& jmp, uint32_t& ret, uint32_t esp_add) {
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
					"\xff\x25XXXX" // JMP [i32]
					// ASSIGN_ZERO:
					"\x31\xc0" // XOR EAX, EAX
					"\xff\x25XXXX" // JMP [i32]
					;
				store_i16(ofs, '\xff\x25'); // jmp (i32)
				jmp = reinterpret_cast<uint32_t>(cursor);
				store_i32(ofs + 2, &jmp);
				memcpy(cursor, code_put, sizeof(code_put) - 1);
				ret = ofs + 0x18;
				store_i8(cursor + 2, esp_add);
				store_i32(cursor + 0x1d, &ret);
				store_i32(cursor + 0x25, &ret);
				cursor += sizeof(code_put) - 1;
			};

			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite1, jmp1, ret1, 0xc);
			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite2, jmp2, ret2, 0xc);
			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite3, jmp3, ret3, 0x18);
		}
	} exo_aviutlfilter;
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
