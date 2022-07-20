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
#include <bit>
#include <vector>
#include <array>
#include <concepts>

/// <summary>
/// バイナリに生の文字列を残さないためのクラス
/// </summary>
template <class CharT, size_t N>
class cryptostring {
	template<class CharT> struct KeyV {};
	template<> struct KeyV<char> { inline static constexpr char value = 0b00101101i8; };
	template<> struct KeyV<wchar_t> { inline static constexpr wchar_t value = 0b1001010110001100i16; };

	template<std::integral T>
	[[nodiscard]] constexpr static T bit_rotate_l(const T x, size_t a) {
		auto ux = std::bit_cast<std::make_unsigned_t<T>>(x);
		return std::bit_cast<T>(std::rotl(ux, a));
	}

	template<std::integral T>
	[[nodiscard]] constexpr static T bit_rotate_r(const T x, size_t a) {
		auto ux = std::bit_cast<std::make_unsigned_t<T>>(x);
		return std::bit_cast<T>(std::rotr(ux, a));
	}

	std::array<CharT, N> ary;
	bool decrypted = false;
public:
	constexpr cryptostring(const CharT(&str)[N]) {
		for (size_t i = 0; i < N; i++) ary[i] = str[i] ^ bit_rotate_l(KeyV<CharT>::value, (i * 3) % (CHAR_BIT * sizeof(CharT)));
	}

	[[nodiscard]] CharT* get() {
		if (!decrypted) [[unlikely]] {
			for (size_t i = 0; i < N; i++) ary[i] ^= bit_rotate_l(KeyV<CharT>::value, (i * 3) % (CHAR_BIT * sizeof(CharT)));
			decrypted = true;
		}
		return ary.data();
	}

	void re_encrypt() {
		if (decrypted) [[likely]] {
			for (size_t i = 0; i < N; i++) ary[i] ^= bit_rotate_l(KeyV<CharT>::value, (i * 3) % (CHAR_BIT * sizeof(CharT)));
			decrypted = false;
		}
	}
};

template <class CharT, size_t N>
inline constexpr cryptostring<CharT, N> make_cryptostring(const CharT(&str)[N]) {
	return cryptostring<CharT, N>(str);
}

inline cryptostring cstr_kernel32_dll("KERNEL32.DLL");
inline cryptostring cstr_user32_dll("USER32.DLL");
inline cryptostring cstr_EnumResourceLanguagesA("EnumResourceLanguagesA");
inline cryptostring cstr_LoadLibraryA("LoadLibraryA");
inline cryptostring cstr_LoadLibraryW("LoadLibraryW");
inline cryptostring cstr_MessageBoxA("MessageBoxA");
inline cryptostring cstr_gdi32_dll("GDI32.DLL");
inline cryptostring cstr_GetGlyphOutlineW("GetGlyphOutlineW");
inline cryptostring cstr_DeleteObject("DeleteObject");
inline cryptostring cstr_CreateFontIndirectW("CreateFontIndirectW");
inline cryptostring cstr_GetModuleHandleA("GetModuleHandleA");
inline cryptostring cstr_GetModuleHandleW("GetModuleHandleW");
inline cryptostring cstr_Module32First("Module32First");
inline cryptostring cstr_Module32FirstW("Module32FirstW");
inline cryptostring cstr_Module32Next("Module32Next");
inline cryptostring cstr_Module32NextW("Module32NextW");
inline cryptostring cstr_EnumFontFamiliesA("EnumFontFamiliesA");
