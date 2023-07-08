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
#ifdef PATCH_SWITCH_FAST_GLOW

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// グローちょっとだけ速度アップ
	// クロス斜め以外：AVX2を使用
	/* クロス斜め：マルチスレッド処理の効率が悪かった部分を修正（AVX2処理にするのは難しい部分）
		スレッドIDによってループ数に大きく差が出る部分があったのでなるべく均等になるように分担
		理想的な均等にするなら「正方形」と「残り」の形にしてから行う必要がある。（関数の作り変えも必要）
		簡易的に実装し元の関数を再利用する。(元より悪くなることは無いはず)
	*/
	inline class Glow_t {

		static void __cdecl lower_right_convolution1_wrap(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl lower_right_convolution2_wrap(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl lower_left_convolution1_wrap(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl lower_left_convolution2_wrap(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);


		static void __cdecl vertical_convolution(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl horizontal_convolution(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl vertical_convolution_intensity3(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl horizontal_convolution_intensity3(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl horizontal_convolution_intensity_blur(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.glow";

	public:

		struct efGlow_var { // 1b2010
			int src_h;
			int src_w;
			int intensity;
			int blur;
			int diffusion_h;
			int diffusion_w;
			int diffusion_length;
			short light_cb;
			short light_cr;
			short light_y;
			short _padding;
			void* buf_temp;
			void* buf_temp2;
			int threshold; // 1b203c
		};
		

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{ // クロス斜め
				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55363, 4);
						h.store_i32(0, &lower_right_convolution1_wrap);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x553cb, 4);
						h.store_i32(0, &lower_right_convolution1_wrap);
					}
				}
				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55373, 4);
						h.store_i32(0, &lower_right_convolution2_wrap);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x553db, 4);
						h.store_i32(0, &lower_right_convolution2_wrap);
					}
				}
				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55383, 4);
						h.store_i32(0, &lower_left_convolution1_wrap);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x553eb, 4);
						h.store_i32(0, &lower_left_convolution1_wrap);
					}
				}
				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55393, 4);
						h.store_i32(0, &lower_left_convolution2_wrap);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x553fb, 4);
						h.store_i32(0, &lower_left_convolution2_wrap);
					}
				}
			}

			{ // クロス斜め以外
				auto cpucmdset = get_CPUCmdSet();
				if (!has_flag(cpucmdset, CPUCmdSet::F_AVX2))return;

				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x551ea, 4);
						h.store_i32(0, &vertical_convolution);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x5528a, 4);
						h.store_i32(0, &vertical_convolution);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x552ca, 4);
						h.store_i32(0, &vertical_convolution);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55309, 4);
						h.store_i32(0, &vertical_convolution);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55444, 4);
						h.store_i32(0, &vertical_convolution);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55464, 4);
						h.store_i32(0, &vertical_convolution);
					}
				}
				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55454, 4);
						h.store_i32(0, &horizontal_convolution);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55474, 4);
						h.store_i32(0, &horizontal_convolution);
					}
				}
				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x5525a, 4);
						h.store_i32(0, &horizontal_convolution_intensity_blur);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x552a3, 4);
						h.store_i32(0, &horizontal_convolution_intensity_blur);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x552e5, 4);
						h.store_i32(0, &horizontal_convolution_intensity_blur);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55326, 4);
						h.store_i32(0, &horizontal_convolution_intensity_blur);
					}
				}
				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x5533b, 4);
						h.store_i32(0, &vertical_convolution_intensity3);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x553ab, 4);
						h.store_i32(0, &vertical_convolution_intensity3);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55422, 4);
						h.store_i32(0, &vertical_convolution_intensity3);
					}
				}
				{
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x5534b, 4);
						h.store_i32(0, &horizontal_convolution_intensity3);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x553bb, 4);
						h.store_i32(0, &horizontal_convolution_intensity3);
					}
					{
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x55410, 4);
						h.store_i32(0, &horizontal_convolution_intensity3);
					}
				}
			}
		}

		void switching(bool flag) { enabled = flag; }

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

	} Glow;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_GLOW
