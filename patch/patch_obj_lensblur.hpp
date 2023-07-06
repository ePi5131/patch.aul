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

#ifdef PATCH_SWITCH_OBJ_LENSBLUR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// レンズブラーのエラー修正
	
	/* オフセットアドレス exedit + 71449 の修正
		サイズ２～３の図形にレンズブラーを付けて
		サイズ固定にチェックが入った状態でレンズブラー範囲を広げる
	*/

	/* オフセットアドレス exedit + 71f95 の修正
		フィルタオブジェクトのレンズブラー(範囲を広げたもの)をシーンオブジェクトで読み込む
		シーンオブジェクト以外でエラーはあまり見られないけど他データを破壊していそうなのでそちらもまとめて修正
	*/

	inline class obj_LensBlur_t {

		static void* __cdecl lbResize_709a0_wrap_12809(void* pix_edit, int w0, int h0, int w1, int h1, void* pix_temp);
		static void* __cdecl lbResize_71420_wrap_126a6(void* pix_edit, int w0, int h0, int w1, int h1, void* pix_temp);



		inline static struct lbResize_var { // 1e42c0
			void* resize_edit;
			int _others1[8];
			void* resize_temp; // 1e42e4
			int _others2[14];
			int resize_step; // 1e4320
			int resize_h; // 1e4324
			int _bufcpy_h; // 1e4328
			int resize_w; // 1e432c
		}*lbresize;

		inline static int lbResize_wh_end;

		static void __cdecl lbResize_set_w_end(void* dst, void* src, int w, int h, int back);
		static void __cdecl lbResize_set_h_end(void* dst, void* src, int w, int h, int back);

		static void lbResize_media(int thread_id, int thread_num, int loop1, int forstep1, int loop2, int forstep2, int flag);
		static void lbResize_filter(int thread_id, int thread_num, int loop1, int forstep1, int loop2, int forstep2, int flag);

		static void __cdecl lbResize_media_interpolation_y(int thread_id, int thread_num, void* n1, void* n2);
		static void __cdecl lbResize_media_interpolation_x(int thread_id, int thread_num, void* n1, void* n2);
		static void __cdecl lbResize_media_interpolation_y_fbb(int thread_id, int thread_num, void* n1, void* n2);
		static void __cdecl lbResize_media_interpolation_x_fbb(int thread_id, int thread_num, void* n1, void* n2);
		static void __cdecl lbResize_filter_interpolation_y(int thread_id, int thread_num, void* n1, void* n2);
		static void __cdecl lbResize_filter_interpolation_x(int thread_id, int thread_num, void* n1, void* n2);
		static void __cdecl lbResize_filter_interpolation_y_fbb(int thread_id, int thread_num, void* n1, void* n2);
		static void __cdecl lbResize_filter_interpolation_x_fbb(int thread_id, int thread_num, void* n1, void* n2);


		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "obj_lensblur";
	public:


		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			{ // オフセットアドレス exedit + 71449 の修正
				ReplaceNearJmp(GLOBAL::exedit_base + 0x01280a, &lbResize_709a0_wrap_12809);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x0126a7, &lbResize_71420_wrap_126a6);
			}

			
			{ // オフセットアドレス exedit + 71f95 の修正、なんかやヴぁそうなバグの修正

				/* 以下の関数にて行われる危険な処理：memcpy( &ycp_edit[-line_size-1], &ycp_edit[-1], (line_size+2) * sizeof(PIXEL_YC));
					おそらく補間処理を行うための前処理として行われている
					危険なのでその前処理は全て削除し、ついでにリサイズ前のw,hを取得したいので置き換える
				*/
				ReplaceNearJmp(GLOBAL::exedit_base + 0x070a08, &lbResize_set_h_end);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x070aab, &lbResize_set_w_end);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x071488, &lbResize_set_h_end);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x07152b, &lbResize_set_w_end);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x072068, &lbResize_set_h_end);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x07210b, &lbResize_set_w_end);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x0728d8, &lbResize_set_h_end);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x07297b, &lbResize_set_w_end);

				lbresize = (lbResize_var*)(GLOBAL::exedit_base + 0x1e42c0);

				/* 前処理が無くても同じように動くように書いたものに置き換える
					リソースのことを考えて関数はできるだけ纏めた
					x方向の処理が従来より効率が良いのでそんなに速度が落ちることは無いはず
				*/
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x070a3f, 4);
					h.store_i32(0, &lbResize_media_interpolation_y);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x070ac5, 4);
					h.store_i32(0, &lbResize_media_interpolation_x);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0714bf, 4);
					h.store_i32(0, &lbResize_media_interpolation_y_fbb);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x071545, 4);
					h.store_i32(0, &lbResize_media_interpolation_x_fbb);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x07209f, 4);
					h.store_i32(0, &lbResize_filter_interpolation_y);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x072125, 4);
					h.store_i32(0, &lbResize_filter_interpolation_x);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x07290f, 4);
					h.store_i32(0, &lbResize_filter_interpolation_y_fbb);
				}
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x072995, 4);
					h.store_i32(0, &lbResize_filter_interpolation_x_fbb);
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
	} LensBlur;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_LENSBLUR
