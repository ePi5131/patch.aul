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

#ifdef PATCH_SWITCH_RENDERING
#include <exedit.hpp>
#include "global.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // 高さ4097以上描画が正常に行えないことがあるのを修正
    inline class rendering_t {

        static int __stdcall mid_render();
        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "rendering";

    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            {
                /*
                    1007caaf 3d00100000         cmp     eax,00001000
                    1007cab4 5e                 pop     esi
                    1007cab5 7d06               jnl     1007cabd
                    1007cab7 40                 inc     eax
                    1007cab8 a328c71e10         mov     [101ec728],eax
                    1007cabd 5f                 pop     edi
                    1007cabe 5d                 pop     ebp
                    1007cabf 5b                 pop     ebx
                    1007cac0 83c41c             add     esp,+1c
                    1007cac3 c3                 ret
                    1007cac4 90                 nop
                    1007cac5 90                 nop
                    1007cac6 90                 nop
                    1007cac7 90                 nop
                    1007cac8 90                 nop
                    1007cac9 90                 nop
                    1007caca 90                 nop
                    1007cacb 90                 nop

                    ↓

                    1007caaf 3dff0f0000         cmp     eax,00000fff ; rendering_data_count == 4095
                    1007cab4 5e                 pop     esi
                    1007cab5 740d               jz      1007cac4
                    1007cab7 40                 inc     eax
                    1007cab8 a328c71e10         mov     [101ec728],eax ; rendering_data_count
                    1007cabd 5f                 pop     edi
                    1007cabe 5d                 pop     ebp
                    1007cabf 5b                 pop     ebx
                    1007cac0 83c41c             add     esp,+1c
                    1007cac3 c3                 ret
                    1007cac4 e8XxXxXxXx         call    ;__stdcall return 0;
                    1007cac9 ebed               jmp     1007cab8
                    1007cacb 90                 nop

                    描画の行データが4096に達する時、描画関数を実行しカウントを0にする
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x7cab0, 27);
                h.store_i16(0, '\xff\x0f');
                h.store_i16(5, '\x74\x0d');
                h.store_i8(20, '\xe8');
                h.replaceNearJmp(21, &mid_render);
                h.store_i16(25, '\xeb\xed');

            }


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

    } Rendering;
} // namespace patch
#endif // ifdef PATCH_SWITCH_RENDERING
