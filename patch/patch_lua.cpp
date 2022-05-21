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

#include "patch_lua.hpp"
#ifdef PATCH_SWITCH_LUA

#include <format>

#include "debug_log.hpp"
#include "global.hpp"
#include "offset_address.hpp"

#include "patch_lua_getvalueex.hpp"
#include "patch_lua_rand.hpp"
#include "patch_lua_randex.hpp"

namespace patch {
    
    static void assign_to_luastate(lua_State* L) {
        // _G, obj の順で積んでる

        lua_pushstring(L, PATCH_VERSION_NAME);
        lua_setfield(L, -3, "_PATCH");

        #ifdef PATCH_SWITCH_LUA_RAND
            if (lua_rand.is_enabled_i()) lua_rand_t::require(L);
        #endif

        #ifdef PATCH_SWITCH_LUA_RANDEX
            if (lua_randex.is_enabled_i()) lua_randex_t::require(L);
        #endif

        #ifdef PATCH_SWITCH_LUA_GETVALUE
            if (lua_getvalueex.is_enabled_i()) lua_getvalueex_t::require(L);
        #endif
    }

    lua_State* __cdecl lua_t::luaGetOrCreateState(int idx) {
        if (idx > 1) return nullptr;
        auto LuaState = (lua_State**)(GLOBAL::exedit_base + 0x1baca8);
        auto& L = LuaState[idx];
        if (L == nullptr) {
            auto loadLua = (BOOL(__cdecl*)())(GLOBAL::exedit_base + OFS::ExEdit::LoadLua);
            if (loadLua() != TRUE) return nullptr;
            auto mod = load_i32<HMODULE>(GLOBAL::exedit_base + OFS::ExEdit::hmodule_lua);
            #define def(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(mod, #name))
                def(lua_isstring);
                def(lua_pushcclosure);
                def(lua_remove);
                def(lua_setmetatable);
                def(luaL_newmetatable);
                def(lua_pushvalue);
            #undef def

            L = luaL_newstate();
            if (L == nullptr)return nullptr;

            luaL_openlibs(L);
            luaL_register(L, "_G", (luaL_Reg*)(GLOBAL::exedit_base + OFS::ExEdit::luaL_Reg_global_table));
            luaL_register(L, "obj", (luaL_Reg*)(GLOBAL::exedit_base + OFS::ExEdit::luaL_Reg_obj_table));

            assign_to_luastate(L);

            lua_pop(L, 2);
        }
        return L;
    }

    lua_State* __cdecl lua_t::luaGetOrCreateState_envpatch(int idx) {

        if(lua_t::state == nullptr) {
            auto loadLua = (BOOL(__cdecl*)())(GLOBAL::exedit_base + OFS::ExEdit::LoadLua);
            if(loadLua() != TRUE) return nullptr;

            auto mod = load_i32<HMODULE>(GLOBAL::exedit_base + OFS::ExEdit::hmodule_lua);
            #define def(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(mod, #name))
                def(lua_isstring);
                def(lua_pushcclosure);
                def(lua_remove);
                def(lua_setmetatable);
                def(luaL_newmetatable);
                def(lua_pushvalue);
            #undef def
            
            auto state = luaL_newstate();
            if(state == nullptr) return nullptr;

            lua_t::state = state;
            luaL_openlibs(state);
            
            luaL_register(state, "_G", (luaL_Reg*)(GLOBAL::exedit_base + OFS::ExEdit::luaL_Reg_global_table));
            luaL_register(state, "obj", (luaL_Reg*)(GLOBAL::exedit_base + OFS::ExEdit::luaL_Reg_obj_table));
            
            assign_to_luastate(state);

            lua_pop(state, 2);

            //inject_viewer();
            return state;
        }
        return lua_t::state;
    }
    
    void __cdecl lua_t::luaUnload() {
        if(state) {
            lua_close(state);
            state = nullptr;
        }
        auto& hmod_lua = load_i32<HMODULE&>(GLOBAL::exedit_base + OFS::ExEdit::hmodule_lua);
        if(hmod_lua != NULL) {
            FreeLibrary(hmod_lua);
            hmod_lua = NULL;
        }
    }

    BOOL __cdecl lua_t::luaUpdatePath(lua_State* L) {
        //debug_log("luaUpdatePath()");
        lua_getglobal(L, "package");
        const auto exedit_dir = load_i32<char*>(GLOBAL::exedit_base + OFS::ExEdit::exeditdir);
        if(auto folder_name = load_i32<char*>(GLOBAL::exedit_base + OFS::ExEdit::sScriptFolderName); folder_name == nullptr || folder_name[0] == '\0') {
            lua_pushstring(L, "{0}?.lua;{0}script\\?.lua;{0}module\\?.lua;{0}module\\?\\init.lua"_fmt(exedit_dir).c_str());
            lua_setfield(L, -2, "path");

            lua_pushstring(L, "{0}?.dll;{0}script\\?.dll;{0}module\\?.dll"_fmt(exedit_dir).c_str());
            lua_setfield(L, -2, "cpath");
        }
        else {
            lua_pushstring(L, "{0}?.lua;{0}script\\?.lua;{0}script\\{1}\\?.lua;{0}module\\?.lua;{0}module\\?\\init.lua"_fmt(exedit_dir, folder_name).c_str());
            lua_setfield(L, -2, "path");

            lua_pushstring(L, "{0}?.dll;{0}script\\?.dll;{0}script\\{1}\\?.dll;{0}module\\?.dll"_fmt(exedit_dir, folder_name).c_str());
            lua_setfield(L, -2, "cpath");
        }
        lua_pop(L, 1);
        return TRUE;
    }

    double __cdecl lua_t::lua_tonumber_remove_stack(lua_State* L, int idx) {
        //view_stack(L, "hoge");
        auto ret = lua_tonumber(L, idx);
        lua_remove(L, idx);
        //view_stack(L, "hoge");
        return ret;
    }

    const char* __cdecl lua_t::lua_tolstring_remove_stack(lua_State* L, int idx, size_t* len) {
        auto ret = lua_tolstring(L, idx, len);
        lua_remove(L, idx);
        return ret;
    }
    
    static int luastateidx_internal = 0;
    BOOL __cdecl lua_t::DoScriptInit() {
        env::store();
        luastateidx_internal++;
        store_i32(GLOBAL::exedit_base + OFS::ExEdit::luastateidx, (std::min)(luastateidx_internal, 1));
        return TRUE;
    }

    BOOL __cdecl lua_t::DoScriptExit() {
        env::load();
        luastateidx_internal--;
        store_i32(GLOBAL::exedit_base + OFS::ExEdit::luastateidx, (std::min)(luastateidx_internal, 1));
        ((BOOL(__cdecl*)(lua_State*))(GLOBAL::exedit_base + OFS::ExEdit::SetLuaPathAndCpath))(luaGetOrCreateState_envpatch(0));
        return TRUE;
    }
} // namespace patch
#endif // ifdef PATCH_SWITCH_LUA
