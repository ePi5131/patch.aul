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
#include <type_traits>
#include <utility>
#include <array>
#include <cstring>

using i8 = uint8_t;
using i16 = uint16_t;
using i32 = uint32_t;
using i64 = uint64_t;

template<std::integral T>
using i_seq = std::array<i8, sizeof(T)>;
template<typename T>
concept storable = std::integral<T> || std::is_same_v<T, i8>;
template<std::integral T1, storable T2>
inline void store_i(auto address, T2 value) {
	const auto src = static_cast<T1>(value);
	std::memcpy(reinterpret_cast<void*>(address), &src, sizeof(T1));
}
template<std::integral T>
inline void store_i(auto address, const i_seq<T>& value) {
	std::memcpy(reinterpret_cast<void*>(address), value.data(), sizeof(T));
}
template<typename T = i_seq<i8>>
inline void store_i8(auto address, const T& value) {
	store_i<i8>(address, value);
}
template<typename T = i_seq<i16>>
inline void store_i16(auto address, const T& value) {
	store_i<i16>(address, value);
}
template<typename T = i_seq<i32>>
inline void store_i32(auto address, const T& value) {
	store_i<i32>(address, value);
}
template<typename T = i_seq<i64>>
inline void store_i64(auto address, const T& value) {
	store_i<i64>(address, value);
}

template<storable T0, class T1>
inline T0 load_i(T1 address) {
	T0 ret;
	std::memcpy(&ret, reinterpret_cast<void*>(address), sizeof(T0));
	return ret;
}
template<class T0 = uint8_t, class T1>
inline T0 load_i8(T1 address) {
	static_assert(sizeof(T0) == sizeof(i8));
	return load_i<T0>(address);
}
template<class T0 = uint16_t, class T1>
inline T0 load_i16(T1 address) {
	static_assert(sizeof(T0) == sizeof(i16));
	return load_i<T0>(address);
}
template<class T0 = uint32_t, class T1>
inline T0 load_i32(T1 address) {
	static_assert(sizeof(T0) == sizeof(i32));
	return load_i<T0>(address);
}
template<class T0 = uint64_t, class T1>
inline T0 load_i64(T1 address) {
	static_assert(sizeof(T0) == sizeof(i64));
	return load_i<T0>(address);
}

template<std::integral TargetType, class T0, class T1>
inline T1 exchange_i(T0 address, T1&& value) {
	const auto ret = load_i<T1>(address);
	store_i<TargetType>(address, value);
	return ret;
}
template<class T0, class T1>
inline T1 exchange_i8(T0 address, T1&& value) {
	return exchange_i<i8>(address, std::forward<T1>(value));
}
template<class T0, class T1>
inline T1 exchange_i16(T0 address, T1&& value) {
	return exchange_i<i16>(address, std::forward<T1>(value));
}
template<class T0, class T1>
inline T1 exchange_i32(T0 address, T1&& value) {
	return exchange_i<i32>(address, std::forward<T1>(value));
}
template<class T0, class T1>
inline T1 exchange_i64(T0 address, T1&& value) {
	return exchange_i<i64>(address, std::forward<T1>(value));
}
