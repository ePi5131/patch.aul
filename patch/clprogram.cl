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

kernel void PolorTransform(global short* dst, global short* src, int obj_w, int obj_h, int obj_line,
	int center_length, int radius, float angle, float uzu, float uzu_a){
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	int x_dist = x - radius;
	int y_dist = y - radius;
	float dist = sqrt((float)(x_dist * x_dist + y_dist * y_dist));

	int range = (int)round((float)obj_w / max(dist, 1.0f) * 57.6115417480468f + uzu_a);
				
	int yy_t256 = (int)round((float)(((obj_h + center_length) << 8) / radius) * dist);
	int yy_range_fr = 0x100 - (yy_t256 & 0xff);
	int yy_begin = (yy_t256 >> 8) - center_length;
				
	int xx_t256 = (int)round((((float)radius - dist) * uzu + angle - atan2((float)y_dist, (float)x_dist)) * (float)obj_w * 40.7436637878417f) - range / 2;
	int xx_range_fr = 0x100 - (xx_t256 & 0xff);
	int xx_begin = (xx_t256 >> 8) % obj_w;
				
	range = max(0x100,range);
	int yy = yy_begin;

	int sum_y = 0;
	int sum_cb = 0;
	int sum_cr = 0;
	int sum_a = 0;

	global short* pix;
	int src_a;

	if (0 <= yy && yy < obj_h) {
		int range_remain = range;
		int xx = xx_begin;
		if (xx_range_fr) {
			pix = src + (xx + yy * obj_line) * 4;
			sum_a = pix[3] * xx_range_fr * yy_range_fr >> 16;
			sum_y = pix[0] * sum_a >> 12;
			sum_cb = pix[1] * sum_a >> 12;
			sum_cr = pix[2] * sum_a >> 12;
			range_remain -= xx_range_fr;
			xx++;
			xx %= obj_w;
		}
		int pix_range = range_remain >> 8;
		for(int i=0;i<pix_range;i++){
			pix = src + (xx + yy * obj_line) * 4;
			src_a = pix[3] * yy_range_fr >> 8;
			sum_y += pix[0] * src_a >> 12;
			sum_cb += pix[1] * src_a >> 12;
			sum_cr += pix[2] * src_a >> 12;
			sum_a += src_a;
			xx++;
			xx %= obj_w;
		}
		range_remain &= 0xff;
		if (range_remain) {
			pix = src + (xx + yy * obj_line) * 4;
			src_a = pix[3] * range_remain * yy_range_fr >> 16;
			sum_y += pix[0] * src_a >> 12;
			sum_cb += pix[1] * src_a >> 12;
			sum_cr += pix[2] * src_a >> 12;
			sum_a += src_a;
		}
	}
	yy++;
	yy_range_fr = 0x100 - yy_range_fr;
	if (0 <= yy && yy < obj_h) {
		int range_remain = range;
		int xx = xx_begin;
		if (xx_range_fr != 0x100) {
			pix = src + (xx + yy * obj_line) * 4;
			src_a = pix[3] * xx_range_fr * yy_range_fr >> 16;
			sum_y += pix[0] * src_a >> 12;
			sum_cb += pix[1] * src_a >> 12;
			sum_cr += pix[2] * src_a >> 12;
			sum_a += src_a;
			range_remain -= xx_range_fr;
			xx++;
			xx %= obj_w;
		}
		int pix_range = range_remain >> 8;
		for(int i=0;i<pix_range;i++){
			pix = src + (xx + yy * obj_line) * 4;
			src_a = pix[3] * yy_range_fr >> 8;
			sum_y += pix[0] * src_a>> 12;
			sum_cb += pix[1] * src_a >> 12;
			sum_cr += pix[2] * src_a >> 12;
			sum_a += src_a;
			xx++;
			xx %= obj_w;
		}
		range_remain &= 0xff;
		if (range_remain) {
			pix = src + (xx + yy * obj_line) * 4;
			src_a = pix[3] * range_remain * yy_range_fr >> 16;
			sum_y += pix[0] * src_a >> 12;
			sum_cb += pix[1] * src_a >> 12;
			sum_cr += pix[2] * src_a >> 12;
			sum_a += src_a;
		}
	}
	dst += (x + y * obj_line) * 4;
	if (sum_a) {
		float a_float = 4096.0f / (float)sum_a;
		dst[0] = (short)round((float)sum_y * a_float);
		dst[1] = (short)round((float)sum_cb * a_float);
		dst[2] = (short)round((float)sum_cr * a_float);
		dst[3] = (short)((sum_a << 8) / range);
	} else {
		dst[0] = dst[1] = dst[2] = dst[3] = 0;
	}
}
kernel void DisplacementMap_move(global short* dst, global short* src, global short* mem,
	int obj_w, int obj_h, int obj_line, int param0, int param1, int ox, int oy) {
	int x = get_global_id(0);
	int y = get_global_id(1);

	dst += (x + y * obj_line) * 4;
	mem += (x + y * obj_line) * 4;

	int p0 = min(mem[0], mem[obj_line * 4]);
	int p1 = max(mem[4], mem[obj_line * 4 + 4]);
	p0 = (p0 - 0x800) * param0 / 5;
	p1 = (p1 - 0x800) * param0 / 5 + 0x1000;
	if (p1 < p0) {
		int tmp = p0;
		p0 = p1;
		p1 = tmp;
	}
	int xx_range = p1 - p0;
	int xx_begin = p0 + (x << 12);
	int xx_end = xx_range + xx_begin;

	p0 = min(mem[1], mem[5]);
	p1 = max(mem[obj_line * 4 + 1], mem[obj_line * 4 + 5]);
	p0 = (p0 - 0x800) * param1 / 5;
	p1 = (p1 - 0x800) * param1 / 5 + 0x1000;
	if (p1 < p0) {
		int tmp = p0;
		p0 = p1;
		p1 = tmp;
	}
	int yy_range = p1 - p0;
	int yy_begin = p0 + (y << 12);
	int yy_end = yy_range + yy_begin;

	if (xx_range < 0x1000) {
		xx_begin += xx_range - 0x1000 >> 1;
		xx_end = xx_begin + 0x1000;
		xx_range = 0x1000;
	}
	if (yy_range < 0x1000) {
		yy_begin += yy_range - 0x1000 >> 1;
		yy_end = yy_begin + 0x1000;
		yy_range = 0x1000;
	}

	int xx_level = 12;
	int yy_level = 12;
	while (0x20000 < xx_range) {
		xx_begin >>= 1;
		xx_end++;
		xx_end >>= 1;
		xx_range = xx_end - xx_begin;
		xx_level--;
	}
	while (0x20000 < yy_range) {
		yy_begin >>= 1;
		yy_end++;
		yy_end >>= 1;
		yy_range = yy_end - yy_begin;
		yy_level--;
	}
	xx_level &= 0x1f;
	yy_level &= 0x1f;

	xx_begin = max(xx_begin, 0);
	xx_end = min(xx_end, obj_w << xx_level);
	yy_begin = max(yy_begin, 0);
	yy_end = min(yy_end, obj_h << yy_level);

	float dsum_y = 0.0f;
	float dsum_cb = 0.0f;
	float dsum_cr = 0.0f;
	float dsum_a = 0.0f;
	int yy = yy_begin;
	while (yy < yy_end) {
		int yy_itr = yy >> yy_level;
		int sum_y = 0;
		int sum_cb = 0;
		int sum_cr = 0;
		int sum_a = 0;
		int xx = xx_begin;
		while (xx < xx_end) {
			int xx_itr = xx >> xx_level;
			int fraction;
			if (xx & 0xfff) {
				fraction = -xx & 0xfff;
			} else {
				fraction = min(xx_end - xx, 0x1000);
			}

			global short* srct = src + (xx_itr + yy_itr * obj_line) * 4;
			int src_a = srct[3] * fraction >> 8;
			sum_y += srct[0] * src_a >> 16;
			sum_cb += srct[1] * src_a >> 16;
			sum_cr += srct[2] * src_a >> 16;
			sum_a += src_a;
			xx += fraction;
		}
		int fraction;
		if (yy & 0xfff) {
			fraction = -yy & 0xfff;
		} else {
			fraction = min(yy_end - yy, 0x1000);
		}
		float fraction_rate = (float)fraction * 0.000244140625f;
		dsum_y += (float)sum_y * fraction_rate;
		dsum_cb += (float)sum_cb * fraction_rate;
		dsum_cr += (float)sum_cr * fraction_rate;
		dsum_a += (float)sum_a * fraction_rate;
		yy += fraction;
	}

	if (256.0f <= dsum_a) {
		float inv_a = 65536.0f / dsum_a;
		dst[0] = (short)round(dsum_y * inv_a);
		dst[1] = (short)round(dsum_cb * inv_a);
		dst[2] = (short)round(dsum_cr * inv_a);
		dst[3] = (short)round(dsum_a / ((float)yy_range / 1024.0f) / ((float)xx_range / 1024.0f));
	} else {
		dst[0] = dst[1] = dst[2] = dst[3] = 0;
	}
}
kernel void DisplacementMap_zoom(global short* dst, global short* src, global short* mem,
	int obj_w, int obj_h, int obj_line, int param0, int param1, int ox, int oy){
	int x = get_global_id(0);
	int y = get_global_id(1);

	dst += (x + y * obj_line) * 4;
	mem += (x + y * obj_line) * 4;

	int xx_min, xx_max, yy_min, yy_max;
	int xx_temp, yy_temp;

	float zoom;
	float xxd = (float)(x * 0x1000 - ox);
	if (0 < param0) {
		zoom = (1024.0f / (float)(param0 + 1000) - 1.0) * 0.00048828125f;
	} else {
		zoom = (float)param0 * -0.00000048828125f;
	}
	float temp = xxd * zoom;
	xx_min = xx_max = x * 0x1000 + (int)((float)(mem[0] - 0x800) * temp);

	xx_temp = x * 0x1000 + (int)((float)(mem[obj_line * 4] - 0x800) * temp);
	xx_min = min(xx_min, xx_temp);
	xx_max = max(xx_max, xx_temp);

	temp = (xxd + 4096.0f) * zoom;
	xx_temp = (x + 1) * 0x1000 + (int)((float)(mem[4] - 0x800) * temp);
	xx_min = min(xx_min, xx_temp);
	xx_max = max(xx_max, xx_temp);

	xx_temp = (x + 1) * 0x1000 + (int)((float)(mem[(obj_line + 1) * 4] - 0x800) * temp);
	int xx_begin = min(xx_min, xx_temp);
	int xx_end = max(xx_max, xx_temp);


	float yyd = (float)(y * 0x1000 - oy);
	if (0 < param1) {
		zoom = (1024.0f / (float)(param1 + 1000) - 1.0) * 0.00048828125f;
	} else {
		zoom = (float)param1 * -0.00000048828125f;
	}
	temp = yyd * zoom;
	yy_min = yy_max = y * 0x1000 + (int)((float)(mem[1] - 0x800) * temp);

	yy_temp = y * 0x1000 + (int)((float)(mem[5] - 0x800) * temp);
	yy_min = min(yy_min, yy_temp);
	yy_max = max(yy_max, yy_temp);

	temp = (yyd + 4096.0f) * zoom;
	yy_temp = (y + 1) * 0x1000 + (int)((float)(mem[(obj_line + 1) * 4] - 0x800) * temp);
	yy_min = min(yy_min, yy_temp);
	yy_max = max(yy_max, yy_temp);

	yy_temp = (y + 1) * 0x1000 + (int)((float)(mem[obj_line * 4 + 5] - 0x800) * temp);
	int yy_begin = min(yy_min, yy_temp);
	int yy_end = max(yy_max, yy_temp);

	int xx_range = xx_end - xx_begin;
	int yy_range = yy_end - yy_begin;

	if (xx_range < 0x1000) {
		xx_begin += xx_range - 0x1000 >> 1;
		xx_end = xx_begin + 0x1000;
		xx_range = 0x1000;
	}
	if (yy_range < 0x1000) {
		yy_begin += yy_range - 0x1000 >> 1;
		yy_end = yy_begin + 0x1000;
		yy_range = 0x1000;
	}

	int xx_level = 12;
	int yy_level = 12;
	while (0x20000 < xx_range) {
		xx_begin >>= 1;
		xx_end++;
		xx_end >>= 1;
		xx_range = xx_end - xx_begin;
		xx_level--;
	}
	while (0x20000 < yy_range) {
		yy_begin >>= 1;
		yy_end++;
		yy_end >>= 1;
		yy_range = yy_end - yy_begin;
		yy_level--;
	}
	xx_level &= 0x1f;
	yy_level &= 0x1f;

	xx_begin = max(xx_begin, 0);
	xx_end = min(xx_end, obj_w << xx_level);
	yy_begin = max(yy_begin, 0);
	yy_end = min(yy_end, obj_h << yy_level);

	float dsum_y = 0.0f;
	float dsum_cb = 0.0f;
	float dsum_cr = 0.0f;
	float dsum_a = 0.0f;
	int yy = yy_begin;
	while (yy < yy_end) {
		int yy_itr = yy >> yy_level;
		int sum_y = 0;
		int sum_cb = 0;
		int sum_cr = 0;
		int sum_a = 0;
		int xx = xx_begin;
		while (xx < xx_end) {
			int xx_itr = xx >> xx_level;
			int fraction;
			if (xx & 0xfff) {
				fraction = -xx & 0xfff;
			} else {
				fraction = min(xx_end - xx, 0x1000);
			}

			global short* srct = src + (xx_itr + yy_itr * obj_line) * 4;
			int src_a = srct[3] * fraction >> 8;
			sum_y += srct[0] * src_a >> 16;
			sum_cb += srct[1] * src_a >> 16;
			sum_cr += srct[2] * src_a >> 16;
			sum_a += src_a;
			xx += fraction;
		}
		int fraction;
		if (yy & 0xfff) {
			fraction = -yy & 0xfff;
		} else {
			fraction = min(yy_end - yy, 0x1000);
		}
		float fraction_rate = (float)fraction * 0.000244140625f;
		dsum_y += (float)sum_y * fraction_rate;
		dsum_cb += (float)sum_cb * fraction_rate;
		dsum_cr += (float)sum_cr * fraction_rate;
		dsum_a += (float)sum_a * fraction_rate;
		yy += fraction;
	}

	if (256.0f <= dsum_a) {
		float inv_a = 65536.0f / dsum_a;
		dst[0] = (short)round(dsum_y * inv_a);
		dst[1] = (short)round(dsum_cb * inv_a);
		dst[2] = (short)round(dsum_cr * inv_a);
		dst[3] = (short)round(dsum_a / ((float)yy_range / 1024.0f) / ((float)xx_range / 1024.0f));
	} else {
		dst[0] = dst[1] = dst[2] = dst[3] = 0;
	}
}
kernel void DisplacementMap_rot(global short* dst, global short* src, global short* mem,
	int obj_w, int obj_h, int obj_line, int param0, int param1, int ox, int oy){
	int x = get_global_id(0);
	int y = get_global_id(1);

	dst += (x + y * obj_line) * 4;
	mem += (x + y * obj_line) * 4;

	int xx_min, xx_max, yy_min, yy_max;
	int xx_temp, yy_temp;

	float xxd = (float)((x << 12) - ox);
	float yyd = (float)((y << 12) - oy);
	float xxd_next = xxd + 4096.0f;
	float yyd_next = yyd + 4096.0f;
	float paramrad = (float)param0 * (float)-0.000003067961642955197f;

	float rad = (float)(mem[0] - 0x800) * paramrad;
	float sinv = sin(rad);
	float cosv = cos(rad);
	xx_min = xx_max = (int)(xxd * cosv - yyd * sinv);
	yy_min = yy_max = (int)(xxd * sinv + yyd * cosv);

	rad = (float)(mem[4] - 0x800) * paramrad;
	sinv = sin(rad);
	cosv = cos(rad);
	xx_temp = (int)(xxd_next * cosv - yyd * sinv);
	yy_temp = (int)(xxd_next * sinv + yyd * cosv);
	xx_min = min(xx_min, xx_temp);
	xx_max = max(xx_max, xx_temp);
	yy_min = min(yy_min, yy_temp);
	yy_max = max(yy_max, yy_temp);

	rad = (float)(mem[obj_line * 4] - 0x800) * paramrad;
	sinv = sin(rad);
	cosv = cos(rad);
	xx_temp = (int)(xxd * cosv - yyd_next * sinv);
	yy_temp = (int)(xxd * sinv + yyd_next * cosv);
	xx_min = min(xx_min, xx_temp);
	xx_max = max(xx_max, xx_temp);
	yy_min = min(yy_min, yy_temp);
	yy_max = max(yy_max, yy_temp);

	rad = (float)(mem[(obj_line + 1) * 4] - 0x800) * paramrad;
	sinv = sin(rad);
	cosv = cos(rad);
	xx_temp = (int)(xxd_next * cosv - yyd_next * sinv);
	yy_temp = (int)(xxd_next * sinv + yyd_next * cosv);
	int xx_begin = min(xx_min, xx_temp) + ox;
	int xx_end = max(xx_max, xx_temp) + ox;
	int yy_begin = min(yy_min, yy_temp) + oy;
	int yy_end = max(yy_max, yy_temp) + oy;

	int xx_range = xx_end - xx_begin;
	int yy_range = yy_end - yy_begin;

	if (xx_range < 0x1000) {
		xx_begin += xx_range - 0x1000 >> 1;
		xx_end = xx_begin + 0x1000;
		xx_range = 0x1000;
	}
	if (yy_range < 0x1000) {
		yy_begin += yy_range - 0x1000 >> 1;
		yy_end = yy_begin + 0x1000;
		yy_range = 0x1000;
	}

	int xx_level = 12;
	int yy_level = 12;
	while (0x20000 < xx_range) {
		xx_begin >>= 1;
		xx_end++;
		xx_end >>= 1;
		xx_range = xx_end - xx_begin;
		xx_level--;
	}
	while (0x20000 < yy_range) {
		yy_begin >>= 1;
		yy_end++;
		yy_end >>= 1;
		yy_range = yy_end - yy_begin;
		yy_level--;
	}
	xx_level &= 0x1f;
	yy_level &= 0x1f;

	xx_begin = max(xx_begin, 0);
	xx_end = min(xx_end, obj_w << xx_level);
	yy_begin = max(yy_begin, 0);
	yy_end = min(yy_end, obj_h << yy_level);

	float dsum_y = 0.0f;
	float dsum_cb = 0.0f;
	float dsum_cr = 0.0f;
	float dsum_a = 0.0f;
	int yy = yy_begin;
	while (yy < yy_end) {
		int yy_itr = yy >> yy_level;
		int sum_y = 0;
		int sum_cb = 0;
		int sum_cr = 0;
		int sum_a = 0;
		int xx = xx_begin;
		while (xx < xx_end) {
			int xx_itr = xx >> xx_level;
			int fraction;
			if (xx & 0xfff) {
				fraction = -xx & 0xfff;
			} else {
				fraction = min(xx_end - xx, 0x1000);
			}

			global short* srct = src + (xx_itr + yy_itr * obj_line) * 4;
			int src_a = srct[3] * fraction >> 8;
			sum_y += srct[0] * src_a >> 16;
			sum_cb += srct[1] * src_a >> 16;
			sum_cr += srct[2] * src_a >> 16;
			sum_a += src_a;
			xx += fraction;
		}
		int fraction;
		if (yy & 0xfff) {
			fraction = -yy & 0xfff;
		} else {
			fraction = min(yy_end - yy, 0x1000);
		}
		float fraction_rate = (float)fraction * 0.000244140625f;
		dsum_y += (float)sum_y * fraction_rate;
		dsum_cb += (float)sum_cb * fraction_rate;
		dsum_cr += (float)sum_cr * fraction_rate;
		dsum_a += (float)sum_a * fraction_rate;
		yy += fraction;
	}

	if (256.0f <= dsum_a) {
		float inv_a = 65536.0f / dsum_a;
		dst[0] = (short)round(dsum_y * inv_a);
		dst[1] = (short)round(dsum_cb * inv_a);
		dst[2] = (short)round(dsum_cr * inv_a);
		dst[3] = (short)round(dsum_a / ((float)yy_range / 1024.0f) / ((float)xx_range / 1024.0f));
	} else {
		dst[0] = dst[1] = dst[2] = dst[3] = 0;
	}
}

kernel void RadiationalBlur_Media(
	global short* dst, global short* src, int src_w, int src_h, int buffer_line,
	int rb_blur_cx, int rb_blur_cy, int rb_obj_cx, int rb_obj_cy, int rb_range, int rb_pixel_range) {

	int x = get_global_id(0);
	int y = get_global_id(1);
	int pixel_itr = x + y * buffer_line;

	x += rb_obj_cx;
	y += rb_obj_cy;
	int cx = rb_blur_cx - x;
	int cy = rb_blur_cy - y;
	int c_dist_times8 = (int)round(sqrt((float)(cx * cx + cy * cy)) * 8.0f);
	int range = rb_range * c_dist_times8 / 1000;

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
				int itr_a = itr.w
				sum_a += itr_a;
				if (0x1000 < itr_a) {
					itr_a = 0x1000;
				}
				sum_y += itr.x * itr_a / 4096;
				sum_cb += itr.y * itr_a / 4096;
				sum_cr += itr.z * itr_a / 4096;
				
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
			dst[pixel_itr * 4 + 3] = 0;
		}
	} else {
		if (x < 0 || y < 0 || src_w <= x || src_h <= y) {
			vstore4((short4)(0, 0, 0, 0), pixel_itr, dst);
		} else {
			vstore4(vload4(x + y * buffer_line, src), pixel_itr, dst);
		}
	}
}

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
kernel void DirectionalBlur_Media(global short* dst, global short* src, int obj_w, int obj_h, int obj_line,
	int x_begin, int x_end, int x_step, int y_begin, int y_end, int y_step, int range) {
	int x = get_global_id(0);
	int y = get_global_id(1);
	int pix_range = range * 2 + 1;

	dst += (x + y * obj_line) * 4;

	int sum_y = 0;
	int sum_cb = 0;
	int sum_cr = 0;
	int sum_a = 0;

	int x_itr = (x + x_begin << 16) + 0x8000 - range * x_step;
	int y_itr = (y + y_begin << 16) + 0x8000 - range * y_step;

	for (int n = 0; n < pix_range; n++) {
		int xx = x_itr >> 16;
		int yy = y_itr >> 16;
		if (0 <= xx && xx < obj_w && 0 <= yy && yy < obj_h) {
			global short* pix = src + (xx + yy * obj_line) * 4;
			int src_a = min((int)pix[3], 0x1000);
			sum_y += pix[0] * src_a >> 12;
			sum_cb += pix[1] * src_a >> 12;
			sum_cr += pix[2] * src_a >> 12;
			sum_a += src_a;
		}
		x_itr += x_step;
		y_itr += y_step;
	}
	if (0 < sum_a) {
		float a_float = 4096.0f / (float)sum_a;
		dst[0] = (short)round((float)sum_y * a_float);
		dst[1] = (short)round((float)sum_cb * a_float);
		dst[2] = (short)round((float)sum_cr * a_float);
	} else {
		dst[0] = dst[1] = dst[2] = 0;
	}
	dst[3] = (short)(sum_a / pix_range);
}
kernel void DirectionalBlur_original_size(global short* dst, global short* src, int obj_w, int obj_h, int obj_line,
	int x_step, int y_step, int range) {
	int x = get_global_id(0);
	int y = get_global_id(1);
	int pix_range = range * 2 + 1;

	dst += (x + y * obj_line) * 4;

	int x_itr = (x << 16) + 0x8000 - range * x_step;
	int y_itr = (y << 16) + 0x8000 - range * y_step;

	int sum_y = 0;
	int sum_cb = 0;
	int sum_cr = 0;
	int sum_a = 0;
	int cnt = 0;

	for (int n = 0; n < pix_range; n++) {
		int xx = x_itr >> 16;
		int yy = y_itr >> 16;
		if (0 <= xx && xx < obj_w && 0 <= yy && yy < obj_h) {
			global short* pix = src + (xx + yy * obj_line) * 4;
			int src_a = min((int)pix[3], 0x1000);
			sum_y += pix[0] * src_a >> 12;
			sum_cb += pix[1] * src_a >> 12;
			sum_cr += pix[2] * src_a >> 12;
			sum_a += src_a;
			cnt++;
		}
		x_itr += x_step;
		y_itr += y_step;
	}
	if(cnt == 0) cnt = 0xffffff;
	if (0 < sum_a) {
		float a_float = 4096.0f / (float)sum_a;
		dst[0] = (short)round((float)sum_y * a_float);
		dst[1] = (short)round((float)sum_cb * a_float);
		dst[2] = (short)round((float)sum_cr * a_float);
	} else {
		dst[0] = dst[1] = dst[2] = 0;
	}
	dst[3] = (short)(sum_a / cnt);
}
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
			global short* pix = src + (xx + yy * scene_line) * 3;
			sum_y += pix[0];
			sum_cb += pix[1];
			sum_cr += pix[2];
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
				cor_a = *(global short*)&src[offset2 + 6] * cor_a >> 12;
				sum_y += *(global float*)&src[offset2] * (float)cor_a;
				sum_cb += src[offset2 + 4] * cor_a;
				sum_cr += src[offset2 + 5] * cor_a;
				sum_a += cor_a;
			}
			sqr += 1 + xx * 2;
			offset2 += 8;
		}
		offset += obj_line * 8;
	}

	dst += (x + y * obj_line) * 8;
	if (0 < sum_a) {
		*(global float*)dst = sum_y / (float)sum_a;
		dst[4] = (char)(((sum_a >> 1) + sum_cb) / sum_a);
		dst[5] = (char)(((sum_a >> 1) + sum_cr) / sum_a);
		*(global short*)&dst[6] = (short)round((float)sum_a * (4096.0f / (float)cor_sum));
	} else {
		*(global int*)dst = 0;
		*(global int*)&dst[4] = 0;
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
				tofloat[0] = *(global short*)&src[offset2];
				tofloat[1] = *(global short*)&src[offset2 + 2];
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
	*(global short*)&dst[0] = tofloat[0];
	*(global short*)&dst[2] = tofloat[1];
	dst[4] = (char)(((sum_a >> 1) + sum_cb) / sum_a);
	dst[5] = (char)(((sum_a >> 1) + sum_cr) / sum_a);
}
