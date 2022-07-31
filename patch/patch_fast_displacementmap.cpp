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
#include "patch_fast_displacementmap.hpp"
#ifdef PATCH_SWITCH_FAST_DISPLACEMENTMAP

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
    BOOL DisplacementMap_t::mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if constexpr (true) {
            if (256 < efpip->obj_w * efpip->obj_h) {
                sw.start();
                try {

                    efDisplacementMap_var& dmap = *reinterpret_cast<efDisplacementMap_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDisplacementMap_var_ptr);
                    auto& ExEditMemory = *(void**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);

                    const auto buf_size = efpip->obj_line * efpip->obj_h * sizeof(ExEdit::PixelYCA);
                    cl::Buffer clmem_src1(cl.context, CL_MEM_READ_ONLY, buf_size);
                    cl.queue.enqueueWriteBuffer(clmem_src1, CL_TRUE, 0, buf_size, efpip->obj_edit);

                    cl::Buffer clmem_src2(cl.context, CL_MEM_READ_ONLY, buf_size);
                    cl.queue.enqueueWriteBuffer(clmem_src2, CL_TRUE, 0, buf_size, ExEditMemory);

                    cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, buf_size);

                    int calc_id = ((ExEdit::Exdata::efDisplacementMap*)efp->exdata_ptr)->calc;
                    if (calc_id < 0 && 2 < calc_id) {
                        calc_id = 0;
                    }

                    auto kernel = cl.readyKernel(
                        cl_func_name[calc_id],
                        clmem_dst,
                        clmem_src1,
                        clmem_src2,
                        efpip->obj_w,
                        efpip->obj_h,
                        efpip->obj_line,
                        dmap.param0,
                        dmap.param1,
                        dmap.ox,
                        dmap.oy
                    );
                    cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->obj_w ,(size_t)efpip->obj_h });

                    cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, buf_size, efpip->obj_temp);
                }
                catch (const cl::Error& err) {
                    debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
                    return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
                }
                sw.stop();
                return TRUE;
            } else {
                return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
            }
        } else {
            sw.start();
            const auto ret = efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
            sw.stop();
            return ret;
        }
    }
}
#endif // ifdef PATCH_SWITCH_FAST_DISPLACEMENTMAP
