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

#ifdef PATCH_SWITCH_DIALOG_NEW_FILE

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

namespace patch {

	// init at exedit load
	// 新規プロジェクト作成ダイアログの画像サイズ入力欄の幅を上げる
	inline class dlg_newfile_t {
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "new_project_editbox";
	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;


			{
				DLGITEMTEMPLATE* width_edit= (DLGITEMTEMPLATE*)(GLOBAL::exedit_base + 0x252d88);
				OverWriteOnProtectHelper h((int)width_edit, sizeof(DLGITEMTEMPLATE));
				width_edit->x = 51; // original = 60
				width_edit->cx = 27; // original = 20
			}
			{
				DLGITEMTEMPLATE* times_static = (DLGITEMTEMPLATE*)(GLOBAL::exedit_base + 0x252da4);
				OverWriteOnProtectHelper h((int)times_static, sizeof(DLGITEMTEMPLATE));
				times_static->x = 78; // original = 81
			}
			{
				DLGITEMTEMPLATE* height_edit = (DLGITEMTEMPLATE*)(GLOBAL::exedit_base + 0x252dc0);
				OverWriteOnProtectHelper h((int)height_edit, sizeof(DLGITEMTEMPLATE));
				height_edit->x = 85; // original = 89
				height_edit->cx = 27; // original = 20
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
	} dialog_new_file;
} // namespace patch

#endif // ifdef PATCH_SWITCH_DIALOG_NEW_FILE
