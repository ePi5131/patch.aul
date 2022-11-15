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

#ifdef PATCH_SWITCH_FAILED_LONGER_PATH

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 読み込もうとしたファイルパスが長くて失敗するときにメッセージを出す/エラーが発生するのを修正

    // 安全性のために読み込める最大数を1バイト減らしています（260バイトを読み込ませた場合、合成モードなどと繋がってバグる）

    inline class failed_longer_path_t {

        inline static const char str_new_longer_msg[] = "ファイルパスが長いため失敗しました\nファイルやフォルダ名を短くするか、Cドライブ直下などの浅い階層に移動して下さい";
        
        static BOOL __cdecl dlg_get_load_name_wrap(AviUtl::ExFunc* a_exfunc, LPSTR name, LPSTR filter, LPSTR def);
        static UINT __stdcall DragQueryFileA_exedit_wrap(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch);
        static UINT __stdcall DragQueryFileA_settingdialog_wrap(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch);


        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "failed_longer_path";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // exedit_dlg_get_load_name
                /*
                        1002093b ff9280000000   call     dword ptr [edx + 0x80] ; aviutl_exfunc->dlg_get_load_name
                        10020941 83c40c         add      esp,0xc
                        ↓
                        1002093b 52             push     edx
                        1002093c e8XxXxXxXx     call     new_func
                        10020941 83c410         add      esp,0x10
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x02093b, 9);
                h.store_i16(0, '\x52\xe8'); // push edx; call (rel32)
                h.replaceNearJmp(2, &dlg_get_load_name_wrap);
                h.store_i8(8, '\x10');
            }
            
            { // ExEdit D&D
                /*
                        1003be8f ff15c8a10910   call     dword ptr [DragQueryFileA]
                        ↓
                        1003be8f 90             nop
                        1003be90 e8XxXxXxXx     call     new_func
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x03be8f, 6);
                h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
                h.replaceNearJmp(2, &DragQueryFileA_exedit_wrap);
            }

            { // SettingDialog D&D
                /*
                        1002e330 ff15c8a10910   call     dword ptr [DragQueryFileA]
                        ↓
                        1002e330 90             nop
                        1002e331 e8XxXxXxXx     call     new_func
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x02e330, 6);
                h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
                h.replaceNearJmp(2, &DragQueryFileA_settingdialog_wrap);
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
    } failed_longer_path;
} // namespace patch

#endif // ifdef PATCH_SWITCH_FAILED_LONGER_PATH
