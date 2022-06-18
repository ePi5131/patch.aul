#pragma once
#include <array>
#include <bit>

#include "util_magic.hpp"

template<std::size_t N, typename Derived>
class restorable_patch_base {
protected:
	std::array<std::byte, N> data;
	std::uintptr_t address;

	bool state;

	restorable_patch_base(std::uintptr_t address) : address(address), state(false) {}

public:
	template<typename T = char>
	requires (sizeof(std::array<std::byte, N>) == sizeof(std::array<T, N>))
	restorable_patch_base(std::uintptr_t address, const std::array<T, N>& data) : data(std::bit_cast<std::array<std::byte, N>>(data)), address(address), state(false) {}
	template<typename ...Args>
	requires (sizeof...(Args) == N)
	restorable_patch_base(std::uintptr_t address, Args&& ...args) : restorable_patch_base(address, { static_cast<char>(args)... }) {}
	void swap_data() {
		static_cast<Derived*>(this)->swap_data();
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
template<std::size_t N>
class restorable_patch : public restorable_patch_base<N, restorable_patch<N>> {
	using base = restorable_patch_base<N, restorable_patch<N>>;
public:
	using base::base;
	void swap_data() {
		OverWriteOnProtectHelper h(this->address, this->data.size());
		for (size_t i = 0; i < this->data.size(); i++) {
			const auto tmp = load_i8<std::byte>(this->address + i);
			store_i8(this->address + i, this->data[i]);
			this->data[i] = tmp;
		}
	}
};

class restorable_patch_function : public restorable_patch_base<5, restorable_patch_function> {
	using base = restorable_patch_base<5, restorable_patch_function>;
public:
	template<typename F>
	requires(std::is_pointer_v<F> && std::is_function_v<std::remove_pointer_t<F>>)
	restorable_patch_function(std::uintptr_t address, F newfunc)
		: base(address)
	{
		this->data[0] = std::byte{ 0xe9 };
		store_i32(&this->data[1], CalcNearJmp(address + 1, reinterpret_cast<i32>(newfunc)));
	}
	void swap_data() {
		OverWriteOnProtectHelper h(this->address, this->data.size());
		const auto tmp = load_i32(this->address + 1);
		i32 d;
		std::memcpy(&d, &this->data[1], sizeof(i32));
		store_i32(this->address, d);
		store_i32(&this->data[1], tmp);
	}
};

template<std::integral IntType>
class restorable_patch_i : public restorable_patch_base<sizeof(IntType), restorable_patch_i<IntType>> {
	using base = restorable_patch_base<sizeof(IntType), restorable_patch_i<IntType>>;
public:
	template<class T>
	restorable_patch_i(std::uintptr_t address, T value)
		: base(address)
	{
		store_i<IntType>(&this->data[0], value);
	}
	void swap_data() {
		OverWriteOnProtectHelper h(this->address, this->data.size());
		const auto tmp = load_i<IntType>(this->address);
		store_i<IntType>(this->address, this->data);
		store_i<IntType>(&this->data[0], tmp);
	}
};
using restorable_patch_i8 = restorable_patch_i<i8>;
using restorable_patch_i16 = restorable_patch_i<i16>;
using restorable_patch_i32 = restorable_patch_i<i32>;
