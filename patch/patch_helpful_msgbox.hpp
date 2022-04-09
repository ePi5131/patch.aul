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
#include "macro.h"
#ifdef PATCH_SWITCH_HELPFUL_MSGBOX

#include <unordered_map>

#include <Windows.h>

#include "cryptostring.hpp"
#include "global.hpp"
#include "util_resource.hpp"
#include "util_magic.hpp"

namespace patch {
	// init at exedit load
	// メッセージボックスの拡張
	// メッセージの翻訳をしたりヘルプを表示したり
	inline class helpful_msgbox_t {

		struct MessageData {
			std::wstring url;
		};

		// メッセージ文字列のポインタからメッセージID(リソースID)に変換
		inline static std::unordered_map<LPCSTR, UINT> strptr2rid;

		// strptr2ridで拡張編集のオフセットのものだけ
		inline static std::pair<LPCSTR, UINT> strptr2rid_ofs_ee[]{
			{ reinterpret_cast<LPCSTR>(0x09da28), 10010 }, // "動画ファイルの読み込みに失敗しました\n対応していないフォーマットの可能性があります"
		};

		// メッセージIDをMessageDataに変換
		inline static std::unordered_map<UINT, MessageData> rid2md{
			{ 10010, { L"https://scrapbox.io/aviutl/**%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%AE%E8%AA%AD%E3%81%BF%E8%BE%BC%E3%81%BF%E3%81%AB%E5%A4%B1%E6%95%97%E3%81%97%E3%81%BE%E3%81%97%E3%81%9F_%E5%AF%BE%E5%BF%9C%E3%81%97%E3%81%A6%E3%81%84%E3%81%AA%E3%81%84%E3%83%95%E3%82%A9%E3%83%BC%E3%83%9E%E3%83%83%E3%83%88%E3%81%AE%E5%8F%AF%E8%83%BD%E6%80%A7%E3%81%8C%E3%81%82%E3%82%8A%E3%81%BE%E3%81%99" }},
		};

		static void UrlConfirm(std::wstring_view url);

		// ヘルプを表示できるメッセージボックス関数
		static int HelpedMsg(HWND hwnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, UINT id);

		static int WINAPI MessageBoxAWrap(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

	public:
		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_HELPFUL_MSGBOX); }

		void operator()() {
			if (!enabled())return;

			for (auto& p : strptr2rid_ofs_ee) {
				strptr2rid.emplace(GLOBAL::exedit_base + p.first, p.second);
			}

			ExchangeFunction(GLOBAL::exedit_hmod, cstr_user32_dll.get(), cstr_MessageBoxA.get(), &MessageBoxAWrap);
		}
	} helpful_msgbox;
} // namespace patch
#endif // ifdef PATCH_SWITCH_HELPFUL_MSGBOX
