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

#ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // ファイル名（パス）が原因で読み込み失敗のエラー表記を変更する

    // **ファイルの読み込みに失敗しました 対応していないフォーマットの可能性があります
    // ↓
    // パス(ファイルもしくはフォルダ名)に使用できない文字が含まれています

    inline class failed_sjis_msgbox_t {

        inline static const char str_new_failed_msg[] = "ファイルパスに使用できない文字が含まれています\nファイル名やフォルダ名を確認してください";
        static int __stdcall MessageBoxA_1(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
        static int __stdcall MessageBoxA_2(LPCSTR path, HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "failed_sjis_msgbox";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // audio & movie
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00522b, 6);
                h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
                h.replaceNearJmp(2, &MessageBoxA_1);
            }

            { // image_file_wndproc
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00e173, 11);
                h.store_i32(0, '\x8d\x44\x24\x24'); // lea  eax,dword ptr [esp+24]
                h.store_i32(4, '\x50\x90\xe8\x00'); // push eax, nop, call (rel32)
                h.replaceNearJmp(7, &MessageBoxA_2);
            }
            { // border_wndproc
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0521c3, 11);
                h.store_i32(0, '\x8d\x44\x24\x24'); // lea  eax,dword ptr [esp+24]
                h.store_i32(4, '\x50\x90\xe8\x00'); // push eax, nop, call (rel32)
                h.replaceNearJmp(7, &MessageBoxA_2);
            }
            { // shadow_wndproc
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x088ef6, 11);
                h.store_i32(0, '\x8d\x44\x24\x24'); // lea  eax,dword ptr [esp+24]
                h.store_i32(4, '\x50\x90\xe8\x00'); // push eax, nop, call (rel32)
                h.replaceNearJmp(7, &MessageBoxA_2);
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
    } failed_sjis_msgbox;
} // namespace patch

#endif // ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX
