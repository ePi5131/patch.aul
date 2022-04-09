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
#ifdef PATCH_SWITCH_LUA

#include <cstdint>
#include <vector>
#include <array>
#include <Windows.h>

#include "util.hpp"
#include "global.hpp"
#include "offset_address.hpp"
#include "mylua.hpp"

namespace patch {
    // init at exedit load
    // Luaの環境を一つにまとめる
    inline class lua_t {
        struct env {
            inline static constexpr std::ptrdiff_t target[] = {
                0x1b2b4c, // sScriptFolderName
                0x1b28d4, // obj_layer_ptr
                0x1baca4, // script_objectp
                0x1b2b20, // efpip_g
                0x1b2368, // textbuffer_pointer
                0x1b28b4, // textbuffer_max
                0x1b2204, // textbuffer_current
                0x1b28d0, // draw_state
                0x1b249c, // something_drawstate1
                0x1b24a0, // something_drawstate2
                0x1b21e4, // draw_target
                0x1b28dc, // some_flag
                0x1b21f0, // obj_setanchor_called_count
            };

            inline static constexpr std::ptrdiff_t target_tmp[] = {
                0x1b24b4, // sScriptFolderName
                0x1bac94, // obj_layer_ptr
                0x1b2b08, // script_objectp
                0x1b24b8, // efpip_g
                0x1b28bc, // textbuffer_pointer
                0x1b29f4, // textbuffer_max
                0x1b21e0, // textbuffer_current
                0x1bacb8, // draw_state
                0x1b2494, // something_drawstate1
                0x1b2498, // something_drawstate2
                0x1b23bc, // draw_target
                0x1b2b40, // some_flag
                0x1b2b2c, // obj_setanchor_called_count
            };

            std::array<DWORD, std::size(target)> tmp;
            std::optional<std::string> folder_name;

            inline static std::vector<env> vec;

            static void load() {
                auto& e = vec.back();
                for(size_t i = 0; i < std::size(target); i++) {
                    store_i32(GLOBAL::exedit_base + target[i], e.tmp[i]);
                }
                if (vec.size()) {
                    for (size_t i = 0; i < std::size(target); i++) {
                        store_i32(GLOBAL::exedit_base + target_tmp[i], vec.back().tmp[i]);
                    }
                }
                vec.pop_back();

            }

            static void store() {
                vec.push_back({});
                auto& e = vec.back();
                for(size_t i = 0; i < std::size(target); i++) {
                    auto val = load_i32<DWORD>(GLOBAL::exedit_base + target[i]);
                    e.tmp[i] = val;
                    store_i32(GLOBAL::exedit_base + target_tmp[i], val);
                }
            }
        };
        inline static lua_State* state = nullptr;

        static lua_State* __cdecl luaGetOrCreateState(int idx);
        static lua_State* __cdecl luaGetOrCreateState_envpatch(int idx);

        static void __cdecl luaUnload();

        static BOOL __cdecl luaUpdatePath(lua_State* L);

        static BOOL __cdecl DoScriptInit();
        static BOOL __cdecl DoScriptExit();

        static double __cdecl lua_tonumber_remove_stack(lua_State* L, int idx);
        inline static auto lua_tonumber_remove_stack_ptr = &lua_tonumber_remove_stack;

        static const char* __cdecl lua_tolstring_remove_stack(lua_State* L, int idx, size_t* len);
        inline static auto lua_tolstring_remove_stack_ptr = &lua_tolstring_remove_stack;

        static bool enabled() {
            return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_LUA);
        }

    public:
        void operator()() {
            if (!enabled())return;

            if (PATCH_SWITCHER_MEMBER(PATCH_SWITCH_LUA_ENV)) {
                ReplaceFunction(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateLuaState, &luaGetOrCreateState_envpatch);
                ReplaceFunction(GLOBAL::exedit_base + OFS::ExEdit::LuaUnload, &luaUnload);
                ReplaceFunction(GLOBAL::exedit_base + OFS::ExEdit::DoScriptInit, &DoScriptInit);
                ReplaceFunction(GLOBAL::exedit_base + OFS::ExEdit::DoScriptExit, &DoScriptExit);
            }
            else {
                ReplaceFunction(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateLuaState, &luaGetOrCreateState);
            }


            if (PATCH_SWITCHER_MEMBER(PATCH_SWITCH_LUA_PATH)) {
                ReplaceFunction(GLOBAL::exedit_base + OFS::ExEdit::SetLuaPathAndCpath, &luaUpdatePath);
            }


            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::lua_pop_nop, 6);
                memset((void*)(GLOBAL::exedit_base + OFS::ExEdit::lua_pop_nop), '\x90', 6);
            }
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::lua_set_nop, 6);
                memset((void*)(GLOBAL::exedit_base + OFS::ExEdit::lua_set_nop), '\x90', 6);
            }
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::lua_tonumber_arg, 8);
                h.store_i8(0, 0xff); // (push) 0xff
                h.store_i32(4, &lua_tonumber_remove_stack_ptr);
            }
            
            OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::lua_tostring_calling1, 4).store_i32(0, &lua_tolstring_remove_stack_ptr);
            OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::lua_tostring_calling2, 4).store_i32(0, &lua_tolstring_remove_stack_ptr);
            OverWriteOnProtectHelper(GLOBAL::exedit_base + OFS::ExEdit::lua_tostring_calling3, 4).store_i32(0, &lua_tolstring_remove_stack_ptr);
        }
    } lua;
} // namespace patch
#endif // ifdef PATCH_SWITCH_LUA
