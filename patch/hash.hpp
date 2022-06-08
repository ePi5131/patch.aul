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
#include <iterator>

#include <boost/scope_exit.hpp>

#include <Windows.h>

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

	static uint32_t Sigma0(uint32_t x) {
		return std::rotr(x, 2) ^ std::rotr(x, 13) ^ std::rotr(x, 22);
	}

	static uint32_t Sigma1(uint32_t x) {
		return std::rotr(x, 6) ^ std::rotr(x, 11) ^ std::rotr(x, 25);
	}

	static uint32_t sigma0(uint32_t x) {
		return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
	}

	static uint32_t sigma1(uint32_t x) {
		return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10);
	}

	static uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) {
		return (x & y) ^ (~x & z);
	}

	static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
		return (x & y) ^ (y & z) ^ (z & x);
	}

public:

	std::byte data[32];

	SHA256(std::string_view filename) {
#if _DEBUG && 1 // 重いので
		std::fill(std::begin(data), std::end(data), std::byte{});
#else
		std::vector<uint8_t> buf;
		{
			auto hFile = CreateFileA(filename.data(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hFile == INVALID_HANDLE_VALUE) throw std::runtime_error("Failed to open file.");
			BOOST_SCOPE_EXIT_ALL(hFile) {
				CloseHandle(hFile);
			};

			DWORD sizehigh;
			auto sizelow = GetFileSize(hFile, &sizehigh);

			buf.resize(sizelow);
			DWORD read;
			if (!ReadFile(hFile, buf.data(), sizelow, &read, nullptr)) throw std::runtime_error("Failed to read file.");
		}
		uint32_t H[8];
		std::memcpy(H, H0, sizeof(H0));

		auto process = [&](const byte* msg) {
			uint32_t W[64];
			for (size_t t = 0; t < 16; t++) {
				W[t] = _byteswap_ulong(*reinterpret_cast<const unsigned long*>(msg + t * 4));
			}
			for (size_t t = 16; t < 64; t++) {
				W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];
			}
			auto a = H[0];
			auto b = H[1];
			auto c = H[2];
			auto d = H[3];
			auto e = H[4];
			auto f = H[5];
			auto g = H[6];
			auto h = H[7];
			for (size_t t = 0; t < 64; t++) {
				auto T1 = h + Sigma1(e) + Ch(e, f, g) + K[t] + W[t];
				auto T2 = Sigma0(a) + Maj(a, b, c);
				h = g;
				g = f;
				f = e;
				e = d + T1;
				d = c;
				c = b;
				b = a;
				a = T1 + T2;
			}
			H[0] += a;
			H[1] += b;
			H[2] += c;
			H[3] += d;
			H[4] += e;
			H[5] += f;
			H[6] += g;
			H[7] += h;
		};

		auto buf_size_d64 = buf.size() / 64;
		auto buf_size_m64 = buf.size() % 64;
		for (size_t i = 0; i < buf_size_d64; i++) process(&buf[i * 64]);

		uint8_t last_msg[64];
		std::memcpy(last_msg, buf.data() + buf_size_d64 * 64, buf_size_m64);
		last_msg[buf_size_m64] = 0x80;
		if (buf_size_m64 < 56) {
			std::memset(last_msg + buf_size_m64 + 1, 0, 58 - buf_size_m64);
			auto size = buf.size();
			last_msg[59] = static_cast<uint8_t>(size >> 29);
			last_msg[60] = static_cast<uint8_t>(size >> 21);
			last_msg[61] = static_cast<uint8_t>(size >> 13);
			last_msg[62] = static_cast<uint8_t>(size >>  5);
			last_msg[63] = static_cast<uint8_t>(size <<  3);
			process(last_msg);
		}
		else {
			std::memset(last_msg + buf_size_m64 + 1, 0, 63 - buf_size_m64);
			process(last_msg);

			std::memset(last_msg, 0, 59);
			auto size = buf.size();
			last_msg[59] = static_cast<uint8_t>(size >> 29);
			last_msg[60] = static_cast<uint8_t>(size >> 21);
			last_msg[61] = static_cast<uint8_t>(size >> 13);
			last_msg[62] = static_cast<uint8_t>(size >>  5);
			last_msg[63] = static_cast<uint8_t>(size <<  3);
			process(last_msg);
		}
		for (size_t i = 0; i < 8; i++) {
			*reinterpret_cast<unsigned long*>(data + i * 4) = _byteswap_ulong(H[i]);
		}
#endif
	}

	template<std::integral... T> requires(sizeof...(T) == 32)
	constexpr SHA256(T&&... list) noexcept : data{ static_cast<std::byte>(std::forward<T>(list))... } {}

	static std::optional<SHA256> make_opt(std::string_view filename) {
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
};

inline bool operator==(const SHA256& a, const SHA256& b) {
	return std::equal(std::begin(a.data), std::end(a.data), std::begin(b.data));
}
