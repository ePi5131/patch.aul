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

#include "macro.h"
#ifdef PATCH_SWITCH_CL
#include <string_view>
#include <new>
#include <delayimp.h>

#include "debug_log.hpp"

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_VERSION_1_0                              1
#define CL_VERSION_1_1                              1
#define CL_VERSION_1_2                              1
#define CL_VERSION_2_0                              0
#define CL_VERSION_2_1                              0
#define CL_VERSION_2_2                              0
#define CL_VERSION_3_0                              0
#pragma warning(push)
#pragma warning(disable: 4005)
#include <CL/cl.hpp>
#pragma warning(pop)
#pragma comment(lib, "opencl.lib")

#include "cryptostring.hpp"
#include "util.hpp"
#include "debug_log.hpp"
#include "config_rw.hpp"

namespace patch::fast {

inline class cl_t {
#pragma region clprogram
	inline static auto program_str = make_cryptostring(R"(
kernel void PolorTransform(global short* dst, global short* src, int src_w, int src_h, int exedit_buffer_line, int center, int radius, float angle, float uzu, float uzu_a) {
	int x = get_global_id(0);
	int y = get_global_id(1);

	float x_centered = x - radius;
	float y_centered = y - radius;
	
	float r = sqrt(x_centered * x_centered + y_centered * y_centered);
	float theta = atan2(y_centered, x_centered);

	int uzu_const = (int)round(src_w * 256 * 1.414f / (max(1.f, r) * 3.1415927f * 2) + uzu_a);

	int src_y_tmp = (int)round((center + src_h) * 256.0f / radius * r);
	int src_ys = 256 - src_y_tmp % 256;
	int src_y = (src_y_tmp / 256) - center;

	int src_x_tmp = (int)round(((radius - r) * uzu + angle - theta) * src_w * 128 / 3.1415927f);
	src_x_tmp -= uzu_const / 2;

	int src_x = (src_x_tmp / 256) % src_w;
	int src_xt = src_x_tmp % 256;

	if (uzu_const < 258) {
		int sum_y = 0;
		int sum_cr = 0;
		int sum_cb = 0;
		int sum_a = 0;

		global short* src_xl = src + (src_x + src_y * exedit_buffer_line) * 4;
		global short* src_xr = src + ((src_x + 1) % src_w + src_y * exedit_buffer_line) * 4;

		if (0 <= src_y && src_y < src_h) {
			int s = (src_xl[3] * (256 - src_xt) * src_ys) / 65536;
			int t = (src_xr[3] * src_xt         * src_ys) / 65536;

			sum_y  = src_xl[0] * s + src_xr[0] * t;
			sum_cb = src_xl[1] * s + src_xr[1] * t;
			sum_cr = src_xl[2] * s + src_xr[2] * t;
			sum_a  = t + s;
		}

		src_xl += exedit_buffer_line * 4;
		src_xr += exedit_buffer_line * 4;

		if (0 <= src_y + 1 && src_y + 1 < src_h) {
			int s = (src_xl[3] * (256 - src_xt) * (256 - src_ys)) / 65536;
			int t = (src_xr[3] * src_xt         * (256 - src_ys)) / 65536;

			sum_y  += src_xl[0] * s + src_xr[0] * t;
			sum_cb += src_xl[1] * s + src_xr[1] * t;
			sum_cr += src_xl[2] * s + src_xr[2] * t;
			sum_a  += t + s;
		}
		global short* dst_p = dst + (x + y * exedit_buffer_line) * 4;
		if (sum_a == 0) {
			dst_p[0] = 0;
			dst_p[1] = 0;
			dst_p[2] = 0;
			dst_p[3] = 0;
		}
		else {
			dst_p[0] = (short)(sum_y  / sum_a);
			dst_p[1] = (short)(sum_cb / sum_a);
			dst_p[2] = (short)(sum_cr / sum_a);
			dst_p[3] = (short)(sum_a);
		}
	}
	else {
		int sum_y  = 0;
		int sum_cb = 0;
		int sum_cr = 0;
		int sum_a  = 0;

		if (0 <= src_y && src_y < src_h) {
			int uzu_repeat = uzu_const;

			global short* itr;
			int x_itr = src_x;
			if (src_xt != 0) {
				itr = src + (x_itr + src_y * exedit_buffer_line) * 4;

				x_itr = (x_itr + 1) % src_w;
				
				int s = (itr[3] * (256 - src_xt) * src_ys) / 65536;

				sum_y  = (itr[0] * s) / 4096;
				sum_cb = (itr[1] * s) / 4096;
				sum_cr = (itr[2] * s) / 4096;
				sum_a  = s;

				uzu_repeat -= 256 - src_xt;
			}
			for(int i = 0; i < uzu_repeat / 256; i++, x_itr = (x_itr + 1) % src_w) {
				itr = src + (x_itr + src_y * exedit_buffer_line) * 4;

				int c = (itr[3] * src_ys) / 256;

				sum_y  += (itr[0] * c) / 4096;
				sum_cb += (itr[1] * c) / 4096;
				sum_cr += (itr[2] * c) / 4096;
				sum_a  += c;
			}
			if (uzu_repeat % 256 != 0) {
				itr = src + (x_itr + src_y * exedit_buffer_line) * 4;
				int t = (itr[3] * (uzu_repeat % 256) * src_ys) / 65536;

				sum_y  += itr[0] * t / 4096;
				sum_cb += itr[1] * t / 4096;
				sum_cr += itr[2] * t / 4096;
				sum_a  += t;
			}
		}

		int src_yt = 256 - src_ys;
		src_y += 1;
		if (0 <= src_y && src_y < src_h) {
			int uzu_repeat = uzu_const;

			global short* itr;
			int x_itr = src_x;
			if (src_xt != 0) {
				itr = src + (x_itr + src_y * exedit_buffer_line) * 4;

				x_itr = (x_itr + 1) % src_w;

				int s = (itr[3] * (256 - src_xt) * src_yt) / 65536;

				sum_y  += (itr[0] * s) / 4096;
				sum_cb += (itr[1] * s) / 4096;
				sum_cr += (itr[2] * s) / 4096;
				sum_a  += s;

				uzu_repeat -= 256 - src_xt;
			}
			
			for(int i = 0; i < uzu_repeat / 256; i++, x_itr = (x_itr + 1) % src_w) {
				itr = src + (x_itr + src_y * exedit_buffer_line) * 4;

				int c = (itr[3] * src_yt) / 256;

				sum_y  += (itr[0] * c) / 4096;
				sum_cb += (itr[1] * c) / 4096;
				sum_cr += (itr[2] * c) / 4096;
				sum_a  += c;
			}
			if (uzu_repeat % 256 != 0) {
				itr = src + (x_itr + src_y * exedit_buffer_line) * 4;
				int t = (itr[3] * (uzu_repeat % 256) * src_yt) / 65536;

				sum_y  += (itr[0] * t) / 4096;
				sum_cb += (itr[1] * t) / 4096;
				sum_cr += (itr[2] * t) / 4096;
				sum_a  += t;
			}
		}
		
		global short* dst_p = dst + (x + y * exedit_buffer_line) * 4;
		if (sum_a == 0) {
			dst_p[0] = 0;
			dst_p[1] = 0;
			dst_p[2] = 0;
			dst_p[3] = 0;
		}
		else {
			float dVar2 = 4096.0f / sum_a;
			dst_p[0] = (short)(round(sum_y  * dVar2));
			dst_p[1] = (short)(round(sum_cb * dVar2));
			dst_p[2] = (short)(round(sum_cr * dVar2));
			dst_p[3] = (short)(sum_a * 256 / uzu_const);
		}
	}
}
)" R"(
kernel void RadiationalBlur_Media(
	global short* dst, global short* src, int src_w, int src_h, int buffer_line,
	int rb_blur_cx, int rb_blur_cy, int rb_obj_cx, int rb_obj_cy, int rb_range, int rb_pixel_range) {

	int xi = get_global_id(0);
	int yi = get_global_id(1);

	int x = xi + rb_obj_cx;
	int y = yi + rb_obj_cy;

	int pixel_itr = xi + yi * buffer_line;
	int cx = rb_blur_cx - x;
	int cy = rb_blur_cy - y;
	int c_dist_times8 = (int)round(sqrt((float)(cx * cx + cy * cy)) * 8.0f);
	int range = (rb_range * c_dist_times8) / 1000;

	if (rb_pixel_range < c_dist_times8) {
		range = rb_pixel_range * rb_range / 1000;
		c_dist_times8 = rb_pixel_range;
	} else if (8 < c_dist_times8) {
		c_dist_times8 *= 8;
		range *= 8;
	} else if (4 < c_dist_times8) {
		c_dist_times8 *= 4;
		range *= 4;
	} else if (2 < c_dist_times8) {
		c_dist_times8 *= 2;
		range *= 2;
	}

	if (2 <= c_dist_times8 && 2 <= range) {
		int sum_a = 0;
		int sum_cr = 0;
		int sum_cb = 0;
		int sum_y = 0;

		for (int i = 0; i < range; i++) {
			int x_itr = x + i * cx / c_dist_times8;
			int y_itr = y + i * cy / c_dist_times8;
			if (0 <= x_itr && x_itr < src_w && 0 <= y_itr && y_itr < src_h) {
				short4 itr = vload4(x_itr + y_itr * buffer_line, src);
				int itr_a = itr.w;
				if (itr_a != 0) {
					sum_a += itr_a;
					if (itr_a < 0x1000) {
						itr_a = 0x1000;
					}
					sum_y += itr.x * itr_a / 4096;
					sum_cb += itr.y * itr_a / 4096;
					sum_cr += itr.z * itr_a / 4096;
				}
			}
		}
		if (sum_a != 0) {
			vstore4(
				(short4)(
					round(sum_y * 4096.0f / sum_a),
					round(sum_cb * 4096.0f / sum_a),
					round(sum_cr * 4096.0f / sum_a),
					sum_a / range
					),
				pixel_itr, dst
			);
		} else {
			dst[pixel_itr * 4 + 3] = (short)(sum_a / range);
		}
	} else {
		if (x < 0 || y < 0 || src_w <= x || src_h <= y) {
			vstore4((short4)(0, 0, 0, 0), pixel_itr, dst);
		} else {
			vstore4(vload4(x + y * buffer_line, src), pixel_itr, dst);
		}
	}
}
)" R"(
kernel void RadiationalBlur_Filter(
	global short* dst, global short* src, int buffer_line,
	int rb_blur_cx, int rb_blur_cy, int rb_range, int rb_pixel_range) {
	int x = get_global_id(0);
	int y = get_global_id(1);

	int cx = rb_blur_cx - x;
	int cy = rb_blur_cy - y;
	int c_dist_times8 = (int)round(sqrt((float)(cx * cx + cy * cy)) * 8.0f);
	int range = rb_range * c_dist_times8 / 1000;
	if (rb_pixel_range < c_dist_times8) {
		range = (rb_pixel_range * rb_range) / 1000;
		c_dist_times8 = rb_pixel_range;
	} else if (8 < c_dist_times8) {
		c_dist_times8 *= 8;
		range *= 8;
	} else if (4 < c_dist_times8) {
		c_dist_times8 *= 4;
		range *= 4;
	} else if (2 < c_dist_times8) {
		c_dist_times8 *= 2;
		range *= 2;
	}

	int offset = (x + y * buffer_line) * 3;
	if (2 <= c_dist_times8 && 2 <= range) {
		int sum_y = 0;
		int sum_cb = 0;
		int sum_cr = 0;
		for (int i = 0; i < range; i++) {
			int x_itr = x + i * cx / c_dist_times8;
			int y_itr = y + i * cy / c_dist_times8;
			int pix_offset = (x_itr + y_itr * buffer_line) * 3;
			sum_y += src[pix_offset];
			sum_cb += src[++pix_offset];
			sum_cr += src[++pix_offset];
		}

		dst[offset] = (short)(sum_y / range);
		dst[++offset] = (short)(sum_cb / range);
		dst[++offset] = (short)(sum_cr / range);
	} else {
		dst[offset] = src[offset];
		dst[offset + 1] = src[offset + 1];
		dst[offset + 2] = src[offset + 2];
	}
}
kernel void RadiationalBlur_Filter_Far(
	global short* dst, global short* src, int scene_w, int scene_h, int buffer_line,
	int rb_blur_cx, int rb_blur_cy, int rb_range, int rb_pixel_range) {
	int x = get_global_id(0);
	int y = get_global_id(1);

	int cx = rb_blur_cx - x;
	int cy = rb_blur_cy - y;
	int c_dist_times8 = (int)round(sqrt((float)(cx * cx + cy * cy)) * 8.0f);
	int range = rb_range * c_dist_times8 / 1000;
	if (rb_pixel_range < c_dist_times8) {
		range = (rb_pixel_range * rb_range) / 1000;
		c_dist_times8 = rb_pixel_range;
	} else if (8 < c_dist_times8) {
		c_dist_times8 *= 8;
		range *= 8;
	} else if (4 < c_dist_times8) {
		c_dist_times8 *= 4;
		range *= 4;
	} else if (2 < c_dist_times8) {
		c_dist_times8 *= 2;
		range *= 2;
	}

	int offset = (x + y * buffer_line) * 3;
	if (2 <= c_dist_times8 && 2 <= range) {
		int sum_y = 0;
		int sum_cb = 0;
		int sum_cr = 0;
		for (int i = 0; i < range; i++) {
			int x_itr = x + i * cx / c_dist_times8;
			int y_itr = y + i * cy / c_dist_times8;
			if (0 <= x_itr && 0 <= y_itr && x_itr < scene_w && y_itr < scene_h) {
				int pix_offset = (x_itr + y_itr * buffer_line) * 3;
				sum_y += src[pix_offset];
				sum_cb += src[++pix_offset];
				sum_cr += src[++pix_offset];
			}
		}

		dst[offset] = (short)(sum_y / range);
		dst[++offset] = (short)(sum_cb / range);
		dst[++offset] = (short)(sum_cr / range);
	} else {
		dst[offset] = src[offset];
		dst[offset + 1] = src[offset + 1];
		dst[offset + 2] = src[offset + 2];
	}
}
)" R"(
kernel void Flash(global short* dst, global short* src, int src_w, int src_h, int exedit_buffer_line,
	int g_cx,
	int g_cy,
	int g_range,
	int g_pixel_range,
	int g_temp_x,
	int g_temp_y,
	int g_r_intensity
) {

	int xi = get_global_id(0);
	int yi = get_global_id(1);

	int x = xi + g_temp_x;
	int y = yi + g_temp_y;

	int pixel_itr = xi + yi * exedit_buffer_line;

	int cx = g_cx - x;
	int cy = g_cy - y;
	int c_dist_times8 = (int)round(sqrt((float)(cx * cx + cy * cy)) * 8.0f);
	int range = g_range * c_dist_times8 / 1000;

	if (g_pixel_range < c_dist_times8) {
		range = g_pixel_range * g_range / 1000;
		c_dist_times8 = g_pixel_range;
	} else if (8 < c_dist_times8) {
		c_dist_times8 *= 8;
		range *= 8;
	} else if (4 < c_dist_times8) {
		c_dist_times8 *= 4;
		range *= 4;
	} else if (2 < c_dist_times8) {
		c_dist_times8 *= 2;
		range *= 2;
	}

	int sum_y, sum_cb, sum_cr;

	if (2 <= c_dist_times8 && 2 <= range) {
		sum_y = sum_cb = sum_cr = 0;
		for (int i = 0; i < range; i++) {
			int x_itr = x + i * cx / c_dist_times8;
			int y_itr = y + i * cy / c_dist_times8;

			if (0 <= x_itr && 0 <= y_itr && x_itr < src_w && y_itr < src_h) {
				short4 itr = vload4(x_itr + y_itr * exedit_buffer_line, src);
				if (itr.w != 0) {
					if (itr.w < 4096) {
						sum_y += itr.x * itr.w / 4096;
						sum_cb += itr.y * itr.w / 4096;
						sum_cr += itr.z * itr.w / 4096;
					} else {
						sum_y += itr.x;
						sum_cb += itr.y;
						sum_cr += itr.z;
					}
				}
			}
		}
		sum_y /= range;
		sum_cb /= range;
		sum_cr /= range;
	} else {
		if (x < 0 || y < 0 || src_w <= x || src_h <= y) {
			vstore4((short4)(0, 0, 0, 0), pixel_itr, dst);
			return;
		} else {
			short4 itr = vload4(x + y * exedit_buffer_line, src);
			sum_y = itr.x * itr.w / 4096;
			sum_cb = itr.y * itr.w / 4096;
			sum_cr = itr.z * itr.w / 4096;
		}
	}

	int ya = sum_y - g_r_intensity;
	if (ya < 1) {
		vstore4((short4)(0, 0, 0, 0), pixel_itr, dst);
	} else {
		sum_cb -= g_r_intensity * sum_cb / sum_y;
		sum_cr -= g_r_intensity * sum_cr / sum_y;
		if (ya < 4096) {
			vstore4(
				(short4)(
					4096,
					sum_cb * 4096 / ya,
					sum_cr * 4096 / ya,
					ya
					),
				pixel_itr, dst
			);
		} else {
			vstore4(
				(short4)(
					ya,
					sum_cb,
					sum_cr,
					4096
					),
				pixel_itr, dst
			);
		}
	}
}
)" R"(
kernel void FlashColor(global short* dst, global short* src, int src_w, int src_h, int exedit_buffer_line,
	int g_cx,
	int g_cy,
	int g_range,
	int g_pixel_range,
	int g_temp_x,
	int g_temp_y,
	int g_r_intensity,
	short g_color_y,
	short g_color_cb,
	short g_color_cr
) {

	int xi = get_global_id(0);
	int yi = get_global_id(1);

	int x = xi + g_temp_x;
	int y = yi + g_temp_y;

	int pixel_itr = xi + yi * exedit_buffer_line;

	int cx = g_cx - x;
	int cy = g_cy - y;
	int c_dist_times8 = (int)round(sqrt((float)(cx * cx + cy * cy)) * 8.0f);
	int range = g_range * c_dist_times8 / 1000;
	if (g_pixel_range < c_dist_times8) {
		range = g_pixel_range * g_range / 1000;
		c_dist_times8 = g_pixel_range;
	} else if (8 < c_dist_times8) {
		c_dist_times8 *= 8;
		range *= 8;
	} else if (4 < c_dist_times8) {
		c_dist_times8 *= 4;
		range *= 4;
	} else if (2 < c_dist_times8) {
		c_dist_times8 *= 2;
		range *= 2;
	}
	int itr_y, itr_cb, itr_cr;

	if (2 <= c_dist_times8 && 2 <= range) {
		int sum_a = 0;
		for (int i = 0; i < range; i++) {
			int x_itr = x + i * cx / c_dist_times8;
			int y_itr = y + i * cy / c_dist_times8;

			if (0 <= x_itr && 0 <= y_itr && x_itr < src_w && y_itr < src_h) {
				short4 itr = vload4(x_itr + y_itr * exedit_buffer_line, src);
				int itr_a = itr.w;
				if (itr_a != 0) {
					if (itr_a < 4096) {
						sum_a += itr_a;
					} else {
						sum_a += 4096;
					}
				}
			}
		}
		sum_a /= range;
		itr_y = g_color_y * sum_a / 4096;
		itr_cb = g_color_cb * sum_a / 4096;
		itr_cr = g_color_cr * sum_a / 4096;
	} else {
		if (x < 0 || y < 0 || src_w <= x || src_h <= y) {
			vstore4((short4)(0, 0, 0, 0), pixel_itr, dst);
			return;
		} else {
			short4 itr = vload4(x + y * exedit_buffer_line, src);
			int itr_a = itr.w;
			itr_y = g_color_y * itr_a / 4096;
			itr_cb = g_color_cb * itr_a / 4096;
			itr_cr = g_color_cr * itr_a / 4096;
		}
	}

	int ya = itr_y - g_r_intensity;
	if (ya < 1) {
		vstore4((short4)(0, 0, 0, 0), pixel_itr, dst);
	} else {
		itr_cb -= g_r_intensity * itr_cb / itr_y;
		itr_cr -= g_r_intensity * itr_cr / itr_y;
		if (ya < 4096) {
			vstore4(
				(short4)(
					4096,
					itr_cb * 4096 / ya,
					itr_cr * 4096 / ya,
					ya
					),
				pixel_itr, dst
			);
		} else {
			vstore4(
				(short4)(
					ya,
					itr_cb,
					itr_cr,
					4096
					),
				pixel_itr, dst
			);
		}
	}
}
)" R"(
kernel void DirectionalBlur_Media(global short* dst, global short* src, int obj_w, int obj_h, int obj_line,
	int x_begin, int x_end, int x_step, int y_begin, int y_end, int y_step, int range) {
	int x = get_global_id(0);
	int y = get_global_id(1);
	int pix_range = range * 2 + 1;

	dst += (x + y * obj_line) * 4;

	uint sum_y = 0;
	int sum_cb = 0;
	int sum_cr = 0;
	int sum_a = 0;

	int x_itr = (x + x_begin << 16) + 0x8000 - range * x_step;
	int y_itr = (y + y_begin << 16) + 0x8000 - range * y_step;

	for (int n = 0; n < pix_range; n++) {
		int xx = x_itr >> 16;
		int yy = y_itr >> 16;
		if (0 <= xx && xx < obj_w && 0 <= yy && yy < obj_h) {
			int offset = (xx + yy * obj_line) * 4;
			int src_a = min((int)src[offset+3], 0x1000);
			sum_y += src[offset] * src_a >> 12;
			sum_cb += src[offset+1] * src_a >> 12;
			sum_cr += src[offset+2] * src_a >> 12;
			sum_a += src_a;
		}
		x_itr += x_step;
		y_itr += y_step;
	}
	if (sum_a) {
		dst[0] = (short)(sum_y * 4096 / sum_a);
		dst[1] = (short)(sum_cb * 4096 / sum_a);
		dst[2] = (short)(sum_cr * 4096 / sum_a);
	} else {
		dst[0] = dst[1] = dst[2] = 0;
	}
	dst[3] = (short)(sum_a / pix_range);
}
)" R"(
kernel void DirectionalBlur_original_size(global short* dst, global short* src, int obj_w, int obj_h, int obj_line,
	int x_step, int y_step, int range) {
	int x = get_global_id(0);
	int y = get_global_id(1);
	int pix_range = range * 2 + 1;

	dst += (x + y * obj_line) * 4;

	int x_itr = (x << 16) + 0x8000 - range * x_step;
	int y_itr = (y << 16) + 0x8000 - range * y_step;

	uint sum_y = 0;
	int sum_cb = 0;
	int sum_cr = 0;
	int sum_a = 0;
	int cnt = 0;

	for (int n = 0; n < pix_range; n++) {
		int xx = x_itr >> 16;
		int yy = y_itr >> 16;
		if (0 <= xx && xx < obj_w && 0 <= yy && yy < obj_h) {
			int offset = (xx + yy * obj_line) * 4;
			int src_a = min((int)src[offset+3], 0x1000);
			sum_y += src[offset] * src_a >> 12;
			sum_cb += src[offset+1] * src_a >> 12;
			sum_cr += src[offset+2] * src_a >> 12;
			sum_a += src_a;
			cnt++;
		}
		x_itr += x_step;
		y_itr += y_step;
	}
	if(cnt == 0) cnt = 0xffffff;
	if (sum_a) {
		dst[0] = (short)(sum_y * 4096 / sum_a);
		dst[1] = (short)(sum_cb * 4096 / sum_a);
		dst[2] = (short)(sum_cr * 4096 / sum_a);
	} else {
		dst[0] = dst[1] = dst[2] = 0;
	}
	dst[3] = (short)(sum_a / cnt);
}
)" R"(
kernel void DirectionalBlur_Filter(global short* dst, global short* src, int scene_w, int scene_h, int scene_line,
	int x_step, int y_step, int range) {
	int x = get_global_id(0);
	int y = get_global_id(1);
	int pix_range = range * 2 + 1;

	dst += (x + y * scene_line) * 3;

	int x_itr = (x << 16) + 0x8000 - range * x_step;
	int y_itr = (y << 16) + 0x8000 - range * y_step;

	int sum_y = 0;
	int sum_cb = 0;
	int sum_cr = 0;
	int cnt = 0;
	for (int n = 0; n < pix_range; n++) {
		int xx = x_itr >> 16;
		int yy = y_itr >> 16;
		if (0 <= xx && xx < scene_w && 0 <= yy && yy < scene_h) {
			int offset = (xx + yy * scene_line) * 3;
			sum_y += src[offset];
			sum_cb += src[offset+1];
			sum_cr += src[offset+2];
			cnt++;
		}
		x_itr += x_step;
		y_itr += y_step;
	}
	if(cnt == 0) cnt = 0xffffff;
	dst[0] = (short)(sum_y / cnt);
	dst[1] = (short)(sum_cb / cnt);
	dst[2] = (short)(sum_cr / cnt);
}
kernel void LensBlur_Media(global char* dst, global char* src, int obj_w, int obj_h, int obj_line,
	int range, int rangep05_sqr, int range_t3m1, int rangem1_sqr) {

	int x = get_global_id(0);
	int y = get_global_id(1);

	int top = -min(y, range);
	int bottom = min(obj_h - y - 1, range);
	int left = -min(x, range);
	int right = min(obj_w - x - 1, range);

	float sum_y = 0.0f;
	int sum_cb = 0;
	int sum_cr = 0;
	int sum_a = 0;

	int cor_sum = 0;

	int offset = (x + left + (y + top) * obj_line) * 8;

	for (int yy = top; yy <= bottom; yy++) {
		int sqr = yy * yy + left * left;
		int offset2 = offset;
		for (int xx = left; xx <= right; xx++) {
			if (sqr < rangep05_sqr) {
				int cor_a;
				if (rangem1_sqr < sqr) {
					cor_a = (rangep05_sqr - sqr << 12) / range_t3m1;
				} else {
					cor_a = 4096;
				}
				cor_sum += cor_a;
				sum_y += *(float*)&src[offset2] * (float)cor_a;
				sum_cb += src[offset2 + 4] * cor_a;
				sum_cr += src[offset2 + 5] * cor_a;
				sum_a += *(short*)&src[offset2 + 6] * cor_a >> 12;
			}
			sqr += 1 + xx * 2;
			offset2 += 8;
		}
		offset += obj_line * 8;
	}

	dst += (x + y * obj_line) * 8;
	if (0 < sum_a) {
		*(float*)dst = sum_y / (float)sum_a;
		dst[4] = (char)(((sum_a >> 1) + sum_cb) / sum_a);
		dst[5] = (char)(((sum_a >> 1) + sum_cr) / sum_a);
		*(short*)&dst[6] = (short)round((float)sum_a * (4096.0f / (float)cor_sum));
	} else {
		*(int*)dst = 0;
		*(int*)&dst[4] = 0;
	}
}
)" R"(
kernel void LensBlur_Filter(global char* dst, global char* src, int scene_w, int scene_h, int scene_line,
	int range, int rangep05_sqr, int range_t3m1, int rangem1_sqr) {

	int x = get_global_id(0);
	int y = get_global_id(1);

	int top = -min(y, range);
	int bottom = min(scene_h - y - 1, range);
	int left = -min(x, range);
	int right = min(scene_w - x - 1, range);

	short tofloat[2];
	float sum_y = 0.0f;
	int sum_cb = 0;
	int sum_cr = 0;
	int sum_a = 0;

	int offset = (x + left + (y + top) * scene_line) * 6;

	for (int yy = top; yy <= bottom; yy++) {

		int sqr = yy * yy + left * left;
		int offset2 = offset;

		for (int xx = left; xx <= right; xx++) {
			if (sqr < rangep05_sqr) {
				int cor_a;
				if (rangem1_sqr < sqr) {
					cor_a = (rangep05_sqr - sqr << 12) / range_t3m1;
				} else {
					cor_a = 4096;
				}
				tofloat[0] = *(short*)&src[offset2];
				tofloat[1] = *(short*)&src[offset2 + 2];
				sum_y += *(float*)tofloat * (float)cor_a;
				sum_cb += src[offset2 + 4] * cor_a;
				sum_cr += src[offset2 + 5] * cor_a;
				sum_a += cor_a;
			}
			sqr += 1 + xx * 2;
			offset2 += 6;
		}
		offset += scene_line * 6;
	}

	dst += (x + y * scene_line) * 6;
	*(float*)tofloat = sum_y / (float)sum_a;
	*(short*)&dst[0] = tofloat[0];
	*(short*)&dst[2] = tofloat[1];
	dst[4] = (char)(((sum_a >> 1) + sum_cb) / sum_a);
	dst[5] = (char)(((sum_a >> 1) + sum_cr) / sum_a);
}

)");
#pragma endregion

	template<size_t i, class Head>
	static void KernelSetArg(cl::Kernel& kernel, Head head) {
		kernel.setArg(i, head);
	}

	template<size_t i, class Head, class... Tail>
	static void KernelSetArg(cl::Kernel& kernel, Head head, Tail... tail) {
		kernel.setArg(i, head);
		KernelSetArg<i + 1>(kernel, tail...);
	}

	bool enabled = true;
	bool enabled_i;
	inline static const char key[] = "fast.cl";

public:
	cl::Platform platform;
	std::vector<cl::Device> devices;
	cl::Context context;

	std::byte program_mem[sizeof(cl::Program)];
	bool program_opt;
	cl::CommandQueue queue;

	HMODULE CLLib;

	enum class State {
		NotYet,
		OK,
		Failed
	} state;

	cl_t() :state(State::NotYet), CLLib(NULL) {}
	~cl_t() {
		FreeLibrary(CLLib);
		if (program_opt) {
			auto program = reinterpret_cast<cl::Program*>(program_mem);
			program->~Program();
		}
	}

	bool init() {
		enabled_i = enabled;

		if (!enabled_i)return true;

		if (![]() {
			__try {
				auto load_ret = __HrLoadAllImportsForDll("OpenCL.dll");
					if (FAILED(load_ret)) {
						[load_ret]() {
							debug_log("OpenCL not available {}", "delay load failed {}"_fmt(load_ret));
						}();
						return false;
					}
				return true;
			}
			__except ([](int code) {
				if (
					code == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
					code == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND)
				) {
					return EXCEPTION_EXECUTE_HANDLER;
				}
				return EXCEPTION_CONTINUE_SEARCH;
			} (GetExceptionCode())) {
				debug_log("OpenCL not available {}\n", "delay load exception");
				return false;
			}
		}()) {
			state = State::Failed;
			return false;
		}


		switch (state) {
		case State::NotYet:
			try {
				cl::Platform::get(&platform);
				platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
				context = cl::Context(devices);
				new (&program_mem[0]) cl::Program(context, program_str.get(), true);
				program_opt = true;
				//program.build(devices);
				program_str.re_encrypt();

				struct DeviceInfo {
					cl::Device device;
					cl_device_type device_type;
					//std::string version;

					constexpr bool operator<(const DeviceInfo& rhs) {
						//if (auto cmpver = version <=> rhs.version; cmpver != 0) return cmpver < 0;
						return this->device_type < rhs.device_type;
					}
				};

				std::vector<DeviceInfo> device_info;
				for (const auto& d : devices) {
					device_info.emplace_back(
						d,
						d.getInfo<CL_DEVICE_TYPE>()
					);
				}
				std::sort(device_info.rbegin(), device_info.rend());
				auto device_itr = devices.begin();
				for (const auto& di : device_info) {
					*device_itr = di.device;
					device_itr++;
				}

				queue = cl::CommandQueue(context, devices[0]);
			}
			catch (const cl::Error& err) {
				program_str.re_encrypt();

				if (err.err() == CL_BUILD_PROGRAM_FAILURE) {
					try {
						if (program_opt) {
							auto program = reinterpret_cast<cl::Program*>(program_mem);
							if (auto status = program->getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0]); status == CL_BUILD_ERROR) {
								debug_log(
									"OpenCL Error (CL_BUILD_PROGRAM_FAILURE : CL_BUILD_ERROR)\n{}",
									program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]).c_str()
								);
							}
							else {
								debug_log("OpenCL Error (CL_BUILD_PROGRAM_FAILURE)\nstatus: {}", status);
							}
							program_opt = false;
							program->~Program();
							state = State::Failed;
							return false;
						}
					}
					catch (const cl::Error& err) {
						debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
						state = State::Failed;
						return false;
					}
				}
				debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
				state = State::Failed;
				return false;
			}
			state = State::OK;
			[[fallthrough]];
		case State::OK:
			return true;
		default:
			return false;
		}
	}

	template<class... Args>
	cl::Kernel readyKernel(std::string_view name, Args&&... args) {
		auto program = reinterpret_cast<cl::Program*>(program_mem);
		cl::Kernel kernel(*program, name.data());
		KernelSetArg<0>(kernel, args...);
		return kernel;
	}

	void switching(bool flag) {
		enabled = flag;
	}

	bool is_enabled() { return enabled; }
	bool is_enabled_i() { return enabled_i; }
	
	void switch_load(ConfigReader& cr) {
		cr.regist(key, [this](json_value_s* value) {
			ConfigReader::load_variable(value, enabled);
		});
	}

	void switch_store(ConfigWriter& cw) {
		cw.append(key, enabled);
	}
} cl;

} // namespace patch::fast

#endif
