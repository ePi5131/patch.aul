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

#include "patch_aviutl_wndproc_override.hpp"

namespace patch {
    LRESULT CALLBACK aviutl_wndproc_override_t::wrap(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        auto org = [=]() { return aviutl_wndproc_orig(hwnd, message, wparam, lparam); };

        switch (message) {
        case WM_MENUSELECT:
            #ifdef PATCH_SWITCH_CONSOLE
            if ((HMENU)lparam == aviutl_hmenu && HIWORD(wparam) & MF_POPUP) {
                console.update_visible_state(aviutl_hmwnu_disp);
            }
            #endif
            return org();
        case WM_ACTIVATE: {
            static bool first_activate_gate = false;
            auto ret = org();
            if (!first_activate_gate) {
                #ifdef PATCH_SWITCH_CONSOLE
                console.update_showwindow();
                #endif
                first_activate_gate = true;
            }
            return ret;
        }
        case WM_COMMAND: {
            auto menuid = LOWORD(wparam);
            if (20000 <= menuid && menuid < 30000) {
                switch (menuid) {
                    case PATCH_MENU_INFO: {
                        MessageBoxA(hwnd, patchaul_info.get(), "patch.aul info", 0);
                        return 0;
                    }
                    #ifdef PATCH_SWITCH_CONSOLE
                    case PATCH_MENU_CONSOLE: {
                        console.menu_console(aviutl_hmwnu_disp);
                        return 0;
                    }
                    #endif
                }
            }
            break;
        }
        case WM_SYSCOMMAND:
            #ifdef PATCH_SWITCH_CONSOLE
            switch (wparam) {
                case SC_RESTORE: {
                    auto ret = org();
                    if (console.visible) {
                        console.showWindow(SW_RESTORE);
                    }
                    return ret;
                }
                case SC_MINIMIZE: {
                    auto ret = org();
                    if (console.visible) {
                        console.showWindow(SW_MINIMIZE);
                    }
                    return ret;
                }
            }
            #endif
            break;
        }
        return org();
    }
} // namespace patch
