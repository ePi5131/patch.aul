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
#include <string>
#include <optional>
#include <format>

#include <Windows.h>

#include "global_minimum.hpp"
#include "offset_address.hpp"

#include "util_others.hpp"

inline std::optional<std::wstring> resource_string_w(UINT rs_id) {
	auto hmod = load_i32<HMODULE>(GLOBAL::aviutl_base + OFS::AviUtl::current_resource_hmod);
	struct {
		const wchar_t* ptr;
		uint32_t padding = 0;
	}value;
	auto size = LoadStringW(hmod, rs_id, (LPWSTR)&value, 0);
	if (size <= 0) {
		hmod = GLOBAL::patchaul_hinst;
		size = LoadStringW(hmod, rs_id, (LPWSTR)&value, 0);
		if (size <= 0) {
			return std::nullopt;
		}
	}
	return std::wstring(value.ptr, size);
}

inline std::optional<std::string> resource_string_a(UINT rs_id) {
	auto hmod = load_i32<HMODULE>(GLOBAL::aviutl_base + OFS::AviUtl::current_resource_hmod);
	struct {
		const wchar_t* ptr;
		uint32_t padding;
	}value;
	auto size = LoadStringW(hmod, rs_id, (LPWSTR)&value, 0);
	if (size <= 0) {
		hmod = GLOBAL::patchaul_hinst;
		size = LoadStringW(hmod, rs_id, (LPWSTR)&value, 0);
		if (size <= 0) {
			return std::nullopt;
		}
	}

	auto asize = WideCharToMultiByte(CP_ACP, 0, value.ptr, size, nullptr, 0, nullptr, nullptr);
	std::string ret(asize, '\0');
	WideCharToMultiByte(CP_ACP, 0, value.ptr, size, ret.data(), asize, nullptr, nullptr);
	return ret;
}


template<class... Args>
inline std::optional<std::string> resource_format_a(UINT rs_id, Args&& ...args) {
	auto format_str = resource_string_a(rs_id);
	if (!format_str.has_value()) return std::nullopt;

	try {
		auto ret = std::vformat(format_str.value(), std::make_format_args(args...));
		return ret;
	}
	catch (const std::format_error&) {
		return std::nullopt;
	}
}

template<class... Args>
inline std::optional<std::wstring> resource_format_w(UINT rs_id, Args&& ...args) {
	auto format_str = resource_string_w(rs_id);
	if (!format_str.has_value()) return std::nullopt;

	try {
		auto ret = std::vformat(format_str.value(), std::make_format_args(args...));
		return ret;
	}
	catch (const std::format_error&) {
		return std::nullopt;
	}
}

inline void patch_resource_message_a(UINT rs_id, UINT uType) {
	auto str = resource_string_a(rs_id);
	if (str.has_value()) {
		MessageBoxA(NULL, str.value().c_str(), "patch.aul", uType);
	}
	else {
		MessageBoxW(NULL, L"resource error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
	}
}

inline void patch_resource_message_w(UINT rs_id, UINT uType) {
	auto str = resource_string_w(rs_id);
	if (str.has_value()) {
		MessageBoxW(NULL, str.value().c_str(), L"patch.aul", uType);
	}
	else {
		MessageBoxW(NULL, L"resource error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
	}
}

template<class... Args>
inline void patch_resource_message_a(UINT rs_id, UINT uType, Args&& ...args) {
	auto str = resource_string_a(rs_id);
	if (str.has_value()) {
		try {
			MessageBoxA(NULL, std::vformat(str.value(), std::make_format_args(args...)).c_str(), "patch.aul", uType);
		}
		catch (const std::format_error&) {
			MessageBoxW(NULL, L"format error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
		}
	}
	else {
		MessageBoxW(NULL, L"resource error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
	}
}

template<class... Args>
inline void patch_resource_message_w(UINT rs_id, UINT uType, Args&& ...args) {
	auto str = resource_string_w(rs_id);
	if (str.has_value()) {
		try {
			MessageBoxW(NULL, std::vformat(str.value(), std::make_wformat_args(args...)).c_str(), L"patch.aul", uType);
		}
		catch (const std::format_error&) {
			MessageBoxW(NULL, L"format error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
		}
	}
	else {
		MessageBoxW(NULL, L"resource error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
	}
}

class patch_resource_message_class_base {
public:
	virtual void fire() = 0;
	virtual ~patch_resource_message_class_base() {}
};

template<class... Args>
class patch_resource_message_format_class_w : public patch_resource_message_class_base {
	UINT rs_id, uType;
	std::tuple<Args...> args;

public:
	patch_resource_message_format_class_w(UINT rs_id, UINT uType, Args&& ...args) : rs_id(rs_id), uType(uType), args(std::forward(args)...) {}

	void fire() {
		auto str = resource_string_w(rs_id);
		if (str.has_value()) {
			try {
				MessageBoxW(NULL, std::vformat(str.value(), std::apply(std::make_format_args, args)).c_str(), L"patch.aul", uType);
			}
			catch (const std::format_error&) {
				MessageBoxW(NULL, L"format error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
			}
		}
		else {
			MessageBoxW(NULL, L"resource error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
		}
	}
};

class patch_resource_message_class_w : public patch_resource_message_class_base {
	UINT rs_id, uType;

public:
	patch_resource_message_class_w(UINT rs_id, UINT uType) : rs_id(rs_id), uType(uType) {}

	void fire() {
		auto str = resource_string_w(rs_id);
		if (str.has_value()) {
			try {
				MessageBoxW(NULL, str.value().c_str(), L"patch.aul", uType);
			}
			catch (const std::format_error&) {
				MessageBoxW(NULL, L"format error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
			}
		}
		else {
			MessageBoxW(NULL, L"resource error ({})"_fmt(rs_id).c_str(), L"patch.aul", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR);
		}
	}
};

inline std::vector<std::unique_ptr<patch_resource_message_class_base>> patch_resource_message_stack;
