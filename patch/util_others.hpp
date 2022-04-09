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
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <format>
#include <concepts>

#include <Windows.h>

#include "offset_address.hpp"

inline HWND pid2hwnd(DWORD pid) {
	auto hwnd = FindWindow(NULL, NULL);
	while (hwnd) {
		if (!GetParent(hwnd) /*&& IsWindowVisible(hwnd)*/) {
			DWORD pid_tmp;
			GetWindowThreadProcessId(hwnd, &pid_tmp);
			if (pid == pid_tmp) return hwnd;
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	return hwnd;
}

template<class Func>
concept modify_menuitem_check_callback = requires(Func func) {
	{ func(std::declval<bool>()) } -> std::convertible_to<bool>;
};

template<modify_menuitem_check_callback Func>
void modify_menuitem_check(HMENU menu, UINT item, BOOL position, Func func) {
	MENUITEMINFOA info = { .cbSize = sizeof(MENUITEMINFO), .fMask = MIIM_STATE };
	GetMenuItemInfoA(menu, item, position, &info);
	info.fState = info.fState & ~MFS_CHECKED | (func(info.fState & MFS_CHECKED) ? MFS_CHECKED : 0);
	SetMenuItemInfoA(menu, item, position, &info);
}

template<class CharT>
struct format_literal_detail : private std::basic_string_view<CharT> {
	format_literal_detail(const CharT* str, std::size_t size) : std::basic_string_view<CharT>(str, size) {}

	template<class... Args>
	auto operator()(Args&& ...args) { return std::format(*this, args...); }
};

inline auto operator""_fmt(const char* str, std::size_t size) {
	return format_literal_detail(str, size);
}

inline auto operator""_fmt(const wchar_t* str, std::size_t size) {
	return format_literal_detail(str, size);
}


template<class OStream, class... Args>
inline auto format_to_os(OStream& ss, const std::basic_string_view<typename OStream::char_type> fmt, Args&& ...args) {
	return std::format_to(std::ostreambuf_iterator<OStream::char_type>(ss), fmt, std::forward<Args>(args)...);
}

inline auto get_local_time() {
	TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformation(&tzi);

	SYSTEMTIME st_u;
	GetSystemTime(&st_u);

	SYSTEMTIME st_l;
	SystemTimeToTzSpecificLocalTime(&tzi, &st_u, &st_l);

	return st_l;
}

// hh:mm:ss 形式のローカル時刻をもらう
inline auto get_local_time_string() {
	auto st_l = get_local_time();
	return "{:02}:{:02}:{:02}"_fmt(st_l.wHour, st_l.wMinute, st_l.wSecond);
}

// 編集プロジェクトの保存 を行う
void save_project(HWND hwnd_owner);

// ANSI -> UTF-16 LE
inline std::wstring string_convert_A2W(std::string_view str) {
	auto size = MultiByteToWideChar(CP_ACP, 0, str.data(), str.size(), nullptr, 0);
	std::wstring ret(size, '\0');
	MultiByteToWideChar(CP_ACP, 0, str.data(), str.size(), ret.data(), size);
	return ret;
}

// UTF-16 LE -> ANSI
inline std::string string_convert_W2A(std::wstring_view str) {
	auto size = WideCharToMultiByte(CP_ACP, 0, str.data(), str.size(), nullptr, 0, nullptr, nullptr);
	std::string ret(size, '\0');
	WideCharToMultiByte(CP_ACP, 0, str.data(), str.size(), ret.data(), size, nullptr, nullptr);
	return ret;
}

// UTF-8 -> UTF-16 LE
inline std::wstring string_convert_U2W(std::u8string_view str) {
	auto size = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCCH>(str.data()), str.size(), nullptr, 0);
	std::wstring ret(size, '\0');
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCCH>(str.data()), str.size(), ret.data(), size);
	return ret;
}

// UTF-16 LE -> UTF-8
inline std::u8string string_convert_W2U(std::wstring_view str) {
	auto size = WideCharToMultiByte(CP_UTF8, 0, str.data(), str.size(), nullptr, 0, nullptr, nullptr);
	std::u8string ret(size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, str.data(), str.size(), reinterpret_cast<LPSTR>(ret.data()), size, nullptr, nullptr);
	return ret;
}
