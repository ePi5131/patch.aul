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

using i8 = uint8_t;
using i16 = uint16_t;
using i32 = uint32_t;
using i64 = uint64_t;

inline void store_i8(auto address, auto value) {
	*std::bit_cast<i8*>(address) = (i8)value;
}
inline void store_i16(auto address, auto value) {
	*std::bit_cast<i16*>(address) = (i16)value;
}
inline void store_i32(auto address, auto value) {
	*std::bit_cast<i32*>(address) = (i32)value;
}
inline void store_i64(auto address, auto value) {
	*std::bit_cast<i64*>(address) = (i64)value;
}

template<class T0 = uint8_t, class T1>
inline T0 load_i8(T1 address) {
	static_assert(sizeof(T0) == sizeof(i8));
	return *std::bit_cast<std::add_pointer_t<T0>>(address);
}
template<class T0 = uint16_t, class T1>
inline T0 load_i16(T1 address) {
	static_assert(sizeof(T0) == sizeof(i16));
	return *std::bit_cast<std::add_pointer_t<T0>>(address);
}
template<class T0 = uint32_t, class T1>
inline T0 load_i32(T1 address) {
	static_assert(sizeof(T0) == sizeof(i32));
	return *std::bit_cast<std::add_pointer_t<T0>>(address);
}
template<class T0 = uint64_t, class T1>
inline T0 load_i64(T1 address) {
	static_assert(sizeof(T0) == sizeof(i64));
	return *std::bit_cast<std::add_pointer_t<T0>>(address);
}

template<class T0, class T1>
inline T1 exchange_i8(T0 address, T1&& value) {
	return (T1)std::exchange(*std::bit_cast<i8*>(address), (i8)value);
}
template<class T0, class T1>
inline T1 exchange_i16(T0 address, T1&& value) {
	return (T1)std::exchange(*std::bit_cast<i16*>(address), (i16)value);
}
template<class T0, class T1>
inline T1 exchange_i32(T0 address, T1&& value) {
	return (T1)std::exchange(*std::bit_cast<i32*>(address), (i32)value);
}
template<class T0, class T1>
inline T1 exchange_i64(T0 address, T1&& value) {
	return (T1)std::exchange(*std::bit_cast<i64*>(address), (i64)value);
}
