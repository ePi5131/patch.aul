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

#include "patch_lua_rand.hpp"
#ifdef PATCH_SWITCH_LUA_RAND

#include <exedit/FilterProcInfo.hpp>

#include "global.hpp"
#include "offset_address.hpp"

namespace patch {
    int lua_rand_t::l_rand_overwrite(lua_State* L) {
        int s = luaL_checkinteger(L, 1);
        int e = luaL_checkinteger(L, 2);
        if (s > e) std::swap(s, e);

        int seed, time;

        if (auto efpip = load_i32<ExEdit::FilterProcInfo*>(GLOBAL::exedit_base + OFS::ExEdit::efpip_g); efpip != nullptr) {
            seed = efpip->obj_layerp->layer_set * 1913 ^ efpip->obj_index * 757;
            time = efpip->frame - efpip->object_start_frame;
        }else {
            seed = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::TraScript_ProcessingObjectIndex) * 1913 ^ load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::TraScript_ProcessingTrackBarIndex) * 757;
            time = static_cast<int>(load_i64<double>(GLOBAL::exedit_base + OFS::ExEdit::TraScript_Time));
        }
        auto n = lua_gettop(L);
        if(n > 2) {
            auto seed_a = luaL_checkinteger(L, 3);
            if(seed_a < 0) seed = -seed_a;
            else seed += seed_a;

            if(n > 3) time = luaL_checkinteger(L, 4);
        }

        auto a = seed + (time << 7 ^ time >> 2 ^ time);
        auto b = (a * 73 ^ 326881591) + a % 7213 + a;
        auto c = b ^ b * 128 ^ (int)b >> 2;
        auto d = (int)((int)c >> 0x10 ^ c);

        if (s == (std::numeric_limits<int>::min)() && e == (std::numeric_limits<int>::max)()) lua_pushinteger(L, d);
        else lua_pushinteger(L, s + d % (e - s + 1));

        return 1;
    }
} // namespace patch
#endif // ifdef PATCH_SWITCH_LUA_RAND
