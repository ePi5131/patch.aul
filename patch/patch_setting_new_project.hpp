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

#ifdef PATCH_SWITCH_SETTING_NEW_PROJECT
#include <exedit.hpp>
#include "global.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// プロジェクト作成時、サイズの変更＞なし　フレームレートの変更＞なし　クリッピング＆リサイズOFF にする

	inline class setting_new_project_t {

		static BOOL __cdecl exedit_edit_open_wrap(int w, int h, int video_rate, int video_scale, int audio_rate, HWND hwnd, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);
		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "setting_new_project";

	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			{
				// ReplaceNearJmp(GLOBAL::exedit_base + 0x004d4a, &exedit_edit_open_wrap); // バックアップファイルから新規作成
				ReplaceNearJmp(GLOBAL::exedit_base + 0x03c0d0, &exedit_edit_open_wrap); // プロジェクト作成前のD&D
				ReplaceNearJmp(GLOBAL::exedit_base + 0x04392f, &exedit_edit_open_wrap); // オブジェクトファイルから新規作成
				ReplaceNearJmp(GLOBAL::exedit_base + 0x043aea, &exedit_edit_open_wrap); // 新規プロジェクトの作成
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

	} setting_new_project;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTING_NEW_PROJECT
