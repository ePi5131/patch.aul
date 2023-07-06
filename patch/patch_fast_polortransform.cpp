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
	BOOL PolorTransform_t::mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		if constexpr (true) {
			if (256 < efpip->obj_w * efpip->obj_h) {
				sw.start();
				try {

					efPolorTransform_var& polor = *reinterpret_cast<efPolorTransform_var*>(GLOBAL::exedit_base + OFS::ExEdit::efPolorTransform_var_ptr);

					const auto src_size = efpip->obj_line * polor.src_h * sizeof(ExEdit::PixelYCA);
					cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, src_size);
					cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->obj_edit);

					const auto dst_size = efpip->obj_line * polor.output_size * 8;
					cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, dst_size);

					auto kernel = cl.readyKernel(
						"PolorTransform",
						clmem_dst,
						clmem_src,
						polor.src_w,
						polor.src_h,
						efpip->obj_line,
						polor.center_length,
						polor.radius,
						static_cast<float>(polor.angle),
						static_cast<float>(polor.uzu),
						static_cast<float>(polor.uzu_a)
					);
					cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)polor.output_size ,(size_t)polor.output_size });

					cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, dst_size, efpip->obj_temp);
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
#endif // ifdef PATCH_SWITCH_FAST_POLORTRANSFORM
