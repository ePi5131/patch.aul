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

#include "patch_failed_sjis_msgbox.hpp"
#ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX


namespace patch {
	int __stdcall failed_sjis_msgbox_t::MessageBoxA_1(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
		auto end = lpCaption + strlen(lpCaption);
		// sjisに0x3fは含まれないのでこれでよい
		if (std::find(lpCaption, end, '?') != end) {
			lpText = str_new_failed_msg;
		}
		return MessageBoxA(hWnd, lpText, lpCaption, uType);
	}

	int __stdcall failed_sjis_msgbox_t::MessageBoxA_2(LPCSTR path, HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
		int ret = MessageBoxA_1(hWnd, lpText, path, uType);
		*(char*)path = '\0';
		return ret;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX
