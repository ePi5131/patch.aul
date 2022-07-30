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

#include "patch_obj_colorcorrection.hpp"

#ifdef PATCH_SWITCH_OBJ_COLORCORRECTION
namespace patch {
    void rgb2hsv_12(int r, int g, int b, int* h, int* s, int* v) {
        int rgbmax = (std::max)({ r, g, b });
        int rgbsub = rgbmax - (std::min)({ r, g, b });
        if (rgbsub) {
            if (rgbmax == r) {
                *h = 600 * (g - b) / rgbsub;
            } else if (rgbmax == g) {
                *h = 600 * (b - r) / rgbsub + 1200;
            } else {
                *h = 600 * (r - g) / rgbsub + 2400;
            }
            if (rgbmax) {
                *s = 4096 * rgbsub / rgbmax;
            } else {
                *s = 0;
            }
        } else {
            *h = 0;
            *s = 0;
        }
        *v = rgbmax;
    }
    void yc2rgb_12(int y, int cb, int cr, int* r, int* g, int* b) {
        *r = y + (cr * 11485 >> 13);
        *g = y - (cb * 2818 + cr * 5849 >> 13);
        *b = y + (cb * 14516 >> 13);
    }
    void yc2hsv_12(int y, int cb, int cr, int* h, int* s, int* v) {
        int r, g, b;
        yc2rgb_12(y, cb, cr, &r, &g, &b);
        rgb2hsv_12(r, g, b, h, s, v);
    }


    void hsv2rgb_12(int h, int s, int v, int* r, int* g, int* b) {
        h %= 3600;
        if (h < 0) {
            h += 3600;
        }

        int tmp = v * s;
        int rgbmin = v - (tmp >> 12);

        switch (h / 600) {
        case 0:
            *r = v;
            *g = v - (tmp / 75 * (600 - h) >> 15);
            *b = rgbmin;
            break;
        case 1:
            *r = v - (tmp / 75 * (h - 600) >> 15);
            *g = v;
            *b = rgbmin;
            break;
        case 2:
            *r = rgbmin;
            *g = v;
            *b = v - (tmp / 75 * (1800 - h) >> 15);
            break;
        case 3:
            *r = rgbmin;
            *g = v - (tmp / 75 * (h - 1800) >> 15);
            *b = v;
            break;
        case 4:
            *r = v - (tmp / 75 * (3000 - h) >> 15);
            *g = rgbmin;
            *b = v;
            break;
        case 5:
            *r = v;
            *g = rgbmin;
            *b = v - (tmp / 75 * (h - 3000) >> 15);
            break;
        }
    }

    void rgb2yc_12(int r, int g, int b, int* y, int* cb, int* cr) {
        *y = r * 4898 + g * 9617 + b * 1867 >> 14;
        *cb = r * -2768 + g * -5423 + b * 8192 >> 14;
        *cr = r * 8192 + g * -6864 + b * -1327 >> 14;
    }
    void hsv2yc_12(int h, int s, int v, int* y, int* cb, int* cr) {
        int r, g, b;
        hsv2rgb_12(h, s, v, &r, &g, &b);
        rgb2yc_12(r, g, b, y, cb, cr);
    }


	void __cdecl obj_ColorCorrection_t::rot_hue(ExEdit::PixelYC* pix, int angle10, int luminance, int brightness, int saturation) {
		int h, s, v;
		yc2hsv_12(pix->y, pix->cb, pix->cr, &h, &s, &v);
		h += angle10;
		int y, cb, cr;
		hsv2yc_12(h, s, v, &y, &cb, &cr);

		pix->y = (y * luminance >> 8) + brightness;
		pix->cb = cb * saturation >> 8;
		pix->cr = cr * saturation >> 8;

	}

	void __cdecl obj_ColorCorrection_t::rot_hue_sat(ExEdit::PixelYC* pix, int angle10, int luminance, int brightness, int saturation) {
		rot_hue(pix, angle10, luminance, brightness, saturation);
		if (pix->y < 0) {
			int ysub = pix->y + 0x400;
			if (0 < ysub) {
				pix->cb = ysub * pix->cb >> 10;
				pix->cr = ysub * pix->cr >> 10;
			} else {
				pix->cb = pix->cr = 0;
			}
			pix->y = 0;
		} else if (0x2000 < pix->y) {
			int ysub = 0x3000 - pix->y;
			if (0 < ysub) {
				pix->cb = ysub * pix->cb >> 12;
				pix->cr = ysub * pix->cr >> 12;
			} else {
				pix->cb = pix->cr = 0;
			}
			pix->y = 0x2000;
		}
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_COLORCORRECTION
