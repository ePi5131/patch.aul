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

#include "patch_fast_directionalblur.hpp"
#ifdef PATCH_SWITCH_FAST_DIRECTIONALBLUR

#include "debug_log.hpp"
#include "patch_fast_cl.hpp"

//#define PATCH_STOPWATCH
#include "stopwatch.hpp"

namespace patch::fast {
    static stopwatch_mem sw;

    BOOL __cdecl DirectionalBlur_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        efDirectionalBlur_var& dblur = *(efDirectionalBlur_var*)uintptr_t(reinterpret_cast<efDirectionalBlur_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDirectionalBlur_var_ptr));

        int range = efp->track[0];
        if (range == 0) {
            return TRUE;
        }

        double rad = efp->track[1] * 0.00174532925199433;
        int x_step = (int)(sin(rad) * -65536.0); // PI / 1800
        int y_step = (int)(cos(rad) * 65536.0);

        int temp_range = range * std::abs(x_step) >> 16;
        dblur.x_begin = -temp_range;
        dblur.x_end = efpip->obj_w + temp_range;

        temp_range = range * std::abs(y_step) >> 16;
        dblur.y_begin = -temp_range;
        dblur.y_end = efpip->obj_h + temp_range;

        if (range < 16) {
            x_step = (int)(x_step + (x_step >> 0x1f & 3)) >> 2;
            y_step = (int)(y_step + (y_step >> 0x1f & 3)) >> 2;
            range <<= 2;
        } else if (range < 32) {
            x_step >>= 1;
            y_step >>= 1;
            range <<= 1;
        } else if (128 < range) {
            x_step *= range;
            y_step *= range;
            x_step = (int)(x_step + (x_step >> 0x1f & 0x7f)) >> 7;
            y_step = (int)(y_step + (y_step >> 0x1f & 0x7f)) >> 7;
            range = 128;
        }

        if (has_flag(efpip->flag, ExEdit::FilterProcInfo::Flag::fast_preview) && 50 < range) {
            dblur.range = 50;
            dblur.x_step = (x_step * range) / 50;
            dblur.y_step = (y_step * range) / 50;
        } else {
            dblur.range = range;
            dblur.x_step = x_step;
            dblur.y_step = y_step;
        }
        if (efp->check[0] == 0) {

            int oversize = dblur.x_end - dblur.x_begin - efpip->obj_line;
            if (0 < oversize) {
                oversize++;
                oversize >>= 1;
                dblur.x_begin += oversize;
                dblur.x_end -= oversize;
            }

            oversize = dblur.y_end - dblur.y_begin - efpip->obj_max_h;
            if (0 < oversize) {
                oversize++;
                oversize >>= 1;
                dblur.y_begin += oversize;
                dblur.y_end -= oversize;
            }


            try {
                const auto src_size = efpip->obj_line * efpip->obj_h * sizeof(ExEdit::PixelYCA);
                cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, src_size);
                cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->obj_edit);

                const auto dst_size = efpip->obj_line * (dblur.y_end - dblur.y_begin) * sizeof(ExEdit::PixelYCA);
                cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, dst_size);

                cl::Kernel kernel;
                kernel = cl.readyKernel(
                    "DirectionalBlur_Media",
                    clmem_dst,
                    clmem_src,
                    efpip->obj_w,
                    efpip->obj_h,
                    efpip->obj_line,
                    dblur.x_begin,
                    dblur.x_end,
                    dblur.x_step,
                    dblur.y_begin,
                    dblur.y_end,
                    dblur.y_step,
                    dblur.range
                );
                cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)(dblur.x_end - dblur.x_begin), (size_t)(dblur.y_end - dblur.y_begin) });

                cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, dst_size, efpip->obj_temp);
            }
            catch (const cl::Error& err) {
                debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
                efp->aviutl_exfunc->exec_multi_thread_func(reinterpret_cast<void(__cdecl*)(int, int, void*, void*)>(GLOBAL::exedit_base + 0xc4a0), efp, efpip);
                return TRUE;
            }

            

            efpip->obj_data.cx += (efpip->obj_w - dblur.x_end - dblur.x_begin) * 0x800;
            efpip->obj_data.cy += (efpip->obj_h - dblur.y_end - dblur.y_begin) * 0x800;
            efpip->obj_w = dblur.x_end - dblur.x_begin;
            efpip->obj_h = dblur.y_end - dblur.y_begin;

        } else { // サイズ固定
            dblur.x_begin = 0;
            dblur.y_begin = 0;
            dblur.x_end = efpip->obj_w;
            dblur.y_end = efpip->obj_h;

            try {
                const auto src_size = efpip->obj_line * efpip->obj_h * sizeof(ExEdit::PixelYCA);
                cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, src_size);
                cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->obj_edit);

                cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, src_size);

                cl::Kernel kernel;
                kernel = cl.readyKernel(
                    "DirectionalBlur_original_size",
                    clmem_dst,
                    clmem_src,
                    efpip->obj_w,
                    efpip->obj_h,
                    efpip->obj_line,
                    dblur.x_step,
                    dblur.y_step,
                    dblur.range
                );
                cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->obj_w, (size_t)efpip->obj_h });

                cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, src_size, efpip->obj_temp);
            }
            catch (const cl::Error& err) {
                debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
                efp->aviutl_exfunc->exec_multi_thread_func(reinterpret_cast<void(__cdecl*)(int, int, void*, void*)>(GLOBAL::exedit_base + 0xc720), efp, efpip);
                return TRUE;
            }
            

        }
        std::swap(efpip->obj_temp, efpip->obj_edit);

        return TRUE;
    }



    void __cdecl DirectionalBlur_t::filter_mt_wrap00cae8(AviUtl::MultiThreadFunc func, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        efDirectionalBlur_var& dblur = *(efDirectionalBlur_var*)uintptr_t(reinterpret_cast<efDirectionalBlur_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDirectionalBlur_var_ptr));

        try {
            const auto src_size = efpip->obj_line * efpip->scene_h * sizeof(ExEdit::PixelYC);
            cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, src_size);
            cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->frame_edit);

            cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, src_size);

            cl::Kernel kernel;
            kernel = cl.readyKernel(
                "DirectionalBlur_Filter",
                clmem_dst,
                clmem_src,
                efpip->scene_w,
                efpip->scene_h,
                efpip->scene_line,
                dblur.x_step,
                dblur.y_step,
                dblur.range
            );
            cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->scene_w, (size_t)efpip->scene_h });

            cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, src_size, efpip->frame_temp);
        }
        catch (const cl::Error& err) {
            debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
            efp->aviutl_exfunc->exec_multi_thread_func(func, efp, efpip);
        }
    }



} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_DIRECTIONALBLUR
