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
#include "patch_fast_glow.hpp"
#ifdef PATCH_SWITCH_FAST_GLOW

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include "debug_log.hpp"
#include <immintrin.h>


namespace patch::fast {


    struct fastGlow256 {
        __m256i data;
        __m256i y, cb, cr;
        __m256i offset;
    };


    void __declspec(noinline) __fastcall fg256_add(fastGlow256* fg256, ExEdit::PixelYCA* src) {
        __m256i ycb256 = _mm256_i32gather_epi32((int*)src, fg256->offset, 1);
        __m256i y256 = _mm256_srai_epi32(_mm256_slli_epi32(ycb256, 16), 16);
        fg256->y = _mm256_add_epi32(fg256->y, y256);
        __m256i cb256 = _mm256_srai_epi32(ycb256, 16);
        fg256->cb = _mm256_add_epi32(fg256->cb, cb256);
        __m256i cbcr256 = _mm256_i32gather_epi32((int*)&src->cb, fg256->offset, 1);
        __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
        fg256->cr = _mm256_add_epi32(fg256->cr, cr256);
    }
    void __declspec(noinline) __fastcall fg256_sub(fastGlow256* fg256, ExEdit::PixelYCA* src) {
        __m256i ycb256 = _mm256_i32gather_epi32((int*)src, fg256->offset, 1);
        __m256i y256 = _mm256_srai_epi32(_mm256_slli_epi32(ycb256, 16), 16);
        fg256->y = _mm256_sub_epi32(fg256->y, y256);
        __m256i cb256 = _mm256_srai_epi32(ycb256, 16);
        fg256->cb = _mm256_sub_epi32(fg256->cb, cb256);
        __m256i cbcr256 = _mm256_i32gather_epi32((int*)&src->cb, fg256->offset, 1);
        __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
        fg256->cr = _mm256_sub_epi32(fg256->cr, cr256);
    }

    void __declspec(noinline) __fastcall fg256_put_average(fastGlow256* fg256, ExEdit::PixelYCA* dst, int n) {
        __m256i ave_y256 = _mm256_div_epi32(fg256->y, fg256->data);
        __m256i ave_cb256 = _mm256_div_epi32(fg256->cb, fg256->data);
        __m256i ave_cr256 = _mm256_div_epi32(fg256->cr, fg256->data);
        for (int i = 0; i < 16; i += 2) {
            dst->y = ave_y256.m256i_i16[i];
            dst->cb = ave_cb256.m256i_i16[i];
            dst->cr = ave_cr256.m256i_i16[i];
            dst = (ExEdit::PixelYCA*)((int)dst + n);
        }
    }


    void __declspec(noinline) __fastcall fg_put256_weight(fastGlow256* fg256, ExEdit::PixelYCA* dst, int n) {
        fastGlow256 fg256s;

        __m256i y256 = _mm256_srai_epi32(fg256->y, 4);
        y256 = _mm256_mullo_epi32(y256, fg256->data);
        fg256s.y = _mm256_srai_epi32(y256, 10);

        __m256i cb256 = _mm256_srai_epi32(fg256->cb, 4);
        cb256 = _mm256_mullo_epi32(cb256, fg256->data);
        fg256s.cb = _mm256_srai_epi32(cb256, 10);

        __m256i cr256 = _mm256_srai_epi32(fg256->cr, 4);
        cr256 = _mm256_mullo_epi32(cr256, fg256->data);
        fg256s.cr = _mm256_srai_epi32(cr256, 10);

        fg256s.offset = fg256->offset;
        fg256_add(&fg256s, dst);

        for (int i = 0; i < 8; i++) {
            int y = fg256s.y.m256i_i32[i];
            if (0x2000 < y) {
                dst->y = 0x2000;
                dst->cb = (short)((fg256s.cb.m256i_i32[i] << 13) / y);
                dst->cr = (short)((fg256s.cr.m256i_i32[i] << 13) / y);
            } else {
                dst->y = (short)y;
                dst->cb = fg256s.cb.m256i_i16[i * 2];
                dst->cr = fg256s.cr.m256i_i16[i * 2];
            }
            dst = (ExEdit::PixelYCA*)((int)dst + n);
        }
    }


    struct fastGlow128 {
        __m128i data;
        int y, cb, cr, a;
    };
    void __declspec(noinline) __fastcall fg_put128_weight(fastGlow128* fg128, ExEdit::PixelYCA* dst) {
        __m128i yc128 = _mm_srai_epi32(_mm_loadu_si128((__m128i*)&fg128->y), 4);
        yc128 = _mm_mullo_epi32(yc128, fg128->data);
        yc128 = _mm_srai_epi32(yc128, 10);
        yc128 = _mm_add_epi32(yc128, _mm_cvtepi16_epi32(_mm_loadu_epi64(dst)));

        int y = yc128.m128i_i32[0];
        if (0x2000 < y) {
            dst->y = 0x2000;
            dst->cb = (yc128.m128i_i32[1] << 13) / y;
            dst->cr = (yc128.m128i_i32[2] << 13) / y;
        } else {
            dst->y = (short)y;
            dst->cb = yc128.m128i_i16[2];
            dst->cr = yc128.m128i_i16[4];
        }
    }


    void __cdecl Glow_t::vertical_convolution(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto glow = (efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        int w = glow->src_w + glow->diffusion_w * 2;
        int h = glow->src_h + glow->diffusion_h * 2;
        int blur = min(glow->blur, (h - 1) / 2);
        int range = blur * 2 + 1;
        int range_h = h - range;
        int linesize = efpip->obj_line * sizeof(struct ExEdit::PixelYCA);
        int x_end = (thi + 1) * w / thn;

        fastGlow256 fg256;
        fg256.data = _mm256_set1_epi32(range);
        fg256.offset = _mm256_set_epi32(7 * sizeof(struct ExEdit::PixelYCA), 6 * sizeof(struct ExEdit::PixelYCA), 5 * sizeof(struct ExEdit::PixelYCA), 4 * sizeof(struct ExEdit::PixelYCA), 3 * sizeof(struct ExEdit::PixelYCA), 2 * sizeof(struct ExEdit::PixelYCA), sizeof(struct ExEdit::PixelYCA), 0);

        int x = thi * w / thn;
        int x_end256 = x_end - 7;
        for (; x < x_end256; x += 8) {
            auto pix1 = (ExEdit::PixelYCA*)((int)glow->buf_temp + x * sizeof(struct ExEdit::PixelYCA));
            auto pix2 = pix1;
            auto mem = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + x * sizeof(struct ExEdit::PixelYCA));

            fg256.y = _mm256_setzero_si256();
            fg256.cb = _mm256_setzero_si256();
            fg256.cr = _mm256_setzero_si256();
            for (int y = 0; y < blur; y++) {
                fg256_add(&fg256, pix1);
                pix1 = (ExEdit::PixelYCA*)((int)pix1 + linesize);
            }
            for (int y = 0; y <= blur; y++) {
                fg256_add(&fg256, pix1);
                pix1 = (ExEdit::PixelYCA*)((int)pix1 + linesize);

                fg256_put_average(&fg256, mem, sizeof(struct ExEdit::PixelYCA));
                mem = (ExEdit::PixelYCA*)((int)mem + linesize);
            }
            for (int y = 0; y < range_h; y++) {
                fg256_sub(&fg256, pix2);
                pix2 = (ExEdit::PixelYCA*)((int)pix2 + linesize);

                fg256_add(&fg256, pix1);
                pix1 = (ExEdit::PixelYCA*)((int)pix1 + linesize);

                fg256_put_average(&fg256, mem, sizeof(struct ExEdit::PixelYCA));
                mem = (ExEdit::PixelYCA*)((int)mem + linesize);
            }
            for (int y = 0; y < blur; y++) {
                fg256_sub(&fg256, pix2);
                pix2 = (ExEdit::PixelYCA*)((int)pix2 + linesize);

                fg256_put_average(&fg256, mem, sizeof(struct ExEdit::PixelYCA));
                mem = (ExEdit::PixelYCA*)((int)mem + linesize);
            }
        }


        for (; x < x_end; x++) {
            auto pix1 = (ExEdit::PixelYCA*)((int)glow->buf_temp + x * sizeof(struct ExEdit::PixelYCA));
            auto pix2 = pix1;
            auto mem = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + x * sizeof(struct ExEdit::PixelYCA));
            int sum_y = 0;
            int sum_cb = 0;
            int sum_cr = 0;
            for (int y = 0; y < blur; y++) {
                sum_y += pix1->y;
                sum_cb += pix1->cb;
                sum_cr += pix1->cr;
                pix1 = (ExEdit::PixelYCA*)((int)pix1 + linesize);
            }
            for (int y = 0; y <= blur; y++) {
                sum_y += pix1->y;
                sum_cb += pix1->cb;
                sum_cr += pix1->cr;
                pix1 = (ExEdit::PixelYCA*)((int)pix1 + linesize);
                mem->y = (short)(sum_y / range);
                mem->cb = (short)(sum_cb / range);
                mem->cr = (short)(sum_cr / range);
                mem = (ExEdit::PixelYCA*)((int)mem + linesize);
            }
            for (int y = 0; y < range_h; y++) {
                sum_y += pix1->y - pix2->y;
                sum_cb += pix1->cb - pix2->cb;
                sum_cr += pix1->cr - pix2->cr;
                pix1 = (ExEdit::PixelYCA*)((int)pix1 + linesize);
                pix2 = (ExEdit::PixelYCA*)((int)pix2 + linesize);
                mem->y = (short)(sum_y / range);
                mem->cb = (short)(sum_cb / range);
                mem->cr = (short)(sum_cr / range);
                mem = (ExEdit::PixelYCA*)((int)mem + linesize);
            }
            for (int y = 0; y < blur; y++) {
                sum_y -= pix2->y;
                sum_cb -= pix2->cb;
                sum_cr -= pix2->cr;
                pix2 = (ExEdit::PixelYCA*)((int)pix2 + linesize);
                mem->y = (short)(sum_y / range);
                mem->cb = (short)(sum_cb / range);
                mem->cr = (short)(sum_cr / range);
                mem = (ExEdit::PixelYCA*)((int)mem + linesize);
            }
        }
    }





    void __cdecl Glow_t::horizontal_convolution(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto glow = (efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        int h = glow->src_h + glow->diffusion_h * 2;
        int w = glow->src_w + glow->diffusion_w * 2;
        int blur = min(glow->blur, (w - 1) / 2);
        int range = blur * 2 + 1;
        int range_w = w - range;
        int linesize = efpip->obj_line * sizeof(struct ExEdit::PixelYCA);
        int y_begin = thi * h / thn;
        int y_end = (thi + 1) * h / thn;
        int offset = y_begin * linesize;

        fastGlow256 fg256;
        fg256.data = _mm256_set1_epi32(range);
        fg256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(linesize));

        int y = y_begin;
        int y_end256 = y_end - 7;
        for (; y < y_end256; y += 8) {
            auto mem1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + offset);
            auto mem2 = mem1;
            auto pix = (ExEdit::PixelYCA*)((int)glow->buf_temp + offset);

            fg256.y = _mm256_setzero_si256();
            fg256.cb = _mm256_setzero_si256();
            fg256.cr = _mm256_setzero_si256();
            for (int x = 0; x < blur; x++) {
                fg256_add(&fg256, mem1);
                mem1++;
            }
            for (int x = 0; x <= blur; x++) {
                fg256_add(&fg256, mem1);
                mem1++;

                fg256_put_average(&fg256, pix, linesize);
                pix++;
            }
            for (int x = 0; x < range_w; x++) {
                fg256_sub(&fg256, mem2);
                mem2++;

                fg256_add(&fg256, mem1);
                mem1++;

                fg256_put_average(&fg256, pix, linesize);
                pix++;
            }
            for (int x = 0; x < blur; x++) {
                fg256_sub(&fg256, mem2);
                mem2++;

                fg256_put_average(&fg256, pix, linesize);
                pix++;
            }
            offset += linesize * 8;
        }

        for (; y < y_end; y++) {
            auto mem1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + offset);
            auto mem2 = mem1;
            auto pix = (ExEdit::PixelYCA*)((int)glow->buf_temp + offset);
            int sum_y = 0;
            int sum_cb = 0;
            int sum_cr = 0;
            for (int x = 0; x < blur; x++) {
                sum_y += mem1->y;
                sum_cb += mem1->cb;
                sum_cr += mem1->cr;
                mem1++;
            }
            for (int x = 0; x <= blur; x++) {
                sum_y += mem1->y;
                sum_cb += mem1->cb;
                sum_cr += mem1->cr;
                mem1++;
                pix->y = (short)(sum_y / range);
                pix->cb = (short)(sum_cb / range);
                pix->cr = (short)(sum_cr / range);
                pix++;
            }
            for (int x = 0; x < range_w; x++) {
                sum_y += mem1->y - mem2->y;
                sum_cb += mem1->cb - mem2->cb;
                sum_cr += mem1->cr - mem2->cr;
                mem1++;
                mem2++;
                pix->y = (short)(sum_y / range);
                pix->cb = (short)(sum_cb / range);
                pix->cr = (short)(sum_cr / range);
                pix++;
            }
            for (int x = 0; x < blur; x++) {
                sum_y -= mem2->y;
                sum_cb -= mem2->cb;
                sum_cr -= mem2->cr;
                mem2++;
                pix->y = (short)(sum_y / range);
                pix->cb = (short)(sum_cb / range);
                pix->cr = (short)(sum_cr / range);
                pix++;
            }
            offset += linesize;
        }
    }


    void __cdecl Glow_t::horizontal_convolution_intensity_blur(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto glow = (efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        int h = glow->src_h + glow->diffusion_h * 2;
        int blur = glow->blur;
        int range_w = glow->src_w + (glow->diffusion_w - blur) * 2 - 1;
        int intensity = glow->intensity;
        int linesize = efpip->obj_line * sizeof(struct ExEdit::PixelYCA);
        int y_begin = thi * h / thn;
        int y_end = (thi + 1) * h / thn;
        int offset = y_begin * linesize;

        fastGlow256 fg256;
        fg256.data = _mm256_set1_epi32(intensity);
        fg256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(linesize));

        int y = y_begin;
        int y_end256 = y_end - 7;
        for (; y < y_end256; y += 8) {
            auto mem1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + offset);
            auto mem2 = mem1;
            auto pix = (ExEdit::PixelYCA*)((int)glow->buf_temp + offset);

            fg256.y = _mm256_setzero_si256();
            fg256.cb = _mm256_setzero_si256();
            fg256.cr = _mm256_setzero_si256();
            for (int x = 0; x < blur; x++) {
                fg256_add(&fg256, mem1);
                mem1++;
            }
            for (int x = 0; x <= blur; x++) {
                fg256_add(&fg256, mem1);
                mem1++;

                fg_put256_weight(&fg256, pix, linesize);
                pix++;
            }
            for (int x = 0; x < range_w; x++) {
                fg256_sub(&fg256, mem2);
                mem2++;

                fg256_add(&fg256, mem1);
                mem1++;

                fg_put256_weight(&fg256, pix, linesize);
                pix++;
            }
            for (int x = 0; x < blur; x++) {
                fg256_sub(&fg256, mem2);
                mem2++;

                fg_put256_weight(&fg256, pix, linesize);
                pix++;
            }
            offset += linesize * 8;
        }


        fastGlow128 fg128;
        fg128.data = _mm_set1_epi32(intensity);
        for (; y < y_end; y++) {
            auto mem1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + offset);
            auto mem2 = mem1;
            auto pix = (ExEdit::PixelYCA*)((int)glow->buf_temp + offset);
            fg128.y = 0;
            fg128.cb = 0;
            fg128.cr = 0;
            for (int x = 0; x < blur; x++) {
                fg128.y += mem1->y;
                fg128.cb += mem1->cb;
                fg128.cr += mem1->cr;
                mem1++;
            }
            for (int x = 0; x <= blur; x++) {
                fg128.y += mem1->y;
                fg128.cb += mem1->cb;
                fg128.cr += mem1->cr;
                mem1++;

                fg_put128_weight(&fg128, pix);
                pix++;
            }
            for (int x = 0; x < range_w; x++) {
                fg128.y += mem1->y - mem2->y;
                fg128.cb += mem1->cb - mem2->cb;
                fg128.cr += mem1->cr - mem2->cr;
                mem1++;
                mem2++;

                fg_put128_weight(&fg128, pix);
                pix++;
            }
            for (int x = 0; x < blur; x++) {
                fg128.y -= mem2->y;
                fg128.cb -= mem2->cb;
                fg128.cr -= mem2->cr;
                mem2++;

                fg_put128_weight(&fg128, pix);
                pix++;
            }
            offset += linesize;
        }
    }



    void vertical_convolution_intensity_main(int thi, int thn, int n, ExEdit::FilterProcInfo* efpip) {
        auto glow = (Glow_t::efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        int w = glow->src_w;
        int h = glow->src_h;
        int diff_h = glow->diffusion_h;
        int diff = min(diff_h / n, (h - 1) / 2);
        int diff2 = diff * 2;
        int range = h - diff2 - 1;
        int linesize = efpip->obj_line * sizeof(struct ExEdit::PixelYCA);
        int intensity = glow->intensity * n;
        int x_end = (thi + 1) * w / thn;

        auto pix_temp = (ExEdit::PixelYCA*)((int)glow->buf_temp + (diff_h - diff) * linesize + glow->diffusion_w * sizeof(struct ExEdit::PixelYCA));

        
        fastGlow256 fg256;
        fg256.data = _mm256_set1_epi32(intensity);
        fg256.offset = _mm256_set_epi32(7 * sizeof(struct ExEdit::PixelYCA), 6 * sizeof(struct ExEdit::PixelYCA), 5 * sizeof(struct ExEdit::PixelYCA), 4 * sizeof(struct ExEdit::PixelYCA), 3 * sizeof(struct ExEdit::PixelYCA), 2 * sizeof(struct ExEdit::PixelYCA), sizeof(struct ExEdit::PixelYCA), 0);

        int x = thi * w / thn;
        int x_end256 = x_end - 7;
        for (; x < x_end256; x += 8) {
            auto mem1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + x * sizeof(struct ExEdit::PixelYCA));
            auto mem2 = mem1;
            auto pix = (ExEdit::PixelYCA*)((int)pix_temp + x * sizeof(struct ExEdit::PixelYCA));

            fg256.y = _mm256_setzero_si256();
            fg256.cb = _mm256_setzero_si256();
            fg256.cr = _mm256_setzero_si256();
            for (int y = 0; y <= diff2; y++) {
                fg256_add(&fg256, mem1);
                mem1 = (ExEdit::PixelYCA*)((int)mem1 + linesize);

                fg_put256_weight(&fg256, pix, sizeof(struct ExEdit::PixelYCA));
                pix = (ExEdit::PixelYCA*)((int)pix + linesize);
            }
            for (int y = 0; y < range; y++) {
                fg256_sub(&fg256, mem2);
                mem2 = (ExEdit::PixelYCA*)((int)mem2 + linesize);
                fg256_add(&fg256, mem1);
                mem1 = (ExEdit::PixelYCA*)((int)mem1 + linesize);

                fg_put256_weight(&fg256, pix, sizeof(struct ExEdit::PixelYCA));
                pix = (ExEdit::PixelYCA*)((int)pix + linesize);
            }
            for (int y = 0; y < diff2; y++) {
                fg256_sub(&fg256, mem2);
                mem2 = (ExEdit::PixelYCA*)((int)mem2 + linesize);

                fg_put256_weight(&fg256, pix, sizeof(struct ExEdit::PixelYCA));
                pix = (ExEdit::PixelYCA*)((int)pix + linesize);
            }
        }
        
        fastGlow128 fg128;
        fg128.data = _mm_set1_epi32(intensity);
        for (; x < x_end; x++) {
            auto mem1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + x * sizeof(struct ExEdit::PixelYCA));
            auto mem2 = mem1;
            auto pix = (ExEdit::PixelYCA*)((int)pix_temp + x * sizeof(struct ExEdit::PixelYCA));
            fg128.y = 0;
            fg128.cb = 0;
            fg128.cr = 0;
            for (int y = 0; y <= diff2; y++) {
                fg128.y += mem1->y;
                fg128.cb += mem1->cb;
                fg128.cr += mem1->cr;
                mem1 = (ExEdit::PixelYCA*)((int)mem1 + linesize);

                fg_put128_weight(&fg128, pix);
                pix = (ExEdit::PixelYCA*)((int)pix + linesize);
            }
            for (int y = 0; y < range; y++) {
                fg128.y += mem1->y - mem2->y;
                fg128.cb += mem1->cb - mem2->cb;
                fg128.cr += mem1->cr - mem2->cr;
                mem1 = (ExEdit::PixelYCA*)((int)mem1 + linesize);
                mem2 = (ExEdit::PixelYCA*)((int)mem2 + linesize);

                fg_put128_weight(&fg128, pix);
                pix = (ExEdit::PixelYCA*)((int)pix + linesize);
            }
            for (int y = 0; y < diff2; y++) {
                fg128.y -= mem2->y;
                fg128.cb -= mem2->cb;
                fg128.cr -= mem2->cr;
                mem2 = (ExEdit::PixelYCA*)((int)mem2 + linesize);

                fg_put128_weight(&fg128, pix);
                pix = (ExEdit::PixelYCA*)((int)pix + linesize);
            }
        }
    }
    void __cdecl Glow_t::vertical_convolution_intensity3(int thi, int thn, ExEdit::Filter * efp, ExEdit::FilterProcInfo * efpip) {
        vertical_convolution_intensity_main(thi, thn, 1, efpip);
        vertical_convolution_intensity_main(thi, thn, 2, efpip);
        vertical_convolution_intensity_main(thi, thn, 4, efpip);
    }



    void horizontal_convolution_intensity_main(int thi, int thn, int n, ExEdit::FilterProcInfo* efpip) {
        auto glow = (Glow_t::efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        int w = glow->src_w;
        int h = glow->src_h;
        int diff_w = glow->diffusion_w;
        int diff = min(diff_w / n, (w - 1) / 2);
        int diff2 = diff * 2;
        int range = w - diff2 - 1;
        int linesize = efpip->obj_line * sizeof(struct ExEdit::PixelYCA);
        int intensity = glow->intensity * n;
        int y_begin = thi * h / thn;
        int y_end = (thi + 1) * h / thn;
        int offset = y_begin * linesize;

        auto pix_temp = (ExEdit::PixelYCA*)((int)glow->buf_temp + glow->diffusion_h * linesize + (diff_w - diff) * sizeof(struct ExEdit::PixelYCA));


        fastGlow256 fg256;
        fg256.data = _mm256_set1_epi32(intensity);
        fg256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(linesize));

        int y = y_begin;
        int y_end256 = y_end - 7;
        for (; y < y_end256; y += 8) {
            auto mem1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + offset);
            auto mem2 = mem1;
            auto pix = (ExEdit::PixelYCA*)((int)pix_temp + offset);

            fg256.y = _mm256_setzero_si256();
            fg256.cb = _mm256_setzero_si256();
            fg256.cr = _mm256_setzero_si256();
            for (int x = 0; x <= diff2; x++) {
                fg256_add(&fg256, mem1);
                mem1++;

                fg_put256_weight(&fg256, pix, linesize);
                pix++;
            }
            for (int x = 0; x < range; x++) {
                fg256_sub(&fg256, mem2);
                mem2++;
                fg256_add(&fg256, mem1);
                mem1++;

                fg_put256_weight(&fg256, pix, linesize);
                pix++;
            }
            for (int x = 0; x < diff2; x++) {
                fg256_sub(&fg256, mem2);
                mem2++;

                fg_put256_weight(&fg256, pix, linesize);
                pix++;
            }
            offset += linesize * 8;
        }

        fastGlow128 fg128;
        fg128.data = _mm_set1_epi32(intensity);
        for (; y < y_end; y++) {
            auto mem1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + offset);
            auto mem2 = mem1;
            auto pix = (ExEdit::PixelYCA*)((int)pix_temp + offset);
            fg128.y = 0;
            fg128.cb = 0;
            fg128.cr = 0;
            for (int x = 0; x <= diff2; x++) {
                fg128.y += mem1->y;
                fg128.cb += mem1->cb;
                fg128.cr += mem1->cr;
                mem1++;

                fg_put128_weight(&fg128, pix);
                pix++;
            }
            for (int x = 0; x < range; x++) {
                fg128.y += mem1->y - mem2->y;
                fg128.cb += mem1->cb - mem2->cb;
                fg128.cr += mem1->cr - mem2->cr;
                mem1++;
                mem2++;

                fg_put128_weight(&fg128, pix);
                pix++;
            }
            for (int x = 0; x < diff2; x++) {
                fg128.y -= mem2->y;
                fg128.cb -= mem2->cb;
                fg128.cr -= mem2->cr;
                mem2++;

                fg_put128_weight(&fg128, pix);
                pix++;
            }
            offset += linesize;
        }
    }
    void __cdecl Glow_t::horizontal_convolution_intensity3(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        horizontal_convolution_intensity_main(thi, thn, 1, efpip);
        horizontal_convolution_intensity_main(thi, thn, 2, efpip);
        horizontal_convolution_intensity_main(thi, thn, 4, efpip);
    }


}
#endif // ifdef PATCH_SWITCH_FAST_GLOW
