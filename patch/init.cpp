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

#include "init.hpp"

#include <set>
#include <string>

#include "cryptostring.hpp"
#include "util_others.hpp"

#include "config.hpp"


void init_t::InitAtDllMain() {
	ExchangeFunction(GLOBAL::aviutl_hmod, cstr_kernel32_dll.get(), cstr_EnumResourceLanguagesA.get(), EnumResourceLanguagesA_Wrap);
}

void init_t::InitAtPatchLoaded() {
	{
		static const char aviutl_version_str[] = {
			'1','.','1','0','\0','\0','\0','\0'
		};
		if (memcmp(reinterpret_cast<void*>(GLOBAL::aviutl_base + OFS::AviUtl::VersionString), aviutl_version_str, sizeof(aviutl_version_str)) != 0) {
			MessageBoxW(NULL, L"patch.aul requires AviUtl *1.10*.\nAviUtl version 1.10以外では動作しません．", L"patch.aul", MB_ICONEXCLAMATION);
			return;
		}
	}
	GLOBAL::patchaul_path = WinWrap::Module{ GLOBAL::patchaul_hinst }.getFileNameW();
	GLOBAL::patchaul_path_a = WinWrap::Module{ GLOBAL::patchaul_hinst }.getFileNameA();

	GLOBAL::patchaul_config_path = GLOBAL::patchaul_path + L".json";

	//GLOBAL::config.load(GLOBAL::patchaul_config_path);
	config2.load(GLOBAL::patchaul_config_path);

	add_dll_ref.add_ref();

	ModulesData::update();

	{
		DWORD oldProtect;
		VirtualProtect(GLOBAL::executable_memory, sizeof(GLOBAL::executable_memory), PAGE_EXECUTE_READWRITE, &oldProtect);
	}
	
	InjectFunction_fastcall(GLOBAL::aviutl_base + OFS::AviUtl::InitAuf, InitAufBefore, 10);

	ExchangeFunction((HMODULE)GLOBAL::aviutl_base, cstr_kernel32_dll.get(), cstr_LoadLibraryA.get(), LoadLibraryAWrap);

	overwrite_resource();


#ifdef PATCH_SWITCH_SPLASH
	if (PATCH_SWITCHER_MEMBER(PATCH_SWITCH_SPLASH)) {
		patch::splash.init();

		patch::splash.set_phase(L"patch.aulの準備中", L"");
		patch::splash.start();
	}
#endif

#ifdef PATCH_SWITCH_CONSOLE
	patch::console.init();
#endif

#ifdef PATCH_SWITCH_EXCEPTION_LOG
	patch::exception_log();
#endif

#ifdef PATCH_SWITCH_SYSINFO_MODIFY
	patch::sysinfo_info_write();
#endif

#ifdef PATCH_SWITCH_ACCESS_KEY
	patch::access_key.init();
#endif

}

void init_t::InitAtExeditLoad() {
	mywindow.init();

#ifdef PATCH_SWITCH_THEME_CC
	patch::theme_cc.init();
#endif

#ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
	patch::tra_aviutlfilter.init();
#endif

#ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
	patch::exo_aviutlfilter.init();
#endif

#ifdef PATCH_SWITCH_EXO_SCENEIDX
	patch::exo_sceneidx.init();
#endif

#ifdef PATCH_SWITCH_EXO_TRACKPARAM
	patch::exo_trackparam.init();
#endif

#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
	patch::exo_trackminusval.init();
#endif

#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
	patch::exo_specialcolorconv.init();
#endif

#ifdef PATCH_SWITCH_DEBUGSTRING
	patch::override_debugstring();
#endif

#ifdef PATCH_SWITCH_TEXT_OP_SIZE
	patch::text_op_size.init();
#endif
	
#ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
	patch::ignore_media_param_reset.init();
#endif

#ifdef PATCH_SWITCH_SCROLL_OBJDLG
	patch::scroll_objdlg.init();
#endif

#ifdef PATCH_SWITCH_SUSIE_LOAD
	patch::susie_load.init();
#endif

#ifdef PATCH_SWITCH_HELPFUL_MSGBOX
	patch::helpful_msgbox.init();
#endif

	patch::setting_dialog();

	#ifdef PATCH_SWITCH_FAST
	patch::fast::fast.init();
	if (patch::fast::fast.is_enabled_i()) {
		#ifdef PATCH_SWITCH_FAST_GETPUTPIXELDATA
			patch::fast::getputpixeldata();
		#endif

		#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
			patch::fast_setting_dialog.init();
		#endif

		#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
			patch::fast_exeditwindow.init();
		#endif

		#ifdef PATCH_SWITCH_CL
			if (patch::fast::cl.init()) {
				if (patch::fast::cl.is_enabled_i()) {
					#ifdef PATCH_SWITCH_FAST_POLORTRANSFORM
						patch::fast::PolorTransform.init();
					#endif
					#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
						patch::fast::RadiationalBlur.init();
					#endif
					#ifdef PATCH_SWITCH_FAST_FLASH
						patch::fast::Flash.init();
					#endif
				}
			}
			else {
				patch_resource_message_w(PATCH_RS_PATCH_CANT_USE_CL, MB_TASKMODAL | MB_ICONEXCLAMATION);
			}
		#endif
	}
	#endif

#ifdef PATCH_SWITCH_UNDO
	patch::undo.init();
	if (patch::undo.is_enabled_i()) {
		#ifdef PATCH_SWITCH_UNDO_REDO
			patch::redo.init();
		#endif
	}
#endif

	//GLOBAL::config.store(GLOBAL::patchaul_config_path);
	config2.store(GLOBAL::patchaul_config_path);
}
		
void init_t::InitAufBefore() {
	patch::aviutl_wndproc_override.go();
}

BOOL WINAPI init_t::EnumResourceLanguagesA_Wrap(HMODULE hModule, LPCSTR lpType, LPCSTR lpName, ENUMRESLANGPROCA lpEnumFunc, LONG_PTR lParam) {
	ExchangeFunction((HMODULE)GLOBAL::aviutl_base, cstr_kernel32_dll.get(), cstr_EnumResourceLanguagesA.get(), EnumResourceLanguagesA);

	InitAtPatchLoaded();

	return FALSE;
}

HMODULE WINAPI init_t::LoadLibraryAWrap(LPCSTR lpLibFileName) {
	HMODULE ret = LoadLibraryA(lpLibFileName);
	if (ret == NULL)return NULL;

	LPCSTR filename = PathFindFileNameA(lpLibFileName);
	if (lstrcmpiA(filename, "exedit.auf") == 0) {
		GLOBAL::exedit_hmod = ret;
		auto filters = reinterpret_cast<AviUtl::GetFilterTableList_t>(GetProcAddress(ret, AviUtl::GetFilterTableListName))();
		if (strcmp(filters[0]->information, "拡張編集(exedit) version 0.92 by ＫＥＮくん") != 0) {
			MessageBoxW(NULL, L"patch.aul requires Exedit version *0.92*.\n拡張編集 version 0.92以外では動作しません．", L"patch.aul", MB_ICONEXCLAMATION);
			return ret;
		}
		original_func_init = std::exchange(filters[0]->func_init, func_initWrap);
		original_func_WndProc = std::exchange(filters[0]->func_WndProc, func_WndProcWrap);
#ifdef _DEBUG
		original_func_proc = std::exchange(filters[0]->func_proc, func_procWrap);
#endif
		InitAtExeditLoad();
	}
	else {
		static std::set<std::string> list = {
			"bakusoku.auf",
			"eclipse_fast.auf",
			"redo.auf",
		};

		std::string check = filename;
		std::transform(check.begin(), check.end(), check.begin(), [](auto c) { return std::tolower(c); });

		if (list.find(check) != list.end()) {
			FreeLibrary(ret);
			
			auto ret = patch_resource_message_w(PATCH_RS_PATCH_CONFLICT_PLUGIN, MB_TASKMODAL | MB_ICONINFORMATION | MB_YESNO, string_convert_A2W(filename));

			if (ret) {
				switch (*ret) {
				case IDYES:
					DeleteFileA(lpLibFileName);
				}
			}

			return NULL;
		}
	}
	return ret;
}
	
BOOL __cdecl init_t::func_WndProcWrap(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp) {
	switch (message) {
		#ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
		case WM_SIZING:
			if (auto ret = patch::exeditwindow_sizing.wndproc(wparam, lparam) == -1) break;
			else return ret;
		#endif
		case AviUtl::FilterPlugin::WindowMessage::Command:
			#ifdef PATCH_SWITCH_UNDO_REDO
			if (wparam == PATCH_EXEDITMENU_REDO) {
				patch::redo.run_redo();
				return TRUE;
			}
			#endif

	}
	return original_func_WndProc(hwnd, message, wparam, lparam, editp, fp);
}

BOOL __cdecl init_t::func_initWrap(AviUtl::FilterPlugin* fp) {
	if (original_func_init(fp) == FALSE) return FALSE;
	
	#ifdef PATCH_SWITCH_UNDO_REDO
		fp->exfunc->add_menu_item(fp, "やり直す", fp->hwnd, PATCH_EXEDITMENU_REDO, 'Y', AviUtl::ExFunc::AddMenuItemFlag::Ctrl);
	#endif

#ifdef PATCH_SWITCH_LUA
	patch::lua.init();
#endif
	
	return TRUE;
}

BOOL __cdecl init_t::func_procWrap(AviUtl::FilterPlugin* fp, AviUtl::FilterProcInfo* fpip) {
	//std::cout << " = = = = = = " << std::endl;
	#ifdef PATCH_SWITCH_ALPHA_BG
		patch::alpha_bg.func_proc(fp, fpip);
	#endif
	return original_func_proc(fp, fpip);
}
