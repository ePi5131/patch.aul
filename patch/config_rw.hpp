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

#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <utility>
#include <array>
#include <unordered_map>
#include <functional>
#include <optional>

#include "json.h"

#include "util_others.hpp"

namespace config_type {

	struct ColorBGR {
		union {
			struct {
				uint8_t b, g, r;
				uint8_t valid;
			};
			uint32_t val;
		};
		ColorBGR() : val{} {}

		ColorBGR(uint32_t x) : val(x) {
			val = x;
			valid = 1;
		}
		ColorBGR(int b, int g, int r) :b(b), g(g), r(r), valid(1) {}
		ColorBGR(std::string_view x) {
			if (x.size() < 6) {
				valid = 0;
				return;
			}
			auto itr = x.data();
			std::from_chars(itr, itr + 2, r, 16);
			itr += 2;
			std::from_chars(itr, itr + 2, g, 16);
			itr += 2;
			std::from_chars(itr, itr + 2, b, 16);
			valid = 1;
		}
		inline static ColorBGR from_rgb(uint32_t x) {
			if (std::is_constant_evaluated()) {
				return (x >> 16 & 0xff) || (x & 0xff00) || (x << 16 & 0xff0000);
			}
			else {
				return _byteswap_ulong(x << 8);
			}
		}

		constexpr uint32_t to_col() const noexcept {
			return val & 0xffffff;
		}
		constexpr uint32_t to_col_rgb() const noexcept {
			if (std::is_constant_evaluated()) {
				return b << 16 | g << 8 | r;
			}
			else {
				return _byteswap_ulong(val) >> 8;
			}
		}
		std::string to_string() const {
			return "{:02x}{:02x}{:02x}"_fmt(r, g, b);
		}
		std::string to_jsonstring() const {
			return "\"{:02x}{:02x}{:02x}\""_fmt(r, g, b);
		}
		constexpr bool is_valid() const noexcept {
			return valid;
		}
		bool operator==(ColorBGR x) const {
			if (this->valid) {
				if (!x.valid)return false;
				else return (this->val & 0xffffff) == (x.val & 0xffffff);
			}
			else return !x.valid;
		}
	};

	// ColorBGR2つの配列、またはColorBGR1つ、またはnulloptを表す
	// [ "ffffff", "ffffff" ] や ["ffffff"]、"ffffff" など
	struct ColorBGR2_Opt {
		std::array<ColorBGR, 2> ary;
		ColorBGR2_Opt() : ary{} {}
		ColorBGR2_Opt(ColorBGR c1, ColorBGR c2) : ary{ c1,c2 } {}

		inline void load(json_value_s* value) {
			if (auto js = json_value_as_string(value)) {
				ary[0] = std::string_view(js->string, js->string_size);
				ary[1].valid = 0;
			}
			else if (auto ja = json_value_as_array(value)) {
				if (ja->length == 0) {
					ary[0].valid = 0;
				}
				else if (ja->length == 1) {
					if (auto js = json_value_as_string(ja->start->value)) {
						ary[0] = std::string_view(js->string, js->string_size);
						ary[1].valid = 0;
					}
				}
				else {
					auto v = ja->start;
					bool valid = true;
					for (size_t i = 0; i < 2; i++) {
						if (auto js = json_value_as_string(v->value)) {
							ary[i] = std::string_view(js->string, js->string_size);
							if (!ary[i].is_valid()) {
								valid = false;
								break;
							}
						}
						v = v->next;
					}
					if (!valid) ary[0].valid = 0;
				}
			}
		}

		std::string to_jsonstring() const {
			if (ary[0].is_valid()) {
				if (ary[1].is_valid())
					return "[ {}, {} ]"_fmt(ary[0].to_jsonstring(), ary[1].to_jsonstring());
				else
					return ary[0].to_jsonstring();
			}
			return std::string{};
		}

		constexpr bool has_value() const {
			return ary[0].is_valid();
		}

		constexpr int count() const {
			if (!ary[0].is_valid()) return 0;
			return 1 + ary[1].is_valid();
		}
	};

	// ColorBGRを2つ持つかnullopt
	struct ColorBGR2 {
		std::array<ColorBGR, 2> ary;
		ColorBGR2() : ary{} {}
		ColorBGR2(ColorBGR c1, ColorBGR c2) : ary{ c1,c2 } {}

		inline void load(json_value_s* value) {
			if (auto ja = json_value_as_array(value)) {
				if (ja->length > 1) {
					auto v = ja->start;
					bool valid = true;
					for (size_t i = 0; i < 2; i++) {
						if (auto js = json_value_as_string(v->value)) {
							ary[i] = std::string_view(js->string, js->string_size);
							if (!ary[i].is_valid()) {
								valid = false;
								break;
							}
						}
						v = v->next;
					}
					if (!valid) ary[0].valid = 0;
				}
			}
		}

		std::string to_jsonstring() const {
			if (ary[0].is_valid())
				return "[ {}, {} ]"_fmt(ary[0].to_jsonstring(), ary[1].to_jsonstring());
			return std::string{};
		}

		constexpr bool has_value() const {
			return ary[0].is_valid();
		}

		constexpr int count() const {
			if (!ary[0].is_valid()) return 0;
			return 1 + ary[1].is_valid();
		}
	};

	struct ColorBGR3 {
		std::array<ColorBGR, 3> ary;
		ColorBGR3() : ary{} {}
		ColorBGR3(ColorBGR c1, ColorBGR c2, ColorBGR c3) : ary{ c1,c2,c3 } {}

		inline void load(json_value_s* value) {
			if (auto ja = json_value_as_array(value)) {
				if (ja->length > 2) {
					auto v = ja->start;
					bool valid = true;
					for (size_t i = 0; i < 3; i++) {
						if (auto js = json_value_as_string(v->value)) {
							ary[i] = std::string_view(js->string, js->string_size);
							if (!ary[i].is_valid()) {
								valid = false;
								break;
							}
						}
						v = v->next;
					}
					if (!valid) ary[0].valid = 0;
				}
			}
		}
		constexpr bool has_value() const {
			return ary[0].is_valid();
		}
		std::string to_jsonstring() const {
			if (has_value())
				return "[ {}, {}, {} ]"_fmt(ary[0].to_jsonstring(), ary[1].to_jsonstring(), ary[2].to_jsonstring());
			return std::string{};
		}
	};

}

template<class T>
concept ConfigWriterHasToJsonString = requires (T x) {
	x.to_jsonstring();
};

template<class T>
concept ConfigWriterCanToStirng = requires(T x) {
	std::to_string(x);
};

template<class T>
concept ConfigReaderHasLoad = requires(T x) {
	x.load(std::declval<json_value_s*>());
};

class ConfigWriter {
	std::stringstream ss;

	struct KeyAndValue {
		std::string key, value;
	};

	std::vector<KeyAndValue> vkv;

	int level;

	inline static void WriteLevel(std::stringstream& ss, int level) {
		for (int i = 0; i < level; i++) ss << '\t';
	}
	// {\n を書く
	inline static void WriteBlockBegin(std::stringstream& ss) {
		ss << "{\n";
	}
	// } を書く
	inline static void WriteBlockEnd(std::stringstream& ss) {
		ss << "}";
	}
	// [["key" : ]] を書く
	inline static void WriteKey(std::stringstream& ss, std::string_view key) {
		ss << "\"" << key << "\" : ";
	}
	template <bool comma = false>
	inline static void WriteReturn(std::stringstream& ss) {
		if constexpr (comma) ss << ",\n";
		else ss << "\n";
	}

public:
	ConfigWriter(int level) : level(level) {}

	void append(std::string_view key, std::string_view value) {
		vkv.emplace_back(std::string(key), std::string(value));
	}

	template<ConfigWriterHasToJsonString T>
	void append(std::string_view key, const T& value) {
		vkv.emplace_back(std::string(key), value.to_jsonstring());
	}

	void append(std::string_view key, bool value) {
		vkv.emplace_back(std::string(key), value ? "true" : "false");
	}

	void append(std::string_view key, const RECT& value) {
		vkv.emplace_back(std::string(key), "[ {}, {}, {}, {} ]"_fmt(value.left, value.top, value.right, value.bottom));
	}

	template<size_t N>
	void append(std::string_view key, const std::array<int, N>& value) {
		if (value.size() > 0) {
			std::stringstream ss;
			ss << "[ ";
			const auto last = value.size() - 1;
			for (size_t i = 0; i < last; i++) {
				ss << value[i] << ", ";
			}
			ss << value[last] << " ]";
			vkv.emplace_back(std::string(key), ss.str());
		}
		else {
			vkv.emplace_back(std::string(key), "[]");
		}
	}

	template<ConfigWriterCanToStirng T>
	void append(std::string_view key, const T& value) {
		vkv.emplace_back(std::string(key), std::to_string(value));
	}

	template<class T>
	void append(std::string_view key, const std::optional<T>& value) {
		if (value) {
			append(key, *value);
		}
	}

	void write(std::stringstream& ss) const {
		auto s = vkv.size();
		if (s == 0)return;

		WriteBlockBegin(ss);

		for (int i = 0; i < s - 1; i++) {
			WriteLevel(ss, level + 1);
			WriteKey(ss, vkv[i].key);
			ss << vkv[i].value;
			WriteReturn<true>(ss);
		}
		WriteLevel(ss, level + 1);
		WriteKey(ss, vkv[s - 1].key);
		ss << vkv[s - 1].value;
		WriteReturn<false>(ss);

		WriteLevel(ss, level);
		WriteBlockEnd(ss);
	}

	int get_level() const { return level; }
};

class ConfigReader {
	json_value_s* value;

	using RegisterFunction = std::function<void(json_value_s*)>;
	using MapType = std::unordered_map<std::string, RegisterFunction>;

	MapType map;


public:
	ConfigReader(json_value_s* value) : value(value) {}

	void load() {
		auto obj = json_value_as_object(value);
		if (obj == nullptr)return;

		for (auto elm = obj->start; elm != nullptr; elm = elm->next) {
			if (auto itr = map.find(elm->name->string); itr != map.end()) {
				itr->second(elm->value);
			}
		}
	}

	template<class T>
	void regist(const std::string& str, T func) {
		map.try_emplace(str, func);
	}


	inline static bool load_variable(json_value_s* jv, bool& value) {
		if (json_value_is_false(jv)) {
			value = false;
			return true;
		}
		else if (json_value_is_true(jv)) {
			value = true;
			return true;
		}
		return false;
	}

	template<std::integral Int>
	inline static bool load_variable(json_value_s* jv, Int& value) {
		if (auto n = json_value_as_number(jv); n) {
			Int ret{};
			std::from_chars(n->number, n->number + n->number_size, ret);
			value = ret;
			return true;
		}
		return false;
	}

	template<std::floating_point Float>
	inline static bool load_variable(json_value_s* jv, Float& value) {
		if (auto n = json_value_as_number(jv); n) {
			Float ret;
			std::from_chars(n->number, n->number + n->number_size, ret);
			value = ret;
			return true;
		}
		return false;
	}

	inline static void load_variable(json_value_s* jv, config_type::ColorBGR& value) {
		if (auto s = json_value_as_string(jv); s) {
			value = config_type::ColorBGR(std::string_view(s->string, s->string_size));
		}
		else {
			value.valid = 0;
		}
	}

	inline static void load_variable(json_value_s* jv, std::optional<RECT>& value) {
		if (auto ja = json_value_as_array(jv); ja) {
			if (ja->length >= 4) {
				auto itr = ja->start;
				std::array<LONG, 4> buf;
				for (size_t i = 0; i < 4; i++, itr = itr->next) {
					if (auto jn = json_value_as_number(itr->value); jn) {
						std::from_chars(jn->number, jn->number + jn->number_size, buf[i]);
					}
					else {
						value = std::nullopt;
						return;
					}
				}
				value.emplace(RECT{
					.left = buf[0],
					.top = buf[1],
					.right = buf[2],
					.bottom = buf[3]
				});
			}
			else {
				value = std::nullopt;
			}
		}
		else {
			value = std::nullopt;
		}
	}

	template<size_t N>
	inline static void load_variable(json_value_s* jv, std::optional<std::array<int, N>>& value) {
		if (auto ja = json_value_as_array(jv); ja) {
			if (ja->length >= N) {
				auto itr = ja->start;
				std::array<int, N> buf;
				for (size_t i = 0; i < N; i++, itr = itr->next) {
					if (auto jn = json_value_as_number(itr->value); jn) {
						std::from_chars(jn->number, jn->number + jn->number_size, buf[i]);
					}
					else {
						value = std::nullopt;
						return;
					}
				}
				value = buf;
			}
			else {
				value = std::nullopt;
				return;
			}
		}
		else {
			int buf;
			if (auto jn = json_value_as_number(jv); jn) {
				std::from_chars(jn->number, jn->number + jn->number_size, buf);
			}
			else {
				value = std::nullopt;
				return;
			}
			value.emplace();
			auto& v = value.value();
			for (size_t i = 0; i < N; i++) {
				v[i] = buf;
			}
			return;
		}
	}

	template<ConfigReaderHasLoad T>
	inline static void load_variable(json_value_s* jv, T& value) {
		value.load(jv);
	}

	template<class T>
	inline static void load_variable(json_value_s* jv, std::optional<T>& value) {
		T t;
		if (load_variable(jv, t)) {
			value.emplace(std::move(t));
		}
		else {
			value = std::nullopt;
		}
	}

};

