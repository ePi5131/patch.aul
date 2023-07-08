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

#ifdef PATCH_SWITCH_YC_RGB_CVT

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // YC_RGB変換の際に幅0のオブジェクトが渡されたときにエラーとなるのを修正(主に改行のみのテキスト)

    inline class yc_rgb_cvt_t {

        static void __cdecl do_multi_thread_func_wrap(AviUtl::MultiThreadFunc func, BOOL flag);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "yc_rgb_cvt";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            {   // rgb2yc normal
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x6f585, 28);
                h.replaceNearJmp(0, do_multi_thread_func_wrap);
                h.replaceNearJmp(24, do_multi_thread_func_wrap);
            }
            {   // rgb2yc
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x6f8a3, 31);
                h.replaceNearJmp(0, do_multi_thread_func_wrap);
                h.replaceNearJmp(27, do_multi_thread_func_wrap);
            }

            {   // yc2rgb
                ReplaceNearJmp(GLOBAL::exedit_base + 0x6fbb8, do_multi_thread_func_wrap);
                // もう1つの方は置き換え不要
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
    } yc_rgb_cvt;
} // namespace patch

#endif // ifdef PATCH_SWITCH_YC_RGB_CVT
