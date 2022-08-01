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

            char blend_yca_normal_bin[] = {
                "\x8b\x4c\x24\x04"         // mov     ecx,dword ptr [esp+04]
                "\x8b\x54\x24\x14"         // mov     edx,dword ptr [esp+14]
                "\xb8\x00\x10\x00\x00"     // mov     eax,00001000
                "\x3b\xd0"                 // cmp     edx,eax
                "\x7c\x1f"                 // jl      10007e20
                "\x66\x89\x41\x06"         // mov     [ecx+06],ax
                "\x66\x8b\x44\x24\x08"     // mov     ax,[esp+08]
                "\x66\x8b\x54\x24\x0c"     // mov     dx,[esp+0c]
                "\x66\x89\x01"             // mov     [ecx],ax
                "\x66\x8b\x44\x24\x10"     // mov     ax,[esp+10]
                "\x66\x89\x51\x02"         // mov     [ecx+02],dx
                "\x66\x89\x41\x04"         // mov     [ecx+04],ax
                "\xc3"                     // ret
                "\x56"                     // push    esi
                "\x0f\xbf\x71\x06"         // movsx   esi,dword ptr [ecx+06]
                "\x3b\xf0"                 // cmp     esi,eax
                "\x7c\x40"                 // jl      10007e69
                "\x66\x89\x41\x06"         // mov     [ecx+06],ax
                "\x0f\xbf\x31"             // movsx   esi,dword ptr [ecx]
                "\x8b\x44\x24\x0c"         // mov     eax,dword ptr [esp+0c]
                "\x2b\xc6"                 // sub     eax,esi
                "\x0f\xaf\xc2"             // imul    eax,edx
                "\xc1\xf8\x0c"             // sar     eax,0c
                "\x66\x01\x01"             // add     [ecx],ax
                "\x0f\xbf\x71\x02"         // movsx   esi,dword ptr [ecx+02]
                "\x8b\x44\x24\x10"         // mov     eax,dword ptr [esp+10]
                "\x2b\xc6"                 // sub     eax,esi
                "\x0f\xaf\xc2"             // imul    eax,edx
                "\xc1\xf8\x0c"             // sar     eax,0c
                "\x66\x01\x41\x02"         // add     [ecx+02],ax
                "\x0f\xbf\x71\x04"         // movsx   esi,dword ptr [ecx+04]
                "\x8b\x44\x24\x14"         // mov     eax,dword ptr [esp+14]
                "\x2b\xc6"                 // sub     eax,esi
                "\x0f\xaf\xc2"             // imul    eax,edx
                "\xc1\xf8\x0c"             // sar     eax,0c
                "\x66\x01\x41\x04"         // add     [ecx+04],ax
                "\x5e"                     // pop     esi
                "\xc3"                     // ret
                "\x85\xf6"                 // test    esi,esi
                "\x7f\x20"                 // jg      10007e90
                "\x66\x89\x51\x06"         // mov     [ecx+06],dx
                "\x66\x8b\x44\x24\x0c"     // mov     ax,[esp+0c]
                "\x66\x8b\x54\x24\x10"     // mov     dx,[esp+10]
                "\x66\x89\x01"             // mov     [ecx],ax
                "\x66\x8b\x44\x24\x14"     // mov     ax,[esp+14]
                "\x66\x89\x51\x02"         // mov     [ecx+02],dx
                "\x66\x89\x41\x04"         // mov     [ecx+04],ax
                "\x5e"                     // pop     esi
                "\xc3"                     // ret
                "\x53"                     // push    ebx
                "\x8b\xd8"                 // mov     ebx,eax
                "\x2b\xde"                 // sub     ebx,esi
                "\x2b\xc2"                 // sub     eax,edx
                "\x0f\xaf\xd8"             // imul    ebx,eax
                "\x0f\xaf\xc6"             // imul    eax,esi
                "\xbe\x00\x08\x00\x01"     // mov     esi,01000800
                "\x2b\xf3"                 // sub     esi,ebx
                "\xc1\xfe\x0c"             // sar     esi,0c
                "\x66\x89\x71\x06"         // mov     [ecx+06],si
                "\x99"                     // cdq
                "\xf7\xfe"                 // idiv    esi
                "\x8b\xd8"                 // mov     ebx,eax
                "\x8b\x44\x24\x1c"         // mov     eax,dword ptr [esp+1c]
                "\xc1\xe0\x0c"             // shl     eax,0c
                "\x99"                     // cdq
                "\xf7\xfe"                 // idiv    esi
                "\x0f\xbf\x11"             // movsx   edx,dword ptr [ecx]
                "\x0f\xaf\xd3"             // imul    edx,ebx
                "\x8b\x74\x24\x10"         // mov     esi,dword ptr [esp+10]
                "\x0f\xaf\xf0"             // imul    esi,eax
                "\x03\xd6"                 // add     edx,esi
                "\xc1\xfa\x0c"             // sar     edx,0c
                "\x66\x89\x11"             // mov     [ecx],dx
                "\x0f\xbf\x51\x02"         // movsx   edx,dword ptr [ecx+02]
                "\x0f\xaf\xd3"             // imul    edx,ebx
                "\x8b\x74\x24\x14"         // mov     esi,dword ptr [esp+14]
                "\x0f\xaf\xf0"             // imul    esi,eax
                "\x03\xd6"                 // add     edx,esi
                "\xc1\xfa\x0c"             // sar     edx,0c
                "\x66\x89\x51\x02"         // mov     [ecx+02],dx
                "\x0f\xbf\x51\x04"         // movsx   edx,dword ptr [ecx+04]
                "\x0f\xaf\xd3"             // imul    edx,ebx
                "\x8b\x74\x24\x18"         // mov     esi,dword ptr [esp+18]
                "\x0f\xaf\xf0"             // imul    esi,eax
                "\x03\xd6"                 // add     edx,esi
                "\xc1\xfa\x0c"             // sar     edx,0c
                "\x66\x89\x51\x04"         // mov     [ecx+04],dx
                "\x5b"                     // pop     ebx
                "\x5e"                     // pop     esi
                "\xc3"                     // ret
            };
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::blend_yca_normal_func, sizeof(blend_yca_normal_bin) - 1);
                memcpy(reinterpret_cast<void*>(h.address()), blend_yca_normal_bin, sizeof(blend_yca_normal_bin) - 1);
            }
            {
                char blend_yc_normal_bin[] = {
                    "\x8b\x4c\x24\x04"         // mov     ecx,dword ptr [esp+04]
                    "\x8b\x54\x24\x14"         // mov     edx,dword ptr [esp+14]
                    "\x81\xfa\x00\x10\x00\x00" // cmp     edx,00001000
                    "\x7c\x1b"                 // jl      10007f4b
                    "\x66\x8b\x44\x24\x08"     // mov     ax,[esp+08]
                    "\x66\x8b\x54\x24\x0c"     // mov     dx,[esp+0c]
                    "\x66\x89\x01"             // mov     [ecx],ax
                    "\x66\x8b\x44\x24\x10"     // mov     ax,[esp+10]
                    "\x66\x89\x51\x02"         // mov     [ecx+02],dx
                    "\x66\x89\x41\x04"         // mov     [ecx+04],ax
                    "\xc3"                     // ret
                    "\x56"                     // push    esi
                };
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::blend_yc_normal_func, 104);
                memcpy(reinterpret_cast<void*>(h.address()), blend_yc_normal_bin, sizeof(blend_yc_normal_bin) - 1);
                memcpy(reinterpret_cast<void*>(h.address() + sizeof(blend_yc_normal_bin) - 1), &blend_yca_normal_bin[61], 60);
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
