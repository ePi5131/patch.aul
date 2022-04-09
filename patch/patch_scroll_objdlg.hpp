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
#ifdef PATCH_SWITCH_SCROLL_OBJDLG

#include <WIndows.h>

#include "offset_address.hpp"
#include "util.hpp"

namespace patch {
    // init at exedit load
    // 設定ダイアログサイズ固定化
    inline class scroll_objdlg_t {

        static void movewindow(HWND hwnd, int x, int y) {
            
            SetWindowPos(hwnd, NULL, x, y, NULL, NULL, SWP_NOZORDER | SWP_NOSIZE);
        }


        static LRESULT CALLBACK a(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
            switch(message) {
            case WM_CREATE: {
                RECT rc;
                GetClientRect(hwnd, &rc);
                SCROLLINFO si = {
                    .cbSize = sizeof(SCROLLINFO),
                    .fMask = SIF_DISABLENOSCROLL | SIF_ALL,
                    .nMin = 0,
                    .nMax = rc.bottom - rc.top,
                    .nPage = 12,
                    .nPos = 0
                };
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE);


                break;
            }
            case WM_SIZE: {
                RECT rc;
                GetClientRect(hwnd, &rc);
                SCROLLINFO si = { .cbSize = sizeof(SCROLLINFO) };
                GetScrollInfo(hwnd, SB_VERT, &si);
                
                
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE);


                break;
            }
            case WM_SIZING: {
                auto& rect = *(RECT*)lparam;
                
                switch(wparam) {
                    case WMSZ_RIGHT: {
                        rect.right = rect.left + 0x1f0;
                    }
                }
                break;
            }
            case WM_VSCROLL:{
                switch(LOWORD(wparam)) {
                    case SB_TOP:
                    case SB_BOTTOM:
                    case SB_LINEUP:
                    case SB_LINEDOWN:
                    case SB_PAGEUP:
                    case SB_PAGEDOWN:
                    case SB_THUMBPOSITION:
                    break;
                }

                break;
            }
            }

            return 0;
        }
    public:
        inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_SCROLL_OBJDLG); }

        void operator()() {
            if (!enabled())return;

            OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::ExtendedFilter_wndcls, 4);
            store_i32(GLOBAL::exedit_base + OFS::ExEdit::ExtendedFilter_wndcls, WS_SYSMENU | WS_VSCROLL);
        }
    }scroll_objdlg;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SCROLL_OBJDLG
