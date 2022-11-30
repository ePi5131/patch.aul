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

#ifdef PATCH_SWITCH_WARNING_DUPLICATE_PLUGINS

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // ファイル名が同名のプラグインが複数フォルダに存在するときに警告出す

    inline class warning_duplicate_t {

#define AVIUTL110_BUILDIN_FILTER_N 27
#define AVIUTL110_FILTER_MAX_N 96

//#define MAX_PLUGIN_NUM 512
        inline static const char str_msg[] = "同じプラグインが存在します。予期せぬ動作を起こす危険があるため片方は削除してください\n\n%s\n%s";
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "warning_duplicate";
    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;
            
            auto editp = (AviUtl::EditHandle*)(GLOBAL::aviutl_base + OFS::AviUtl::edit_handle_ptr);
            HWND aviutl_hwnd = editp->aviutl_window_info.main_window;

            auto check_duplicate_plugins = [](char* wild_ext, HWND aviutl_hwnd) {
                char* aviutl_dir = reinterpret_cast<char* (__stdcall*)(void)>(GLOBAL::aviutl_base + OFS::AviUtl::get_exe_dir)();
                char str[261];
                wsprintfA(str, (char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_percent_s_percent_s), aviutl_dir, wild_ext);
                WIN32_FIND_DATA wfd;
                HANDLE hfile = FindFirstFileA(str, &wfd);
                if (hfile != INVALID_HANDLE_VALUE) {
                    do {
                        wsprintfA(str, (char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_percent_s_percent_s_percent_s), aviutl_dir, (char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_plugins_backslash), wfd.cFileName);
                        if (PathFileExistsA(str)) {
                            char str0[261];
                            wsprintfA(str0, (char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_percent_s_percent_s), aviutl_dir, wfd.cFileName);
                            char msg[640];
                            wsprintf(msg, str_msg, str0, str);
                            MessageBoxA(aviutl_hwnd, msg, wild_ext, MB_ICONWARNING | MB_OK);
                        }
                    } while (FindNextFileA(hfile, &wfd));
                    FindClose(hfile);
                }
            };
            check_duplicate_plugins((char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_asterisk_dot_aul), aviutl_hwnd);
            check_duplicate_plugins((char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_asterisk_dot_auc), aviutl_hwnd);
            check_duplicate_plugins((char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_asterisk_dot_aui), aviutl_hwnd);
            check_duplicate_plugins((char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_asterisk_dot_auo), aviutl_hwnd);



            BOOL checked[AVIUTL110_FILTER_MAX_N - AVIUTL110_BUILDIN_FILTER_N];
            for (int i = AVIUTL110_FILTER_MAX_N - AVIUTL110_BUILDIN_FILTER_N - 1; 0 <= i; i--) {
                checked[i] = TRUE;
            }

            auto filter_n = (int*)(GLOBAL::aviutl_base + OFS::AviUtl::filter_n);
            auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
            for (int i = AVIUTL110_BUILDIN_FILTER_N; i < *filter_n - 1; i++) {
                if (checked[i - AVIUTL110_BUILDIN_FILTER_N]) {
                    auto fp1 = (AviUtl::FilterPlugin*)a_exfunc->get_filterp(i);
                    char path1[261];
                    GetModuleFileNameA(fp1->dll_hinst, path1, MAX_PATH);
                    for (int j = i + 1; j < *filter_n; j++) {
                        auto fp2 = (AviUtl::FilterPlugin*)a_exfunc->get_filterp(j);
                        char path2[261];
                        GetModuleFileNameA(fp2->dll_hinst, path2, MAX_PATH);
                        if (lstrcmpiA(PathFindFileNameA(path1), PathFindFileNameA(path2)) == 0) {
                            checked[j - AVIUTL110_BUILDIN_FILTER_N] = FALSE;
                            if (lstrcmpiA(path1, path2)) {
                                char msg[640];
                                wsprintf(msg, str_msg, path1, path2);
                                MessageBoxA(aviutl_hwnd, msg, (char*)(GLOBAL::aviutl_base + OFS::AviUtl::str_asterisk_dot_auf), MB_ICONWARNING | MB_OK);
                            }
                        }
                    }
                }
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
    } WarningDuplicate;
} // namespace patch

#endif // ifdef PATCH_SWITCH_WARNING_DUPLICATE_PLUGINS
