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

#include "patch_keyconfig.hpp"
#ifdef PATCH_SWITCH_KEYCONFIG

namespace patch {
    /*
    一つ目のカッコ内は言語リソース変更で変わってしまうので、それ以降のみの一致を許容する必要がある
    ・[編集][プラグイン1][メニュー1]
    ・[Edit][プラグイン1][メニュー1]
    ・[編集(&E)][プラグイン1][メニュー1]
    */
    int __stdcall KeyConfig_t::lstrcmpA_wrap(char* menu_data_str, char* keyfile_data_str) {
        if (lstrcmpA(menu_data_str, keyfile_data_str) == 0) {
            return 0;
        }

        // 文字列の後ろから比較していき、"]["が2回出現するまで一致していたら同じと判断
        // プラグイン名やメニューアイテム名に"]["が含まれていれば判定は狂うけど問題になるケースはほぼ無いはず

        int m = lstrlenA(menu_data_str) - 1; // 最後の']'の分
        int k = lstrlenA(keyfile_data_str) - 1;
        int count = 0;
        int flag = 0;
        while (0 < m && 0 < k) {
            m--; k--;
            if (menu_data_str[m] != keyfile_data_str[k]) {
                return 1;
            }
            switch (menu_data_str[m]) {
            case '[':
                flag = 1;
                break;
            case ']':
                if (flag) {
                    if (count) {
                        return 0;
                    }
                    count++;
                }
                // flag = 0;
                // break;
            default:
                flag = 0;
            }
        }
        return 1;
    }

} // namespace patch
#endif // ifdef PATCH_SWITCH_KEYCONFIG
