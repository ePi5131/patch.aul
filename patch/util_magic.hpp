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
#include <algorithm>

#include <Windows.h>

#include "global.hpp"

#include "util_int.hpp"
#include "util_pe.hpp"

inline i32 CalcNearJmp(i32 address, i32 jmp_address) {
	return jmp_address - (address + 4);
}

class OverWriteOnProtectHelper {
	uintptr_t m_address, m_size;
	DWORD m_oldProtect;
public:
	template<class T>
	OverWriteOnProtectHelper(T address, uintptr_t size) noexcept : m_address((uintptr_t)address), m_size(size) {
		VirtualProtect(reinterpret_cast<LPVOID>(m_address), m_size, PAGE_EXECUTE_READWRITE, &m_oldProtect);
	}
	~OverWriteOnProtectHelper() noexcept {
		VirtualProtect(reinterpret_cast<LPVOID>(m_address), m_size, m_oldProtect, &m_oldProtect);
	}

	template<class T = i_seq<i8>>
	void store_i8(auto address, const T& value) const {
		::store_i8(m_address + address, value);
	}
	template<class T = i_seq<i16>>
	void store_i16(auto address, const T& value) const {
		::store_i16(m_address + address, value);
	}
	template<class T = i_seq<i32>>
	void store_i32(auto address, const T& value) const {
		::store_i32(m_address + address, value);
	}
	template<class T = i_seq<i64>>
	void store_i64(auto address, const T& value) const {
		::store_i64(m_address + address, value);
	}

	template<class T0 = uint8_t, class T1>
	T0 load_i8(T1 address) const {
		return ::load_i8<T0>(m_address + address);
	}
	template<class T0 = uint16_t, class T1>
	T0 load_i16(T1 address) const {
		return ::load_i16<T0>(m_address + address);
	}
	template<class T0 = uint32_t, class T1>
	T0 load_i32(T1 address) const {
		return ::load_i32<T0>(m_address + address);
	}
	template<class T0 = uint64_t, class T1>
	T0 load_i64(T1 address) const {
		return ::load_i64<T0>(m_address + address);
	}

	void replaceNearJmp(i32 offset, void* jmp_address) {
		store_i32(offset, CalcNearJmp(m_address + offset, reinterpret_cast<i32>(jmp_address)));
	}

	auto address() const {
		return m_address;
	}
	auto address(uintptr_t ofs) const {
		return m_address + ofs;
	}
};

/// <summary>
/// ニアージャンプ・コールを書き換える
/// </summary>
/// <param name="address">書き換える対象のアドレス</param>
/// <param name="jmp_address">代わりに飛ばして欲しいアドレス</param>
inline void ReplaceNearJmp(i32 address, void* jmp_address) {
	OverWriteOnProtectHelper(address, 4).replaceNearJmp(0, jmp_address);
}

// 既存の関数を破壊して，自分の関数を実行する
inline class ReplaceFunction_t {
	static const int asm_size = 5;

public:
	// 乗っ取りたい関数があるアドレス,ジャンプさせる関数のポインタ,元の関数の内容が返る場所
	template<class T, size_t N = asm_size, std::enable_if_t<N >= asm_size, std::nullptr_t> = nullptr>
	void operator()(T address, const void* function, std::byte(&original)[N]) noexcept {
		auto adr = std::bit_cast<i32>(address);
		OverWriteOnProtectHelper h(adr, asm_size);
		std::copy(adr, adr + N, original);
		store_i8(adr, '\xe9'); // jmp rel32
		store_i32(adr + 1, CalcNearJmp(address + 1, (i32)function));
	}

	template<class T>
	void operator()(T address, const void* function) noexcept {
		auto adr = std::bit_cast<i32>(address);
		OverWriteOnProtectHelper h(adr, asm_size);
		store_i8(adr, '\xe9'); // jmp rel32
		store_i32(adr + 1, CalcNearJmp(adr + 1, (i32)function));
	}
} ReplaceFunction;


// 乗っ取りたいモジュール, 乗っ取る関数があるDLLのファイル名, 乗っ取る関数の名前, 新しい関数へのポインタ
inline BOOL ExchangeFunction(HMODULE hModule, std::string_view modname, std::string_view funcname, void* function) noexcept {
	auto ptr = search_import(hModule, modname, funcname);
	if (!ptr)return FALSE;
	DWORD flOldProtect;
	if (VirtualProtect(ptr, 4, PAGE_EXECUTE_READWRITE, &flOldProtect) == FALSE) return FALSE;
	store_i32(ptr, function);
	return VirtualProtect(ptr, 4, flOldProtect, &flOldProtect);
}

/// <summary>
/// 指定したアドレスの関数の直前に、自分の関数を実行する
/// 実行後元の関数に戻る
/// __stdcall,__cdecl専用
/// </summary>
/// <param name="address"> 中断したい関数のアドレス </param>
/// <param name="function"> 挿入する関数 </param>
/// <param name="asm_word_n"> 命令単位に合った数(7以上) </param>
/// <returns> TRUE </returns>
inline bool InjectFunction_stdcall(uint32_t address, const void* function, size_t asm_word_n) noexcept {
	std::byte* cursor = GLOBAL::executable_memory_cursor;

	store_i8(cursor, '\xb8'); // mov eax, (i32)
	store_i32(cursor + 1, function);
	store_i16(cursor + 5, { 0xff, 0xd0 }); // call eax
	
	std::copy((std::byte*)address, (std::byte*)address + asm_word_n, cursor + 7);
	store_i16(cursor + asm_word_n + 7, { 0xff, 0x25 }); // jmp [(i32)]
	store_i32(cursor + asm_word_n + 9, cursor + asm_word_n + 13);
	store_i32(cursor + asm_word_n + 13, address + asm_word_n);
	GLOBAL::executable_memory_cursor += asm_word_n + 17;

	{
		OverWriteOnProtectHelper protect(address, 7);
		store_i8(address, '\xb8'); // mov eax, (i32)
		store_i32(address + 1, cursor);
		store_i16(address + 5, { 0xff, 0xe0 }); // call eax
	}
	return TRUE;
}

/// <summary>
/// 指定したアドレスの関数の直前に、自分の関数を実行する
/// 実行後元の関数に戻る
/// __cdecl専用 (__stdcallと一緒だけど)
/// </summary>
/// <param name="address"> 中断したい関数のアドレス </param>
/// <param name="function"> 挿入する関数 </param>
/// <param name="asm_word_n"> 命令単位に合った数(7以上) </param>
/// <returns> TRUE </returns>
inline bool InjectFunction_cdecl(uint32_t address, const void* function, size_t asm_word_n) noexcept {
	return InjectFunction_stdcall(address, function, asm_word_n);
}

/// <summary>
/// 指定したアドレスの関数の直前に、自分の関数を実行する
/// 実行後元の関数に戻る
/// __fastcall専用
/// </summary>
/// <param name="address"> 中断したい関数のアドレス </param>
/// <param name="function"> 挿入する関数 </param>
/// <param name="asm_word_n"> 命令単位に合った数(7以上) </param>
/// <returns> TRUE </returns>
inline bool InjectFunction_fastcall(uint32_t address, void(*func)(), size_t asm_word_n) {
	if (asm_word_n < 7)return false;
	OverWriteOnProtectHelper helper(address, asm_word_n);

	auto bridge = GLOBAL::executable_memory_cursor;

	store_i16(bridge, { 0x51, 0x52 }); // PUSH ECX; PUSH EDX
	store_i8(bridge + 2, '\xb8'); // MOV EAX, (i32)
	store_i32(bridge + 3, func);
	store_i16(bridge + 7, { 0xff, 0xd0 }); // CALL EAX
	store_i16(bridge + 9, { 0x5a, 0x59 }); // POP EDX; POP ECX
	std::copy((std::byte*)address, (std::byte*)address + asm_word_n, bridge + 11);
	store_i16(bridge + asm_word_n + 11, { 0xff, 0x25 }); // JMP (i32)
	store_i32(bridge + asm_word_n + 13, bridge + asm_word_n + 17);
	store_i32(bridge + asm_word_n + 17, address + asm_word_n);
	GLOBAL::executable_memory_cursor += asm_word_n + 21;

	store_i8(address, '\xb8'); // MOV EAX,
	store_i32(address + 1, bridge);
	store_i16(address + 5, { 0xff, 0xe0 }); // JMP EAX

	return true;
}
