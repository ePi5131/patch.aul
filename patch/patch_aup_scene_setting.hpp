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

#ifdef PATCH_SWITCH_AUP_SCENE_SETTING

#include "global.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// プロジェクトの保存/読み込み時にシーン設定の一部情報（グリッド設定など）が欠けるのを修正
	inline class aup_scene_setting_t {

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "aup_scene_setting";


	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			{
				char save_scene_setting_all[] = {
					"\x8d\x7e\x48"             // lea     edi,dword ptr [esi+48] ; dst
					"\x56"                     // push    esi
					"\x8d\x75\x04"             // lea     esi,dword ptr [ebp+04] ; src
					"\xb9\x16\x00\x00\x00"     // mov     ecx,00000016 ; scenesetting.widthから22項目
					"\xf3\xa5"                 // rep     movsd
					"\x5e"                     // pop     esi
					"\xeb\x2e"                 // jmp     skip,2e (10032781)
				};
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x032742, sizeof(save_scene_setting_all) - 1);
				memcpy(reinterpret_cast<void*>(h.address()), save_scene_setting_all, sizeof(save_scene_setting_all) - 1);
			}
			{
				/*
					10031b48 8b4648             mov     eax,dword ptr [esi+48]
					10031b4b c1e205             shl     edx,05
					10031b4e 8982587a1710       mov     dword ptr [edx+10177a58],eax
					10031b54 ...
					↓
					10031b48 8bc7               mov     eax,edi
					10031b4a 90                 nop
					10031b4b c1e205             shl     edx,05
					10031b4e 8dbaXxXxXxXx       lea     edi,dword ptr [edx+exedit+177a58] ; Xの部分は書きかえなければ良い
					10031b54 ...       load_scene_setting_all
				*/
				char load_scene_setting_all[] = {
					"\x83\xc6\x48"             // add     esi,+48
					"\xb9\x15\x00\x00\x00"     // mov     ecx,00000015 ; こっちは21(22項目目はこの後に別の判定がある)
					"\xf3\xa5"                 // rep     movsd
					"\x8b\xf8"                 // mov     edi,eax
					"\x81\xee\x9c\x00\x00\x00" // sub     esi,0000009c ; 21*4 + 0x48
					"\x81\xfb\x2b\x23\x00\x00" // cmp     ebx,0000232b
					"\xeb\x74"                 // jmp     skip,74 (10031be2)
				};
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x031b49, 25);
				h.store_i16(0, '\xc7\x90');
				h.store_i16(5, '\x8d\xba');
				memcpy(reinterpret_cast<void*>(h.address() + 11), load_scene_setting_all, sizeof(load_scene_setting_all) - 1);
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

	} aup_scene_setting;
} // namespace patch
#endif // ifdef PATCH_SWITCH_AUP_SCENE_SETTING
