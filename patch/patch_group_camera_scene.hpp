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

#ifdef PATCH_SWITCH_GROUP_CAMERA_SCENE
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"


namespace patch {

    // init at exedit load
    // グループ制御とカメラ制御を併用しているシーンを呼び出すと起こる不具合を修正
    inline class group_camera_scene_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "group_camera_scene";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto& cursor = GLOBAL::executable_memory_cursor;

            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x01b056, 6);
            h.store_i16(0, '\x90\xe8');
            h.replaceNearJmp(2, cursor);
            /*
                1001b056 8b5004             mov     edx,dword ptr [eax+04] ; edx = obj->layer_disp
                1001b059 8b4d10             mov     ecx,dword ptr [ebp+10]

                ↓

                00000000 8b90c0050000       mov     edx,dword ptr [eax+000005c0] ; ebp = obj->layer_setting
                00000000 8b4d10             mov     ecx,dword ptr [ebp+10]
            */

            static const char code_put[] =
                "\x8b\x90\xc0\x05\x00\x00" // mov     edx,dword ptr [eax+000005c0]
                "\x8b\x4d\x10"             // mov     ecx,dword ptr [ebp+10]
                "\xc3"                     // ret
                ;

            memcpy(cursor, code_put, sizeof(code_put) - 1);
            cursor += sizeof(code_put) - 1;
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
    } group_camera_scene;
} // namespace patch

#endif // ifdef PATCH_SWITCH_GROUP_CAMERA_SCENE
