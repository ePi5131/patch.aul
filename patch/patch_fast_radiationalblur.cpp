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
#include "patch_fast_cl.hpp"

//#define PATCH_STOPWATCH
#include "stopwatch.hpp"

namespace patch::fast {
    static stopwatch_mem sw;

    BOOL RadiationalBlur_t::media_mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if constexpr (true) {
            sw.start();
            try {
                auto& rb = *(efRadiationalBlur_var*)uintptr_t(reinterpret_cast<efRadiationalBlur_var*>(GLOBAL::exedit_base + OFS::ExEdit::efRadiationalBlur_var_ptr));

                const auto src_size = efpip->obj_line * efpip->obj_h * sizeof(ExEdit::PixelYCA);
                cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, src_size);
                cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->obj_edit);

                const auto dst_size = efpip->obj_line * (rb.h - rb.obj_cy) * sizeof(ExEdit::PixelYCA);
                cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, dst_size);

                auto kernel = cl.readyKernel(
                    "RadiationalBlur_Media",
                    clmem_dst,
                    clmem_src,
                    efpip->obj_w,
                    efpip->obj_h,
                    efpip->scene_line,
                    rb.blur_cx,
                    rb.blur_cy,
                    rb.obj_cx,
                    rb.obj_cy,
                    rb.range,
                    rb.pixel_range
                );
                cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)rb.w - rb.obj_cx ,(size_t)rb.h - rb.obj_cy });

                cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, dst_size, efpip->obj_temp);
            }
            catch (const cl::Error& err) {
                debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
                return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
            }

            sw.stop();
            return TRUE;
        } else {
            sw.start();
            const auto ret = efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
            sw.stop();
            return ret;
        }
    }

    BOOL RadiationalBlur_t::filter_mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if constexpr (true) {
            sw.start();
            try {
                const auto buf_size = efpip->scene_line * efpip->scene_h * sizeof(ExEdit::PixelYC);
                cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, buf_size);
                cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, buf_size, efpip->frame_edit);

                cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, buf_size);
                auto& rb = *(efRadiationalBlur_var*)uintptr_t(reinterpret_cast<efRadiationalBlur_var*>(GLOBAL::exedit_base + OFS::ExEdit::efRadiationalBlur_var_ptr));
                auto kernel = cl.readyKernel(
                    "RadiationalBlur_Filter",
                    clmem_dst,
                    clmem_src,
                    efpip->scene_line,
                    rb.blur_cx,
                    rb.blur_cy,
                    rb.range,
                    rb.pixel_range
                );
                cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->scene_w ,(size_t)efpip->scene_h });

                cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, buf_size, efpip->frame_temp);
            }
            catch (const cl::Error& err) {
                debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
                return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
            }

            sw.stop();
            return TRUE;
        } else {
            sw.start();
            const auto ret = efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
            sw.stop();
            return ret;
        }
    }

    BOOL RadiationalBlur_t::filter_mt_far_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if constexpr (true) {
            sw.start();
            try {
                const auto buf_size = efpip->scene_line * efpip->scene_h * sizeof(ExEdit::PixelYC);
                cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, buf_size);
                cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, buf_size, efpip->frame_edit);

                cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, buf_size);
                auto& rb = *(efRadiationalBlur_var*)uintptr_t(reinterpret_cast<efRadiationalBlur_var*>(GLOBAL::exedit_base + OFS::ExEdit::efRadiationalBlur_var_ptr));
                auto kernel = cl.readyKernel(
                    "RadiationalBlur_Filter_Far",
                    clmem_dst,
                    clmem_src,
                    efpip->scene_w,
                    efpip->scene_h,
                    efpip->scene_line,
                    rb.blur_cx,
                    rb.blur_cy,
                    rb.range,
                    rb.pixel_range
                );
                cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->scene_w ,(size_t)efpip->scene_h });

                cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, buf_size, efpip->frame_temp);
            }
            catch (const cl::Error& err) {
                debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
                return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
            }

            sw.stop();
            return TRUE;
        } else {
            sw.start();
            const auto ret = efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
            sw.stop();
            return ret;
        }
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
