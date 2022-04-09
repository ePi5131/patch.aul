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
#include <cstdint>
#include <bit>
#include <string>
#include <Windows.h>

#include "global_minimum.hpp"
#include "configfile.hpp"

namespace GLOBAL {
	inline HMODULE exedit_hmod;
	inline uint32_t& exedit_base = (uint32_t&)exedit_hmod;
	inline std::byte executable_memory[USN_PAGE_SIZE * 16];
	inline std::byte* executable_memory_cursor = executable_memory;
	inline std::wstring patchaul_path;
	inline std::wstring patchaul_config_path;
	inline std::string patchaul_path_a;
	inline Config config;
}
