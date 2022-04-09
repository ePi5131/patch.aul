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

#include "patch_fast_radiationalblur.hpp"
#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR

#include "debug_log.hpp"
#include "mycl.hpp"

//#define PATCH_STOPWATCH
#include "stopwatch.hpp"

namespace patch::fast {
    static stopwatch_mem sw;

    BOOL RadiationalBlur_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if constexpr (true) {
            sw.start();

            auto exedit_max_w = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_w);
            auto exedit_max_h = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h);
            auto exedit_buffer_line = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);

            int g_X;
            int g_Y;
            int g_Range;

            int pixel_range;
            int g_cx;
            int g_cy;
            int result_x_max;
            int result_y_max;

            int range = efp->track[0];
            if (range == 0) return TRUE;

            int cx = efpip->obj_w / 2 + efp->track[1];
            int cy = efpip->obj_h / 2 + efp->track[2];

            pixel_range = (std::max)({ std::abs(cx), std::abs(cy), std::abs(cx - efpip->obj_w), std::abs(cy - efpip->obj_h) });

            int wh_l2 = (int)std::round(std::sqrt(efpip->obj_w * efpip->obj_w + efpip->obj_h * efpip->obj_h));
            if (wh_l2 < pixel_range) pixel_range = wh_l2;

            if (has_flag(efpip->flag, ExEdit::FilterProcInfo::Flag::fast_preview))
                if (50 < pixel_range) pixel_range = 50;

            auto inv_range = 1000 - range;
            pixel_range = (inv_range * pixel_range) / 1000 + pixel_range / 2;

            // -cx*range/(1000-range)
            g_cx = cx - cx * 1000 / inv_range;
            if (g_cx > 0) g_cx = 0;

            g_cy = cy - cy * 1000 / inv_range;
            if (g_cy > 0) g_cy = 0;

            int var_x = (cx - efpip->obj_w) - (cx - efpip->obj_w) * 1000 / inv_range;
            if (var_x < 0) var_x = 0;

            int var_y = (cy - efpip->obj_h) - (cy - efpip->obj_h) * 1000 / inv_range;
            if (var_y < 0) var_y = 0;

            var_x += efpip->obj_w;
            var_y += efpip->obj_h;
            result_x_max = var_x;

            while (true) {
                if (exedit_max_w >= var_x - g_cx) {
                    result_y_max = var_y;
                    if (efpip->scene_w + efpip->obj_w * 2 < var_x - g_cx);
                    else break;
                }
                if (g_cx < efpip->obj_w - var_x) {
                    g_cx++;
                }
                else {
                    var_x--;
                    result_x_max = var_x;
                }
            }
            while ((exedit_max_h < var_y - g_cy || (efpip->scene_h + efpip->obj_h * 2 < var_y - g_cy))) {
                if (g_cy < efpip->obj_h - var_y) {
                    g_cy++;
                }
                else {
                    var_y--;
                    result_y_max = var_y;
                }
            }

            // サイズ固定
            if (efp->check[0]) {
                g_cx = 0;
                g_cy = 0;
                result_x_max = efpip->obj_w;
                result_y_max = efpip->obj_h;
            }
            g_X = cx;
            g_Y = cy;
            g_Range = range;

            const auto dst_w = result_x_max - g_cx;
            const auto dst_h = result_y_max - g_cy;

            try {
                const auto src_size = exedit_buffer_line * efpip->obj_h * sizeof(ExEdit::PixelYCA);
                cl::Buffer clmem_src(cl_manager.context, CL_MEM_READ_ONLY, src_size);
                cl_manager.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->obj_edit);

                const auto dst_size = exedit_buffer_line * dst_h * sizeof(ExEdit::PixelYCA);
                cl::Buffer clmem_dst(cl_manager.context, CL_MEM_WRITE_ONLY, dst_size);

                auto kernel = cl_manager.readyKernel(
                    "RadiationalBlur",
                    clmem_dst,
                    clmem_src,
                    efpip->obj_w,
                    efpip->obj_h,
                    exedit_buffer_line,
                    g_X,
                    g_Y,
                    g_Range,
                    pixel_range,
                    g_cx,
                    g_cy,
                    result_x_max,
                    result_y_max
                );
                cl_manager.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)result_x_max - g_cx ,(size_t)result_y_max - g_cy });

                cl_manager.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, dst_size, efpip->obj_temp);
            }
            catch (const cl::Error& err) {
                debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
                return FALSE;
            }

            std::swap(efpip->obj_edit, efpip->obj_temp);
            efpip->obj_data.cx += (efpip->obj_w - result_x_max - g_cx) * 2048;
            efpip->obj_data.cy += (efpip->obj_h - result_y_max - g_cy) * 2048;
            efpip->obj_w = dst_w;
            efpip->obj_h = dst_h;

            sw.stop();
            return TRUE;
        }
        else {
            sw.start();
            const auto ret = ((BOOL(*)(ExEdit::Filter*, ExEdit::FilterProcInfo*))(GLOBAL::exedit_base + OFS::ExEdit::efRadiationalBlur_func_proc))(efp, efpip);
            sw.stop();
            return ret;
        }
    }
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
