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

#include "patch_blend.hpp"

#ifdef PATCH_SWITCH_BLEND
namespace patch {

	void __cdecl blend_t::blend_yca_add(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		src_y  += (dst->y  * dst->a) >> 12;
		src_cb += (dst->cb * dst->a) >> 12;
		src_cr += (dst->cr * dst->a) >> 12;
		if (0x2000 < src_y) {
			if (src_y < 0x3000) {
				src_cb = ((0x3000 - src_y) * src_cb) >> 12;
				src_cr = ((0x3000 - src_y) * src_cr) >> 12;
			} else {
				src_cb = src_cr = 0;
			}
			src_y = 0x2000;
		}
		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}


	void __cdecl blend_t::blend_yca_sub(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		src_y  = ((dst->y  * dst->a) >> 12) - src_y;
		src_cb = ((dst->cb * dst->a) >> 12) - src_cb;
		src_cr = ((dst->cr * dst->a) >> 12) - src_cr;
		if (src_y < 0) {
			if (src_y <= -0x400) {
				src_cb = src_cr = 0;
			} else {
				src_cb = ((src_y + 0x400) * src_cb) >> 10;
				src_cr = ((src_y + 0x400) * src_cr) >> 10;
			}
			src_y = 0;
		}
		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}

	void __cdecl blend_t::blend_yca_mul(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		int fix_y  = (dst->y  * dst->a) >> 12;
		int fix_cb = (dst->cb * dst->a) >> 12;
		int fix_cr = (dst->cr * dst->a) >> 12;
		int src_t = (std::max)(0, ((src_cr * 0x59ba) >> 14) + src_y);
		int dst_t = (std::max)(0, ((fix_cr * 0x59ba) >> 14) + fix_y);
		int r = (std::min)((src_t * dst_t) >> 12, 0x2000);

		src_t = (std::max)(0, ((src_cb * -0x1604 + src_cr * -0x2db2) >> 14) + src_y);
		dst_t = (std::max)(0, ((fix_cb * -0x1604 + fix_cr * -0x2db2) >> 14) + fix_y);
		int g = (std::min)((src_t * dst_t) >> 12, 0x2000);

		src_t = (std::max)(0, ((src_cb * 0x7168) >> 14) + src_y);
		dst_t = (std::max)(0, ((fix_cb * 0x7168) >> 14) + fix_y);
		int b = (std::min)((src_t * dst_t) >> 12, 0x2000);

		src_y  = (r * 0x1322 + g *  0x2591 + b * 0x74b ) >> 14;
		src_cb = (r * -0xad0 + g * -0x152f + b * 0x2000) >> 14;
		src_cr = (r * 0x2000 + g * -0x1ad0 + b * -0x52f) >> 14;

		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}

	void __cdecl blend_t::blend_yca_screen(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		int fix_y  = (dst->y  * dst->a) >> 12;
		int fix_cb = (dst->cb * dst->a) >> 12;
		int fix_cr = (dst->cr * dst->a) >> 12;
		int src_t = (std::max)(0, ((src_cr * 0x59ba) >> 14) + src_y);
		int dst_t = (std::max)(0, ((fix_cr * 0x59ba) >> 14) + fix_y);
		int r = dst_t - ((dst_t * src_t) >> 12) + src_t;

		src_t = (std::max)(0, ((src_cb * -0x1604 + src_cr * -0x2db2) >> 14) + src_y);
		dst_t = (std::max)(0, ((fix_cb * -0x1604 + fix_cr * -0x2db2) >> 14) + fix_y);
		int g = dst_t - ((dst_t * src_t) >> 12) + src_t;

		src_t = (std::max)(0, ((src_cb * 0x7168) >> 14) + src_y);
		dst_t = (std::max)(0, ((fix_cb * 0x7168) >> 14) + fix_y);
		int b = dst_t - ((dst_t * src_t) >> 12) + src_t;

		src_y  = (r * 0x1322 + g *  0x2591 + b * 0x74b ) >> 14;
		src_cb = (r * -0xad0 + g * -0x152f + b * 0x2000) >> 14;
		src_cr = (r * 0x2000 + g * -0x1ad0 + b * -0x52f) >> 14;

		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}

	void __cdecl blend_t::blend_yca_overlay(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		int fix_y  = (dst->y  * dst->a) >> 12;
		int fix_cb = (dst->cb * dst->a) >> 12;
		int fix_cr = (dst->cr * dst->a) >> 12;
		int src_t = (std::max)(0, ((src_cr * 0x59ba) >> 14) + src_y);
		int dst_t = (std::max)(0, ((fix_cr * 0x59ba) >> 14) + fix_y);
		int r = (dst_t * src_t) >> 11;
		if (0x800 <= dst_t) {
			r = (dst_t + src_t) * 2 - 0x1000 - r;
		}

		src_t = (std::max)(0, ((src_cb * -0x1604 + src_cr * -0x2db2) >> 14) + src_y);
		dst_t = (std::max)(0, ((fix_cb * -0x1604 + fix_cr * -0x2db2) >> 14) + fix_y);
		int g = (dst_t * src_t) >> 11;
		if (0x800 <= dst_t) {
			g = (dst_t + src_t) * 2 - 0x1000 - g;
		}

		src_t = (std::max)(0, ((src_cb * 0x7168) >> 14) + src_y);
		dst_t = (std::max)(0, ((fix_cb * 0x7168) >> 14) + fix_y);
		int b = (dst_t * src_t) >> 11;
		if (0x800 <= dst_t) {
			b = (dst_t + src_t) * 2 - 0x1000 - b;
		}

		src_y  = (r * 0x1322 + g *  0x2591 + b * 0x74b ) >> 14;
		src_cb = (r * -0xad0 + g * -0x152f + b * 0x2000) >> 14;
		src_cr = (r * 0x2000 + g * -0x1ad0 + b * -0x52f) >> 14;

		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}

	void __cdecl blend_t::blend_yca_cmpmax(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		int fix_y  = (dst->y  * dst->a) >> 12;
		int fix_cb = (dst->cb * dst->a) >> 12;
		int fix_cr = (dst->cr * dst->a) >> 12;
		int r = (std::max)(((src_cr * 0x59ba) >> 14) + src_y, ((fix_cr * 0x59ba) >> 14) + fix_y);
		int g = (std::max)(((src_cb * -0x1604 + src_cr * -0x2db2) >> 14) + src_y, ((fix_cb * -0x1604 + fix_cr * -0x2db2) >> 14) + fix_y);
		int b = (std::max)(((src_cb * 0x7168) >> 14) + src_y, ((fix_cb * 0x7168) >> 14) + fix_y);

		src_y  = (r * 0x1322 + g *  0x2591 + b * 0x74b ) >> 14;
		src_cb = (r * -0xad0 + g * -0x152f + b * 0x2000) >> 14;
		src_cr = (r * 0x2000 + g * -0x1ad0 + b * -0x52f) >> 14;

		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}

	void __cdecl blend_t::blend_yca_cmpmin(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		int fix_y  = (dst->y  * dst->a) >> 12;
		int fix_cb = (dst->cb * dst->a) >> 12;
		int fix_cr = (dst->cr * dst->a) >> 12;
		int r = (std::min)(((src_cr * 0x59ba) >> 14) + src_y, ((fix_cr * 0x59ba) >> 14) + fix_y);
		int g = (std::min)(((src_cb * -0x1604 + src_cr * -0x2db2) >> 14) + src_y, ((fix_cb * -0x1604 + fix_cr * -0x2db2) >> 14) + fix_y);
		int b = (std::min)(((src_cb * 0x7168) >> 14) + src_y, ((fix_cb * 0x7168) >> 14) + fix_y);

		src_y  = (r * 0x1322 + g *  0x2591 + b * 0x74b ) >> 14;
		src_cb = (r * -0xad0 + g * -0x152f + b * 0x2000) >> 14;
		src_cr = (r * 0x2000 + g * -0x1ad0 + b * -0x52f) >> 14;

		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}


	void __cdecl blend_t::blend_yca_luminance(ExEdit::PixelYCA* dst, short src_y, short src_cb, short src_cr, short src_a) {
		if (0x1000 <= src_a) {
			dst->y  = src_y;
			dst->cb = (dst->cb * dst->a) >> 12;
			dst->cr = (dst->cr * dst->a) >> 12;
			dst->a  = 0x1000;
		} else if (0x1000 <= dst->a) {
			dst->y += ((src_y - dst->y) * src_a) >> 12;
			dst->a = 0x1000;
		} else if (dst->a <= 0) {
			dst->y  = src_y;
			dst->cb = 0;
			dst->cr = 0;
			dst->a  = src_a;
		} else {
			short new_a = (0x1000800 - (0x1000 - dst->a) * (0x1000 - src_a)) >> 12;
			dst->y  = (((dst->y * dst->a) >> 12) * (0x1000 - src_a) + src_y * src_a) / new_a;
			dst->cb = dst->cb * dst->a / new_a;
			dst->cr = dst->cr * dst->a / new_a;
			dst->a  = new_a;
		}
	}

	void __cdecl blend_t::blend_yca_colordiff(ExEdit::PixelYCA* dst, short src_y, short src_cb, short src_cr, short src_a) {
		if (0x1000 <= src_a) {
			dst->y  = (dst->y * dst->a) >> 12;
			dst->cb = src_cb;
			dst->cr = src_cr;
			dst->a  = 0x1000;
		} else if (0x1000 <= dst->a) {
			dst->cb += ((src_cb - dst->cb) * src_a) >> 12;
			dst->cr += ((src_cr - dst->cr) * src_a) >> 12;
			dst->a  = 0x1000;
		} else if (dst->a <= 0) {
			dst->y  = 0;
			dst->cb = src_cb;
			dst->cr = src_cr;
			dst->a  = src_a;
		} else {
			int new_a = (0x1000800 - (0x1000 - dst->a) * (0x1000 - src_a)) >> 12;
			int new_dst_a = (0x1000 - src_a) * dst->a / new_a;
			int new_src_a = (src_a << 12) / new_a;
			dst->y  = dst->y * dst->a / new_a;
			dst->cb = (dst->cb * new_dst_a + src_cb * new_src_a) >> 12;
			dst->cr = (dst->cr * new_dst_a + src_cr * new_src_a) >> 12;
			dst->a = new_a;
		}
	}

	void __cdecl blend_t::blend_yca_shadow(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		int fix_y  = (dst->y  * dst->a) >> 12;
		int fix_cb = (dst->cb * dst->a) >> 12;
		int fix_cr = (dst->cr * dst->a) >> 12;
		int src_t = (std::max)(0, ((src_cr * 0x59ba) >> 14) + src_y);
		int dst_t = (std::max)(0, ((fix_cr * 0x59ba) >> 14) + fix_y);
		int r = (std::max)(0, dst_t + src_t - 0x1000);

		src_t = (std::max)(0, ((src_cb * -0x1604 + src_cr * -0x2db2) >> 14) + src_y);
		dst_t = (std::max)(0, ((fix_cb * -0x1604 + fix_cr * -0x2db2) >> 14) + fix_y);
		int g = (std::max)(0, dst_t + src_t - 0x1000);

		src_t = (std::max)(0, ((src_cb * 0x7168) >> 14) + src_y);
		dst_t = (std::max)(0, ((fix_cb * 0x7168) >> 14) + fix_y);
		int b = (std::max)(0, dst_t + src_t - 0x1000);

		src_y  = (r * 0x1322 + g *  0x2591 + b * 0x74b ) >> 14;
		src_cb = (r * -0xad0 + g * -0x152f + b * 0x2000) >> 14;
		src_cr = (r * 0x2000 + g * -0x1ad0 + b * -0x52f) >> 14;

		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}

	void __cdecl blend_t::blend_yca_lightdark(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		int fix_y  = (dst->y  * dst->a) >> 12;
		int fix_cb = (dst->cb * dst->a) >> 12;
		int fix_cr = (dst->cr * dst->a) >> 12;
		int src_t = ((src_cr * 0x59ba) >> 14) + src_y;
		int dst_t = ((fix_cr * 0x59ba) >> 14) + fix_y;
		int r = std::clamp(dst_t + src_t * 2 - 0x1000, 0, 0x2000);

		src_t = ((src_cb * -0x1604 + src_cr * -0x2db2) >> 14) + src_y;
		dst_t = ((fix_cb * -0x1604 + fix_cr * -0x2db2) >> 14) + fix_y;
		int g = std::clamp(dst_t + src_t * 2 - 0x1000, 0, 0x2000);

		src_t = ((src_cb * 0x7168) >> 14) + src_y;
		dst_t = ((fix_cb * 0x7168) >> 14) + fix_y;
		int b = std::clamp(dst_t + src_t * 2 - 0x1000, 0, 0x2000);

		src_y  = (r * 0x1322 + g *  0x2591 + b * 0x74b ) >> 14;
		src_cb = (r * -0xad0 + g * -0x152f + b * 0x2000) >> 14;
		src_cr = (r * 0x2000 + g * -0x1ad0 + b * -0x52f) >> 14;

		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}

	void __cdecl blend_t::blend_yca_difference(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a) {
		int sub_y  = src_y  - ((dst->y  * dst->a) >> 12);
		int sub_cb = src_cb - ((dst->cb * dst->a) >> 12);
		int sub_cr = src_cr - ((dst->cr * dst->a) >> 12);

		int r = abs(((sub_cr * 0x59ba) >> 14) + sub_y);
		int g = abs(((sub_cb * -0x1604 + sub_cr * -0x2db2) >> 14) + sub_y);
		int b = abs(((sub_cb * 0x7168) >> 14) + sub_y);

		src_y  = (r * 0x1322 + g *  0x2591 + b * 0x74b ) >> 14;
		src_cb = (r * -0xad0 + g * -0x152f + b * 0x2000) >> 14;
		src_cr = (r * 0x2000 + g * -0x1ad0 + b * -0x52f) >> 14;

		blend_yca_normal(dst, src_y, src_cb, src_cr, src_a);
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_BLEND