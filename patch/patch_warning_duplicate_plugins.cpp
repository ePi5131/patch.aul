#include "patch_warning_duplicate_plugins.hpp"
#include <set>
#include <string>
#include <ranges>
#include <cctype>
#include <format>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <winwrap.hpp>

#include "util_others.hpp"
#include "cryptostring.hpp"

static std::string get_lower_filename(LPCSTR path) {
	std::string ret{ PathFindFileNameA(path) };
	std::ranges::for_each(ret, [](char& c) { c = static_cast<char>(std::tolower(c)); });
	return ret;
}

static bool warning_duplicate_message(const std::wstring& first, const std::wstring& second) {
	const auto r_main = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING);
	const auto r_content = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_CONTENT);
	const auto r_yes = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_YES);
	const auto r_ignore = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_IGNORE);
	const auto r_ex_ctl = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_EX_CTL);
	const auto r_cl_ctl = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_CL_CTL);

	const auto link_first = std::format(LR"(<a href="1">{}</a>)", first);
	const auto link_second = std::format(LR"(<a href="2">{}</a>)", second);
	const auto r_ex_info = resource_format_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_EX_INFO, link_first, link_second);

	enum ID : int {
		Yes,
		Ignore,
	};

	TASKDIALOG_BUTTON vtdb[] = {
		{ static_cast<int>(ID::Yes), r_yes->c_str() },
		{ static_cast<int>(ID::Ignore), r_ignore->c_str() },
	};

	struct CBDATA {
		const std::wstring& first;
		const std::wstring& second;
	} cbdata{
		first,
		second
	};
	TASKDIALOGCONFIG tdc{
		.cbSize = sizeof(TASKDIALOGCONFIG),
		.hwndParent = NULL,
		.hInstance = GLOBAL::patchaul_hinst,
		.dwFlags = TDF_SIZE_TO_CONTENT | TDF_ENABLE_HYPERLINKS,
		.dwCommonButtons = 0,
		.pszWindowTitle = L"patch.aul",
		.pszMainIcon = TD_WARNING_ICON,
		.pszMainInstruction = r_main->c_str(),
		.pszContent = r_content->c_str(),
		.cButtons = std::size(vtdb),
		.pButtons = vtdb,
		.nDefaultButton = static_cast<int>(ID::Yes),
		.cRadioButtons = 0,
		.pRadioButtons = nullptr,
		.nDefaultRadioButton = NULL,
		.pszVerificationText = nullptr,
		.pszExpandedInformation = r_ex_info->c_str(),
		.pszExpandedControlText = r_ex_ctl->c_str(),
		.pszCollapsedControlText = r_cl_ctl->c_str(),
		.pszFooterIcon = NULL,
		.pszFooter = nullptr,
		.pfCallback = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData) -> HRESULT {
			const auto& cbdata = *reinterpret_cast<CBDATA*>(lpRefData);
			static cryptostring url{L"https://scrapbox.io/aviutl/"};
			switch (msg) {
				case TDN_HELP:
					web_confirm(url.get());
					return NULL;
				case TDN_HYPERLINK_CLICKED: {
					const auto tag = reinterpret_cast<LPCWSTR>(lParam);
					switch (tag[0]) {
					case L'1':
						open_explorer(cbdata.first);
						break;
					case L'2':
						open_explorer(cbdata.second);
						break;
					}
					break;
				}
			}
			return NULL;
		},
		.lpCallbackData = reinterpret_cast<LONG_PTR>(&cbdata),
		.cxWidth = 0
	};
		
	int nButton;
	TaskDialogIndirect(&tdc, &nButton, nullptr, nullptr);
	switch (nButton) {
	case ID::Yes:
		return false;
	case ID::Ignore:
		return true;
	default:
		MessageBoxW(NULL, L"なんで？", L"", 0);
		return true;
	}
}

namespace {
	// { filename, fullpath }
	using path_cache = std::unordered_map<std::string, std::string>;
}

static HMODULE warning_duplicate_common(path_cache& map, LPCSTR lpLibFileName) {
	auto filename = get_lower_filename(lpLibFileName);

	if (const auto itr = map.find(filename); itr != map.cend()) {
		if (itr->second == lpLibFileName) return LoadLibraryA(lpLibFileName);

		if (warning_duplicate_message(string_convert_A2W(itr->second), string_convert_A2W(lpLibFileName))) {
			return LoadLibraryA(lpLibFileName);
		}
		else {
			return NULL;
		}
	}
	else {
		map.try_emplace(filename, lpLibFileName);
		return LoadLibraryA(lpLibFileName);
	}
}

namespace patch {

HMODULE WINAPI warning_duplicate_t::LoadLibraryA_auc_wrap(LPCSTR lpLibFileName) {
	static path_cache map;
	return warning_duplicate_common(map, lpLibFileName);
}

HMODULE WINAPI warning_duplicate_t::LoadLibraryA_aui_wrap(LPCSTR lpLibFileName) {
	static path_cache map;
	return warning_duplicate_common(map, lpLibFileName);
}

HMODULE WINAPI warning_duplicate_t::LoadLibraryA_auo_wrap(LPCSTR lpLibFileName) {
	static path_cache map;
	return warning_duplicate_common(map, lpLibFileName);
}

HMODULE WINAPI warning_duplicate_t::LoadLibraryA_auf_wrap(LPCSTR lpLibFileName) {
	static path_cache map;
	return warning_duplicate_common(map, lpLibFileName);
}

HMODULE WINAPI warning_duplicate_t::LoadLibraryA_aul_wrap(LPCSTR lpLibFileName) {
	static path_cache map;
	return warning_duplicate_common(map, lpLibFileName);
}

}
