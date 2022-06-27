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

#include "patch_helpful_msgbox.hpp"

#ifdef PATCH_SWITCH_HELPFUL_MSGBOX
namespace patch {

	int WINAPI helpful_msgbox_t::MessageBoxAWrap(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
		std::optional<std::string> localText, localCaption;
		UINT id = 0;
		if (auto itr = strptr2rid.find(lpText); itr != strptr2rid.end()) {
			id = itr->second;
			localText = resource_string_a(id);
			if (localText) lpText = localText->c_str();
		}
		if (auto itr = strptr2rid.find(lpCaption); itr != strptr2rid.end()) {
			localCaption = resource_string_a(itr->second);
			if (localCaption)lpCaption = localCaption->c_str();
		}
		return HelpedMsg(hWnd, lpText, lpCaption, uType, id);
	}

	int helpful_msgbox_t::HelpedMsg(HWND hwnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, UINT id) {
		MSGBOXPARAMSA param{
			.cbSize = sizeof(param),
			.hwndOwner = hwnd,
			.hInstance = NULL,
			.lpszText = lpText,
			.lpszCaption = lpCaption,
			.dwStyle = uType | (id ? MB_HELP : 0),
			.dwContextHelpId = id,
			.lpfnMsgBoxCallback = [](LPHELPINFO hip) {
				const auto& itr = rid2md.find(hip->dwContextId);
				if (itr == rid2md.end()) {
					MessageBoxW(NULL, L"ヘルプは設定されていません", L"patch.aul", MB_ICONINFORMATION | MB_TASKMODAL);
					return;
				}
				const auto& data = itr->second;

				UrlConfirm(data.url);
			}
		};
		return MessageBoxIndirectA(&param);
	}

	void helpful_msgbox_t::UrlConfirm(std::wstring_view url) {
		auto ret = MessageBoxW(NULL, format(L"次のリンクを開こうとしています。同意しますか？\n{}", url).c_str(), L"patch.aul", MB_ICONQUESTION | MB_YESNO | MB_TASKMODAL);
		switch (ret) {
		case IDYES:
			ShellExecuteW(NULL, L"open", url.data(), NULL, NULL, SW_SHOW);
			break;
		default:
			break;
		}
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_HELPFUL_MSGBOX
