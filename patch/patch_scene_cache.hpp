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

#ifdef PATCH_SWITCH_SCENE_CACHE

#include <chrono>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// Rootで取得した場合のみシーンのキャッシュをとる
	// 仕様：シーンの画像構成に掛かった時間が64msを超えた時にキャッシュを生成する。シーンを切り替えた時点でキャッシュは破棄されます。

	inline class scene_cache_t {
		

		static void* __cdecl get_scene_image_wrap(ExEdit::ObjectFilterIndex ofi, ExEdit::FilterProcInfo* efpip, int scene_idx, int frame, int subframe, int* w, int* h);
		static void delete_scene_cache();

		inline static void* (__cdecl* get_scene_image)(ExEdit::ObjectFilterIndex, ExEdit::FilterProcInfo*, int, int, int, int*, int*);

		inline static auto threshold_time_ms = 64;
		inline static const char key_threshold_time[] = "threshold_time";
		
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "scene_cache";

    public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			get_scene_image = reinterpret_cast<decltype(get_scene_image)>(GLOBAL::exedit_base + OFS::ExEdit::get_scene_image);

			{   // scene_obj
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0835bd, 4);
				h.replaceNearJmp(0, &get_scene_image_wrap);
			}
			/*
			{   // mask
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x068a2d, 4);
				h.replaceNearJmp(0, &get_scene_image_wrap);
			}*/

			{   // シーン切り替え時に初期化
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x02be77, 5);
				h.store_i8(0, '\xe9'); // jmp
				h.replaceNearJmp(1, &delete_scene_cache);
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
		
		void config_load(ConfigReader& cr) {
			cr.regist(key_threshold_time, [this](json_value_s* value) {
				ConfigReader::load_variable(value, threshold_time_ms);
			});
		}

		void config_store(ConfigWriter& cw) {
			cw.append(key_threshold_time, threshold_time_ms);
		}

	} scene_cache;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SCENE_CACHE
