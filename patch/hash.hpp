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
#include <optional>
#include <bit>
#include <concepts>
#include <type_traits>
#include <stdexcept>
#include <ranges>
#include <algorithm>

#include <Windows.h>

#include "scope_exit.hpp"

struct SHA256 {
private:
	inline constexpr static uint32_t K[] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
	};

	inline constexpr static uint32_t H0[] = { 0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19 };

	inline static uint32_t Sigma0(uint32_t x) {
		return std::rotr(x, 2) ^ std::rotr(x, 13) ^ std::rotr(x, 22);
	}

	inline static uint32_t Sigma1(uint32_t x) {
		return std::rotr(x, 6) ^ std::rotr(x, 11) ^ std::rotr(x, 25);
	}

	inline static uint32_t sigma0(uint32_t x) {
		return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
	}

	inline static uint32_t sigma1(uint32_t x) {
		return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10);
	}

	inline static uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) {
		return (x & y) ^ (~x & z);
	}

	inline static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
		return (x & y) ^ (y & z) ^ (z & x);
	}

public:

	std::byte data[32];

	SHA256(const std::string& filename);

	template<std::ranges::input_range R>
	requires std::same_as<std::ranges::range_value_t<R>, std::byte>
	constexpr SHA256(R&& range) noexcept {
		std::ranges::copy(std::forward<R>(range), std::ranges::begin(data));
	}

	template<std::integral... T> requires(sizeof...(T) == std::extent_v<decltype(data)>)
	constexpr SHA256(T&&... list) noexcept : data{ static_cast<std::byte>(std::forward<T>(list))... } {}

	static std::optional<SHA256> make_opt(const std::string& filename) {
		try {
			return SHA256(filename);
		}
		catch (const std::runtime_error&) {
			return std::nullopt;
		}
	}

	std::string tostring() const {
		std::string ret;
		ret.reserve(64);

		static const char chs[] = "0123456789ABCDEF";

		for (auto b : data) {
			ret.append(1, chs[(std::to_integer<uint32_t>(b) >> 4) & 0xf]);
			ret.append(1, chs[std::to_integer<uint32_t>(b) & 0xf]);
		}

		return ret;
	}

	inline friend bool operator==(const SHA256& a, const SHA256& b) {
		return std::ranges::equal(a.data, b.data);
	}
};


struct FNV1_32 {
	uint32_t hash;
	
	static constexpr uint32_t basis = 0x811c9dc5;
	
	constexpr FNV1_32() : hash(basis) {}
	
	constexpr FNV1_32(const uint8_t* ptr, size_t len) : hash(basis) {
		const uint8_t* end = ptr + len;
 		for (; ptr != end; ++ptr) step(*ptr);
	}
	
	constexpr void step(uint8_t x) {
		// hash *= 0x01000193
		hash +=
			(hash << 1) +
			(hash << 4) +
			(hash << 7) +
			(hash << 8) +
			(hash << 24);
		hash ^= x;
	}

	template<class T>
	requires(sizeof(T) > 1 && std::is_trivial_v<T>)
	constexpr void step(T x) {
		uint8_t ax[sizeof(T)];
		std::memcpy(ax, std::addressof(x), sizeof(T));
		step(ax[0]);
		step(ax[1]);
		step(ax[2]);
		step(ax[3]);
	}

	constexpr operator uint32_t() const {
		return hash;
	}
};
