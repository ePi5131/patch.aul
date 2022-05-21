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
#include "patch_fast_polortransform.hpp"
#ifdef PATCH_SWITCH_FAST_POLORTRANSFORM

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include "patch_fast_cl.hpp"
#include "debug_log.hpp"

//#define PATCH_STOPWATCH
#include "stopwatch.hpp"
static stopwatch_mem sw;

namespace patch::fast {
    BOOL PolorTransform_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if constexpr (true) {
            sw.start();

            auto center = efp->track[0];
            auto zoom = efp->track[1];
            auto angle = efp->track[2];
            auto uzu = efp->track[3];
            auto src_w = efpip->obj_w;
            auto src_h = efpip->obj_h;

            auto exedit_max_w = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_w);
            auto exedit_max_h = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h);
            auto exedit_buffer_line = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);

            if (angle < 0) {
                angle = angle + ((-angle - 1) / 3600 + 1) * 3600;
            }

            auto g_center_length = center;
            auto g_radius = zoom * static_cast<int>((src_w + src_h) / std::numbers::pi) / 1000 + center;
            auto g_angle = angle * std::numbers::pi / 1800 + 3.5 * std::numbers::pi;

            if (exedit_max_w < g_radius * 2) g_radius = exedit_max_w / 2;
            if (exedit_max_h < g_radius * 2) g_radius = exedit_max_h / 2;
            if (g_radius <= g_center_length) return FALSE;

            auto g_output_size = g_radius * 2;
            if (g_output_size == 0) return FALSE;

            auto g_uzu = (uzu * 2 * std::numbers::pi) / ((g_radius - g_center_length) * 100);
            if (uzu < 0) {
                g_angle += uzu / 100 * 2 * std::numbers::pi;
            }
            auto g_uzu_a = abs(g_uzu * src_w * 1.414 * 256 * std::numbers::pi);

            if (g_uzu_a > src_w * 256) {
                g_uzu_a = src_w * 256;
            }
            try {
                const auto src_size = exedit_buffer_line * src_h * sizeof(ExEdit::PixelYCA);
                cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, src_size);
                cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->obj_edit);

                const auto dst_size = exedit_buffer_line * g_output_size * 8;
                cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, exedit_buffer_line * g_output_size * 8);

                auto kernel = cl.readyKernel(
                    "PolorTransform",
                    clmem_dst,
                    clmem_src,
                    src_w,
                    src_h,
                    exedit_buffer_line,
                    g_center_length,
                    g_radius,
                    static_cast<float>(g_angle),
                    static_cast<float>(g_uzu),
                    static_cast<float>(g_uzu_a)
                );
                cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)g_output_size ,(size_t)g_output_size });

                cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, dst_size, efpip->obj_temp);
            }
            catch (const cl::Error& err) {
                debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
                return FALSE;
            }

            std::swap(efpip->obj_edit, efpip->obj_temp);
            efpip->obj_w = efpip->obj_h = g_output_size;
            efpip->obj_data.cx = 0;
            efpip->obj_data.cy = 0;

            sw.stop();
            return TRUE;
        }
        else {
            sw.start();

            auto center = efp->track[0];
            auto zoom = efp->track[1];
            auto angle = efp->track[2];
            auto uzu = efp->track[3];
            auto src_w = efpip->obj_w;
            auto src_h = efpip->obj_h;

            auto exedit_max_w = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_w);
            auto exedit_max_h = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h);
            auto exedit_buffer_line = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);

            int g_output_size, g_src_w, g_src_h, g_center_length, g_radius;
            double g_uzu, g_uzu_a, g_angle;

            g_src_w = src_w;
            g_src_h = src_h;
            g_center_length = center;

            if (angle < 0) {
                angle += ((-angle - 1) / 3600 + 1) * 3600;
            }

            g_radius = zoom * static_cast<int>((src_w + src_h) / std::numbers::pi) / 1000 + center;
            g_angle = angle * std::numbers::pi / 1800 + 3.5 * std::numbers::pi;

            if (exedit_max_w < g_radius * 2) g_radius = exedit_max_w / 2;
            if (exedit_max_h < g_radius * 2) g_radius = exedit_max_h / 2;
            if (g_radius <= g_center_length) return FALSE;

            g_output_size = g_radius * 2;
            if (g_output_size == 0) return FALSE;

            g_uzu = (uzu * 2 * std::numbers::pi) / ((g_radius - g_center_length) * 100);
            if (uzu < 0) {
                g_angle -= uzu / 100 * 2 * std::numbers::pi;
            }
            g_uzu_a = abs(g_uzu * src_w * 1.414 * 256 / std::numbers::pi);

            if (g_uzu_a > src_w * 256) {
                g_uzu_a = src_w * 256;
            }

            for (int y = 0; y < g_output_size; y++) {
                auto dst_p = efpip->obj_temp + exedit_buffer_line * y;

                for (int x = 0; x < g_output_size; x++) {
                    auto x_centered = x - g_radius;
                    auto y_centered = y - g_radius;

                    auto r = std::sqrt(x_centered * x_centered + y_centered * y_centered);
                    auto theta = std::atan2(y_centered, x_centered);

                    // 円周で領域の対角線(笑)を割る
                    auto uzu_const = (int)std::round(g_src_w * 256 * 1.414 / ((std::max)(1., r) * std::numbers::pi * 2) + g_uzu_a);

                    auto src_y_tmp = (int)std::round((g_center_length + g_src_h) * 256.0 / g_radius * r);
                    auto src_ys = 256 - src_y_tmp % 256;
                    auto src_y = (src_y_tmp / 256) - g_center_length;

                    // 角度を [0, g_src_w * 128) に変換する
                    auto src_x_tmp = (int)std::round(((g_radius - r) * g_uzu + g_angle - theta) * g_src_w * 128 / std::numbers::pi);
                    src_x_tmp -= uzu_const / 2;

                    auto src_x = (src_x_tmp / 256) % g_src_w;
                    auto src_xt = src_x_tmp % 256;

                    if (uzu_const < 258) {
                        int sum_y  = 0;
                        int sum_cr = 0;
                        int sum_cb = 0;
                        int sum_a  = 0;

                        auto src_xl = efpip->obj_edit + src_x + src_y * exedit_buffer_line;
                        auto src_xr = efpip->obj_edit + (src_x + 1) % g_src_w + src_y * exedit_buffer_line;

                        if (0 <= src_y && src_y < g_src_h) {
                            auto s = (src_xl->a * (256 - src_xt) * src_ys) / 65536;
                            auto t = (src_xr->a * src_xt * src_ys) / 65536;

                            sum_y  = src_xl->y  * s + src_xr->y  * t;
                            sum_cb = src_xl->cb * s + src_xr->cb * t;
                            sum_cr = src_xl->cr * s + src_xr->cr * t;
                            sum_a  = t + s;
                        }

                        src_xl += exedit_buffer_line;
                        src_xr += exedit_buffer_line;

                        if (0 <= src_y + 1 && src_y + 1 < g_src_h) {
                            auto s = (src_xl->a * (256 - src_xt) * (256 - src_ys)) / 65536;
                            auto t = (src_xr->a * src_xt * (256 - src_ys)) / 65536;

                            sum_y  += src_xl->y  * s + src_xr->y  * t;
                            sum_cb += src_xl->cb * s + src_xr->cb * t;
                            sum_cr += src_xl->cr * s + src_xr->cr * t;
                            sum_a  += t + s;
                        }

                        if (sum_a == 0) {
                            dst_p->y  = 0;
                            dst_p->cb = 0;
                            dst_p->cr = 0;
                            dst_p->a  = 0;
                        }
                        else {
                            dst_p->y  = static_cast<short>(sum_y  / sum_a);
                            dst_p->cb = static_cast<short>(sum_cb / sum_a);
                            dst_p->cr = static_cast<short>(sum_cr / sum_a);
                            dst_p->a  = static_cast<short>(sum_a);
                        }
                    }
                    else {
                        int sum_y  = 0;
                        int sum_cb = 0;
                        int sum_cr = 0;
                        int sum_a  = 0;

                        if (0 <= src_y && src_y < g_src_h) {
                            auto uzu_repeat = uzu_const;

                            ExEdit::PixelYCA* itr;
                            auto x_itr = src_x;
                            if (src_xt != 0) {
                                itr = efpip->obj_edit + x_itr + src_y * exedit_buffer_line;

                                x_itr = (x_itr + 1) % g_src_w;

                                auto s = (itr->a * (256 - src_xt) * src_ys) / 65536;

                                sum_y  = (itr->y  * s) / 4096;
                                sum_cb = (itr->cb * s) / 4096;
                                sum_cr = (itr->cr * s) / 4096;
                                sum_a  = s;

                                uzu_repeat -= 256 - src_xt;
                            }
                            for (int i = 0; i < uzu_repeat / 256; i++, x_itr = (x_itr + 1) % g_src_w) {
                                itr = efpip->obj_edit + x_itr + src_y * exedit_buffer_line;

                                auto c = (itr->a * src_ys) / 256;

                                sum_y  += (itr->y  * c) / 4096;
                                sum_cb += (itr->cb * c) / 4096;
                                sum_cr += (itr->cr * c) / 4096;
                                sum_a  += c;
                            }
                            if (uzu_repeat % 256 != 0) {
                                itr = efpip->obj_edit + x_itr + src_y * exedit_buffer_line;
                                auto t = (itr->a * (uzu_repeat % 256) * src_ys) / 65536;

                                sum_y  += itr->y  * t / 4096;
                                sum_cb += itr->cb * t / 4096;
                                sum_cr += itr->cr * t / 4096;
                                sum_a  += t;
                            }
                        }

                        auto src_yt = 256 - src_ys;
                        src_y += 1;
                        if (0 <= src_y && src_y < g_src_h) {
                            int uzu_repeat = uzu_const;

                            ExEdit::PixelYCA* itr;
                            auto x_itr = src_x;
                            if (src_xt != 0) {
                                itr = efpip->obj_edit + x_itr + src_y * exedit_buffer_line;

                                x_itr = (x_itr + 1) % g_src_w;

                                auto s = (itr->a * (256 - src_xt) * src_yt) / 65536;

                                sum_y  += (itr->y  * s) / 4096;
                                sum_cb += (itr->cb * s) / 4096;
                                sum_cr += (itr->cr * s) / 4096;
                                sum_a  += s;

                                uzu_repeat -= 256 - src_xt;
                            }

                            for (int i = 0; i < uzu_repeat / 256; i++, x_itr = (x_itr + 1) % g_src_w) {
                                itr = efpip->obj_edit + x_itr + src_y * exedit_buffer_line;

                                auto c = (itr->a * src_yt) / 256;

                                sum_y  += (itr->y  * c) / 4096;
                                sum_cb += (itr->cb * c) / 4096;
                                sum_cr += (itr->cr * c) / 4096;
                                sum_a  += c;
                            }
                            if (uzu_repeat % 256 != 0) {
                                itr = efpip->obj_edit + x_itr + src_y * exedit_buffer_line;
                                auto t = (itr->a * (uzu_repeat % 256) * src_yt) / 65536;

                                sum_y  += (itr->y  * t) / 4096;
                                sum_cb += (itr->cb * t) / 4096;
                                sum_cr += (itr->cr * t) / 4096;
                                sum_a  += t;
                            }
                        }

                        if (sum_a == 0) {
                            dst_p->y  = 0;
                            dst_p->cb = 0;
                            dst_p->cr = 0;
                            dst_p->a  = 0;
                        }
                        else {
                            double dVar2 = 4096.0 / sum_a;
                            dst_p->y  = static_cast<short>(std::round(sum_y  * dVar2));
                            dst_p->cb = static_cast<short>(std::round(sum_cb * dVar2));
                            dst_p->cr = static_cast<short>(std::round(sum_cr * dVar2));
                            dst_p->a  = static_cast<short>(sum_a * 256 / uzu_const);
                        }
                    }
                    dst_p++;
                }
            }

            std::swap(efpip->obj_edit, efpip->obj_temp);
            efpip->obj_w = efpip->obj_h = g_output_size;
            efpip->obj_data.cx = 0;
            efpip->obj_data.cy = 0;

            sw.stop();
            return TRUE;
        }
    }
}
#endif // ifdef PATCH_SWITCH_FAST_POLORTRANSFORM
