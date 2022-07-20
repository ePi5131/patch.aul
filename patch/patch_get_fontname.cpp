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
#include "iostream"

namespace patch {
        // フォント名の読み先を変える
        // 文字数は64までなので32文字より長いなら元の場所から読む
        // 両方長い場合は無理
        int CALLBACK get_fontname_t::enumfontfamproc_wrap(ENUMLOGFONT* elf, ENUMTEXTMETRICA* param_2, DWORD fonttype, HDC hdc) {
            static LPCSTR FontName;
            static int c;
            static tagSIZE size{};
            static LONG& cxMax = load_i32<LONG&>(GLOBAL::exedit_base + 0x23638c);
            static HWND& sFont = load_i32<HWND&>(GLOBAL::exedit_base + 0x23630c);
            static int len;
            std::cout << elf->elfFullName << std::endl;
            std::cout << elf->elfLogFont.lfFaceName << std::endl;
            std::cout << elf->elfStyle << "\n" << std::endl;

            if (*(char*)(elf->elfLogFont.lfFaceName) != '@') {
                FontName = (LPCSTR)(elf->elfFullName);
                len = strlen(FontName);
                if (len > 32) {
                    if (strlen(elf->elfLogFont.lfFaceName) > 32) {
                        // なぜか\0が置かれずelfFullNameと繋がっちゃうパターンがある
                        // 読めなくてもせめてテキストオブジェクトがバグらないようにしたい
                        // 無効として無視する(リストに入れない) or 入る分だけ入れとく(読めない) or 無効であるとわかる文字列をねじ込む
                        return 1;
                    }
                    else {
                        FontName = (LPCSTR)(elf->elfLogFont.lfFaceName);
                    }
                }
                c = lstrlenA(FontName);
                GetTextExtentPoint32A(hdc, FontName, c, &size);
                if (cxMax < size.cx) {
                    cxMax = size.cx;
                }
                SendMessageA(sFont, CB_ADDSTRING, 0, (LPARAM)FontName);
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
