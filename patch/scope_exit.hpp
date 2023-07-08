/*
    https://scrapbox.io/ePi5131/そろそろオレオレscope_exitを持っておきたい
	
    Copyright © 2023 ePi
    This work is free. You can redistribute it and/or modify it under the
    terms of the Do What The Fuck You Want To Public License, Version 2,
    as published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
*/

#pragma once
#include <functional>
#include <memory>
#include <type_traits>

template<class T>
class scope_exit {
	scope_exit(T) = delete;
};

template<std::invocable Func>
class scope_exit<Func> {
	Func func;
	bool enabled;

public:
	constexpr scope_exit(Func&& f) noexcept(std::is_nothrow_constructible_v<Func, Func>) : func{ std::forward<Func>(f) }, enabled(true) {}

	constexpr ~scope_exit() noexcept(std::is_nothrow_invocable_v<Func>) {
		if (enabled) std::invoke(func);
	}

	constexpr void release() noexcept {
		enabled = false;
	}

	constexpr void exit() noexcept(std::is_nothrow_invocable_v<Func>) {
		if (enabled) {
			func->invoke();
			enabled = false;
		}
	}

	scope_exit() = delete;
	scope_exit(const scope_exit&) = delete;
	scope_exit& operator=(const scope_exit&) = delete;
	scope_exit(scope_exit&&) = delete;
	scope_exit& operator=(scope_exit&&) = delete;
};

template<>
class scope_exit<void> {
	struct Base {
		constexpr virtual ~Base() = default;
		constexpr virtual void invoke() = 0;
	};

	template<std::invocable Func>
	struct Value : Base {
		Func value;
		constexpr Value(Func&& v) : value{ std::forward<Func>(v) } {}
		constexpr ~Value() = default;
		constexpr void invoke() noexcept(std::is_nothrow_invocable_v<Func>) { std::invoke(value); }
	};

	std::unique_ptr<Base> func;

public:
	constexpr scope_exit() noexcept : func(nullptr) {}

	template<std::invocable Func>
	constexpr scope_exit(Func&& f)
		noexcept(noexcept(std::make_unique<Value<Func>>(std::forward<Func>(f))))
		: func(std::make_unique<Value<Func>>(std::forward<Func>(f))) {}

	constexpr scope_exit(scope_exit&& s) noexcept {
		*this = std::move(s);
	}

	constexpr scope_exit& operator=(scope_exit&& right) noexcept {
		func = std::move(right.func);
		return *this;
	}

	constexpr ~scope_exit() noexcept(false) {
		if (func) func->invoke();
	}

	constexpr void release() noexcept {
		func = nullptr;
	}

	constexpr operator bool() const noexcept {
		return static_cast<bool>(func);
	}

	constexpr void exit() noexcept(false) {
		if (func) {
			func->invoke();
			func.reset();
		}
	}

	scope_exit(const scope_exit&) = delete;
	scope_exit& operator=(const scope_exit&) = delete;
};

#define SCOPE_EXIT_AUTO_IMPL2(a,b) a##b
#define SCOPE_EXIT_AUTO_IMPL(a,b) SCOPE_EXIT_AUTO_IMPL2(a,b)
#define SCOPE_EXIT_AUTO scope_exit SCOPE_EXIT_AUTO_IMPL(SCOPE_EXIT_AUTO_,__LINE__)
