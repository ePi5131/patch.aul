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

#ifdef PATCH_SWITCH_AUP_LAYER_SETTING

#include "global.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// プロジェクトの保存時、レイヤー情報を保存する基準を変える
	// 従来：オブジェクトが存在するレイヤーのみ保存する（レイヤー情報が初期値であってもわざわざ保存する）
	// 変更：レイヤー情報が初期値以外のみ保存する

	// 保存時のみのパッチで互換性あり
	// パッチ有りで保存→パッチなしで読み込み：全てのレイヤー情報が読み込める

	inline class aup_layer_setting_t {

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "aup_layer_setting";

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			/*
				100326b8 3bcb               cmp     ecx,ebx ; ecx=ObjectCount, ebx=i(0～ObjectCount)
				100326ba 895c2410           mov     dword ptr [esp+10],ebx ; 0
				100326be 895c2414           mov     dword ptr [esp+14],ebx ; 0
				100326c2 7e3c               jng     10032700
				100326c4 8b0c85a88f1610     mov     ecx,dword ptr [eax*4+10168fa8] ; ecx=SortedObjectTable[idx]
				100326cb 40                 inc     eax ; idx++
				100326cc 8b91c4050000       mov     edx,dword ptr [ecx+000005c4]
				100326d2 c6041601           mov     byte ptr [esi+edx],01
				100326d6 8b0c85a48f1610     mov     ecx,dword ptr [eax*4+10168fa4]
				100326dd 8b91c4050000       mov     edx,dword ptr [ecx+000005c4]
				100326e3 8b89c0050000       mov     ecx,dword ptr [ecx+000005c0]
				100326e9 8d1492             lea     edx,dword ptr [edx+edx*4]
				100326ec 8d1492             lea     edx,dword ptr [edx+edx*4]
				100326ef 8d1491             lea     edx,dword ptr [ecx+edx*4]
				100326f2 c6042a01           mov     byte ptr [edx+ebp],01
				100326f6 8b0d50621410       mov     ecx,dword ptr [10146250]
				100326fc 3bc1               cmp     eax,ecx
				100326fe 7cc4               jl      100326c4

				for(int i=0;i<ObjectCount;i++){
					obj = SortedObjectTable + i;
					scene_save_flag[obj->scene_set] = 1;
					layer_save_flag[obj->scene_set * 100 + obj->layer_set] = 1;
				}

				↓


				100326b8 3bcb               cmp     ecx,ebx
				100326ba 895c2410           mov     dword ptr [esp+10],ebx
				100326be 895c2414           mov     dword ptr [esp+14],ebx
				100326c2 7e1c               jng     100326e0
				100326c4 8b0c85a88f1610     mov     ecx,dword ptr [eax*4+10168fa8]
				100326cb 40                 inc     eax
				100326cc 8b91c4050000       mov     edx,dword ptr [ecx+000005c4]
				100326d2 c6041601           mov     byte ptr [esi+edx],01

				100326d6 8b0d506214Xx       mov     ecx,dword ptr [10146250]
				100326dc 3bc1               cmp     eax,ecx
				100326de 7ce4               jl      100326c4


				100326e0 b810270000         mov     eax,0x2710 ; 10000
				100326e5 48                 dec     eax
				100326e6 8b0c85988418Xx     mov     ecx,dword ptr [eax*4 + exedit+188498] ; scene_setting
				100326ed 85c9               test    ecx,ecx
				100326ef 7409               jz      skip,9 ; 100326fa

				100326f1 8bd0               mov     edx,eax
				100326f3 c1ea01             shr     edx,1
				100326f6 c6042a01           mov     byte ptr [edx+ebp],01

				100326fa 85c0               test    eax,eax
				100326fc 75e7               jnz     back,19 ; 100326e5

				100326fe 90                 nop
				100326ff 90                 nop

				for(int i=0;i<ObjectCount;i++){
					obj = SortedObjectTable + i;
					scene_save_flag[obj->scene_set] = 1;
				}
				for (int i = 0; i < 5000; i++) {
					if (layersetting[i].flag != 0 || layersetting[i].name != 0) {
						layer_save_flag[i] = 1;
					}
				}
			*/

			{
			}
			{
				/*
					for (int i = 0; i < 5000; i++) {
						if (layersetting[i].flag != 0 || layersetting[i].name != 0) {
							layer_save[i] = 1;
						}
					}
				*/
				char code_put[] = {
			/*\x8b*/"\x0d\x50\x62\x14\x00"        // mov     ecx,dword ptr [exedit+146250]
					"\x3b\xc1"                    // cmp     eax,ecx
					"\x7c\xe4"                    // jl      100326c4
					"\xb8\x10\x27\x00\x00"        // mov     eax,0x2710 ; 10000
					"\x48"                        // dec     eax
					"\x8b\x0c\x85\x98\x84\x18\x00"// mov     ecx,dword ptr [eax*4+ exedit+188498]
					"\x85\xc9"                    // test    ecx,ecx
					"\x74\x09"                    // jz      skip,9 ; 100326fa
					"\x8b\xd0"                    // mov     edx,eax
					"\xc1\xea\x01"                // shr     edx,1
					"\xc6\x04\x2a\x01"            // mov     byte ptr [edx+ebp],01
					"\x85\xc0"                    // test    eax,eax
					"\x75\xe7"                    // jnz     back,19 ; 100326e5
					"\x90"                        // nop
					"\x90"                        // nop
				};

				*(int*)(&code_put[1]) = GLOBAL::exedit_base + 0x146250;
				*(int*)(&code_put[18]) = GLOBAL::exedit_base + 0x188498;

				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0326c3, 61);
				h.store_i8(0, 0x1c);

				memcpy(reinterpret_cast<void*>(h.address()+20), code_put, sizeof(code_put) - 1);
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

	} aup_layer_setting;
} // namespace patch
#endif // ifdef PATCH_SWITCH_AUP_LAYER_SETTING
