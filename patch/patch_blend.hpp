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

#ifdef PATCH_SWITCH_BLEND

#include <memory>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 合成モード関数の修正(主にアルファチャンネル有りシーンオブジェクト)
	inline class blend_t {
		static void __cdecl blend_yca_add(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_sub(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_mul(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_screen(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_overlay(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_cmpmax(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_cmpmin(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_luminance(ExEdit::PixelYCA* dst, short src_y, short src_cb, short src_cr, short src_a);
		static void __cdecl blend_yca_colordiff(ExEdit::PixelYCA* dst, short src_y, short src_cb, short src_cr, short src_a);
		static void __cdecl blend_yca_shadow(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_lightdark(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_difference(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "blend";


	public:

		inline static void(__cdecl* blend_yca_normal)(void* dst, int src_y, int src_cb, int src_cr, int src_a);

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			blend_yca_normal = reinterpret_cast<decltype(blend_yca_normal)>(GLOBAL::exedit_base + OFS::ExEdit::blend_yca_normal_func);
			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x9fbb4, 48);
				h.store_i32(0, &blend_yca_add);
				h.store_i32(4, &blend_yca_sub);
				h.store_i32(8, &blend_yca_mul);
				h.store_i32(12, &blend_yca_screen);
				h.store_i32(16, &blend_yca_overlay);
				h.store_i32(20, &blend_yca_cmpmax);
				h.store_i32(24, &blend_yca_cmpmin);
				h.store_i32(28, &blend_yca_luminance);
				h.store_i32(32, &blend_yca_colordiff);
				h.store_i32(36, &blend_yca_shadow);
				h.store_i32(40, &blend_yca_lightdark);
				h.store_i32(44, &blend_yca_difference);
			}

			static constinit auto blend_yca_normal_bin = binstr_array(
				"8b4c2404"   // mov     ecx,dword ptr [esp+04]
				"8b542414"   // mov     edx,dword ptr [esp+14]
				"b800100000" // mov     eax,00001000
				"3bd0"       // cmp     edx,eax
				"7c1f"       // jl      10007e20
				"66894106"   // mov     [ecx+06],ax
				"668b442408" // mov     ax,[esp+08]
				"668b54240c" // mov     dx,[esp+0c]
				"668901"     // mov     [ecx],ax
				"668b442410" // mov     ax,[esp+10]
				"66895102"   // mov     [ecx+02],dx
				"66894104"   // mov     [ecx+04],ax
				"c3"         // ret
				"56"         // push    esi
				"0fbf7106"   // movsx   esi,dword ptr [ecx+06]
				"3bf0"       // cmp     esi,eax
				"7c40"       // jl      10007e69
				"66894106"   // mov     [ecx+06],ax
				"0fbf31"     // movsx   esi,dword ptr [ecx]
				"8b44240c"   // mov     eax,dword ptr [esp+0c]
				"2bc6"       // sub     eax,esi
				"0fafc2"     // imul    eax,edx
				"c1f80c"     // sar     eax,0c
				"660101"     // add     [ecx],ax
				"0fbf7102"   // movsx   esi,dword ptr [ecx+02]
				"8b442410"   // mov     eax,dword ptr [esp+10]
				"2bc6"       // sub     eax,esi
				"0fafc2"     // imul    eax,edx
				"c1f80c"     // sar     eax,0c
				"66014102"   // add     [ecx+02],ax
				"0fbf7104"   // movsx   esi,dword ptr [ecx+04]
				"8b442414"   // mov     eax,dword ptr [esp+14]
				"2bc6"       // sub     eax,esi
				"0fafc2"     // imul    eax,edx
				"c1f80c"     // sar     eax,0c
				"66014104"   // add     [ecx+04],ax
				"5e"         // pop     esi
				"c3"         // ret
				"85f6"       // test    esi,esi
				"7f20"       // jg      10007e90
				"66895106"   // mov     [ecx+06],dx
				"668b44240c" // mov     ax,[esp+0c]
				"668b542410" // mov     dx,[esp+10]
				"668901"     // mov     [ecx],ax
				"668b442414" // mov     ax,[esp+14]
				"66895102"   // mov     [ecx+02],dx
				"66894104"   // mov     [ecx+04],ax
				"5e"         // pop     esi
				"c3"         // ret
				"53"         // push    ebx
				"8bd8"       // mov     ebx,eax
				"2bde"       // sub     ebx,esi
				"2bc2"       // sub     eax,edx
				"0fafd8"     // imul    ebx,eax
				"0fafc6"     // imul    eax,esi
				"be00080001" // mov     esi,01000800
				"2bf3"       // sub     esi,ebx
				"c1fe0c"     // sar     esi,0c
				"66897106"   // mov     [ecx+06],si
				"99"         // cdq
				"f7fe"       // idiv    esi
				"8bd8"       // mov     ebx,eax
				"8b44241c"   // mov     eax,dword ptr [esp+1c]
				"c1e00c"     // shl     eax,0c
				"99"         // cdq
				"f7fe"       // idiv    esi
				"0fbf11"     // movsx   edx,dword ptr [ecx]
				"0fafd3"     // imul    edx,ebx
				"8b742410"   // mov     esi,dword ptr [esp+10]
				"0faff0"     // imul    esi,eax
				"03d6"       // add     edx,esi
				"c1fa0c"     // sar     edx,0c
				"668911"     // mov     [ecx],dx
				"0fbf5102"   // movsx   edx,dword ptr [ecx+02]
				"0fafd3"     // imul    edx,ebx
				"8b742414"   // mov     esi,dword ptr [esp+14]
				"0faff0"     // imul    esi,eax
				"03d6"       // add     edx,esi
				"c1fa0c"     // sar     edx,0c
				"66895102"   // mov     [ecx+02],dx
				"0fbf5104"   // movsx   edx,dword ptr [ecx+04]
				"0fafd3"     // imul    edx,ebx
				"8b742418"   // mov     esi,dword ptr [esp+18]
				"0faff0"     // imul    esi,eax
				"03d6"       // add     edx,esi
				"c1fa0c"     // sar     edx,0c
				"66895104"   // mov     [ecx+04],dx
				"5b"         // pop     ebx
				"5e"         // pop     esi
				"c3"         // ret
			);
			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::blend_yca_normal_func, blend_yca_normal_bin.size());
				h.copy_from(blend_yca_normal_bin.data(), blend_yca_normal_bin.size());
			}
			{
				static constinit auto blend_yc_normal_bin = binstr_array(
					"8b4c2404"     // mov     ecx,dword ptr [esp+04]
					"8b542414"     // mov     edx,dword ptr [esp+14]
					"81fa00100000" // cmp     edx,00001000
					"7c1b"         // jl      10007f4b
					"668b442408"   // mov     ax,[esp+08]
					"668b54240c"   // mov     dx,[esp+0c]
					"668901"       // mov     [ecx],ax
					"668b442410"   // mov     ax,[esp+10]
					"66895102"     // mov     [ecx+02],dx
					"66894104"     // mov     [ecx+04],ax
					"c3"           // ret
					"56"           // push    esi
				);
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::blend_yc_normal_func, blend_yc_normal_bin.size() + 60);
				h.copy_from(0, blend_yc_normal_bin.data(), blend_yc_normal_bin.size());
				h.copy_from(blend_yc_normal_bin.size(), &blend_yca_normal_bin[61], 60);
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

	} blend;
} // namespace patch
#endif // ifdef PATCH_SWITCH_BLEND
