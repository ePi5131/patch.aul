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
				10028a84 MOV EAX, DWORD PTR [ESI + CCH] ; filter_param_ptr->track_scale
				↓

				10028a84 NOP
				10028a85 CALL func

				func	 MOV AL, BYTE PTR [ESI + 3H]
						 TEST AL, 04H
						 JNZ SKIP, 03H
						 XOR EAX, EAX
						 RET
						 MOV EAX, DWORD PTR [ESI + CCH] ; filter_param_ptr->track_scale
						 RET
			*/

			static constinit auto code_put = binstr_array(
				"8a4603"       // MOV AL, BYTE PTR [ESI + 3H]
				"a804"         // TEST AL, 04H
				"7503"         // JNZ SKIP, 03H
				"33c0"         // XOR EAX, EAX
				"c3"           // RET
				"8b86cc000000" // MOV EAX, DWORD PTR [ESI + CCH] ; filter_param_ptr->track_scale
				"c3"           // RET
			);
			std::memcpy(cursor, code_put.data(), code_put.size());

			auto apply = [&cursor](uint32_t ofs, std::optional<restorable_patch>& rp) {
				char injection[6];
				injection[0] = 0x90_i8; // nop
				injection[1] = 0xe8_i8; // call rel32
				store_i32(&injection[2], CalcNearJmp(ofs + 2, reinterpret_cast<i32>(cursor)));
				rp.emplace(ofs, injection, sizeof(injection));
			};

			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite1, rp1);
			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite2, rp2);
			apply(GLOBAL::exedit_base + OFS::ExEdit::ConvertFilter2Exo_TrackScaleJudge_Overwrite3, rp3);

			cursor += code_put.size();

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
