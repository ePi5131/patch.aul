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
#ifdef PATCH_SWITCH_DEBUGSTRING

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "patch_console.hpp"

namespace patch {
	// init at exedit load
	// require console_init
	// OutputDebugStringの内容をコンソールに表示する
	inline class override_debugstring_t {
		static void __stdcall debug_print_override(LPCSTR lpOutputString);
		static void __stdcall exedit_lua_error_override(LPCSTR lpOutputString);
		
		inline static constexpr auto* debug_print_override_ptr = &debug_print_override;
		inline static constexpr auto* exedit_lua_error_override_ptr = &exedit_lua_error_override;

		bool enabled;
		bool enabled_i;

	public:
		void operator()() {
			enabled_i = enabled;
			if (!enabled_i)return;
			if (!console.is_valid()) return;
			
			OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::OutputDebugString_calling_err1, 4).store_i32(0, &exedit_lua_error_override_ptr);
			OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::OutputDebugString_calling_err2, 4).store_i32(0, &exedit_lua_error_override_ptr);
			OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::OutputDebugString_calling_err3, 4).store_i32(0, &exedit_lua_error_override_ptr);
			OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::OutputDebugString_calling_dbg, 4).store_i32(0, &debug_print_override_ptr);
		}
	} override_debugstring;
} // namespace patch
#endif // #ifdef PATCH_SWITCH_DEBUGSTRING
