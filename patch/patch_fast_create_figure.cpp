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

#include "patch_fast_create_figure.hpp"
#ifdef PATCH_SWITCH_FAST_CREATE_FIGURE


//#define PATCH_STOPWATCH

namespace patch::fast {

	void __cdecl CreateFigure_t::CreateFigure_circle(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 73d20
		auto& figure = *reinterpret_cast<CreateFigure_var*>(GLOBAL::exedit_base + OFS::ExEdit::CreateFigure_var_ptr);

        int begin_thread = thread_id * ((efpip->obj_h + 1) >> 1) / thread_num;
        int end_thread = (thread_id + 1) * ((efpip->obj_h + 1) >> 1) / thread_num;
        int obj_half_w = (efpip->obj_w + 1) >> 1;
        ExEdit::PixelYCA yca = { figure.color_y, figure.color_cb, figure.color_cr, 0 };

		int inner = 0;
		int sizesq = figure.size * figure.size;
		int size_t8 = sizesq * 8 / efpip->obj_w * figure.size / efpip->obj_h;

		if (figure.line_width) {
			inner = figure.size - figure.line_width * 2 - 1;
			if (inner < 0) {
				inner = 0;
			}
		}

		int yy = begin_thread * 2 - efpip->obj_h + 1;


		if (inner) {
			int inner_t8 = (((figure.size * inner * 8) / efpip->obj_w) * figure.size) / efpip->obj_h;
			int innersq = inner * inner;
			for (int y = begin_thread; y < end_thread; y++) {
				int yysq = yy * figure.size / efpip->obj_h;
				yysq *= yysq;
				int innersq_myyaq = yysq + inner_t8 - innersq;

				ExEdit::PixelYCA* pixlt = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y;
				ExEdit::PixelYCA* pixlb = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * (efpip->obj_h - 1 - y);
				ExEdit::PixelYCA* pixrt = pixlt + efpip->obj_w - 1;
				ExEdit::PixelYCA* pixrb = pixlb + efpip->obj_w - 1;
				int xx = 1 - efpip->obj_w;
				for (int x = 0; x < obj_half_w; x++) {
					int xxsq = xx * figure.size / efpip->obj_w;
					xxsq *= xxsq;
					int a = sizesq - yysq - xxsq;
					if (a <= 0) {
						a = 0;
					} else if (a < size_t8) {
						a = a * 0x1000 / size_t8;
					} else {
						a = 0x1000;
					}
					int a_inner = xxsq + innersq_myyaq;
					if (a_inner < 0) {
						a = 0;
					} else if (a_inner < inner_t8) {
						a = a * a_inner / inner_t8;
					}

					yca.a = (short)a;
					*pixlt = *pixrt = *pixlb = *pixrb = yca;
					pixlt++; pixrt--; pixlb++; pixrb--;
					xx += 2;
				}
				yy += 2;
			}
		} else {
			for (int y = begin_thread; y < end_thread; y++) {
				int yysq = yy * figure.size / efpip->obj_h;
				yysq *= yysq;
				ExEdit::PixelYCA* pixlt = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y;
				ExEdit::PixelYCA* pixlb = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * (efpip->obj_h - 1 - y);
				ExEdit::PixelYCA* pixrt = pixlt + efpip->obj_w - 1;
				ExEdit::PixelYCA* pixrb = pixlb + efpip->obj_w - 1;
				int xx = 1 - efpip->obj_w;
				int x;
				for (x = 0; x < obj_half_w; x++) {
					int xxsq = xx * figure.size / efpip->obj_w;
					xxsq *= xxsq;
					int a = sizesq - yysq - xxsq;
					if (a <= 0) {
						a = 0;
					} else if (a < size_t8) {
						a = a * 0x1000 / size_t8;
					} else {
						break;
					}
					yca.a = (short)a;
					*pixlt = *pixrt = *pixlb = *pixrb = yca;
					pixlt++; pixrt--; pixlb++; pixrb--;
					xx += 2;
				}
				yca.a = 0x1000;
				for (; x < obj_half_w; x++) {
					*pixlt = *pixrt = *pixlb = *pixrb = yca;
					pixlt++; pixrt--; pixlb++; pixrb--;
				}
				yy += 2;
			}
		}
	}


	void __cdecl CreateFigure_t::CreateFigure_polygons(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		auto& figure = *reinterpret_cast<CreateFigure_var*>(GLOBAL::exedit_base + OFS::ExEdit::CreateFigure_var_ptr);
		
		int xp[10];
		int xylen[10];
		int yp[10];

		int inner = 0;
		if (figure.type == 10) {
			double angle = 0.0;
			double sina0 = 0.0;
			double cosa0 = 1.0;
			for (int i = 0; i < 10; i += 2) {
				angle += 1.256637061435917;
				double sina1 = sin(angle);
				double cosa1 = cos(angle);
				double xx0 = sina1 * 65536.0;
				xp[i] = (int)xx0;
				double yy0 = cosa1 * 65536.0;
				yp[i] = (int)-yy0;
				double xx1 = sina0 * 65536.0;
				xp[i + 1] = (int)xx1;
				double yy1 = cosa0 * 65536.0;
				yp[i + 1] = (int)-yy1;
				xylen[i] = (int)((sina0 * xx0 + cosa0 * yy0) * (double)figure.size) + 0x20000;
				xylen[i + 1] = (int)((sina1 * xx1 + cosa1 * yy1) * (double)figure.size) + 0x20000;
				sina0 = sina1;
				cosa0 = cosa1;
			}
			if (figure.line_width) {
				inner = (int)((double)figure.line_width * 40642.48062015503876) + 0x20000;
			}
		} else {
			double angle_rate = 3.141592653589793 / (double)figure.type;
			double angle0 = 0.0;
			for (int i = 0; i < figure.type; i++) {
				double angle1 = angle_rate + angle0;
				double xx = sin(angle1) * 65536.0;
				xp[i] = (int)xx;
				double yy = cos(angle1) * 65536.0;
				yp[i] = (int)-yy;
				xylen[i] = (int)((sin(angle0) * xx + cos(angle0) * yy) * (double)figure.size) + 0x20000;
				angle0 = angle1 + angle_rate;
			}
			if (figure.line_width) {
				inner = (int)((double)(figure.line_width << 17) * cos(angle_rate)) + 0x20000;
			}
		}


        int thread_begin = thread_id * efpip->obj_h / thread_num;
        int thread_end = (thread_id + 1) * efpip->obj_h / thread_num;
        int obj_half_w = (efpip->obj_w + 1) >> 1;
        int a;
        ExEdit::PixelYCA yca = { figure.color_y, figure.color_cb, figure.color_cr, 0 };

		double angle_rate = (double)figure.type / 6.283185307179586;
		int yy = (thread_begin * 2 - efpip->obj_h) + 1;
		for (int y = thread_begin; y < thread_end; y++) {

			ExEdit::PixelYCA* pixl = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y;
			ExEdit::PixelYCA* pixr = pixl + efpip->obj_w - 1;

			int xx = 1 - efpip->obj_w;
			for (int x = 0; x < obj_half_w; x++) {
				int pt = (int)((atan2((double)-xx, (double)yy) + 3.141592653589793) * angle_rate) % figure.type;
				int dist = xylen[pt] - xp[pt] * xx - yp[pt] * yy;
				if (0x40000 <= dist) {
					a = 0x1000;
				} else if (dist <= 0) {
					a = 0;
				} else {
					a = dist >> 6;
				}

				if (inner) {
					int subinner = dist - inner;
					if (0 < subinner) {
						if (subinner < 0x40000) {
							a = a * (0x40000 - subinner) >> 18;
						} else {
							a = 0;
						}
					}
				}
				yca.a = (short)a;
				*pixl = *pixr = yca;
				pixl++; pixr--;
				xx += 2;
			}
			yy += 2;
		}
	}

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_CREATE_FIGURE
