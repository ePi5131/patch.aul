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
#ifdef PATCH_SWITCH_COLORPALETTE_CACHE

#include "global.hpp"
#include "config_rw.hpp"
#include "util.hpp"

namespace patch {

	// bmpのキャッシュをとる際に、カラーパレット分のデータ容量を取り忘れているのを修正
	inline class colorpalette_cache_t {


		inline static int __cdecl fix_size_2a6a(uintptr_t avi_file_handle) {
			int video_decode_w = *(int*)(avi_file_handle + 20);
			int video_decode_h = *(int*)(avi_file_handle + 24);
			short video_decode_bit = *(short*)(avi_file_handle + 30);

			int size = (video_decode_bit * video_decode_w + 7) >> 3;
			size = (size + 3 & 0xfffffffc) * video_decode_h;

			switch (video_decode_bit) {
			case 32:case 24:case 16:
				size += 16;
				break;
			case 8:
				size += 1040;
				break;
			case 4:
				size += 80;
				break;
			default:
				size += 24;
			}
			return size;
		}
		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "colorpalette_cache";

	public:
		void switching(bool flag) {
			enabled = flag;
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{
				/*
					00402a68 89442418           mov     dword ptr [esp+18],eax
					00402a6c 8d0c8510000000     lea     ecx,dword ptr [eax*4+00000010]
					00402a73 51                 push    ecx
					↓
					00402a68 56                 push    esi ; avi_file_handle
					00402a69 e8xxxxxxxx         call    fix_size_2a6a()
					00402a6e 5e                 pop     esi
					00402a6f 89442418           mov     dword ptr [esp+18],eax
					00402a73 50                 push    eax


					00402a88 c1e102             shl     ecx,02
					↓
					00402a88 83e910             sub     ecx,10

				*/

				OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x2a68, 35);
				h.store_i16(0, '\x56\xe8');
				h.replaceNearJmp(2, &fix_size_2a6a);
				h.store_i32(6, '\x5e\x89\x44\x24');
				h.store_i16(10, '\x18\x50');

				h.store_i16(32, '\x83\xe9');
				h.store_i8(34, '\x10');
			}

		}

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}


	} colorpalette_cache;
} // namespace patch
#endif // ifdef PATCH_SWITCH_COLORPALETTE_CACHE
