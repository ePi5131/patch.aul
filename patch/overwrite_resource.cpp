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

#include "overwrite_resource.hpp"
#include "patch_access_key.hpp"


//template <class Func, class... Args>inline std::invoke_result<Func, Args...> resource_func_wrap_helper(Func func) {}

HRSRC WINAPI overwrite_resource_t::FindResourceA_Wrap(HMODULE hModule, LPCSTR lpName, LPCSTR lpType) {
	if (hModule != GLOBAL::patchaul_hinst) return (*FindResourceA_org)(hModule, lpName, lpType);
	auto ret = (*FindResourceA_org)(hModule, lpName, lpType);
	if (ret) return ret;
	return (*FindResourceA_org)(default_default_resource_hmod, lpName, lpType);
}

HCURSOR WINAPI overwrite_resource_t::LoadCursorA_Wrap(HINSTANCE hInstance, LPCSTR lpCursorName) {
	if (hInstance != GLOBAL::patchaul_hinst) return (*LoadCursorA_org)(hInstance, lpCursorName);
	if ((*FindResourceA_org)(hInstance, lpCursorName, RT_CURSOR)) {
		return (*LoadCursorA_org)(hInstance, lpCursorName);
	}
	return (*LoadCursorA_org)(default_default_resource_hmod, lpCursorName);
}
HICON WINAPI overwrite_resource_t::LoadIconA_Wrap(HINSTANCE hInstance, LPCSTR lpIconName) {
	if (hInstance != GLOBAL::patchaul_hinst) return (*LoadIconA_org)(hInstance, lpIconName);
	if ((*FindResourceA_org)(hInstance, lpIconName, RT_ICON)) {
		return (*LoadIconA_org)(hInstance, lpIconName);
	}
	return (*LoadIconA_org)(default_default_resource_hmod, lpIconName);
}
HANDLE WINAPI overwrite_resource_t::LoadImageA_Wrap(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad) {
	if (hInst != GLOBAL::patchaul_hinst) return (*LoadImageA_org)(hInst, name, type, cx, cy, fuLoad);
	auto ret = (*LoadImageA_org)(hInst, name, type, cx, cy, fuLoad);
	if (ret) return ret;
	return (*LoadImageA_org)(default_default_resource_hmod, name, type, cx, cy, fuLoad);
}
HMENU WINAPI overwrite_resource_t::LoadMenuA_Wrap(HINSTANCE hInstance, LPCSTR lpMenuName) {
	HMENU ret;
	do {
		if (hInstance != GLOBAL::patchaul_hinst) {
			ret = (*LoadMenuA_org)(hInstance, lpMenuName);
			break;
		}
		if ((*FindResourceA_org)(hInstance, lpMenuName, RT_MENU)) {
			ret = (*LoadMenuA_org)(hInstance, lpMenuName);
			break;
		}
		ret = (*LoadMenuA_org)(default_default_resource_hmod, lpMenuName);
		break;
	} while (0);
	#ifdef PATCH_SWITCH_ACCESS_KEY
		return patch::access_key.modify(lpMenuName, ret);
	#else
		return ret;
	#endif
}
int WINAPI overwrite_resource_t::LoadStringA_Wrap(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int cchBufferMax) {
	if (hInstance != GLOBAL::patchaul_hinst) return (*LoadStringA_org)(hInstance, uID, lpBuffer, cchBufferMax);
	auto ret = (*LoadStringA_org)(hInstance, uID, lpBuffer, cchBufferMax);
	if (ret) return ret;
	return (*LoadStringA_org)(default_default_resource_hmod, uID, lpBuffer, cchBufferMax);
}

INT_PTR WINAPI overwrite_resource_t::DialogBoxParamA_Wrap(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam) {
	if (hInstance != GLOBAL::patchaul_hinst) return (*DialogBoxParamA_org)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	if ((*FindResourceA_org)(hInstance, lpTemplateName, RT_DIALOG)) {
		return (*DialogBoxParamA_org)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	}
	return (*DialogBoxParamA_org)(default_default_resource_hmod, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}

HWND WINAPI overwrite_resource_t::CreateDialogParamA_Wrap(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam) {
	if (hInstance != GLOBAL::patchaul_hinst) return (*CreateDialogParamA_org)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	if ((*FindResourceA_org)(hInstance, lpTemplateName, RT_DIALOG)) {
		return (*CreateDialogParamA_org)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);;
	}
	return (*CreateDialogParamA_org)(default_default_resource_hmod, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}

BOOL APIENTRY overwrite_resource_t::GetSaveFileNameA_Wrap(LPOPENFILENAMEA ofnap) {
	if (ofnap->hInstance != GLOBAL::patchaul_hinst || ofnap->lpTemplateName == NULL) return (*GetSaveFileNameA_org)(ofnap);
	auto fr_ret = (*FindResourceA_org)(GLOBAL::patchaul_hinst, ofnap->lpTemplateName, RT_DIALOG);
	if (fr_ret) return (*GetSaveFileNameA_org)(ofnap);
	ofnap->hInstance = default_default_resource_hmod;
	auto ret = (*GetSaveFileNameA_org)(ofnap);
	ofnap->hInstance = GLOBAL::patchaul_hinst;
	return ret;
}

BOOL APIENTRY overwrite_resource_t::GetOpenFileNameA_Wrap(LPOPENFILENAMEA ofnap) {
	if (ofnap->hInstance != GLOBAL::patchaul_hinst || ofnap->lpTemplateName == NULL) return (*GetOpenFileNameA_org)(ofnap);
	auto fr_ret = (*FindResourceA_org)(GLOBAL::patchaul_hinst, ofnap->lpTemplateName, RT_DIALOG);
	if (fr_ret) return (*GetOpenFileNameA_org)(ofnap);
	ofnap->hInstance = default_default_resource_hmod;
	auto ret = (*GetOpenFileNameA_org)(ofnap);
	ofnap->hInstance = GLOBAL::patchaul_hinst;
	return ret;
}
