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

#ifdef PATCH_SWITCH_WARNING_DUPLICATE_PLUGINS

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at dllload
	// ファイル名が同名のプラグインが複数フォルダに存在するときに警告出す
	inline class warning_duplicate_t {

		//#define MAX_PLUGIN_NUM 512
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "warning_duplicate";

		static HMODULE WINAPI LoadLibraryA_auc_wrap(LPCSTR lpLibFileName);
		static HMODULE WINAPI LoadLibraryA_aui_wrap(LPCSTR lpLibFileName);
		static HMODULE WINAPI LoadLibraryA_auo_wrap(LPCSTR lpLibFileName);
		static HMODULE WINAPI LoadLibraryA_auf_wrap(LPCSTR lpLibFileName);
		static HMODULE WINAPI LoadLibraryA_aul_wrap(LPCSTR lpLibFileName);

		inline static const auto LoadLibraryA_auc_wrap_ptr = &LoadLibraryA_auc_wrap;
		inline static const auto LoadLibraryA_aui_wrap_ptr = &LoadLibraryA_aui_wrap;
		inline static const auto LoadLibraryA_auo_wrap_ptr = &LoadLibraryA_auo_wrap;
		inline static const auto LoadLibraryA_auf_wrap_ptr = &LoadLibraryA_auf_wrap;
		inline static const auto LoadLibraryA_aul_wrap_ptr = &LoadLibraryA_aul_wrap;

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x024cec, 4).store_i32(0, &LoadLibraryA_auc_wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x024efc, 4).store_i32(0, &LoadLibraryA_auc_wrap_ptr);

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x0257a7, 4).store_i32(0, &LoadLibraryA_aui_wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x025b77, 4).store_i32(0, &LoadLibraryA_aui_wrap_ptr);

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x02aee1, 4).store_i32(0, &LoadLibraryA_auo_wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x02b304, 4).store_i32(0, &LoadLibraryA_auo_wrap_ptr);
			
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x02ea80, 4).store_i32(0, &LoadLibraryA_auf_wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x02ec75, 4).store_i32(0, &LoadLibraryA_auf_wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x02ef4d, 4).store_i32(0, &LoadLibraryA_auf_wrap_ptr);

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x054437, 4).store_i32(0, &LoadLibraryA_aul_wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x05469d, 4).store_i32(0, &LoadLibraryA_aul_wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x0548b9, 4).store_i32(0, &LoadLibraryA_aul_wrap_ptr);
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
	} WarningDuplicate;
} // namespace patch

#endif // ifdef PATCH_SWITCH_WARNING_DUPLICATE_PLUGINS
