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
#ifdef PATCH_SWITCH_LUA_RANDEX

#include <bit>

#include "config_rw.hpp"

#include "mylua.hpp"

namespace patch {
	// 正しい乱数分布になるようなrandexクラスを追加
	inline class lua_randex_t {
		struct Xoshiro128pp {
			uint32_t s[4];
			Xoshiro128pp(unsigned int seed1, unsigned int seed2) : s{ seed1,seed2,1000000007,998244353 } {
				for (int i = 0; i < 32; i++) calc();
			}
			uint32_t calc() {
				auto ret = std::rotl(s[0] + s[3], 7) + s[0];
				auto t = s[1] << 9;
				s[2] ^= s[0];
				s[3] ^= s[1];
				s[1] ^= s[2];
				s[0] ^= s[3];

				s[2] ^= t;
				s[3] = std::rotl(s[3], 11);
				return ret;
			}
			int calc(int s, int e) {
				auto r = calc();
				if (s > e) std::swap(s, e);
				if (s == (std::numeric_limits<int>::min)() && e == (std::numeric_limits<int>::max)()) return std::bit_cast<int>(r);
				return s + r % (e - s + 1);
			}
		};

		static int l_randex(lua_State* L);

		static void setmetatable(lua_State* L, int index) {
			luaL_getmetatable(L, "randex");
			lua_setmetatable(L, index > 0 ? index : index - 1);
		}

		static Xoshiro128pp* create_object(lua_State* L, int seed, int time) {
			auto udata = lua_newuserdata(L, sizeof(Xoshiro128pp));
			auto rnd = new(udata) Xoshiro128pp(seed, time);
			setmetatable(L, -1);
			return rnd;
		}

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "lua.randex";
	public:
		void init() {
			enabled_i = enabled;
		}

		static void require(lua_State* L) {
			luaL_newmetatable(L, "randex");
			lua_pushcclosure(L, [](lua_State* L) {
				auto rnd = static_cast<Xoshiro128pp*>(lua_touserdata(L, 1));
				int s = luaL_checkinteger(L, 2);
				int e = luaL_checkinteger(L, 3);
				lua_pushinteger(L, rnd->calc(s, e));
				return 1;
			}, 0);
			lua_setfield(L, -2, "__call");

			lua_pop(L, 1);


			lua_pushcfunction(L, l_randex);
			lua_setfield(L, -3, "randex");
			lua_pushcfunction(L, l_randex);
			lua_setfield(L, -2, "randex");
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
	} lua_randex;
}
#endif
