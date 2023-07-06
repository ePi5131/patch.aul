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

#ifdef PATCH_SWITCH_TRA_CHANGE_DRAWFILTER

#include <memory>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 標準描画-拡張描画-パーティクル出力 を切り替えた際にトラックの設定値(移動フレーム間隔)が0になるのを修正
	// 値を引き継ぐための関数に足りていなかったので追加した関数を置き換える
	inline class tra_change_drawfilter_t {
		static int __cdecl switch_drawfilter_trackdata_to_mem(int* data, int object_idx, int track_begin, int track_id);
		static int __cdecl switch_drawfilter_mem_to_trackdata(int* data, int object_idx, int track_begin, int pre_track_exists, int track_id);

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "tra_change_drawfilter";

		inline static ExEdit::Object** ObjectArrayPointer_ptr;

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			ObjectArrayPointer_ptr = reinterpret_cast<decltype(ObjectArrayPointer_ptr)>(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);

			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x44280, 5);
				h.store_i8(0, '\xe9'); // jmp
				h.replaceNearJmp(1, &switch_drawfilter_trackdata_to_mem);
			}
			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x44530, 5);
				h.store_i8(0, '\xe9'); // jmp
				h.replaceNearJmp(1, &switch_drawfilter_mem_to_trackdata);
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

	} tra_change_drawfilter;
} // namespace patch
#endif // ifdef PATCH_SWITCH_TRA_CHANGE_DRAWFILTER
