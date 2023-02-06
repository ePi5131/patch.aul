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

#ifdef PATCH_SWITCH_SCRIPT_SORT_PATCH
#include <Windows.h>

#include <exedit.hpp>

#include "cryptostring.hpp"
#include "util_others.hpp"
#include "util_resource.hpp"
#include "config_rw.hpp"

namespace patch {

    inline class patch_script_sort_t {
        inline static LONG dwOldLong;
        static LONG __stdcall scipt_sort_SetWindowLongA_Wrap(HWND hWnd, int nIndex, LONG dwNewLong) {
            if (hWnd == *(HWND*)(GLOBAL::exedit_base + OFS::ExEdit::settingdialog_hwnd)) {
                if (nIndex == GWL_WNDPROC) {
                    if (dwNewLong == 0) {
                        dwNewLong = dwOldLong;
                    } else {
                        dwOldLong = SetWindowLongA(hWnd, nIndex, dwNewLong);
                        return dwOldLong;
                    }
                }
            }
            return SetWindowLongA(hWnd, nIndex, dwNewLong);
        }

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "patch_script_sort";
    public:
        void init(HMODULE ret) {
            enabled_i = enabled;
            if (!enabled_i)return;

            if (auto ptr = search_import(ret, cstr_user32_dll.get(), cstr_SetWindowLongA.get())) {
                OverWriteOnProtectHelper(ptr, 4).store_i32(0, &scipt_sort_SetWindowLongA_Wrap);
            }
        }

        void operator()(HWND hwnd) {
            if (!enabled_i)return;
            int dlg_obj_idx = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SettingDialog_ObjIdx);
            if (dlg_obj_idx < 0) {
                return;
            }
            int filter_idx = reinterpret_cast<int(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::get_last_filter_idx)(dlg_obj_idx);
            auto ofi = (ExEdit::ObjectFilterIndex)((filter_idx << 16) + dlg_obj_idx + 1);
            auto efp = reinterpret_cast<ExEdit::Filter*(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_filterp)(ofi);
            if (*(int*)&efp->flag & 0x40000000) {
                filter_idx--;
                ofi = (ExEdit::ObjectFilterIndex)((filter_idx << 16) + dlg_obj_idx + 1);
            }
            #define GET_HWND_COMBOBOX 6
            HWND cb_hwnd = reinterpret_cast<HWND(__cdecl*)(ExEdit::ObjectFilterIndex, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_4c)(ofi, GET_HWND_COMBOBOX, 0);
            SendMessageA(hwnd, WM_CTLCOLOREDIT, 0, (LPARAM)cb_hwnd);
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
    } patch_script_sort;
}
#endif
