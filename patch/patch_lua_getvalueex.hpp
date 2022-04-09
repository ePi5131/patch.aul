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
#ifdef PATCH_SWITCH_LUA_GETVALUE

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

#include "mylua.hpp"

namespace patch {
	// obj.getvalue で中心座標をもらえるようにする
	class lua_getvalueex_t {
		static int __cdecl lua_getvalue_override(lua_State* L);

		// index
		static int __cdecl lua_getvalueex_main(lua_State* L);

	public:

		inline static void require(lua_State* L) {
			lua_pushcfunction(L, &lua_getvalue_override);
			lua_setfield(L, -2, "getvalue");

			lua_pushcfunction(L, &lua_getvalueex_main);
			lua_setfield(L, -2, "getvalueex");
		}
	};
} // namespace patch
#endif // ifdef PATCH_SWITCH_LUA_GETVALUE
