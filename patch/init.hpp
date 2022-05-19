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
#include <winwrap.hpp>

#include "cryptostring.hpp"
#include "add_dll_ref.hpp"
#include "util.hpp"
#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "patch.hpp"
#include "debug_log.hpp"

#include "overwrite_resource.hpp"
#include "patch_exception_log.hpp"

#include "mywindow.hpp"

inline class init_t {
public:

	inline static BOOL(__cdecl* original_func_WndProc)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);
	inline static BOOL(__cdecl* original_func_init)(AviUtl::FilterPlugin* fp);
	inline static BOOL(__cdecl* original_func_proc)(AviUtl::FilterPlugin* fp, AviUtl::FilterProcInfo* fpip);

	static BOOL __cdecl func_WndProcWrap(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);
	static BOOL __cdecl func_initWrap(AviUtl::FilterPlugin* fp);
	static BOOL __cdecl func_procWrap(AviUtl::FilterPlugin* fp, AviUtl::FilterProcInfo* fpip);
	static BOOL WINAPI EnumResourceLanguagesA_Wrap(HMODULE hModule, LPCSTR lpType, LPCSTR lpName, ENUMRESLANGPROCA lpEnumFunc, LONG_PTR lParam);
	static HMODULE WINAPI LoadLibraryAWrap(LPCSTR lpLibFileName);

	// DllMain呼び出しのタイミングでやる処理
	// できるだけ少なくしたい
	static void InitAtDllMain();

	// LoadLibrary("patch.aul")直後にやる処理
	// EnumResourceLanguagesAの乗っ取りで実現
	// EnumResourceLanguagesAが失敗したことにして，patch.aulは正しい言語拡張リソースではないことにする
	static void InitAtPatchLoaded();

	// exedit.aufのLoadLibrary直後にやる処理
	// 拡張編集へのインジェクションはここでやる
	static void InitAtExeditLoad();

	// フィルタプラグインの読み込みを開始するタイミングでやる処理
	static void InitAufBefore();


} init;
