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
#include "util_format.hpp"
#include <concepts>

#include <Windows.h>
#include <CommCtrl.h>

#include <intrin.h>

#include <aviutl/flag.hpp>

#include "global.hpp"
#include "resource.h"
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

template<class OStream, class... Args> requires std::is_same_v<typename OStream::char_type, char>
inline auto format_to_os(OStream& ss, const std::string_view fmt, Args&& ...args) {
	return std::vformat_to(std::ostreambuf_iterator<typename OStream::char_type>(ss), fmt, std::make_format_args(args...));
}

template<class OStream, class... Args> requires std::is_same_v<typename OStream::char_type, wchar_t>
inline auto format_to_os(OStream& ss, const std::wstring_view fmt, Args&& ...args) {
    return std::vformat_to(std::ostreambuf_iterator<typename OStream::char_type>(ss), fmt, std::make_wformat_args(args...));
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
	return format("{:02}:{:02}:{:02}", st_l.wHour, st_l.wMinute, st_l.wSecond);
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

enum class CPUCmdSet : uint32_t {
    F_TSC              = 1 << 0,
    F_MMX              = 1 << 1,
    F_SSE              = 1 << 2,
    F_SSE2             = 1 << 3,
    F_3DNOW            = 1 << 4,
    F_3DNOWEXT         = 1 << 5,
    F_SMT              = 1 << 6,
    F_64BIT            = 1 << 7,
    F_SSE3             = 1 << 8,
    F_SSSE3            = 1 << 9,
    F_SSE4A            = 1 << 10,
    F_SSE41            = 1 << 12,
    F_SSE42            = 1 << 13,
    F_AESNI            = 1 << 14,
    F_AVX              = 1 << 15,
    F_AVX2             = 1 << 16,
    F_FMA4             = 1 << 11,
    F_AVX512F          = 1 << 17,
    F_AVX512DQ         = 1 << 18,
    F_AVX512_IFMA      = 1 << 19,
    F_AVX512PF         = 1 << 20,
    F_AVX512ER         = 1 << 21,
    F_AVX512CD         = 1 << 22,
    F_AVX512BW         = 1 << 23,
    F_AVX512VL         = 1 << 24,
    F_AVX512_VBMI      = 1 << 25,
    F_AVX512_VPOPCNTDQ = 1 << 26,
    F_AVX512_4VNNIW    = 1 << 27,
    F_AVX512_4FMAPS    = 1 << 28,
};
template<>struct AviUtl::detail::flag::ops_def<CPUCmdSet>:std::true_type{};

/*
    参考: https://www.timbreofprogram.info/blog/archives/951
*/
inline CPUCmdSet get_CPUCmdSet() {
    static CPUCmdSet ret = {};
    static bool inited = false;

    if (inited) return ret;

    int cpuinfo[4];

    __cpuid(cpuinfo, 0x00000000);
    auto basicmax = cpuinfo[0];

    __cpuid(cpuinfo, 0x80000000);
    auto extendmax = cpuinfo[0];

    __cpuid(cpuinfo, 0x00000001);
    if (cpuinfo[3] & (1u << 4)) ret |= CPUCmdSet::F_TSC;
    if (cpuinfo[3] & (1u << 23)) ret |= CPUCmdSet::F_MMX;
    if (cpuinfo[3] & (1u << 25)) ret |= CPUCmdSet::F_SSE;
    if (cpuinfo[3] & (1u << 26)) ret |= CPUCmdSet::F_SSE2;
    if (cpuinfo[3] & (1u << 28)) ret |= CPUCmdSet::F_SMT;

    if (cpuinfo[2] & (1u << 0)) ret |= CPUCmdSet::F_SSE3;
    if (cpuinfo[2] & (1u << 9)) ret |= CPUCmdSet::F_SSSE3;
    if (cpuinfo[2] & (1u << 19)) ret |= CPUCmdSet::F_SSE41;
    if (cpuinfo[2] & (1u << 20)) ret |= CPUCmdSet::F_SSE42;
    if (cpuinfo[2] & (1u << 25)) ret |= CPUCmdSet::F_AESNI;
    if (cpuinfo[2] & (1u << 28)) ret |= CPUCmdSet::F_AVX;

    if (static_cast<unsigned int>(extendmax) < 0x80000000u) return ret;


    __cpuid(cpuinfo, 0x80000001);
    if (cpuinfo[3] & (1u << 31)) ret |= CPUCmdSet::F_3DNOW;
    if (cpuinfo[3] & (1u << 30)) ret |= CPUCmdSet::F_3DNOWEXT;
    if (cpuinfo[3] & (1u << 29)) ret |= CPUCmdSet::F_64BIT;

    if (cpuinfo[2] & (1u << 6)) ret |= CPUCmdSet::F_SSE4A;
    if (cpuinfo[2] & (1u << 16)) ret |= CPUCmdSet::F_FMA4;


    if (basicmax < 7) return ret;

    __cpuidex(cpuinfo, 7, 0);
    if (cpuinfo[1] & (1u << 5)) ret |= CPUCmdSet::F_AVX2;
    if (cpuinfo[1] & (1u << 16)) ret |= CPUCmdSet::F_AVX512F;
    if (cpuinfo[1] & (1u << 17)) ret |= CPUCmdSet::F_AVX512DQ;
    if (cpuinfo[1] & (1u << 21)) ret |= CPUCmdSet::F_AVX512_IFMA;
    if (cpuinfo[1] & (1u << 26)) ret |= CPUCmdSet::F_AVX512PF;
    if (cpuinfo[1] & (1u << 27)) ret |= CPUCmdSet::F_AVX512ER;
    if (cpuinfo[1] & (1u << 28)) ret |= CPUCmdSet::F_AVX512CD;
    if (cpuinfo[1] & (1u << 30)) ret |= CPUCmdSet::F_AVX512BW;
    if (cpuinfo[1] & (1u << 31)) ret |= CPUCmdSet::F_AVX512VL;

    if (cpuinfo[2] & (1u << 1)) ret |= CPUCmdSet::F_AVX512_VBMI;
    if (cpuinfo[2] & (1u << 14)) ret |= CPUCmdSet::F_AVX512_VPOPCNTDQ;

    if (cpuinfo[3] & (1u << 2)) ret |= CPUCmdSet::F_AVX512_4VNNIW;
    if (cpuinfo[3] & (1u << 3)) ret |= CPUCmdSet::F_AVX512_4FMAPS;

    inited = true;
    return ret;
}
