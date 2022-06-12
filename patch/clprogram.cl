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

kernel void RadiationalBlur(
    global short* dst, global short* src, int src_w, int src_h, int exedit_buffer_line,
	int X,
	int Y,
	int Range,
	int pixel_range,
	int Cx,
	int Cy,
	int result_x_max,
	int result_y_max
) {
    int xi = get_global_id(0);
    int yi = get_global_id(1);

    int y = yi + Cy;
    int y_detail = y * 0x10000 + 0x8000;

    int pixel_itr = xi + yi * exedit_buffer_line;
    int x = xi + Cx;
    int x_detail = x * 0x10000 + 0x8000;
    int cx = X - x;
    int cy = Y - y;
    int c_dist_times8 = (int)round(sqrt((float)(cx * cx + cy * cy)) * 8.0f);
    int range = (Range * c_dist_times8) / 1000;

    if (pixel_range < c_dist_times8) {
        range = pixel_range * Range / 1000;
        c_dist_times8 = pixel_range;
    }
    else if (8 < c_dist_times8) {
        c_dist_times8 *= 8;
        range *= 8;
    }
    else if (4 < c_dist_times8) {
        c_dist_times8 *= 4;
        range *= 4;
    }
    else if (2 < c_dist_times8) {
        c_dist_times8 *= 2;
        range *= 2;
    }

    if ((c_dist_times8 < 2) || (range < 2)) {
        if (x_detail < 0x8000 || y_detail < 0x8000 || src_w <= x || src_h <= y) {
            vstore4((short4)(0, 0, 0, 0), pixel_itr, dst);
        }
        else {
            vstore4(vload4(x + y * exedit_buffer_line, src), pixel_itr, dst);
        }
    }
    else {
        int sum_a = 0;
        int sum_cr = 0;
        int sum_cb = 0;
        int sum_y = 0;
        int x_detail_itr = x_detail;
        int y_detail_itr = y_detail;

        if (0 < range) {
            for (int i = 0; i < range; i++) {
                int x_itr = x_detail_itr / 0x10000;
                int y_itr = y_detail_itr / 0x10000;
                if (0 <= x_itr && x_itr < src_w && 0 <= y_itr && y_itr < src_h) {
                    short4 itr = vload4(x_itr + y_itr * exedit_buffer_line, src);
                    int itr_a = itr.w;
                    if (itr_a != 0) {
                        if (itr_a < 0x1000) {
                            sum_y  += itr.x * itr_a / 4096;
                            sum_cb += itr.y * itr_a / 4096;
                            sum_cr += itr.z * itr_a / 4096;
                        }
                        else {
                            sum_y  += itr.x;
                            sum_cb += itr.y;
                            sum_cr += itr.z;
                        }
                        sum_a += itr_a;
                    }
                }
                x_detail_itr += (cx * 0x10000) / c_dist_times8;
                y_detail_itr += (cy * 0x10000) / c_dist_times8;
            }
            if (sum_a != 0) {
                vstore4(
                    (short4)(
                        round(sum_y  * 4096.0f / sum_a),
                        round(sum_cb * 4096.0f / sum_a),
                        round(sum_cr * 4096.0f / sum_a),
                        sum_a / range
                    ),
                    pixel_itr, dst
                );
            }
            else{
                dst[pixel_itr * 4 + 3] = (short)(sum_a / range);
            }
        }
        else{
            dst[pixel_itr * 4 + 3] = (short)(sum_a / range);
        }
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
            short* pix = src + (xx + yy * obj_line) * 4;
            int src_a = min((int)pix[3], 0x1000);
            sum_y += pix[0] * src_a >> 12;
            sum_cb += pix[1] * src_a >> 12;
            sum_cr += pix[2] * src_a >> 12;
            sum_a += src_a;
        }
        x_itr += x_step;
        y_itr += y_step;
    }
    if (sum_a) {
        dst[0] = (short)(sum_y * 4096 / sum_a);
        dst[1] = (short)(sum_cb * 4096 / sum_a);
        dst[2] = (short)(sum_cr * 4096 / sum_a);
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

    uint sum_y = 0;
    int sum_cb = 0;
    int sum_cr = 0;
    int sum_a = 0;
    int cnt = 0;

    for (int n = 0; n < pix_range; n++) {
        int xx = x_itr >> 16;
        int yy = y_itr >> 16;
        if (0 <= xx && xx < obj_w && 0 <= yy && yy < obj_h) {
            short* pix = src + (xx + yy * obj_line) * 4;
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
    if (sum_a) {
        dst[0] = (short)(sum_y * 4096 / sum_a);
        dst[1] = (short)(sum_cb * 4096 / sum_a);
        dst[2] = (short)(sum_cr * 4096 / sum_a);
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
            short* pix = src + (xx + yy * scene_line) * 3;
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
