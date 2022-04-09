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

#include "patch_fast_exeditwindow.hpp"
#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
#pragma comment(lib, "Msimg32.lib")

#include "util_others.hpp"

//#define PATCH_STOPWATCH
#include "stopwatch.hpp"

namespace patch {
	static stopwatch_mem sw;

	void __cdecl fast_exeditwindow_t::FUN_10036a70_Wrap_gradation(HDC hDC, LPRECT prect, int r1, int g1, int b1, int r2, int g2, int b2, int gradation_left, int gradation_right) {
		static GRADIENT_RECT gr[] = { { .UpperLeft = 0, .LowerRight = 1 } };

		TRIVERTEX tv[2];
		tv[0].y = prect->top;
		tv[1].y = prect->bottom;
		tv[0].Alpha = 0;
		tv[1].Alpha = 0;

		auto real_diff = gradation_right - gradation_left;
		if (prect->left < gradation_left) {
			if (prect->right < gradation_left) {
				SetDCBrushColor(hDC, RGB(r1, g1, b1));
				FillRect(hDC, prect, (HBRUSH)GetStockObject(DC_BRUSH));
				return;
			}

			SetDCBrushColor(hDC, RGB(r1, g1, b1));
			RECT rect{
				prect->left, prect->top,
				gradation_left, prect->bottom
			};
			FillRect(hDC, prect, (HBRUSH)GetStockObject(DC_BRUSH));

			const auto cr1 = r1 << 8, cg1 = g1 << 8, cb1 = b1 << 8;
			const auto cr2 = r2 << 8, cg2 = g2 << 8, cb2 = b2 << 8;

			tv[0].x = gradation_left;
			tv[0].Red   = static_cast<COLOR16>(cr1);
			tv[0].Green = static_cast<COLOR16>(cg1);
			tv[0].Blue  = static_cast<COLOR16>(cb1);

			if (gradation_right < prect->right) {
				SetDCBrushColor(hDC, RGB(r2, g2, b2));
				RECT rect{
					gradation_right, prect->top,
					prect->right, prect->bottom
				};
				FillRect(hDC, &rect, (HBRUSH)GetStockObject(DC_BRUSH));
				tv[1].x = gradation_right;
				tv[1].Red   = static_cast<COLOR16>(cr2);
				tv[1].Green = static_cast<COLOR16>(cg2);
				tv[1].Blue  = static_cast<COLOR16>(cb2);
			}
			else {
				tv[1].x = prect->right;
				if (real_diff == 0) {
					tv[1].Red   = static_cast<COLOR16>(cr2);
					tv[1].Green = static_cast<COLOR16>(cg2);
					tv[1].Blue  = static_cast<COLOR16>(cb2);
				}
				else {
					tv[1].Red   = static_cast<COLOR16>(cr1 + (cr2 - cr1) * (prect->right - gradation_left) / real_diff);
					tv[1].Green = static_cast<COLOR16>(cg1 + (cg2 - cg1) * (prect->right - gradation_left) / real_diff);
					tv[1].Blue  = static_cast<COLOR16>(cb1 + (cb2 - cb1) * (prect->right - gradation_left) / real_diff);
				}
			}
			GradientFill(hDC, tv, std::size(tv), &gr, std::size(gr), GRADIENT_FILL_RECT_H);
		}
		else {
			if (gradation_right < prect->left) {
				SetDCBrushColor(hDC, RGB(r2, g2, b2));
				FillRect(hDC, prect, (HBRUSH)GetStockObject(DC_BRUSH));
				return;
			}

			const auto cr1 = r1 << 8, cg1 = g1 << 8, cb1 = b1 << 8;
			const auto cr2 = r2 << 8, cg2 = g2 << 8, cb2 = b2 << 8;

			tv[0].x = prect->left;
			if (real_diff == 0) {
				tv[0].Red   = cr1;
				tv[0].Green = cg1;
				tv[0].Blue  = cb1;
			}
			else {
				tv[0].Red   = static_cast<COLOR16>(cr1 + (cr2 - cr1) * (prect->left - gradation_left) / real_diff);
				tv[0].Green = static_cast<COLOR16>(cg1 + (cg2 - cg1) * (prect->left - gradation_left) / real_diff);
				tv[0].Blue  = static_cast<COLOR16>(cb1 + (cb2 - cb1) * (prect->left - gradation_left) / real_diff);
			}

			if (gradation_right < prect->right) {
				SetDCBrushColor(hDC, RGB(r2, g2, b2));
				RECT rect{
					gradation_right, prect->top,
					prect->right, prect->bottom
				};
				FillRect(hDC, &rect, (HBRUSH)GetStockObject(DC_BRUSH));
				tv[1].x = gradation_right;
				tv[1].Red   = static_cast<COLOR16>(cr2);
				tv[1].Green = static_cast<COLOR16>(cg2);
				tv[1].Blue  = static_cast<COLOR16>(cb2);
			}
			else {
				tv[1].x = prect->right;
				if (real_diff == 0) {
					tv[1].Red   = static_cast<COLOR16>(cr2);
					tv[1].Green = static_cast<COLOR16>(cg2);
					tv[1].Blue  = static_cast<COLOR16>(cb2);
				}
				else {
					tv[1].Red   = static_cast<COLOR16>(cr1 + (cr2 - cr1) * (prect->right - gradation_left) / real_diff);
					tv[1].Green = static_cast<COLOR16>(cg1 + (cg2 - cg1) * (prect->right - gradation_left) / real_diff);
					tv[1].Blue  = static_cast<COLOR16>(cb1 + (cb2 - cb1) * (prect->right - gradation_left) / real_diff);
				}
			}
			GradientFill(hDC, tv, std::size(tv), &gr, std::size(gr), GRADIENT_FILL_RECT_H);
		}
	}

	void __cdecl fast_exeditwindow_t::FUN_10036a70_Wrap_step(HDC hDC, LPRECT prect, int r1, int g1, int b1, int r2, int g2, int b2, int gradation_left, int gradation_right) {
		auto steps = PATCH_SWITCHER_MEMBER(fast_exeditwindow_step).value();

		auto brush = (HBRUSH)GetStockObject(DC_BRUSH);

		if (steps == 1) {
			SetDCBrushColor(hDC, RGB(r1, g1, b1));
			FillRect(hDC, prect, brush);
			return;
		}

		auto real_diff = gradation_right - gradation_left;
		if (prect->left < gradation_left) {
			if (prect->right < gradation_left) {
				SetDCBrushColor(hDC, RGB(r1, g1, b1));
				FillRect(hDC, prect, brush);
				return;
			}

			SetDCBrushColor(hDC, RGB(r1, g1, b1));
			RECT rect{
				prect->left, prect->top,
				gradation_left, prect->bottom
			};
			FillRect(hDC, prect, brush);
			
			if (gradation_right < prect->right) {
				SetDCBrushColor(hDC, RGB(r2, g2, b2));
				RECT rect{
					gradation_right, prect->top,
					prect->right, prect->bottom
				};
				FillRect(hDC, &rect, brush);
			}
		}
		else {
			if (gradation_right < prect->left) {
				SetDCBrushColor(hDC, RGB(r2, g2, b2));
				FillRect(hDC, prect, brush);
				return;
			}

			if (gradation_right < prect->right) {
				SetDCBrushColor(hDC, RGB(r2, g2, b2));
				RECT rect{
					gradation_right, prect->top,
					prect->right, prect->bottom
				};
				FillRect(hDC, &rect, brush);
			}
		}

		LONG L, R;
		R = gradation_left;

		RECT rect;
		rect.top = prect->top;
		rect.bottom = prect->bottom;

		for (int i = 0; i < steps; i++) {
			L = R;
			R = gradation_left + (gradation_right - gradation_left) * (i + 1) / steps;

			if (R < prect->left) continue;
			if (prect->right < L) continue;

			int dL = std::max<>(prect->left, L);
			int dR = std::min<>(prect->right, R);

			SetDCBrushColor(hDC, RGB(
				r1 + (r2 - r1) * i / (steps - 1),
				g1 + (g2 - g1) * i / (steps - 1),
				b1 + (b2 - b1) * i / (steps - 1)
			));
			rect.left = dL;
			rect.right = dR;
			FillRect(hDC, &rect, brush);
		}
	}

	static HDC frontdc;
	static HBITMAP bitmap;
	static HBITMAP bitmap_old;
	static int width, height;

#define DO 1
	HDC WINAPI fast_exeditwindow_t::GetDC_Wrap(HWND hwnd) {
		sw.start();
#if DO
		frontdc = GetDC(hwnd);

		RECT rect;
		GetClientRect(hwnd, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		auto backdc = CreateCompatibleDC(frontdc);
		bitmap = CreateCompatibleBitmap(frontdc, width, height);
		bitmap_old = (HBITMAP)SelectObject(backdc, bitmap);
		BitBlt(backdc, 0, 0, width, height, frontdc, 0, 0, SRCCOPY);

		return backdc;
#else
		return GetDC(hwnd);
#endif
	}
	int WINAPI fast_exeditwindow_t::ReleaseDC_Wrap(HWND hwnd, HDC hdc) {
#if DO
		BitBlt(frontdc, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
		SelectObject(hdc, bitmap_old);
		DeleteObject(bitmap);
		DeleteObject(hdc);
#endif
		sw.stop();
		return ReleaseDC(hwnd, frontdc);
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
