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

#ifdef PATCH_SWITCH_OBJ_COLORCORRECTION

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// 色調補正の色相計算が正しくないのを修正

	inline class obj_ColorCorrection_t {

		static void rot_hue(ExEdit::PixelYC* pix, int angle10, int luminance, int brightness, int saturation);
		static void rot_hue_sat(ExEdit::PixelYC* pix, int angle10, int luminance, int brightness, int saturation);
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "obj_colorcorrection";
	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			{ // メディアオブジェクト
				char code_effect[] =
					"\x8b\x44\x24\x2c"         // mov     eax,dword ptr [esp+2c] ; efp
					"\x8b\x48\x44"             // mov     ecx,dword ptr [eax+44] ; efp->track
					"\x8b\x41\x08"             // mov     eax,dword ptr [ecx+08] ; efp->track[2]
					"\x53"                     // push    ebx
					"\x57"                     // push    edi
					"\xff\x74\x24\x38"         // push    dword ptr [esp+38]
					"\x50"                     // push    eax
					"\x56"                     // push    esi
					"\xe8XXXX"                 // call    rot_hue()
					"\x83\xc4\x14"             // add     esp,+14
					"\xe9\x8d\x00\x00\x00"     // jmp     10014996
					;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x148ea, sizeof(code_effect) - 1);
				memcpy(reinterpret_cast<void*>(h.address()), code_effect, sizeof(code_effect) - 1);
				h.replaceNearJmp(19, rot_hue);
			}


			{ // メディアオブジェクト // 飽和する
				{
					char code_effect_sat[] =
						"\x8b\x44\x24\x34"         // mov     eax,dword ptr [esp+34] ; efp
						"\x8b\x48\x44"             // mov     ecx,dword ptr [eax+44] ; efp->track
						"\x8b\x41\x08"             // mov     eax,dword ptr [ecx+08] ; efp->track[2]
						"\xff\x74\x24\x2c"         // push    dword ptr [esp+2c]
						"\xff\x74\x24\x28"         // push    dword ptr [esp+28]
						"\xff\x74\x24\x18"         // push    dword ptr [esp+18]
						"\x50"                     // push    eax
						"\x53"                     // push    ebx
						"\xe8XXXX"                 // call    rot_hue_sat()
						"\x83\xc4\x14"             // add     esp,+14
						"\x8b\x44\x24\x38"         // mov     eax,dword ptr [esp+38] ; efpip
						"\xe9\xea\x00\x00\x00"     // jmp     10014f8c
						;
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x14e79, sizeof(code_effect_sat) - 1);
					memcpy(reinterpret_cast<void*>(h.address()), code_effect_sat, sizeof(code_effect_sat) - 1);
					h.replaceNearJmp(25, rot_hue_sat);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x14e3b, 1);
					h.store_i8(0, 0x1c);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x14fa9, 1);
					h.store_i8(0, 0x1c);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x14fb4, 1);
					h.store_i8(0, 0x1c);
				}
			}


			{ // フィルタオブジェクト
				char code_filter[] =
					"\x8b\x44\x24\x2c"         // mov     eax,dword ptr [esp+2c] ; efp
					"\x8b\x48\x44"             // mov     ecx,dword ptr [eax+44] ; efp->track
					"\x8b\x41\x08"             // mov     eax,dword ptr [ecx+08] ; efp->track[2]
					"\x57"                     // push    edi
					"\x55"                     // push    ebp
					"\xff\x74\x24\x38"         // push    dword ptr [esp+38]
					"\x50"                     // push    eax
					"\x56"                     // push    esi
					"\xe8XXXX"                 // call    rot_hue()
					"\x83\xc4\x14"             // add     esp,+14
					"\x8b\x4c\x24\x18"         // mov     ecx,dword ptr [esp+18] ; x
					"\x83\xc6\x06"             // add     esi,+06
					"\xe9\x8e\x00\x00\x00"     // jmp     10014c67
					;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x14bb3, sizeof(code_filter) - 1);
				memcpy(reinterpret_cast<void*>(h.address()), code_filter, sizeof(code_filter) - 1);
				h.replaceNearJmp(19, rot_hue);
			}


			{ // フィルタオブジェクト 飽和する
				{
					char code_filter_sat[] =
						"\x8b\x44\x24\x34"         // mov     eax,dword ptr [esp+34] ; efp
						"\x8b\x48\x44"             // mov     ecx,dword ptr [eax+44] ; efp->track
						"\x8b\x41\x08"             // mov     eax,dword ptr [ecx+08] ; efp->track[2]
						"\xff\x74\x24\x10"         // push    dword ptr [esp+10]
						"\xff\x74\x24\x28"         // push    dword ptr [esp+28]
						"\xff\x74\x24\x34"         // push    dword ptr [esp+34]
						"\x50"                     // push    eax
						"\x53"                     // push    ebx
						"\xe8XXXX"                 // call    rot_hue_sat()
						"\x83\xc4\x14"             // add     esp,+14
						"\x8b\x4c\x24\x30"         // mov     ecx,dword ptr [esp+30]
						"\x8b\x44\x24\x38"         // mov     eax,dword ptr [esp+38] ; efpip
						"\xe9\xea\x00\x00\x00"     // jmp     10015354
						;
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x1523d, sizeof(code_filter_sat) - 1);
					memcpy(reinterpret_cast<void*>(h.address()), code_filter_sat, sizeof(code_filter_sat) - 1);
					h.replaceNearJmp(25, rot_hue_sat);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x15210, 1);
					h.store_i8(0, 0x1c);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x1536e, 1);
					h.store_i8(0, 0x1c);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x1537b, 1);
					h.store_i8(0, 0x1c);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x1545e, 1);
					h.store_i8(0, 0x1c);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x15469, 1);
					h.store_i8(0, 0x1c);
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
	} ColorCorrection;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_COLORCORRECTION
