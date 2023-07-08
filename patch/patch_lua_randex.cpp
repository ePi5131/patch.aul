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

#include "patch_lua_randex.hpp"
#ifdef PATCH_SWITCH_LUA_RANDEX

#include <exedit/FilterProcInfo.hpp>

#include "util_magic.hpp"
#include "offset_address.hpp"

namespace patch {

	int lua_randex_t::l_randex(lua_State* L) {
		int seed, time;

		if (auto efpip = *(ExEdit::FilterProcInfo**)(GLOBAL::exedit_base + OFS::ExEdit::efpip_g); efpip != nullptr) {
			seed = efpip->obj_layerp->layer_set * 1913 ^ efpip->obj_index * 757;
			time = efpip->frame - efpip->object_start_frame;
		}
		else {
			seed = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::TraScript_ProcessingObjectIndex) * 1913 ^ load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::TraScript_ProcessingTrackBarIndex) * 757;
			time = static_cast<int>(load_i64<double>(GLOBAL::exedit_base + OFS::ExEdit::TraScript_Time));
		}
		auto n = lua_gettop(L);
		if (n > 0) {
			auto seed_a = luaL_checkinteger(L, 1);
			if (seed_a < 0) seed = -seed_a;
			else seed += seed_a;

			if (n > 1) time = luaL_checkinteger(L, 2);
		}

		create_object(L, seed, time);

		return 1;
	}
#endif
}