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
#ifdef PATCH_SWITCH_LUA_RAND

#include "mylua.hpp"

namespace patch {
    // rand関数が0除算例外になるパターンを潰す
    inline class lua_rand_t {
        static int l_rand_overwrite(lua_State* L);

    public:
        static void require(lua_State* L) {
            lua_pushcfunction(L, &l_rand_overwrite);
            lua_setfield(L, -3, "rand");
            lua_pushcfunction(L, &l_rand_overwrite);
            lua_setfield(L, -2, "rand");
        }
    } lua_rand;
} // namespace patch
#endif // ifdef PATCH_SWITCH_LUA_RAND
