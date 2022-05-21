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
#ifdef PATCH_SWITCH_ACCESS_KEY

#include "global.hpp"
#include "config_rw.hpp"

namespace patch {

	// LoadMenuA 時にmodifyを噛ませる
	// メニューにアクセスキーの追加をする
	inline class access_key_t {

		struct Menu {
			enum {
				File,
				Filter,
				Setting,
				Edit,
				Profile,
				View,
				Other,
				Count
			};
		};

		char map_key[Menu::Count] = {
			'F',
			'L',
			'S',
			'E',
			'P',
			'V',
			'O',
		};

		// どのメニューがあるかでどのフォルダかを調べる
		UINT mark_map[Menu::Count] = {
			5097,
			5183,
			5127,
			1020,
			8001,
			9200,
			1006
		};

		inline static bool find_menu(HMENU hMenu, UINT id) {
			auto menu_n = GetMenuItemCount(hMenu);
			for (int i = 0; i < menu_n; i++) {
				MENUITEMINFOW mii{
					.cbSize = sizeof(mii),
					.fMask = MIIM_ID | MIIM_SUBMENU,
				};
				GetMenuItemInfoW(hMenu, i, TRUE, &mii);
				if (mii.hSubMenu != NULL) {
					auto ret = find_menu(mii.hSubMenu, id);
					if (ret)return true;
				}
				if (mii.wID == id) return true;
			}

			return false;
		}

		bool enabled = false;
		bool enabled_i;

		inline static const char key[] = "access_key";

	public:
		void switching(bool flag) {
			enabled = flag;
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }
		
		void init() {
			enabled_i = enabled;
		}

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}


		HMENU modify(LPCSTR key, HMENU hMenu) {
			if (!enabled_i) return hMenu;

			if (lstrcmpiA(key, "AVIUTL") != 0)return hMenu;

			auto find_submenu = [](HMENU hMenu, int idx, UINT id) {
				auto sub = GetSubMenu(hMenu, idx);

				auto menu_n = GetMenuItemCount(sub);
				for (int i = 0; i < menu_n; i++) {
					MENUITEMINFOW mii{
						.cbSize = sizeof(mii),
						.fMask = MIIM_ID,
					};
					GetMenuItemInfoW(sub, i, TRUE, &mii);
					if (mii.wID == id) return true;
				}

				return false;
			};

			for (int i = 0; i < Menu::Count; i++) {
				auto menu_n = GetMenuItemCount(hMenu);
				for (int j = 0; j < menu_n; j++) {
					MENUITEMINFOW mii{
						.cbSize = sizeof(mii),
						.fMask = MIIM_STRING | MIIM_ID,
						.dwTypeData = NULL
					};
					GetMenuItemInfoW(hMenu, j, TRUE, &mii);

					if (find_menu(GetSubMenu(hMenu, j), mark_map[i])) {
						std::wstring buf;
						buf.resize(mii.cch);
						mii.fMask = MIIM_STRING;
						mii.dwTypeData = buf.data();
						mii.cch = buf.size() + 1;
						GetMenuItemInfoW(hMenu, j, TRUE, &mii);
					
						buf.append(L"(&");
						buf.push_back(map_key[i]);
						buf.push_back(L')');
						
						mii.fMask = MIIM_STRING;
						mii.dwTypeData = buf.data();
						mii.cch = buf.size() + 1;
						SetMenuItemInfoW(hMenu, j, TRUE, &mii);
					}
				}
			}
			return hMenu;
		}


	} access_key;
} // namespace patch
#endif // ifdef PATCH_SWITCH_ACCESS_KEY
