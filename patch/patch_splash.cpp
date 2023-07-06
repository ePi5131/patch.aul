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

#include "patch_splash.hpp"

#ifdef PATCH_SWITCH_SPLASH
namespace patch {
	LRESULT CALLBACK splash_t::SplashWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
		using namespace Gdiplus;

		static GdiplusStartupInput gdi_input;
		static ULONG_PTR gdi_token;

		static HDC image_dc;
		static int w, h;

		switch (message) {
		case WM_CREATE: {
			Gdiplus::GdiplusStartup(&gdi_token, &gdi_input, NULL);

			auto hmod = GLOBAL::patchaul_hinst;
			auto resource_splash = FindResourceA(hmod, "SPLASH", RT_RCDATA);
			auto resource_splash_size = SizeofResource(hmod, resource_splash);
			auto resource_splash_data = LoadResource(hmod, resource_splash);
			auto resource_splash_ptr = LockResource(resource_splash_data);

			auto hglobal = GlobalAlloc(0, resource_splash_size);
			if (hglobal == NULL) return -1;
			auto ptr = GlobalLock(hglobal);
			if (ptr == nullptr) {
				GlobalFree(hglobal);
				return -1;
			}

			memcpy(ptr, resource_splash_ptr, resource_splash_size);

			GlobalUnlock(ptr);
			UnlockResource(resource_splash_ptr);

			IStream* is;
			CreateStreamOnHGlobal(hglobal, FALSE, &is);
			
			Gdiplus::Image image_plus(is);

			is->Release();
			GlobalFree(hglobal);
			
			w = image_plus.GetWidth();
			h = image_plus.GetHeight();

			auto hdc = GetDC(hwnd);
			image_dc = CreateCompatibleDC(hdc);
			auto image_bmp = CreateCompatibleBitmap(hdc, w, h);
			SelectObject(image_dc, image_bmp);
			Graphics graphics(image_dc);
			graphics.DrawImage(&image_plus, 0, 0);
			ReleaseDC(hwnd, hdc);

			RECT window_rect;
			GetWindowRect(GetDesktopWindow(), &window_rect);
			SetWindowLongA(hwnd, GWL_STYLE, 0);
			SetWindowPos(hwnd, HWND_TOPMOST, (window_rect.right - w) / 2, (window_rect.bottom - h) / 2, w, h, SWP_FRAMECHANGED);
			return 0;
		}
		case WM_DESTROY:
			DeleteDC(image_dc);
			GdiplusShutdown(gdi_token);
			PostQuitMessage(0);
			return 0;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			return DefWindowProcA(hwnd, message, wparam, lparam);
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			
			BitBlt(hdc, 0, 0, w, h, image_dc, 0, 0, SRCCOPY);
			//SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);
			{
				std::lock_guard lock(mutex);
				TextOutW(hdc, 2, h - 40, state.phase.c_str(), state.phase.length());
				TextOutW(hdc, 2, h - 20, state.detail.c_str(), state.detail.length());
			}
			
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_ERASEBKGND:
			return 1;

		default:
			return DefWindowProcA(hwnd, message, wparam, lparam);
		}
	}
	
	void __cdecl InitEnd() {
		patch::splash.finish();
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_SPLASH
