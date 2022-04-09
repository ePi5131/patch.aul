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
#ifdef PATCH_SWITCH_SYSINFO_MODIFY

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "version.hpp"

namespace patch {
	// init at dllload
	// Sysinfo::versionの書き換え
	inline class sysinfo_info_write_t {
	private:
		inline static const char str[] = "1.10 (patched " PATCH_VERSION_STR ")";
	public:
		void operator()() {
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + OFS::AviUtl::getsys_versionstr_arg, 4).store_i32(0, &str);
		}
	} sysinfo_info_write;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SYSINFO_MODIFY
