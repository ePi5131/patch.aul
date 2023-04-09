#include "patch_warning_duplicate_plugins.hpp"
#include <set>
#include <string>
#include <ranges>
#include <cctype>
#include <format>
#include <filesystem>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <winwrap.hpp>

#include "util_others.hpp"
#include "cryptostring.hpp"

namespace {
	struct fileinfo {
		std::filesystem::path fullpath;
		std::filesystem::file_time_type update_time;
	};

	// { filename, fullpath }
	using path_cache = std::unordered_map<std::filesystem::path, fileinfo>;
}

static bool warning_duplicate_message(
	const std::filesystem::path& first, const std::filesystem::file_time_type& first_time,
	const std::filesystem::path& second, const std::filesystem::file_time_type& second_time
) {
	using namespace std::string_view_literals;

	const auto r_main = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING);
	const auto r_content = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_CONTENT);
	const auto r_yes = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_YES);
	const auto r_ignore = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_IGNORE);
	const auto r_ex_ctl = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_EX_CTL);
	const auto r_cl_ctl = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_CL_CTL);
	const auto r_newer = resource_string_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_NEWER);

	const auto cmp_first_second_time = first_time <=> second_time;

	auto normalize_time = [](const std::filesystem::file_time_type& t) {
		const auto utc = std::chrono::clock_cast<std::chrono::utc_clock>(t);
		const auto local = std::chrono::local_time{ utc.time_since_epoch() };
		return std::chrono::floor<std::chrono::seconds>(local);
	};

	const auto link_first = std::format(
		LR"(<a href="1">{}</a> : {:%Y/%m/%d %H:%M:%S}{})",
		first.wstring(),
		normalize_time(first_time),
		cmp_first_second_time > 0 ? r_newer.value() : L""sv
	);
	const auto link_second = std::format(
		LR"(<a href="2">{}</a> : {:%Y/%m/%d %H:%M:%S}{})",
		second.wstring(),
		normalize_time(second_time),
		cmp_first_second_time < 0 ? r_newer.value() : L""sv
	);
	const auto r_ex_info = resource_format_w(PATCH_RS_PATCH_DUP_PLUGIN_WARNING_EX_INFO, link_first, link_second);

	enum ID : int {
		Yes,
		Ignore,
	};

	const TASKDIALOG_BUTTON vtdb[] = {
		{ static_cast<int>(ID::Yes), r_yes->c_str() },
		{ static_cast<int>(ID::Ignore), r_ignore->c_str() },
	};

	const struct CBDATA {
		const std::filesystem::path& first;
		const std::filesystem::path& second;
	} cbdata{
		first,
		second
	};
	const TASKDIALOGCONFIG tdc{
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
						open_explorer(cbdata.first.wstring());
						break;
					case L'2':
						open_explorer(cbdata.second.wstring());
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


static HMODULE warning_duplicate_common(path_cache& map, LPCSTR lpLibFileName) {
	static auto exe_dir = std::filesystem::path{ WinWrap::Module{GetModuleHandleA(NULL)}.getFileNameW() }.parent_path();

	const std::filesystem::path arg_path{ lpLibFileName };
	const auto arg_filename = arg_path.filename();

	auto safe_last_write_time = [](const std::filesystem::path& path) -> std::filesystem::file_time_type {
		try {
			return std::filesystem::last_write_time(path);
		}
		catch (const std::filesystem::filesystem_error&) {
			return {};
		}
	};

	if (const auto itr = map.find(arg_filename); itr != map.cend()) {
		if (itr->second.fullpath == arg_path) return LoadLibraryA(lpLibFileName);
		
		const auto update_time = safe_last_write_time(arg_path);

		if (warning_duplicate_message(std::filesystem::relative(itr->second.fullpath, exe_dir), itr->second.update_time, std::filesystem::relative(arg_path, exe_dir), update_time)) {
			return LoadLibraryA(lpLibFileName);
		}
		else {
			return NULL;
		}
	}
	else {
		const auto update_time = safe_last_write_time(arg_path);;
		map.try_emplace(arg_filename, arg_path, update_time);
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
