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
#include <bit>
#include <cstdint>
#include <Windows.h>
#include <winwrap.hpp>

namespace GLOBAL {
	inline HMODULE aviutl_hmod;
	inline uint32_t& aviutl_base = (uint32_t&)aviutl_hmod;
	inline HINSTANCE patchaul_hinst;

	inline void init_minimum(HINSTANCE patch_hmod) {
		GLOBAL::patchaul_hinst = patch_hmod;
		GLOBAL::aviutl_base = std::bit_cast<uint32_t>(WinWrap::Module::getCallingProcessModule());
	}
}
