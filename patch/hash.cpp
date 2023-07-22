#include "hash.hpp"

SHA256::SHA256(const std::string& filename) {
#if _DEBUG && 1 // 重いので
	std::fill(std::begin(data), std::end(data), std::byte{});
#else
	std::vector<uint8_t> buf;
	{
		auto hFile = CreateFileA(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) throw std::runtime_error("Failed to open file.");
		SCOPE_EXIT_AUTO{ [hFile] {
			CloseHandle(hFile);
		} };

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
			W[t] = std::byteswap(*reinterpret_cast<const unsigned long*>(msg + t * 4));
		}
		for (size_t t = 16; t < 64; t++) {
			W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];
		}
		auto [a, b, c, d, e, f, g, h] = H;
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
		last_msg[62] = static_cast<uint8_t>(size >> 5);
		last_msg[63] = static_cast<uint8_t>(size << 3);
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
		last_msg[62] = static_cast<uint8_t>(size >> 5);
		last_msg[63] = static_cast<uint8_t>(size << 3);
		process(last_msg);
	}
	for (size_t i = 0; i < 8; i++) {
		*reinterpret_cast<unsigned long*>(data + i * 4) = _byteswap_ulong(H[i]);
	}
#endif
}
