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
#include "util.hpp"
#include "util_pe.hpp"
#include "util_resource.hpp"
#include "offset_address.hpp"

#define overwrite_resource_t_func_org_def(name) inline static std::add_pointer_t<decltype(&name)> name##_org
#define overwrite_resource_t_func_ptr_def(name) inline const static auto name##_ptr = &name

inline class overwrite_resource_t {
public:
	static HRSRC WINAPI FindResourceA_Wrap(HMODULE hModule, LPCSTR lpName, LPCSTR lpType);
	static BOOL APIENTRY GetSaveFileNameA_Wrap(LPOPENFILENAMEA ofnap);
	static BOOL APIENTRY GetOpenFileNameA_Wrap(LPOPENFILENAMEA ofnap);

	static HCURSOR WINAPI LoadCursorA_Wrap(HINSTANCE hInstance, LPCSTR lpCursorName);
	static HICON WINAPI LoadIconA_Wrap(HINSTANCE hInstance, LPCSTR lpIconName);
	static HANDLE WINAPI LoadImageA_Wrap(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad);
	static HMENU WINAPI LoadMenuA_Wrap(HINSTANCE hInstance, LPCSTR lpMenuName);
	static int WINAPI LoadStringA_Wrap(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int cchBufferMax);
	
	static INT_PTR WINAPI DialogBoxParamA_Wrap(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	static HWND WINAPI CreateDialogParamA_Wrap(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	
private:

	inline static HMODULE default_default_resource_hmod;

	overwrite_resource_t_func_ptr_def(FindResourceA_Wrap);
	overwrite_resource_t_func_ptr_def(GetSaveFileNameA_Wrap);
	overwrite_resource_t_func_ptr_def(GetOpenFileNameA_Wrap);
	overwrite_resource_t_func_ptr_def(LoadCursorA_Wrap);
	overwrite_resource_t_func_ptr_def(LoadIconA_Wrap);
	overwrite_resource_t_func_ptr_def(LoadImageA_Wrap);
	overwrite_resource_t_func_ptr_def(LoadMenuA_Wrap);
	overwrite_resource_t_func_ptr_def(LoadStringA_Wrap);
	overwrite_resource_t_func_ptr_def(DialogBoxParamA_Wrap);
	overwrite_resource_t_func_ptr_def(CreateDialogParamA_Wrap);

	overwrite_resource_t_func_org_def(FindResourceA);
	overwrite_resource_t_func_org_def(GetSaveFileNameA);
	overwrite_resource_t_func_org_def(GetOpenFileNameA);
	overwrite_resource_t_func_org_def(LoadCursorA);
	overwrite_resource_t_func_org_def(LoadIconA);
	overwrite_resource_t_func_org_def(LoadImageA);
	overwrite_resource_t_func_org_def(LoadMenuA);
	overwrite_resource_t_func_org_def(LoadStringA);
	overwrite_resource_t_func_org_def(DialogBoxParamA);
	overwrite_resource_t_func_org_def(CreateDialogParamA);

	static void InitAtResourceLoaded() {
		for (auto& s : patch_resource_message_stack) {
			s->fire();
		}
		patch_resource_message_stack.clear();
	}
	inline static auto InitAtResourceLoaded_ptr = &InitAtResourceLoaded;

public:
	void operator()() {
		/*
			・LoadLibrary直後のEnumResourceLanguagesAを消し飛ばす
			　・patch.aulのリソースを別に登録させない
			　・patch.aulは正しい言語拡張リソースではなかったということにする
			　　・これで必ず毎回読まれるようになる

			・AviUtlのデフォルトのリソースのHMODULEをこれにする
			　・メニューを読んであとから編集～はしない

			 ・SaveDialogとOpenDIalogのTEMPLETEどうしよう...
			 　・これも乗っ取っちまえ
		*/
	
		// reinterpret_cast<i32>(search_import((HMODULE)GLOBAL::aviutl_base, "COMDLG32.DLL", "GetOpenFileNameA")),
		auto reloc_list = search_reloc((HMODULE)GLOBAL::aviutl_base, {
			0x06f07c, // GetOpenFileNameA 
			0x06f074, // GetSaveFileNameA
			0x06f0dc, // FindResourceA
			0x06f400, // LoadCursorA
			0x06f394, // LoadIconA
			0x06f3a0, // LoadImageA
			0x06f3b8, // LoadMenuA
			0x06f3b4, // LoadStringA
			0x06f3b0, // DialogBoxParamA
			0x06f3ac, // CreateDialogParamA
		});

		for (auto v : reloc_list[0x06f07c])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &GetOpenFileNameA_Wrap_ptr);

		for (auto v : reloc_list[0x06f074])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &GetSaveFileNameA_Wrap_ptr);

		for (auto v : reloc_list[0x06f0dc])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &FindResourceA_Wrap_ptr);

		for (auto v : reloc_list[0x06f400])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &LoadCursorA_Wrap_ptr);

		for (auto v : reloc_list[0x06f394])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &LoadIconA_Wrap_ptr);

		for (auto v : reloc_list[0x06f3a0])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &LoadImageA_Wrap_ptr);

		for (auto v : reloc_list[0x06f3b8])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &LoadMenuA_Wrap_ptr);

		for (auto v : reloc_list[0x06f3b4])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &LoadStringA_Wrap_ptr);

		for (auto v : reloc_list[0x06f3b0])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &DialogBoxParamA_Wrap_ptr);

		for (auto v : reloc_list[0x06f3ac])
			OverWriteOnProtectHelper(reinterpret_cast<i32>(v), 4).store_i32(0, &CreateDialogParamA_Wrap_ptr);

#define def(name, ofs) name = reinterpret_cast<decltype(name)>(GLOBAL::aviutl_base + ofs)
		def(GetOpenFileNameA_org  , 0x06f07c);
		def(GetSaveFileNameA_org  , 0x06f074);
		def(FindResourceA_org     , 0x06f0dc);
		def(LoadCursorA_org       , 0x06f400);
		def(LoadIconA_org         , 0x06f394);
		def(LoadImageA_org        , 0x06f3a0);
		def(LoadMenuA_org         , 0x06f3b8);
		def(LoadStringA_org       , 0x06f3b4);
		def(DialogBoxParamA_org   , 0x06f3b0);
		def(CreateDialogParamA_org, 0x06f3ac);
#undef def

		/*
		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f07c, 4).store_i32(0, GetOpenFileNameA_Wrap);
		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f074, 4).store_i32(0, GetSaveFileNameA_Wrap);
		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f0dc, 4).store_i32(0, FindResourceA_Wrap);

		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f400, 4).store_i32(0, LoadCursorA_Wrap);
		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f394, 4).store_i32(0, LoadIconA_Wrap);
		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f3a0, 4).store_i32(0, LoadImageA_Wrap);
		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f3b8, 4).store_i32(0, LoadMenuA_Wrap);
		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f3b4, 4).store_i32(0, LoadStringA_Wrap);

		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f3b0, 4).store_i32(0, DialogBoxParamA_Wrap);
		OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x06f3ac, 4).store_i32(0, CreateDialogParamA_Wrap);
		*/

		// default_resource_hmod
		auto default_resource_hmod_ptr = (HMODULE*)(GLOBAL::aviutl_base + OFS::AviUtl::default_resource_hmod);
		default_default_resource_hmod = std::exchange(*default_resource_hmod_ptr, GLOBAL::patchaul_hinst);

		const char code[] =
			"\x50" // push eax
			"\xff\x15xxxx" // call [i32]
			"\x58" // pop eax
			"\xc3"; // ret

		OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x0548e0, sizeof(code) - 1);
		memcpy((void*)(GLOBAL::aviutl_base + 0x0548e0), code, sizeof(code) - 1);
		h.store_i32(3, &InitAtResourceLoaded_ptr);

	}

} overwrite_resource;

