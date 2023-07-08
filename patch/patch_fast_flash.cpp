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

#include "patch_fast_flash.hpp"
#ifdef PATCH_SWITCH_FAST_FLASH

#include "debug_log.hpp"
#include "patch_fast_cl.hpp"

//#define PATCH_STOPWATCH
#include "stopwatch.hpp"

namespace patch::fast {
	static stopwatch_mem sw;


	BOOL Flash_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		if constexpr (true) {
			sw.start();

			auto exedit_max_w = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_w);
			auto exedit_max_h = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h);
			auto exedit_buffer_line = load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
			efFlash_var& flash = *(efFlash_var*)uintptr_t(reinterpret_cast<efFlash_var*>(GLOBAL::exedit_base + OFS::ExEdit::efFlash_var_ptr));

			int intensity = efp->track[0];
			int cx = efp->track[1];
			int cy = efp->track[2];
			flash.range = 750;


			unsigned int* exdata = (unsigned int*)efp->exdata_ptr;

			int pixel_range, temp_range;


			if (intensity == 0) {
				return TRUE;
			}
			flash.corrected_intensity = (intensity * 4096) / 1000;

			flash.r_intensity = 4096 - flash.corrected_intensity;
			if (flash.r_intensity < 0) {
				flash.r_intensity = 0;
			}
			flash.cx = cx + efpip->obj_w / 2;
			flash.cy = cy + efpip->obj_h / 2;


			pixel_range = (std::max)({ std::abs(flash.cx), std::abs(flash.cy), std::abs(flash.cx - efpip->obj_w), std::abs(flash.cy - efpip->obj_h) });

			temp_range = (int)round(sqrt(efpip->obj_h * efpip->obj_h + efpip->obj_w * efpip->obj_w));
			if (temp_range < pixel_range) {
				pixel_range = temp_range;
			}

			if (has_flag(efpip->flag, ExEdit::FilterProcInfo::Flag::fast_preview))
				if (50 < pixel_range) pixel_range = 50;


			flash.pixel_range = pixel_range / 4 + pixel_range / 2;


			if (efp->check[2]) { // サイズ固定
				flash.temp_x = 0;
				flash.temp_y = 0;
				flash.temp_w = efpip->obj_w;
				flash.temp_h = efpip->obj_h;
			} else {
				int tx, ty, tw, th;
				if (flash.cx < 0) {
					tx = 0;
				} else {
					tx = -3 * flash.cx;
				}
				if (flash.cy < 0) {
					ty = 0;
				} else {
					ty = -3 * flash.cy;
				}
				if (efpip->obj_w < flash.cx) {
					tw = 0;
				} else {
					tw = 3 * (efpip->obj_w - flash.cx);
				}
				if (efpip->obj_h < flash.cy) {
					th = 0;
				} else {
					th = 3 * (efpip->obj_h - flash.cy);
				}

				tx = tx * flash.corrected_intensity / 4096;
				ty = ty * flash.corrected_intensity / 4096;
				tw = (tw * flash.corrected_intensity / 4096) + efpip->obj_w;
				th = (th * flash.corrected_intensity / 4096) + efpip->obj_h;

				int tmin = (std::min)({ exedit_max_w, efpip->scene_w + efpip->obj_w * 2 });
				int sub1 = tw - tx - tmin;
				if (0 < sub1) {
					int sub2 = efpip->obj_w - tx - tw;
					if (0 < sub2) {
						if (sub2 < sub1) {
							tx += sub2;
							sub1 -= sub2;
						} else {
							tx += sub1;
							sub1 = 0;
						}
					} else {
						if (-sub2 < sub1) {
							tw += sub2;
							sub1 += sub2;
						} else {
							tw -= sub1;
							sub1 = 0;
						}
					}
					if (sub1 & 1) {
						tw--;
						sub1--;
					}
					if (0 < sub1) {
						tx += sub1 / 2;
						tw -= sub1 / 2;
					}
				}
				tmin = (std::min)({ exedit_max_h, efpip->scene_h + efpip->obj_h * 2 });
				sub1 = th - ty - tmin;
				if (0 < sub1) {
					int sub2 = efpip->obj_h - ty - th;
					if (0 < sub2) {
						if (sub2 < sub1) {
							ty += sub2;
							sub1 -= sub2;
						} else {
							ty += sub1;
							sub1 = 0;
						}
					} else {
						if (-sub2 < sub1) {
							th += sub2;
							sub1 += sub2;
						} else {
							th -= sub1;
							sub1 = 0;
						}
					}
					if (sub1 & 1) {
						th--;
						sub1--;
					}
					if (0 < sub1) {
						ty += sub1 / 2;
						th -= sub1 / 2;
					}
				}
				flash.temp_x = tx;
				flash.temp_y = ty;
				flash.temp_w = tw;
				flash.temp_h = th;
			}
			const auto dst_w = flash.temp_w - flash.temp_x;
			const auto dst_h = flash.temp_h - flash.temp_y;

			reinterpret_cast<void(__cdecl*)(short*,short*,short*,int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&flash.color_y, &flash.color_cb, &flash.color_cr, exdata[0] & 0xffffff);
			try {
				const auto src_size = exedit_buffer_line * efpip->obj_h * sizeof(ExEdit::PixelYCA);
				cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, src_size);
				cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->obj_edit);

				const auto dst_size = exedit_buffer_line * dst_h * sizeof(ExEdit::PixelYCA);
				cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, dst_size);

				cl::Kernel kernel;
				if (exdata[0] & 0x1000000) { // 指定なし(元画像の色)
					kernel = cl.readyKernel(
						"Flash",
						clmem_dst,
						clmem_src,
						efpip->obj_w,
						efpip->obj_h,
						exedit_buffer_line,
						flash.cx,
						flash.cy,
						flash.range,
						flash.pixel_range,
						flash.temp_x,
						flash.temp_y,
						flash.r_intensity
					);
				} else {
					kernel = cl.readyKernel(
						"FlashColor",
						clmem_dst,
						clmem_src,
						efpip->obj_w,
						efpip->obj_h,
						exedit_buffer_line,
						flash.cx,
						flash.cy,
						flash.range,
						flash.pixel_range,
						flash.temp_x,
						flash.temp_y,
						flash.r_intensity,
						flash.color_y,
						flash.color_cb,
						flash.color_cr
					);
				}
				cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)dst_w, (size_t)dst_h });

				cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, dst_size, efpip->obj_temp);
			}
			catch (const cl::Error& err) {
				debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
				return FALSE;
			}


			switch (exdata[1]) { //合成モード
			case 0: // 前方に合成
				efp->exfunc->bufcpy(efpip->obj_temp, -flash.temp_x, -flash.temp_y, efpip->obj_edit, 0, 0, efpip->obj_w, efpip->obj_h, 0, 0x11000003);
				break;
			case 1: // 後方に合成
				efp->exfunc->bufcpy(efpip->obj_temp, -flash.temp_x, -flash.temp_y, efpip->obj_edit, 0, 0, efpip->obj_w, efpip->obj_h, 0, 3);
				break;
				//  case 2: // 光成分のみ
			}

			std::swap(efpip->obj_temp, efpip->obj_edit);

			efpip->obj_data.cx += (efpip->obj_w - flash.temp_w - flash.temp_x) * 2048;
			efpip->obj_data.cy += (efpip->obj_h - flash.temp_h - flash.temp_y) * 2048;
			efpip->obj_w = flash.temp_w - flash.temp_x;
			efpip->obj_h = flash.temp_h - flash.temp_y;

			sw.stop();
			return TRUE;
		} else {
			sw.start();
			const auto ret = ((BOOL(*)(ExEdit::Filter*, ExEdit::FilterProcInfo*))(GLOBAL::exedit_base + OFS::ExEdit::efRadiationalBlur_func_proc))(efp, efpip);
			sw.stop();
			return ret;
		}
	}
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_FLASH
