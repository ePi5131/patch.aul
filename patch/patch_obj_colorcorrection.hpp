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
				static constinit auto code_effect = binstr_array(
					"8b44242c"                     // mov     eax,dword ptr [esp+2c] ; efp
					"8b4844"                       // mov     ecx,dword ptr [eax+44] ; efp->track
					"8b4108"                       // mov     eax,dword ptr [ecx+08] ; efp->track[2]
					"53"                           // push    ebx
					"57"                           // push    edi
					"ff742438"                     // push    dword ptr [esp+38]
					"50"                           // push    eax
					"56"                           // push    esi
					"e8" PATCH_BINSTR_DUMMY_32(19) // call    rot_hue()
					"83c414"                       // add     esp,+14
					"e98d000000"                   // jmp     10014996
				);
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x148ea, code_effect.size());
				h.copy_from(code_effect.data(), code_effect.size());
				h.replaceNearJmp(19, rot_hue);
			}


			{ // メディアオブジェクト // 飽和する
				{
					static constinit auto code_effect_sat = binstr_array(
						"8b442434"                     // mov     eax,dword ptr [esp+34] ; efp
						"8b4844"                       // mov     ecx,dword ptr [eax+44] ; efp->track
						"8b4108"                       // mov     eax,dword ptr [ecx+08] ; efp->track[2]
						"ff74242c"                     // push    dword ptr [esp+2c]
						"ff742428"                     // push    dword ptr [esp+28]
						"ff742418"                     // push    dword ptr [esp+18]
						"50"                           // push    eax
						"53"                           // push    ebx
						"e8" PATCH_BINSTR_DUMMY_32(25) // call    rot_hue_sat()
						"83c414"                       // add     esp,+14
						"8b442438"                     // mov     eax,dword ptr [esp+38] ; efpip
						"e9ea000000"                   // jmp     10014f8c
					);
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x14e79, code_effect_sat.size());
					h.copy_from(code_effect_sat.data(), code_effect_sat.size());
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
				static constinit auto code_filter = binstr_array(
					"8b44242c"                     // mov     eax,dword ptr [esp+2c] ; efp
					"8b4844"                       // mov     ecx,dword ptr [eax+44] ; efp->track
					"8b4108"                       // mov     eax,dword ptr [ecx+08] ; efp->track[2]
					"57"                           // push    edi
					"55"                           // push    ebp
					"ff742438"                     // push    dword ptr [esp+38]
					"50"                           // push    eax
					"56"                           // push    esi
					"e8" PATCH_BINSTR_DUMMY_32(19) // call    rot_hue()
					"83c414"                       // add     esp,+14
					"8b4c2418"                     // mov     ecx,dword ptr [esp+18] ; x
					"83c606"                       // add     esi,+06
					"e98e000000"                   // jmp     10014c67
				);
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x14bb3, code_filter.size());
				h.copy_from(code_filter.data(), code_filter.size());
				h.replaceNearJmp(19, rot_hue);
			}


			{ // フィルタオブジェクト 飽和する
				{
					static constinit auto code_filter_sat = binstr_array(
						"8b442434"                     // mov     eax,dword ptr [esp+34] ; efp
						"8b4844"                       // mov     ecx,dword ptr [eax+44] ; efp->track
						"8b4108"                       // mov     eax,dword ptr [ecx+08] ; efp->track[2]
						"ff742410"                     // push    dword ptr [esp+10]
						"ff742428"                     // push    dword ptr [esp+28]
						"ff742434"                     // push    dword ptr [esp+34]
						"50"                           // push    eax
						"53"                           // push    ebx
						"e8" PATCH_BINSTR_DUMMY_32(25) // call    rot_hue_sat()
						"83c414"                       // add     esp,+14
						"8b4c2430"                     // mov     ecx,dword ptr [esp+30]
						"8b442438"                     // mov     eax,dword ptr [esp+38] ; efpip
						"e9ea000000"                   // jmp     10015354
					);
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x1523d, code_filter_sat.size());
					h.copy_from(code_filter_sat.data(), code_filter_sat.size());
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
