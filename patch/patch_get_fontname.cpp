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
#include "patch_get_fontname.hpp"

namespace patch {
        // フォント名の読み先を変える
        // 文字数は64までなので32文字以上なら元の場所から読む
        // 両方長い場合は無理
        int CALLBACK get_fontname_t::enumfontfamproc_wrap(ENUMLOGFONT* param_1, NEWTEXTMETRIC* param_2, DWORD param_3, HDC param_4) {
            static LPCSTR lpString;
            static int c;
            static tagSIZE* psizl;
            static tagSIZE tStack8{};
            static LONG& cxMax = load_i32<LONG&>(GLOBAL::exedit_base + 0x23638c);
            static HWND& sFont = load_i32<HWND&>(GLOBAL::exedit_base + 0x23630c);
            static int len;

            psizl = &tStack8;
            lpString = (LPCSTR)(param_1->elfFullName);
            len = strlen(lpString);
            if (len > 31) {
                lpString = (LPCSTR)(param_1->elfLogFont.lfFaceName);
            }
            if (*(char*)(lpString) != '@') {
                c = lstrlenA(lpString);
                GetTextExtentPoint32A(param_4, lpString, c, psizl);
                if (cxMax < tStack8.cx) {
                    cxMax = tStack8.cx;
                }
                SendMessageA(sFont, CB_ADDSTRING, 0, (LPARAM)lpString);
            }
            return 1;
        }
        // ついでにEnumFontFamiliesをExに変更
        // 読めてない/意味のないフォントを一掃できるらしい
        int WINAPI get_fontname_t::EnumFontFamiliesA(HDC hdc, LPLOGFONTA lplf, FONTENUMPROCA enumfontfamproc, LPARAM lpfam) {
            static LOGFONTA lf{};
            lf.lfPitchAndFamily = DEFAULT_PITCH;
            return EnumFontFamiliesExA(hdc, &lf, enumfontfamproc, lpfam, 0);
        }
}
