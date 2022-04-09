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
#include "global.hpp"

/*
    patch内でluaを使うとき用のコード群
*/

using lua_State = struct lua_State;
using lua_Number = double;
using lua_Integer = ptrdiff_t;
using lua_CFunction = int(__cdecl*)(lua_State* L);
using lua_Reader = const char* (__cdecl*)(lua_State* L, void* data, size_t* size);
using lua_Writer = int(__cdecl*)(lua_State* L, const void* data, size_t sz, void* ud);

constexpr const int LUA_REGISTRYINDEX = -10000;
constexpr const int LUA_ENVIRONINDEX = -10001;
constexpr const int LUA_GLOBALSINDEX = -10002;
constexpr auto lua_upvalueindex(int i) { return LUA_GLOBALSINDEX - i; }

enum {
    LUA_TNONE = -1,
    LUA_TNIL = 0,
    LUA_TBOOLEAN = 1,
    LUA_TLIGHTUSERDATA = 2,
    LUA_TNUMBER = 3,
    LUA_TSTRING = 4,
    LUA_TTABLE = 5,
    LUA_TFUNCTION = 6,
    LUA_TUISERDATA = 7,
    LUA_TTHREAD = 8,
};

enum {
    LUA_YIELD = 1,
    LUA_ERRRUN = 2,
    LUA_ERRSYNTAX = 3,
    LUA_ERRMEM = 4,
    LUA_ERRERR = 5,
};

struct luaL_Reg {
    const char* name;
    lua_CFunction* func;
};

#define defluafunc(offset, ret, ...) (*reinterpret_cast<ret (__cdecl**)(__VA_ARGS__)>(GLOBAL::exedit_base + offset))
#define lua_close			    defluafunc(0x1b24ac, void, lua_State*)
#define lua_createtable		    defluafunc(0x1b2b30, void, lua_State*, int, int)
#define lua_getfield		    defluafunc(0x1bacc8, void, lua_State*, int, const char*)
#define lua_gettable            defluafunc(0x1bab68, void, lua_State*, int)
#define lua_gettop			    defluafunc(0x1b23ac, int, lua_State*)
#define lua_isnumber		    defluafunc(0x1b2b38, int, lua_State*, int)
#define lua_isuserdata		    defluafunc(0x1b24a4, int, lua_State*, int)
#define lua_newuserdata         defluafunc(0x1b248c, void*, lua_State*, size_t)
#define lua_pcall			    defluafunc(0x1bac8c, int, lua_State*, int, int, int)
#define lua_pushboolean		    defluafunc(0x1b2b44, void*, lua_State*, int)
#define lua_pushinteger		    defluafunc(0x1bab78, void*, lua_State*, int)
#define lua_pushlightuserdata   defluafunc(0x1b2b00, void, lua_State*, void*)
#define lua_pushnil             defluafunc(0x1b23c0, void, lua_State*)
#define lua_pushnumber		    defluafunc(0x1b28d8, void, lua_State*, lua_Number)
#define lua_pushstring          defluafunc(0x1b28e0, void, lua_State*, const char*)
#define lua_setfield            defluafunc(0x1bab5c, void, lua_State*, int, const char*)
#define lua_sethook             defluafunc(0x1b23b4, int, lua_State*, lua_Hook, int, int)
#define lua_settable            defluafunc(0x1b2320, void, lua_State*, int)
#define lua_settop			    defluafunc(0x1b23c4, void, lua_State*, int)
#define lua_toboolean           defluafunc(0x1b2b14, int, lua_State*, int)
#define lua_tointeger           defluafunc(0x1b29f0, lua_Integer, lua_State*, int)
#define lua_tolstring		    defluafunc(0x1b21ec, const char*, lua_State*, int, size_t*)
#define lua_tonumber		    defluafunc(0x1b21e8, lua_Number, lua_State*, int)
#define lua_touserdata		    defluafunc(0x1b2490, void*, lua_State*, int)
#define lua_type			    defluafunc(0x1bab70, int, lua_State*, int)

#define luaL_checkinteger	    defluafunc(0x1b2b48, lua_Integer, lua_State*, int)
#define luaL_checklstring	    defluafunc(0x1bacb4, const char*, lua_State*, int, size_t*)
#define luaL_checknumber	    defluafunc(0x1b220c, lua_Number, lua_State*, int)
#define luaL_error              defluafunc(0x1b2488, int, lua_State*, const char*, ...)
#define luaL_loadstring         defluafunc(0x1b2b1c, int, lua_State*, const char*)
#define luaL_newstate		    defluafunc(0x1b2208, lua_State*, void)
#define luaL_openlibs		    defluafunc(0x1b28e4, void, lua_State*)
#define luaL_register		    defluafunc(0x1b2b24, void, lua_State*, const char*, luaL_Reg*)

inline int(__cdecl* lua_isstring)(lua_State* L, int idx);
inline void(__cdecl* lua_pushcclosure)(lua_State* L, lua_CFunction fn, int n);
inline void(__cdecl* lua_remove)(lua_State* L, int index);
inline void(__cdecl* lua_setmetatable)(lua_State* L, int index);
inline int(__cdecl* luaL_newmetatable)(lua_State* L, const char* tname);
inline void(__cdecl* lua_pushvalue)(lua_State* L, int index);

inline bool lua_isfunction(lua_State* L, int index) { return lua_type(L, index) == LUA_TFUNCTION; }
inline bool lua_istable(lua_State* L, int index) { return lua_type(L, index) == LUA_TTABLE; }
inline bool lua_islightuserdata(lua_State* L, int index) { return lua_type(L, index) == LUA_TLIGHTUSERDATA; }
inline bool lua_isnil(lua_State* L, int index) { return lua_type(L, index) == LUA_TNIL; }
inline bool lua_isboolean(lua_State* L, int index) { return lua_type(L, index) == LUA_TBOOLEAN; }
inline bool lua_isthread(lua_State* L, int index) { return lua_type(L, index) == LUA_TTHREAD; }
inline bool lua_isnone(lua_State* L, int index) { return lua_type(L, index) == LUA_TNONE; }
inline bool lua_isnoneornil(lua_State* L, int index) { return lua_type(L, index) <= 0; }

//template<size_t N> inline auto lua_pushliteral(lua_State* L, const char(&s)[N]) { return lua_pushlstring(L, s, N); }
inline auto lua_tostring(lua_State* L, int i) { return lua_tolstring(L, i, nullptr); }
inline auto luaL_checkstring(lua_State* L, int i) { return luaL_checklstring(L, i, nullptr); }
inline auto lua_pushcfunction(lua_State* L, lua_CFunction f) { return lua_pushcclosure(L, f, 0); }
inline auto lua_pop(lua_State* L, int n) { return lua_settop(L, -n - 1); }
inline auto lua_setglobal(lua_State* L, const char* s) { return lua_setfield(L, LUA_GLOBALSINDEX, s); }
inline auto lua_getglobal(lua_State* L, const char* s) { return lua_getfield(L, LUA_GLOBALSINDEX, s); }
inline auto lua_newtable(lua_State* L) { return lua_createtable(L, 0, 0); }

inline auto luaL_getmetatable(lua_State* L, const char* tname) { return lua_getfield(L, LUA_REGISTRYINDEX, tname); }
