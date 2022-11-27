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

#include "patch_obj_lensblur.hpp"

#ifdef PATCH_SWITCH_OBJ_LENSBLUR
namespace patch {
	void* __cdecl obj_LensBlur_t::lbResize_709a0_wrap_12809(void* pix_edit, int w0, int h0, int w1, int h1, void* pix_temp) {
		if (w0 && h0 && w1 && h1) {
			return reinterpret_cast<void* (__cdecl*)(void*, int, int, int, int, void*)>(GLOBAL::exedit_base + 0x0709a0)(pix_edit, w0, h0, w1, h1, pix_temp);
		}
		return pix_edit;
	}

	void* __cdecl obj_LensBlur_t::lbResize_71420_wrap_126a6(void* pix_edit, int w0, int h0, int w1, int h1, void* pix_temp) {
		if (w0 && h0 && w1 && h1) {
			return reinterpret_cast<void* (__cdecl*)(void*, int, int, int, int, void*)>(GLOBAL::exedit_base + 0x071420)(pix_edit, w0, h0, w1, h1, pix_temp);
		}
		return pix_edit;
	}


	void __cdecl obj_LensBlur_t::lbResize_set_w_end(void* dst, void* src, int w, int h, int back) {
		lbResize_wh_end = w - 1;
	}
	void __cdecl obj_LensBlur_t::lbResize_set_h_end(void* dst, void* src, int w, int h, int back) {
		lbResize_wh_end = h - 1;
	}




	struct PixelYC_fbb {
		float y;
		byte cb;
		byte cr;
	};

	void obj_LensBlur_t::lbResize_media(int thread_id, int thread_num, int loop1, int forstep1, int loop2, int forstep2, int flag) {
		forstep1 *= sizeof(struct ExEdit::PixelYCA);
		forstep2 *= sizeof(struct ExEdit::PixelYCA);
		int i_end = (thread_id + 1) * loop1 / thread_num;
		for (int i = thread_id * loop1 / thread_num; i < i_end; i++) {
			int ii = lbresize->resize_step * i + (lbresize->resize_step - 0x10000) / 2;
			int fraction = ii & 0xffff;
			int inv_fraction = 0x10000 - fraction;
			ii >>= 16;
			auto dst = (ExEdit::PixelYCA*)((int)lbresize->resize_temp + i * forstep1);
			auto src0 = (ExEdit::PixelYCA*)((int)lbresize->resize_edit + max(0, ii) * forstep1);
			auto src1 = (ExEdit::PixelYCA*)((int)lbresize->resize_edit + min(ii + 1, lbResize_wh_end) * forstep1);

			for (int j = 0; j < loop2; j++) {
				int src0_a = (src0->a * inv_fraction + 0x800) >> 12;
				int src1_a = (src1->a * fraction + 0x800) >> 12;
				int a = src0_a + src1_a;
				if (a <= 0) {
					dst->y = dst->cb = dst->cr = dst->a = 0;
				} else {
					if (flag) {
						((PixelYC_fbb*)dst)->y = (((PixelYC_fbb*)src0)->y * (float)src0_a + ((PixelYC_fbb*)src1)->y * (float)src1_a) / (float)a;
						((PixelYC_fbb*)dst)->cb = (byte)((((PixelYC_fbb*)src0)->cb * src0_a + ((PixelYC_fbb*)src1)->cb * src1_a) / a);
						((PixelYC_fbb*)dst)->cr = (byte)((((PixelYC_fbb*)src0)->cr * src0_a + ((PixelYC_fbb*)src1)->cr * src1_a) / a);
					} else {
						dst->y = (short)((src0->y * src0_a + src1->y * src1_a) / a);
						dst->cb = (short)((src0->cb * src0_a + src1->cb * src1_a) / a);
						dst->cr = (short)((src0->cr * src0_a + src1->cr * src1_a) / a);
					}
					dst->a = (short)((a + 8) >> 4);
				}
				dst = (ExEdit::PixelYCA*)((int)dst + forstep2);
				src0 = (ExEdit::PixelYCA*)((int)src0 + forstep2);
				src1 = (ExEdit::PixelYCA*)((int)src1 + forstep2);
			}
		}
	}


	void obj_LensBlur_t::lbResize_filter(int thread_id, int thread_num, int loop1, int forstep1, int loop2, int forstep2, int flag) {
		forstep1 *= sizeof(struct AviUtl::PixelYC);
		forstep2 *= sizeof(struct AviUtl::PixelYC);
		int i_end = (thread_id + 1) * loop1 / thread_num;
		for (int i = thread_id * loop1 / thread_num; i < i_end; i++) {
			int ii = lbresize->resize_step * i + (lbresize->resize_step - 0x10000) / 2;
			int fraction = ii & 0xffff;
			int inv_fraction = 0x10000 - fraction;
			ii >>= 16;
			auto dst = (AviUtl::PixelYC*)((int)lbresize->resize_temp + i * forstep1);
			auto src0 = (AviUtl::PixelYC*)((int)lbresize->resize_edit + max(0, ii) * forstep1);
			auto src1 = (AviUtl::PixelYC*)((int)lbresize->resize_edit + min(ii + 1, lbResize_wh_end) * forstep1);

			for (int j = 0; j < loop2; j++) {
				if (flag) {
					((PixelYC_fbb*)dst)->y = (((PixelYC_fbb*)src0)->y * (float)inv_fraction + ((PixelYC_fbb*)src1)->y * (float)fraction) / 65536.0f;
					((PixelYC_fbb*)dst)->cb = (byte)((((PixelYC_fbb*)src0)->cb * inv_fraction + ((PixelYC_fbb*)src1)->cb * fraction + 0x8000) >> 16);
					((PixelYC_fbb*)dst)->cr = (byte)((((PixelYC_fbb*)src0)->cr * inv_fraction + ((PixelYC_fbb*)src1)->cr * fraction + 0x8000) >> 16);
				} else {
					dst->y = (short)((src0->y * inv_fraction + src1->y * fraction + 0x8000) >> 16);
					dst->cb = (short)((src0->cb * inv_fraction + src1->cb * fraction + 0x8000) >> 16);
					dst->cr = (short)((src0->cr * inv_fraction + src1->cr * fraction + 0x8000) >> 16);
				}
				dst = (AviUtl::PixelYC*)((int)dst + forstep2);
				src0 = (AviUtl::PixelYC*)((int)src0 + forstep2);
				src1 = (AviUtl::PixelYC*)((int)src1 + forstep2);
			}
		}
	}


	void __cdecl obj_LensBlur_t::lbResize_media_interpolation_y(int thread_id, int thread_num, void* n1, void* n2) {
		// exedit + 70df0 相当
		int exedit_buffer_line = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
		lbResize_media(thread_id, thread_num, lbresize->resize_h, exedit_buffer_line, lbresize->resize_w, 1, 0);
	}
	void __cdecl obj_LensBlur_t::lbResize_media_interpolation_x(int thread_id, int thread_num, void* n1, void* n2) {
		// exedit + 71270 相当
		int exedit_buffer_line = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
		lbResize_media(thread_id, thread_num, lbresize->resize_w, 1, lbresize->resize_h, exedit_buffer_line, 0);
	}
	void __cdecl obj_LensBlur_t::lbResize_media_interpolation_y_fbb(int thread_id, int thread_num, void* n1, void* n2) {
		// exedit + 71870 相当
		int exedit_buffer_line = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
		lbResize_media(thread_id, thread_num, lbresize->resize_h, exedit_buffer_line, lbresize->resize_w, 1, 1);
	}
	void __cdecl obj_LensBlur_t::lbResize_media_interpolation_x_fbb(int thread_id, int thread_num, void* n1, void* n2) {
		// exedit + 71ce0 相当
		int exedit_buffer_line = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
		lbResize_media(thread_id, thread_num, lbresize->resize_w, 1, lbresize->resize_h, exedit_buffer_line, 1);
	}
	void __cdecl obj_LensBlur_t::lbResize_filter_interpolation_y(int thread_id, int thread_num, void* n1, void* n2) {
		// exedit + 72390 相当
		int si_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_YC_vram_w);
		lbResize_filter(thread_id, thread_num, lbresize->resize_h, si_vram_w, lbresize->resize_w, 1, 0);
	}
	void __cdecl obj_LensBlur_t::lbResize_filter_interpolation_x(int thread_id, int thread_num, void* n1, void* n2) {
		// exedit + 72720 相当
		int si_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_YC_vram_w);
		lbResize_filter(thread_id, thread_num, lbresize->resize_w, 1, lbresize->resize_h, si_vram_w, 0);
	}
	void __cdecl obj_LensBlur_t::lbResize_filter_interpolation_y_fbb(int thread_id, int thread_num, void* n1, void* n2) {
		// exedit + 72c20 相当
		int si_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_YC_vram_w);
		lbResize_filter(thread_id, thread_num, lbresize->resize_h, si_vram_w, lbresize->resize_w, 1, 1);
	}
	void __cdecl obj_LensBlur_t::lbResize_filter_interpolation_x_fbb(int thread_id, int thread_num, void* n1, void* n2) {
		// exedit + 72f90 相当
		int si_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_YC_vram_w);
		lbResize_filter(thread_id, thread_num, lbresize->resize_w, 1, lbresize->resize_h, si_vram_w, 1);
	}


} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_LENSBLUR
