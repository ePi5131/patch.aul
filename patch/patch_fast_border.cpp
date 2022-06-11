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
#include "patch_fast_border.hpp"
#ifdef PATCH_SWITCH_FAST_BORDER

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include "debug_log.hpp"
#include <immintrin.h>

//#define PATCH_STOPWATCH
#include "stopwatch.hpp"

namespace patch::fast {


    // AVX2が有効かどうかを判定する関数を追加してください
    // AVX2が無効の場合でも縁取り関連のバグ修正は行われるようになっています
    BOOL enable_avx2() {
        auto cpucmdset = get_CPUCmdSet();
        return has_flag(cpucmdset, CPUCmdSet::F_AVX2);
    }


    void efBorder_horizontal_convolution_alpha_simd(int thread_id, int thread_num, void* param1, void* param2);
    void efBorder_horizontal_convolution_alpha_simd2(int thread_id, int thread_num, void* param1, void* param2);
    void efBorder_vertical_convolution_alpha_and_put_color_simd(int thread_id, int thread_num, void* param1, void* param2);
    void efBorder_vertical_convolution_alpha_and_put_color_simd2(int thread_id, int thread_num, void* param1, void* param2);
    void efBorder_vertical_convolution_alpha_simd(int thread_id, int thread_num, void* param1, void* param2);
    void efBorder_vertical_convolution_alpha_simd2(int thread_id, int thread_num, void* param1, void* param2);

    BOOL Border_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if constexpr (true) {
            static stopwatch_mem sw{};
            sw.start();

            auto& border = *(reinterpret_cast<efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr));
            auto& ExEditMemory = *(void**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);

            if (efp->track[0] <= 0) return TRUE;

            auto exdata = (ExEdit::Exdata::efBorder*)efp->exdata_ptr;

            int obj_w = efpip->obj_w;
            int obj_h = efpip->obj_h;

            int temp;
            int add_size = efp->track[0] * 2;

            border.inv_range = 0x1000 - efp->track[1] * 4096 / 1000;


            add_size = (std::min)({ add_size, efpip->obj_line - obj_w, efpip->obj_max_h - obj_h });
            add_size &= 0xfffffffe;

            obj_w += add_size;
            obj_h += add_size;

            // efpip->obj_tempに画像を読み込む
            int file_w = 0;
            int file_h = 0;
            if (exdata->file[0] != '\0') {
                if (efp->exfunc->load_image((ExEdit::PixelYCA*)ExEditMemory, exdata->file, &file_w, &file_h, 0, 0)) {
                    for (int i = 0; i < obj_h; i += file_h) {
                        for (int j = 0; j < obj_w; j += file_w) {
                            efp->exfunc->bufcpy(efpip->obj_temp, j, i, ExEditMemory, 0, 0, file_w, file_h, 0, 0x13000003);
                        }
                    }
                }
            }


            border.ExEditMemory = (unsigned short*)ExEditMemory;
            border.add_size = add_size;
            border.alpha = (int)round(65536.0 / ((double)efp->track[1] * add_size * 0.01 + 1.0));
            temp = border.alpha;
            int sft = 0;
            while (sft < 16 && 64 < temp) {
                temp >>= 1;
                sft++;
            }
            border.alpha = temp;
            border._alpha_shift = 16 - sft;

            reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&border.color_y, &border.color_cb, &border.color_cr, *(int*)&exdata->color & 0xffffff);

            if (efpip->obj_w <= add_size) {
                efp->aviutl_exfunc->exec_multi_thread_func(efBorder_horizontal_convolution_alpha_simd2, efp, efpip);
            } else {
                efp->aviutl_exfunc->exec_multi_thread_func(efBorder_horizontal_convolution_alpha_simd, efp, efpip);
            }

            if (efpip->obj_h <= add_size) {
                if (file_w == 0 || file_h == 0) { // 画像なし
                    efp->aviutl_exfunc->exec_multi_thread_func(efBorder_vertical_convolution_alpha_and_put_color_simd2, efp, efpip);
                } else { // 画像あり
                    efp->aviutl_exfunc->exec_multi_thread_func(efBorder_vertical_convolution_alpha_simd2, efp, efpip);
                }
            } else {
                if (file_w == 0 || file_h == 0) { // 画像なし
                    efp->aviutl_exfunc->exec_multi_thread_func(efBorder_vertical_convolution_alpha_and_put_color_simd, efp, efpip);
                } else { // 画像あり
                    efp->aviutl_exfunc->exec_multi_thread_func(efBorder_vertical_convolution_alpha_simd, efp, efpip);
                }
            }


            efp->exfunc->bufcpy(efpip->obj_temp, add_size / 2, add_size / 2, efpip->obj_edit, 0, 0, efpip->obj_w, efpip->obj_h, 0, 3);

            std::swap(efpip->obj_temp, efpip->obj_edit);

            efpip->obj_w = obj_w;
            efpip->obj_h = obj_h;


            sw.stop();
            return TRUE;
        }
        else {
            static stopwatch_mem sw{};
            sw.start();
            auto ret = ((decltype(ExEdit::Filter::func_proc))(GLOBAL::exedit_base + OFS::ExEdit::efBorder_func_proc_ptr))(efp, efpip);
            sw.stop();
            return ret;
        }
    }



#define ALPHA_TEMP_MAX 0xFFFF

    void efBorder_horizontal_convolution_alpha_simd(int thread_id, int thread_num, void* param1, void* param2) {
        auto& border = *reinterpret_cast<Border_t::efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr);
        auto efp = static_cast<ExEdit::Filter*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        int begin_thread = efpip->obj_h * thread_id / thread_num;
        int end_thread = efpip->obj_h * (thread_id + 1) / thread_num;

        int y = begin_thread;

        if (enable_avx2()) {
            __m256i offset256 = _mm256_mullo_epi32(_mm256_set_epi32(14, 12, 10, 8, 6, 4, 2, 0), _mm256_set1_epi32(efpip->obj_line));
            __m256i border_alpha256 = _mm256_set1_epi32(border.alpha);
            __m256i a_mem_max256 = _mm256_set1_epi32(ALPHA_TEMP_MAX);
            int end_y = end_thread - 7;
            for (; y < end_y; y += 8) {

                int* pix1 = (int*)((ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line) + 1;
                int* pix2 = pix1;
                unsigned short* mem = border.ExEditMemory + y * efpip->obj_line;

                __m256i a_sum256 = _mm256_setzero_si256();
                int x;
                for (x = 0; x <= border.add_size; x++) {
                    __m256i a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pix1, offset256, 4), 16);
                    a_sum256 = _mm256_add_epi32(a_sum256, a256);
                    a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_mem_max256);

                    for (int i = 0; i < 8; i++) {
                        mem[i * efpip->obj_line] = (unsigned short)a256.m256i_u32[i];
                    }

                    pix1 += 2;
                    mem++;
                }


                for (; x < efpip->obj_w; x++) {
                    __m256i a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pix2, offset256, 4), 16);
                    a_sum256 = _mm256_sub_epi32(a_sum256, a256);
                    a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pix1, offset256, 4), 16);
                    a_sum256 = _mm256_add_epi32(a_sum256, a256);
                    a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_mem_max256);

                    for (int i = 0; i < 8; i++) {
                        mem[i * efpip->obj_line] = (unsigned short)a256.m256i_u32[i];
                    }

                    pix1 += 2;
                    pix2 += 2;
                    mem++;
                }
                int end_x = efpip->obj_w + border.add_size;
                for (; x < end_x; x++) {
                    __m256i a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pix2, offset256, 4), 16);
                    a_sum256 = _mm256_sub_epi32(a_sum256, a256);
                    a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_mem_max256);

                    for (int i = 0; i < 8; i++) {
                        mem[i * efpip->obj_line] = (unsigned short)a256.m256i_u32[i];
                    }

                    pix2 += 2;
                    mem++;
                }
            }
        }

        for (; y < end_thread; y++) {
            short* pixa1 = (short*)((ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line) + 3;
            short* pixa2 = pixa1;
            unsigned short* mem = border.ExEditMemory + y * efpip->obj_line;
            int a_sum = 0;

            int x;
            for (x = 0; x <= border.add_size; x++) {
                a_sum += *pixa1;
                *mem = (unsigned short)min(a_sum * border.alpha >> border._alpha_shift, ALPHA_TEMP_MAX);

                pixa1 += 4;
                mem++;
            }

            for (; x < efpip->obj_w; x++) {
                a_sum += *pixa1 - *pixa2;
                *mem = (unsigned short)min(a_sum * border.alpha >> border._alpha_shift, ALPHA_TEMP_MAX);

                pixa1 += 4;
                pixa2 += 4;
                mem++;
            }
            for (x = 0; x < border.add_size; x++) {
                a_sum -= *pixa2;
                *mem = (unsigned short)min(a_sum * border.alpha >> border._alpha_shift, ALPHA_TEMP_MAX);

                pixa2 += 4;
                mem++;
            }
        }
    }

    void efBorder_horizontal_convolution_alpha_simd2(int thread_id, int thread_num, void* param1, void* param2) { // 51ae0
        auto& border = *reinterpret_cast<Border_t::efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr);
        auto efp = static_cast<ExEdit::Filter*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        int begin_thread = efpip->obj_h * thread_id / thread_num;
        int end_thread = efpip->obj_h * (thread_id + 1) / thread_num;

        int y = begin_thread;

        if (enable_avx2()) {
            __m256i offset256 = _mm256_mullo_epi32(_mm256_set_epi32(14, 12, 10, 8, 6, 4, 2, 0), _mm256_set1_epi32(efpip->obj_line));
            __m256i border_alpha256 = _mm256_set1_epi32(border.alpha);
            __m256i a_mem_max256 = _mm256_set1_epi32(ALPHA_TEMP_MAX);
            int end_y = end_thread - 7;
            for (; y < end_y; y += 8) {

                int* pix1 = (int*)((ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line) + 1;
                int* pix2 = pix1;
                unsigned short* mem = border.ExEditMemory + y * efpip->obj_line;

                __m256i a_sum256 = _mm256_setzero_si256();
                __m256i a256;
                int x;
                for (x = 0; x < efpip->obj_w; x++) {
                    a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pix1, offset256, 4), 16);
                    a_sum256 = _mm256_add_epi32(a_sum256, a256);
                    a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_mem_max256);

                    for (int i = 0; i < 8; i++) {
                        mem[i * efpip->obj_line] = (unsigned short)a256.m256i_u32[i];
                    }

                    pix1 += 2;
                    mem++;
                }

                for (; x < border.add_size; x++) {
                    for (int i = 0; i < 8; i++) {
                        mem[i * efpip->obj_line] = (unsigned short)a256.m256i_u32[i];
                    }
                    mem++;
                }
                for (x = 0; x < efpip->obj_w; x++) {
                    a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pix2, offset256, 4), 16);
                    a_sum256 = _mm256_sub_epi32(a_sum256, a256);
                    a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_mem_max256);

                    for (int i = 0; i < 8; i++) {
                        mem[i * efpip->obj_line] = (unsigned short)a256.m256i_u32[i];
                    }

                    pix2 += 2;
                    mem++;
                }
            }
        }

        for (; y < end_thread; y++) {
            short* pixa1 = (short*)((ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line) + 3;
            short* pixa2 = pixa1;
            unsigned short* mem = border.ExEditMemory + y * efpip->obj_line;
            unsigned int a_sum = 0;
            unsigned short a;

            int x;
            for (x = 0; x < efpip->obj_w; x++) {
                a_sum += *pixa1;
                *mem = a = (unsigned short)min(a_sum * border.alpha >> border._alpha_shift, ALPHA_TEMP_MAX);

                pixa1 += 4;
                mem++;
            }

            for (; x < border.add_size; x++) {
                *mem = a;
                mem++;
            }
            for (x = 0; x < efpip->obj_w; x++) {
                a_sum -= *pixa2;
                *mem = (unsigned short)min(a_sum * border.alpha >> border._alpha_shift, ALPHA_TEMP_MAX);

                pixa2 += 4;
                mem++;
            }
        }
    }

    void efBorder_vertical_convolution_alpha_and_put_color_simd(int thread_id, int thread_num, void* param1, void* param2) {
        auto& border = *reinterpret_cast<Border_t::efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr);
        auto efp = static_cast<ExEdit::Filter*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        ExEdit::PixelYCA* pix;
        ExEdit::PixelYCA color = { border.color_y, border.color_cb, border.color_cr,0 };

        unsigned short* mem1;
        unsigned short* mem2;

        int begin_thread = (efpip->obj_w + border.add_size) * thread_id / thread_num;
        int end_thread = (efpip->obj_w + border.add_size) * (thread_id + 1) / thread_num;

        int x = begin_thread;

        if (enable_avx2()) {
            __m256i color256 = _mm256_set1_epi64x(*(long long*)&color);
            __m256i border_alpha256 = _mm256_set1_epi64x(border.alpha);
            __m256i a_pix_max256 = _mm256_set1_epi64x(0x1000);
            int end_x = end_thread - 3;
            for (; x < end_x; x += 4) {
                pix = (ExEdit::PixelYCA*)efpip->obj_temp + x;
                mem1 = mem2 = border.ExEditMemory + x;

                __m256i a_sum256 = _mm256_setzero_si256();

                int y;
                for (y = 0; y <= border.add_size; y++) {
                    a_sum256 = _mm256_add_epi64(a_sum256, _mm256_cvtepu16_epi64(*(__m128i*)mem1));
                    __m256i pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, border._alpha_shift);
                    pix256 = _mm256_min_epi32(pix256, a_pix_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    *(__m256i*)pix = _mm256_or_si256(pix256, color256);

                    pix += efpip->obj_line;
                    mem1 += efpip->obj_line;
                }
                for (; y < efpip->obj_h; y++) {
                    a_sum256 = _mm256_sub_epi64(a_sum256, _mm256_cvtepu16_epi64(*(__m128i*)mem2));
                    a_sum256 = _mm256_add_epi64(a_sum256, _mm256_cvtepu16_epi64(*(__m128i*)mem1));
                    __m256i pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, border._alpha_shift);
                    pix256 = _mm256_min_epi32(pix256, a_pix_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    *(__m256i*)pix = _mm256_or_si256(pix256, color256);

                    pix += efpip->obj_line;
                    mem1 += efpip->obj_line;
                    mem2 += efpip->obj_line;
                }

                for (y = 0; y < border.add_size; y++) {
                    a_sum256 = _mm256_sub_epi64(a_sum256, _mm256_cvtepu16_epi64(*(__m128i*)mem2));
                    __m256i pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, border._alpha_shift);
                    pix256 = _mm256_min_epi32(pix256, a_pix_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    *(__m256i*)pix = _mm256_or_si256(pix256, color256);

                    pix += efpip->obj_line;
                    mem2 += efpip->obj_line;
                }
            }
        }

        for (; x < end_thread; x++) {
            pix = (ExEdit::PixelYCA*)efpip->obj_temp + x;
            mem1 = mem2 = border.ExEditMemory + x;

            unsigned int a_sum = 0;
            int y;
            for (y = 0; y <= border.add_size; y++) {
                a_sum += *mem1;
                if (a_sum == 0) {
                    pix->a = 0;
                } else {
                    color.a = (short)min(a_sum * border.alpha >> border._alpha_shift, 0x1000);
                    *pix = color;
                }

                pix += efpip->obj_line;
                mem1 += efpip->obj_line;
            }
            for (; y < efpip->obj_h; y++) {
                a_sum += *mem1 - *mem2;
                if (a_sum == 0) {
                    pix->a = 0;
                } else {
                    color.a = (short)min(a_sum * border.alpha >> border._alpha_shift, 0x1000);
                    *pix = color;
                }

                pix += efpip->obj_line;
                mem1 += efpip->obj_line;
                mem2 += efpip->obj_line;
            }

            for (int y = 0; y < border.add_size; y++) {
                a_sum -= *mem2;
                if (a_sum == 0) {
                    pix->a = 0;
                } else {
                    color.a = (short)min(a_sum * border.alpha >> border._alpha_shift, 0x1000);
                    *pix = color;
                }

                pix += efpip->obj_line;
                mem2 += efpip->obj_line;
            }
        }
    }

    void efBorder_vertical_convolution_alpha_and_put_color_simd2(int thread_id, int thread_num, void* param1, void* param2) {
        auto& border = *reinterpret_cast<Border_t::efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr);
        auto efp = static_cast<ExEdit::Filter*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        ExEdit::PixelYCA* pix;
        ExEdit::PixelYCA color = { border.color_y, border.color_cb, border.color_cr,0 };

        unsigned short* mem1;
        unsigned short* mem2;

        int begin_thread = (efpip->obj_w + border.add_size) * thread_id / thread_num;
        int end_thread = (efpip->obj_w + border.add_size) * (thread_id + 1) / thread_num;

        int x = begin_thread;

        if (enable_avx2()) {
            __m256i color256 = _mm256_set1_epi64x(*(long long*)&color);
            __m256i border_alpha256 = _mm256_set1_epi64x(border.alpha);
            __m256i a_pix_max256 = _mm256_set1_epi64x(0x1000);
            int end_x = end_thread - 3;
            for (; x < end_x; x += 4) {
                pix = (ExEdit::PixelYCA*)efpip->obj_temp + x;
                mem1 = mem2 = border.ExEditMemory + x;

                __m256i a_sum256 = _mm256_setzero_si256();
                __m256i pix256;

                int y;
                for (y = 0; y < efpip->obj_h; y++) {
                    a_sum256 = _mm256_add_epi64(a_sum256, _mm256_cvtepu16_epi64(*(__m128i*)mem1));
                    pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, border._alpha_shift);
                    pix256 = _mm256_min_epi32(pix256, a_pix_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    *(__m256i*)pix = pix256 = _mm256_or_si256(pix256, color256);

                    pix += efpip->obj_line;
                    mem1 += efpip->obj_line;
                }
                for (; y < border.add_size; y++) {
                    *(__m256i*)pix = pix256;
                    pix += efpip->obj_line;
                }

                for (y = 0; y < efpip->obj_h; y++) {
                    a_sum256 = _mm256_sub_epi64(a_sum256, _mm256_cvtepu16_epi64(*(__m128i*)mem2));
                    pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, border._alpha_shift);
                    pix256 = _mm256_min_epi32(pix256, a_pix_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    *(__m256i*)pix = _mm256_or_si256(pix256, color256);

                    pix += efpip->obj_line;
                    mem2 += efpip->obj_line;
                }
            }
        }

        for (; x < end_thread; x++) {
            pix = (ExEdit::PixelYCA*)efpip->obj_temp + x;
            mem1 = mem2 = border.ExEditMemory + x;

            unsigned int a_sum = 0;

            int y;
            for (y = 0; y < efpip->obj_h; y++) {
                a_sum += *mem1;
                if (a_sum == 0) {
                    pix->a = 0;
                } else {
                    color.a = (short)min(a_sum * border.alpha >> border._alpha_shift, 0x1000);
                    *pix = color;
                }

                pix += efpip->obj_line;
                mem1 += efpip->obj_line;
            }

            for (; y < border.add_size; y++) {
                *pix = color;
                pix += efpip->obj_line;
            }

            for (y = 0; y < efpip->obj_h; y++) {
                a_sum -= *mem2;
                if (a_sum == 0) {
                    pix->a = 0;
                } else {
                    color.a = (short)min(a_sum * border.alpha >> border._alpha_shift, 0x1000);
                    *pix = color;
                }

                pix += efpip->obj_line;
                mem2 += efpip->obj_line;
            }
        }
    }

    void efBorder_vertical_convolution_alpha_simd(int thread_id, int thread_num, void* param1, void* param2) {
        auto& border = *reinterpret_cast<Border_t::efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr);
        auto efp = static_cast<ExEdit::Filter*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        unsigned short* mem1;
        unsigned short* mem2;

        int begin_thread = (efpip->obj_w + border.add_size) * thread_id / thread_num;
        int end_thread = (efpip->obj_w + border.add_size) * (thread_id + 1) / thread_num;

        int x = begin_thread;

        if (enable_avx2()) {
            __m256i offset256 = _mm256_set_epi32(14, 12, 10, 8, 6, 4, 2, 0);
            __m256i border_alpha256 = _mm256_set1_epi32(border.alpha);
            __m256i a_pix_max256 = _mm256_set1_epi32(0x1000);
            int end_x = end_thread - 7;
            for (; x < end_x; x += 8) {
                int* pixa = (int*)((ExEdit::PixelYCA*)efpip->obj_temp + x) + 1;

                mem1 = mem2 = border.ExEditMemory + x;

                __m256i a_sum256 = _mm256_setzero_si256();

                int y;
                for (y = 0; y <= border.add_size; y++) {
                    a_sum256 = _mm256_add_epi32(a_sum256, _mm256_cvtepu16_epi32(*(__m128i*)mem1));
                    __m256i a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_pix_max256);
                    __m256i pixa256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pixa, offset256, 4), 16);
                    a256 = _mm256_mullo_epi32(a256, pixa256);
                    a256 = _mm256_srli_epi32(a256, 12);
                    short* pixas = (short*)pixa + 1;
                    for (int i = 0; i < 8; i++) {
                        pixas[i * 4] = (unsigned short)a256.m256i_u32[i];
                    }

                    pixa += efpip->obj_line * 2;
                    mem1 += efpip->obj_line;
                }

                for (; y < efpip->obj_h; y++) {
                    a_sum256 = _mm256_add_epi32(a_sum256, _mm256_cvtepu16_epi32(*(__m128i*)mem1));
                    a_sum256 = _mm256_sub_epi32(a_sum256, _mm256_cvtepu16_epi32(*(__m128i*)mem2));
                    __m256i a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_pix_max256);
                    __m256i pixa256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pixa, offset256, 4), 16);
                    a256 = _mm256_mullo_epi32(a256, pixa256);
                    a256 = _mm256_srli_epi32(a256, 12);
                    short* pixas = (short*)pixa + 1;
                    for (int i = 0; i < 8; i++) {
                        pixas[i * 4] = (unsigned short)a256.m256i_u32[i];
                    }

                    pixa += efpip->obj_line * 2;
                    mem1 += efpip->obj_line;
                    mem2 += efpip->obj_line;
                }

                for (y = 0; y < border.add_size; y++) {
                    a_sum256 = _mm256_sub_epi32(a_sum256, _mm256_cvtepu16_epi32(*(__m128i*)mem2));
                    __m256i a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_pix_max256);
                    __m256i pixa256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pixa, offset256, 4), 16);
                    a256 = _mm256_mullo_epi32(a256, pixa256);
                    a256 = _mm256_srli_epi32(a256, 12);
                    short* pixas = (short*)pixa + 1;
                    for (int i = 0; i < 8; i++) {
                        pixas[i * 4] = (unsigned short)a256.m256i_u32[i];
                    }

                    pixa += efpip->obj_line * 2;
                    mem2 += efpip->obj_line;
                }
            }
        }


        for (; x < end_thread; x++) {
            short* pixa = (short*)((ExEdit::PixelYCA*)efpip->obj_temp + x) + 3;
            mem1 = mem2 = border.ExEditMemory + x;

            int a;
            unsigned int a_sum = 0;

            int y;
            for (y = 0; y <= border.add_size; y++) {
                a_sum += *mem1;
                a = a_sum * border.alpha >> border._alpha_shift;
                if (a < 0x1000) {
                    *pixa = (short)(*pixa * a >> 12);
                }

                pixa += efpip->obj_line * 4;
                mem1 += efpip->obj_line;
            }

            for (; y < efpip->obj_h; y++) {
                a_sum += *mem1 - *mem2;
                a = a_sum * border.alpha >> border._alpha_shift;
                if (a < 0x1000) {
                    *pixa = (short)(*pixa * a >> 12);
                }

                pixa += efpip->obj_line * 4;
                mem1 += efpip->obj_line;
                mem2 += efpip->obj_line;
            }

            for (y = 0; y < border.add_size; y++) {
                a_sum -= *mem2;
                a = a_sum * border.alpha >> border._alpha_shift;
                if (a < 0x1000) {
                    *pixa = (short)(*pixa * a >> 12);
                }

                pixa += efpip->obj_line * 4;
                mem2 += efpip->obj_line;
            }
        }
    }

    void efBorder_vertical_convolution_alpha_simd2(int thread_id, int thread_num, void* param1, void* param2) {
        auto& border = *reinterpret_cast<Border_t::efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr);
        auto efp = static_cast<ExEdit::Filter*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        unsigned short* mem1;
        unsigned short* mem2;

        int begin_thread = (efpip->obj_w + border.add_size) * thread_id / thread_num;
        int end_thread = (efpip->obj_w + border.add_size) * (thread_id + 1) / thread_num;

        int x = begin_thread;

        if (enable_avx2()) {
            __m256i offset256 = _mm256_set_epi32(14, 12, 10, 8, 6, 4, 2, 0);
            __m256i border_alpha256 = _mm256_set1_epi32(border.alpha);
            __m256i a_pix_max256 = _mm256_set1_epi32(0x1000);
            int end_x = end_thread - 7;
            for (; x < end_x; x += 8) {
                int* pixa = (int*)((ExEdit::PixelYCA*)efpip->obj_temp + x) + 1;

                mem1 = mem2 = border.ExEditMemory + x;

                __m256i a_sum256 = _mm256_setzero_si256();
                __m256i a256;

                int y;
                for (y = 0; y < efpip->obj_h; y++) {
                    a_sum256 = _mm256_add_epi32(a_sum256, _mm256_cvtepu16_epi32(*(__m128i*)mem1));
                    a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_pix_max256);
                    __m256i pixa256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pixa, offset256, 4), 16);
                    pixa256 = _mm256_mullo_epi32(a256, pixa256);
                    pixa256 = _mm256_srli_epi32(pixa256, 12);
                    short* pixas = (short*)pixa + 1;
                    for (int i = 0; i < 8; i++) {
                        pixas[i * 4] = (unsigned short)pixa256.m256i_u32[i];
                    }

                    pixa += efpip->obj_line * 2;
                    mem1 += efpip->obj_line;
                }

                for (; y < border.add_size; y++) {
                    __m256i pixa256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pixa, offset256, 4), 16);
                    pixa256 = _mm256_mullo_epi32(a256, pixa256);
                    pixa256 = _mm256_srli_epi32(pixa256, 12);
                    short* pixas = (short*)pixa + 1;
                    for (int i = 0; i < 8; i++) {
                        pixas[i * 4] = (unsigned short)pixa256.m256i_u32[i];
                    }

                    pixa += efpip->obj_line * 2;
                }

                for (y = 0; y < efpip->obj_h; y++) {
                    a_sum256 = _mm256_sub_epi32(a_sum256, _mm256_cvtepu16_epi32(*(__m128i*)mem2));
                    a256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, border._alpha_shift);
                    a256 = _mm256_min_epu32(a256, a_pix_max256);
                    __m256i pixa256 = _mm256_srli_epi32(_mm256_i32gather_epi32(pixa, offset256, 4), 16);
                    pixa256 = _mm256_mullo_epi32(a256, pixa256);
                    pixa256 = _mm256_srli_epi32(pixa256, 12);
                    short* pixas = (short*)pixa + 1;
                    for (int i = 0; i < 8; i++) {
                        pixas[i * 4] = (unsigned short)pixa256.m256i_u32[i];
                    }

                    pixa += efpip->obj_line * 2;
                    mem2 += efpip->obj_line;
                }
            }
        }

        for (; x < end_thread; x++) {
            short* pixa = (short*)((ExEdit::PixelYCA*)efpip->obj_temp + x) + 3;
            mem1 = mem2 = border.ExEditMemory + x;

            int a;
            unsigned int a_sum = 0;

            int y;
            for (y = 0; y < efpip->obj_h; y++) {
                a_sum += *mem1;
                a = a_sum * border.alpha >> border._alpha_shift;
                if (a < 0x1000) {
                    *pixa = (short)(*pixa * a >> 12);
                }

                pixa += efpip->obj_line * 4;
                mem1 += efpip->obj_line;
            }
            
            if (a < 0x1000) {
                for (; y < border.add_size; y++) {
                    *pixa = (short)(*pixa * a >> 12);
                    pixa += efpip->obj_line * 4;
                }
            } else {
                pixa += efpip->obj_line * 4 * (border.add_size - efpip->obj_h);
            }

            for (y = 0; y < efpip->obj_h; y++) {
                a_sum -= *mem2;
                a = a_sum * border.alpha >> border._alpha_shift;
                if (a < 0x1000) {
                    *pixa = (short)(*pixa * a >> 12);
                }

                pixa += efpip->obj_line * 4;
                mem2 += efpip->obj_line;
            }
        }
    }

}
#endif // ifdef PATCH_SWITCH_FAST_BORDER
