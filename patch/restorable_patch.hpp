#pragma once
#include <vector>
#include <bit>

#include "util_magic.hpp"

class restorable_patch {
protected:
	std::vector<std::byte> data;
	std::uintptr_t address;

	bool state;

	restorable_patch(std::uintptr_t address, std::vector<std::byte>&& data) : data(std::move(data)), address(address), state(false) {}

public:
	restorable_patch(std::uintptr_t address, void* data, size_t length) : data(static_cast<std::byte*>(data), static_cast<std::byte*>(data) + length), address(address), state(false) {}

	void swap_data() {
		OverWriteOnProtectHelper h(address, data.size());
		for (size_t i = 0; i < data.size(); i++) {
			auto tmp = load_i8<std::byte>(address + i);
			store_i8(address + i, data[i]);
			data[i] = tmp;
		}
	}

	void switch_true_to_false() {
		swap_data();
	}

	void switch_false_to_true() {
		swap_data();
	}

	void switching(bool flag) {
		if (flag) {
			if (!state) {
				switch_false_to_true();
				state = flag;
			}
		}
		else {
			if (state) {
				switch_true_to_false();
				state = flag;
			}
		}
	}
};

class restorable_patch_function : public restorable_patch {
	static std::vector<std::byte> make_data(std::uintptr_t address, void* newfunc) {
		static std::vector<std::byte> ret;
		ret.resize(5);
		ret[0] = std::byte{ 0xe9 };
		store_i32(&ret[1], CalcNearJmp(address + 1, reinterpret_cast<i32>(newfunc)));
		return ret;
	}
public:
	restorable_patch_function(std::uintptr_t address, void* newfunc) : restorable_patch(address, std::move(make_data(address, newfunc))) {}
};

class restorable_patch_i8 : public restorable_patch {
	static std::vector<std::byte> make_data(i8 value) {
		static std::vector<std::byte> ret;
		ret.resize(1);
		store_i8(&ret[0], value);
		return ret;
	}
public:
	template<class T>
	restorable_patch_i8(std::uintptr_t address, T value) : restorable_patch(address, std::move(make_data((i8)value))) {}
};

class restorable_patch_i16 : public restorable_patch {
	static std::vector<std::byte> make_data(i16 value) {
		static std::vector<std::byte> ret;
		ret.resize(2);
		store_i16(&ret[0], value);
		return ret;
	}
public:
	template<class T>
	restorable_patch_i16(std::uintptr_t address, T value) : restorable_patch(address, std::move(make_data((i16)value))) {}
};

class restorable_patch_i32 : public restorable_patch {
	static std::vector<std::byte> make_data(i32 value) {
		static std::vector<std::byte> ret;
		ret.resize(4);
		store_i32(&ret[0], value);
		return ret;
	}
public:
	template<class T>
	restorable_patch_i32(std::uintptr_t address, T value) : restorable_patch(address, std::move(make_data((i32)value))) {}
};
