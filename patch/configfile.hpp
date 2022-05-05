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
#include <memory>
#include <array>
#include <optional>
#include <charconv>
#include <concepts>
#include <format>
#include <boost/scope_exit.hpp>

#include <Windows.h>

#include "json.h"
#include "macro.h"
#include "util_others.hpp"
#include "util_resource.hpp"

#ifdef PATCH_SWITCH_CONSOLE
void console_get_setting_rect(std::optional<RECT>& rect);
#endif

class Config {
	inline static constexpr bool key_equal(json_object_element_s* elm, std::string_view key) {
		return std::equal(elm->name->string, elm->name->string + elm->name->string_size, key.data(), key.data() + key.size());
	}

public:
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

		inline bool load(json_object_element_s* elm, std::string_view key) {
			if (key_equal(elm, key)) {
				if (auto js = json_value_as_string(elm->value)) {
					ary[0] = std::string_view(js->string, js->string_size);
					ary[1].valid = 0;
				}
				else if (auto ja = json_value_as_array(elm->value)) {
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
				return true;
			}
			return false;
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
			if(!ary[0].is_valid()) return 0;
			return 1 + ary[1].is_valid();
		}
	};

	// ColorBGRを2つ持つかnullopt
	struct ColorBGR2 {
		std::array<ColorBGR, 2> ary;
		ColorBGR2() : ary{} {}
		ColorBGR2(ColorBGR c1, ColorBGR c2) : ary{ c1,c2 } {}

		inline bool load(json_object_element_s* elm, std::string_view key) {
			if (key_equal(elm, key)) {
				if (auto ja = json_value_as_array(elm->value)) {
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
				return true;
			}
			return false;
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
			if(!ary[0].is_valid()) return 0;
			return 1 + ary[1].is_valid();
		}
	};
		
	struct ColorBGR3 {
		std::array<ColorBGR, 3> ary;
		ColorBGR3() : ary{} {}
		ColorBGR3(ColorBGR c1, ColorBGR c2, ColorBGR c3) : ary{ c1,c2,c3 } {}

		inline bool load(json_object_element_s* elm, std::string_view key) {
			if (key_equal(elm, key)) {
				if (auto ja = json_value_as_array(elm->value)) {
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
				return true;
			}
			return false;
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

	struct to_json {
		HANDLE hFile;
		int level;
		size_t& value_i;
		size_t value_n;
		inline void operator()(const char* key, ColorBGR val) {
			if (val.is_valid()) {
				WriteLevel(hFile, level);
				WriteKey(hFile, key);
				MyWriteFile(hFile, val.to_jsonstring());
				WriteReturn(hFile, ++value_i < value_n);
			}
		}
		inline void operator()(const char* key, const ColorBGR2& val) {
			if (val.has_value()) {
				WriteLevel(hFile, level);
				WriteKey(hFile, key);
				MyWriteFile(hFile, val.to_jsonstring());
				WriteReturn(hFile, ++value_i < value_n);
			}
		}
		inline void operator()(const char* key, const ColorBGR2_Opt& val) {
			if (val.has_value()) {
				WriteLevel(hFile, level);
				WriteKey(hFile, key);
				MyWriteFile(hFile, val.to_jsonstring());
				WriteReturn(hFile, ++value_i < value_n);
			}
		}
		inline void operator()(const char* key, const ColorBGR3& val) {
			if (val.has_value()) {
				WriteLevel(hFile, level);
				WriteKey(hFile, key);
				MyWriteFile(hFile, val.to_jsonstring());
				WriteReturn(hFile, ++value_i < value_n);
			}
		}
		inline void operator()(const char* key, const std::optional<int>& val) {
			if (val.has_value()) {
				WriteLevel(hFile, level);
				WriteKey(hFile, key);
				MyWriteFile(hFile, std::to_string(*val));
				WriteReturn(hFile, ++value_i < value_n);
			}
		}
		inline void operator()(const char* key, bool val) {
			WriteLevel(hFile, level);
			WriteKey(hFile, key);
			MyWriteFile(hFile, val ? "true" : "false");
			WriteReturn(hFile, ++value_i < value_n);
		}
		inline void operator()(const char* key, const std::optional<double>& val) {
			if (val) {
				WriteLevel(hFile, level);
				WriteKey(hFile, key);
				MyWriteFile(hFile, std::to_string(*val));
				WriteReturn(hFile, ++value_i < value_n);
			}
		}
		inline void operator()(const char* key, const std::optional<RECT>& val) {
			if (val) {
				WriteLevel(hFile, level);
				WriteKey(hFile, key);

				MyWriteFile(hFile, "[ {}, {}, {}, {} ]"_fmt(val->left, val->top, val->right, val->bottom));
				WriteReturn(hFile, ++value_i < value_n);
			}
		}

		template<size_t N>
		inline void operator()(const char* key, const std::optional<std::array<int, N>>& val) {
			if (val) {
				WriteLevel(hFile, level);
				WriteKey(hFile, key);
				auto& v = val.value();
				MyWriteFile(hFile, "[ {}"_fmt(v[0]));
				for (size_t i = 1; i < N; i++) {
					MyWriteFile(hFile, ", {}"_fmt(v[i]));
				}
				MyWriteFile(hFile, " ]");
				WriteReturn(hFile, ++value_i < value_n);
			}
		}

		template<class T>
		inline void operator()(T& t) {
			if (auto x = t.store(hFile, level + 1)) {
				WriteReturn(hFile, (value_i += x) < value_n);
			}
		}
	};

private:

	inline static const char tab[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

	inline static void MyWriteFile(HANDLE hFile, std::string_view str) {
		DWORD ignore;
		WriteFile(hFile, str.data(), str.length(), &ignore, NULL);
	}
	inline static void WriteLevel(HANDLE hFile, int level) {
		DWORD ignore;
		WriteFile(hFile, tab, level, &ignore, NULL);
	}
	// {\n を書く
	inline static void WriteBlockBegin(HANDLE hFile) {
		MyWriteFile(hFile, "{\n");
	}
	// <level>} を書く
	inline static void WriteBlockEnd(HANDLE hFile, int level) {
		WriteLevel(hFile, level);
		MyWriteFile(hFile, "}");
	}
	// [["key" : ]] を書く
	inline static void WriteKey(HANDLE hFile, std::string_view key) {
		MyWriteFile(hFile, "\"");
		MyWriteFile(hFile, key.data());
		MyWriteFile(hFile, "\" : ");
	}
	inline static void WriteComma(HANDLE hFile) {
		MyWriteFile(hFile, ",");
	}
	inline static void WriteReturn(HANDLE hFile, bool comma) {
		if (comma) MyWriteFile(hFile, ",\n");
		else MyWriteFile(hFile, "\n");
	}
	template <bool comma = false>
	inline static void WriteReturn(HANDLE hFile) {
		if constexpr (comma) MyWriteFile(hFile, ",\n");
		else MyWriteFile(hFile, "\n");
	}

	inline static bool load_variable(json_object_element_s* elm, std::string_view key, bool& value) {
		if (key_equal(elm, key)) {
			if (json_value_is_false(elm->value)) value = false;
			else if(json_value_is_true(elm->value)) value = true;
			return true;
		}
		return false;
	}
	inline static bool load_variable(json_object_element_s* elm, std::string_view key, std::optional<bool>& value) {
		if (key_equal(elm, key)) {
			if (json_value_is_false(elm->value)) value = false;
			else if (json_value_is_true(elm->value)) value = true;
			return true;
		}
		return false;
	}

	template<std::integral Int>
	inline static bool load_variable(json_object_element_s* elm, std::string_view key, std::optional<Int>& value) {
		if (key_equal(elm, key)) {
			if (auto jv = json_value_as_number(elm->value); jv) {
				Int ret{};
				std::from_chars(jv->number, jv->number + jv->number_size, ret);
				value = ret;
			}
			return true;
		}
		return false;
	}

	template<std::floating_point Float>
	inline static bool load_variable(json_object_element_s* elm, std::string_view key, std::optional<Float>& value) {
		if (key_equal(elm, key)) {
			if (auto jv = json_value_as_number(elm->value); jv) {
				Float ret;
				std::from_chars(jv->number, jv->number + jv->number_size, ret);
				value = ret;
			}
			return true;
		}
		return false;
	}

	inline static bool load_variable(json_object_element_s* elm, std::string_view key, ColorBGR& value) {
		if (key_equal(elm, key)) {
			if (auto jv = json_value_as_string(elm->value); jv) {
				value = ColorBGR(std::string_view(jv->string, jv->string_size));
			}
			else {
				value.valid = 0;
			}
			return true;
		}
		return false;
	}
	inline static bool load_variable(json_object_element_s* elm, std::string_view key, std::optional<RECT>& value) {
		if (key_equal(elm, key)) {
			if (auto ja = json_value_as_array(elm->value); ja) {
				if (ja->length >= 4) {
					auto itr = ja->start;
					std::array<LONG, 4> buf;
					for (size_t i = 0; i < 4; i++, itr = itr->next) {
						if (auto jn = json_value_as_number(itr->value); jn) {
							std::from_chars(jn->number, jn->number + jn->number_size, buf[i]);
						}
						else {
							value = std::nullopt;
							return true;
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
					return true;
				}
			}
			else {
				value = std::nullopt;
				return true;
			}
		}
		return false;
	}
	template<size_t N>
	inline static bool load_variable(json_object_element_s* elm, std::string_view key, std::optional<std::array<int, N>>& value) {
		if (key_equal(elm, key)) {
			if (auto ja = json_value_as_array(elm->value); ja) {
				if (ja->length >= N) {
					auto itr = ja->start;
					std::array<int, N> buf;
					for (size_t i = 0; i < N; i++, itr = itr->next) {
						if (auto jn = json_value_as_number(itr->value); jn) {
							std::from_chars(jn->number, jn->number + jn->number_size, buf[i]);
						}
						else {
							value = std::nullopt;
							return true;
						}
					}
					value = buf;
				}
				else {
					value = std::nullopt;
					return true;
				}
			}
			else {
				int buf;
				if (auto jn = json_value_as_number(elm->value); jn) {
					std::from_chars(jn->number, jn->number + jn->number_size, buf);
				}
				else {
					value = std::nullopt;
					return true;
				}
				value.emplace();
				auto& v = value.value();
				for (size_t i = 0; i < N; i++) {
					v[i] = buf;
				}
				return true;
			}
		}
		return false;
	}

public:
#ifdef PATCH_SWITCH_CONSOLE
	struct {
		bool visible = false;
		std::optional<RECT> rect = std::nullopt;

		inline static const char name[] = "console";
		inline static const char key_visible[] = "visible";
		inline static const char key_rect[] = "rect";

		void load(json_value_s* value) {
			auto obj = json_value_as_object(value);
			if (obj == nullptr)return;

			for (auto elm = obj->start; elm != nullptr; elm = elm->next) {
				if (load_variable(elm, key_visible, visible)) continue;
				if (load_variable(elm, key_rect, rect)) continue;
			}
		}

		int store(HANDLE hFile, int level) {
			WriteLevel(hFile, level++);
			WriteKey(hFile, name);
			WriteBlockBegin(hFile);

			console_get_setting_rect(rect);

			const auto value_n = value_count();
			size_t value_i = 0;

			to_json tj{hFile, level, value_i, value_n};
			tj(key_visible, visible);
			tj(key_rect, rect);

			WriteBlockEnd(hFile, --level);

			return value_count();
		}

		size_t value_count() const {
			size_t ret = 1;
			if (rect.has_value())ret++;
			return ret;
		}
	} console;
#endif

#ifdef PATCH_SWITCH_THEME_CC
	struct {
		inline static const char name[] = "theme_cc";

		struct {
			inline static const char name[] = "layer";

			std::optional<int> height_large;
			std::optional<int> height_medium;
			std::optional<int> height_small;
			ColorBGR2_Opt link_col;
			ColorBGR2_Opt clipping_col;
			ColorBGR2_Opt lock_col;
			std::optional<double> hide_alpha;
			//std::optional<int> name_height;

			inline static const char key_large[] = "height_large";
			inline static const char key_medium[] = "height_medium";
			inline static const char key_small[] = "height_small";
			inline static const char key_link_col[] = "link_col";
			inline static const char key_clipping_col[] = "clipping_col";
			inline static const char key_lock_col[] = "lock_col";
			inline static const char key_hide_alpha[] = "hide_alpha";
			//inline static const char key_name_height[] = "name_height";

			void load(json_value_s* value) {
				auto obj = json_value_as_object(value);
				if (obj == nullptr)return;

				for (auto elm = obj->start; elm != nullptr; elm = elm->next) {
					if (load_variable(elm, key_large, height_large)) continue;
					if (load_variable(elm, key_medium, height_medium)) continue;
					if (load_variable(elm, key_small, height_small)) continue;
					if (link_col.load(elm, key_link_col)) continue;
					if (clipping_col.load(elm, key_clipping_col)) continue;
					if (lock_col.load(elm, key_lock_col)) continue;
					if (load_variable(elm, key_hide_alpha, hide_alpha)) continue;
				}
			}

			size_t store(HANDLE hFile, int level) {
				const auto value_n = value_count();
				if (!value_n) return 0;

				WriteLevel(hFile, level++);
				WriteKey(hFile, name);
				WriteBlockBegin(hFile);

				size_t value_i = 0;

				to_json tj{ hFile, level, value_i, value_n };
				tj(key_large, height_large);
				tj(key_medium, height_medium);
				tj(key_small, height_small);
				tj(key_link_col, link_col);
				tj(key_clipping_col, clipping_col);
				tj(key_lock_col, lock_col);
				tj(key_hide_alpha, hide_alpha);

				WriteBlockEnd(hFile, --level);

				return value_n;
			}

			constexpr size_t value_count() const noexcept {
				return
					height_large.has_value() +
					height_medium.has_value() +
					height_small.has_value() +
					link_col.has_value() +
					clipping_col.has_value() +
					lock_col.has_value() + 
					hide_alpha.has_value();
			}
		} layer;

		struct {
			inline static const char name[] = "object";

			ColorBGR3 media_col;
			ColorBGR3 mfilter_col;
			ColorBGR3 audio_col;
			ColorBGR3 afilter_col;
			ColorBGR3 control_col;
			ColorBGR3 inactive_col;
			ColorBGR clipping_col;
			std::optional<int> clipping_height;
			std::optional<std::array<int, 3>> midpt_size;
			ColorBGR2 name_col;
			
			inline static const char key_media_col[] = "media_col";
			inline static const char key_mfilter_col[] = "mfilter_col";
			inline static const char key_audio_col[] = "audio_col";
			inline static const char key_afilter_col[] = "afilter_col";
			inline static const char key_control_col[] = "control_col";
			inline static const char key_inactive_col[] = "inactive_col";
			inline static const char key_clipping_col[] = "clipping_col";
			inline static const char key_clipping_height[] = "clipping_height";
			inline static const char key_midpt_size[] = "midpt_size";
			inline static const char key_name_col[] = "name_col";

			void load(json_value_s* value) {
				auto obj = json_value_as_object(value);
				if (obj == nullptr)return;

				for (auto elm = obj->start; elm != nullptr; elm = elm->next) {
					if (media_col.load(elm, key_media_col)) continue;
					if (mfilter_col.load(elm, key_mfilter_col)) continue;
					if (audio_col.load(elm, key_audio_col)) continue;
					if (afilter_col.load(elm, key_afilter_col)) continue;
					if (control_col.load(elm, key_control_col)) continue;
					if (inactive_col.load(elm, key_inactive_col)) continue;
					if (load_variable(elm, key_clipping_col, clipping_col)) continue;
					if (load_variable(elm, key_clipping_height, clipping_height)) continue;
					if (load_variable(elm, key_midpt_size, midpt_size)) continue;
					if (name_col.load(elm, key_name_col)) continue;
				}
			}

			size_t store(HANDLE hFile, int level) {
				const auto value_n = value_count();
				if (!value_n) return 0;

				WriteLevel(hFile, level++);
				WriteKey(hFile, name);
				WriteBlockBegin(hFile);

				size_t value_i = 0;

				to_json tj{ hFile, level, value_i, value_n };
				tj(key_media_col, media_col);
				tj(key_mfilter_col, mfilter_col);
				tj(key_audio_col, audio_col);
				tj(key_afilter_col, afilter_col);
				tj(key_control_col, control_col);
				tj(key_inactive_col, inactive_col);
				tj(key_clipping_col, clipping_col);
				tj(key_clipping_height, clipping_height);
				tj(key_midpt_size, midpt_size);
				tj(key_name_col, name_col);
					
				WriteBlockEnd(hFile, --level);

				return value_n;
			}

			constexpr size_t value_count() const {
				return
					media_col.has_value() +
					mfilter_col.has_value() +
					audio_col.has_value() +
					afilter_col.has_value() +
					control_col.has_value() +
					inactive_col.has_value() +
					clipping_col.is_valid() +
					clipping_height.has_value() +
					midpt_size.has_value() +
					name_col.has_value();
			}
		
		} object;

		struct {
			inline static const char name[] = "timeline";

			ColorBGR2 scale_col;
			ColorBGR2 bpm_grid_col;

			inline static const char key_scale_col[] = "scale_col";
			inline static const char key_bpm_grid_col[] = "bpm_grid_col";


			void load(json_value_s* value) {
				auto obj = json_value_as_object(value);
				if (obj == nullptr)return;

				for (auto elm = obj->start; elm != nullptr; elm = elm->next) {
					if (scale_col.load(elm, key_scale_col)) continue;
					if (bpm_grid_col.load(elm, key_bpm_grid_col)) continue;
				}
			}

			size_t store(HANDLE hFile, int level) {
				const auto value_n = value_count();
				if (!value_n) return 0;

				WriteLevel(hFile, level++);
				WriteKey(hFile, name);
				WriteBlockBegin(hFile);

				size_t value_i = 0;

				to_json tj{ hFile, level, value_i, value_n };
				tj(key_scale_col, scale_col);
				tj(key_bpm_grid_col, bpm_grid_col);

				WriteBlockEnd(hFile, --level);

				return value_n;
			}

			constexpr size_t value_count() const {
				return
					scale_col.has_value() +
					bpm_grid_col.has_value();
			}
		} timeline;

		void load(json_value_s* value) {
			auto obj = json_value_as_object(value);
			if (obj == nullptr)return;

			for (auto elm = obj->start; elm != nullptr; elm = elm->next) {
				if (key_equal(elm, layer.name)) layer.load(elm->value);
				else if (key_equal(elm, object.name)) object.load(elm->value);
				else if (key_equal(elm, timeline.name)) timeline.load(elm->value);
			}
		}
		
		int store(HANDLE hFile, int level) {
			const auto value_n = value_count();
			if (!value_n) return 0;
			size_t value_i = 0;

			WriteLevel(hFile, level++);
			WriteKey(hFile, name);
			WriteBlockBegin(hFile);

			if (auto x = layer.store(hFile, level))
				WriteReturn(hFile, (value_i += x) < value_n);

			if(auto x = object.store(hFile, level))
				WriteReturn(hFile, (value_i += x) < value_n);

			if (auto x = timeline.store(hFile, level))
				WriteReturn(hFile, (value_i += x) < value_n);

			WriteBlockEnd(hFile, --level);

			return value_n;
		}

		constexpr size_t value_count() const {
			return
				layer.value_count() +
				object.value_count() +
				timeline.value_count();
		}

	} theme_cc;
#endif

	struct {
		inline static const char name[] = "switch";

		#ifdef PATCH_SWITCH_ACCESS_KEY
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_ACCESS_KEY, false)
		#endif
		#ifdef PATCH_SWITCH_SUSIE_LOAD
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_SUSIE_LOAD, true)
		#endif
		#ifdef PATCH_SWITCH_SPLASH
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_SPLASH, true)
		#endif
		#ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_TRA_AVIUTL_FILTER, true)
		#endif
		#ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_EXO_AVIUTL_FILTER, true)
		#endif
		#ifdef PATCH_SWITCH_EXO_SCENEIDX
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_EXO_SCENEIDX, true)
		#endif
		#ifdef PATCH_SWITCH_EXO_TRACKPARAM
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_EXO_TRACKPARAM, true)
		#endif
		#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_EXO_TRACK_MINUSVAL, true)
		#endif
		#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_EXO_SPECIALCOLORCONV, true)
		#endif
		#ifdef PATCH_SWITCH_TEXT_OP_SIZE
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_TEXT_OP_SIZE, true)
		#endif
		#ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET, false)
		#endif
		#ifdef PATCH_SWITCH_FONT_DIALOG
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_FONT_DIALOG, true)
		#endif
		#ifdef PATCH_SWITCH_SCROLL_OBJDLG
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_SCROLL_OBJDLG, true)
		#endif
		#ifdef PATCH_SWITCH_ALPHA_BG
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_ALPHA_BG, true)
		#endif
		#ifdef PATCH_SWITCH_HELPFUL_MSGBOX
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_HELPFUL_MSGBOX, true)
		#endif
		#ifdef PATCH_SWITCH_THEME_CC
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_THEME_CC, true)
		#endif
		#ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_EXEDITWINDOW_SIZING, true)
		#endif
		#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_SETTINGDIALOG_MOVE, true)
		#endif
		
		#ifdef PATCH_SWITCH_UNDO
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_UNDO, true)
			#ifdef PATCH_SWITCH_UNDO_REDO
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_UNDO_REDO, true, undo.redo)
				#ifdef PATCH_SWITCH_UNDO_REDO_SHIFT
					PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_UNDO_REDO_SHIFT, true, undo.redo.shift)
				#endif
			#endif
		#endif

		#ifdef PATCH_SWITCH_CONSOLE
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_CONSOLE, true)
			PATCH_SWITCHER_DEFINE_EX(console_escape, true, console.escape)
			PATCH_SWITCHER_DEFINE_EX(console_input, false, console.input)
			PATCH_SWITCHER_DEFINE_EX(console_debug_string, true, console.debug_string)
			PATCH_SWITCHER_DEFINE_EX(console_debugstring_time, true, console.debug_string.time)
		#endif
				
		#ifdef PATCH_SWITCH_LUA
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_LUA, true)
			#ifdef PATCH_SWITCH_LUA_ENV
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_LUA_ENV, false, lua.env)
			#endif
			#ifdef PATCH_SWITCH_LUA_RAND
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_LUA_RAND, true, lua.rand)
			#endif
			#ifdef PATCH_SWITCH_LUA_RANDEX
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_LUA_RANDEX, true, lua.randex)
			#endif
			#ifdef PATCH_SWITCH_LUA_GETVALUE
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_LUA_GETVALUE, true, lua.getvalue)
			#endif
			#ifdef PATCH_SWITCH_LUA_PATH
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_LUA_PATH, false, lua.path)
			#endif
		#endif

		#ifdef PATCH_SWITCH_FAST
			PATCH_SWITCHER_DEFINE(PATCH_SWITCH_FAST, true)
			#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_FAST_EXEDITWINDOW, true, fast.exeditwindow)
				std::optional<int> fast_exeditwindow_step = 0; inline static constexpr const char key_fast_exeditwindow_step[] = "fast.exeditwindow.step";
			#endif
			#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_FAST_SETTINGDIALOG, true, fast.settingdialog)
			#endif
			
			#ifdef PATCH_SWITCH_CL
				PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_CL, true, fast.cl)
				#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
					PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_FAST_RADIATIONALBLUR, true, fast.radiationalblur)
				#endif
				#ifdef PATCH_SWITCH_FAST_POLORTRANSFORM
					PATCH_SWITCHER_DEFINE_EX(PATCH_SWITCH_FAST_POLORTRANSFORM, true, fast.polortransform)
				#endif
			#endif
		#endif

		
		void load(json_value_s* value) {
			auto obj = json_value_as_object(value);
			if (obj == nullptr)return;

			for (auto elm = obj->start; elm != nullptr; elm = elm->next) {
				#ifdef PATCH_SWITCH_ACCESS_KEY
					PATCH_SWITCHER_IF(PATCH_SWITCH_ACCESS_KEY)
				#endif
				#ifdef PATCH_SWITCH_SUSIE_LOAD
					PATCH_SWITCHER_IF(PATCH_SWITCH_SUSIE_LOAD)
				#endif
				#ifdef PATCH_SWITCH_SPLASH
					PATCH_SWITCHER_IF(PATCH_SWITCH_SPLASH)
				#endif
				#ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
					PATCH_SWITCHER_IF(PATCH_SWITCH_TRA_AVIUTL_FILTER)
				#endif
				#ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
					PATCH_SWITCHER_IF(PATCH_SWITCH_EXO_AVIUTL_FILTER)
				#endif
				#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
					PATCH_SWITCHER_IF(PATCH_SWITCH_EXO_TRACK_MINUSVAL)
				#endif
				#ifdef PATCH_SWITCH_EXO_SCENEIDX
					PATCH_SWITCHER_IF(PATCH_SWITCH_EXO_SCENEIDX)
				#endif
				#ifdef PATCH_SWITCH_EXO_TRACKPARAM
					PATCH_SWITCHER_IF(PATCH_SWITCH_EXO_TRACKPARAM)
				#endif
				#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
					PATCH_SWITCHER_IF(PATCH_SWITCH_EXO_SPECIALCOLORCONV)
				#endif
				#ifdef PATCH_SWITCH_TEXT_OP_SIZE
					PATCH_SWITCHER_IF(PATCH_SWITCH_TEXT_OP_SIZE)
				#endif
				#ifdef PATCH_SWITCH_TEXT_OP_SIZE
					PATCH_SWITCHER_IF(PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET)
				#endif
				#ifdef PATCH_SWITCH_FONT_DIALOG
					PATCH_SWITCHER_IF(PATCH_SWITCH_FONT_DIALOG)
				#endif
				#ifdef PATCH_SWITCH_SCROLL_OBJDLG
					PATCH_SWITCHER_IF(PATCH_SWITCH_SCROLL_OBJDLG)
				#endif
				#ifdef PATCH_SWITCH_ALPHA_BG
					PATCH_SWITCHER_IF(PATCH_SWITCH_ALPHA_BG)
				#endif
				#ifdef PATCH_SWITCH_HELPFUL_MSGBOX
					PATCH_SWITCHER_IF(PATCH_SWITCH_HELPFUL_MSGBOX)
				#endif
				#ifdef PATCH_SWITCH_THEME_CC
					PATCH_SWITCHER_IF(PATCH_SWITCH_THEME_CC)
				#endif
				#ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
					PATCH_SWITCHER_IF(PATCH_SWITCH_EXEDITWINDOW_SIZING)
				#endif
				#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
					PATCH_SWITCHER_IF(PATCH_SWITCH_SETTINGDIALOG_MOVE)
				#endif

				#ifdef PATCH_SWITCH_UNDO
					PATCH_SWITCHER_IF(PATCH_SWITCH_UNDO)
					#ifdef PATCH_SWITCH_UNDO_REDO
						PATCH_SWITCHER_IF(PATCH_SWITCH_UNDO_REDO)
						#ifdef PATCH_SWITCH_UNDO_REDO_SHIFT
							PATCH_SWITCHER_IF(PATCH_SWITCH_UNDO_REDO_SHIFT)
						#endif
					#endif
				#endif

				#ifdef PATCH_SWITCH_CONSOLE
					PATCH_SWITCHER_IF(PATCH_SWITCH_CONSOLE)
					PATCH_SWITCHER_IF(console_escape)
					PATCH_SWITCHER_IF(console_input)
					#ifdef PATCH_SWITCH_DEBUGSTRING
						PATCH_SWITCHER_IF(console_debug_string)
						PATCH_SWITCHER_IF(console_debugstring_time)
					#endif
				#endif

				#ifdef PATCH_SWITCH_LUA
					PATCH_SWITCHER_IF(PATCH_SWITCH_LUA)
					#ifdef PATCH_SWITCH_LUA_ENV
						PATCH_SWITCHER_IF(PATCH_SWITCH_LUA_ENV)
					#endif
					#ifdef PATCH_SWITCH_LUA_RAND
						PATCH_SWITCHER_IF(PATCH_SWITCH_LUA_RAND)
					#endif
					#ifdef PATCH_SWITCH_LUA_RANDEX
						PATCH_SWITCHER_IF(PATCH_SWITCH_LUA_RANDEX)
					#endif
					#ifdef PATCH_SWITCH_LUA_GETVALUE
						PATCH_SWITCHER_IF(PATCH_SWITCH_LUA_GETVALUE)
					#endif
					#ifdef PATCH_SWITCH_LUA_PATH
						PATCH_SWITCHER_IF(PATCH_SWITCH_LUA_PATH)
					#endif
				#endif

				#ifdef PATCH_SWITCH_FAST
					PATCH_SWITCHER_IF(PATCH_SWITCH_FAST)

					#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
						PATCH_SWITCHER_IF(PATCH_SWITCH_FAST_EXEDITWINDOW)
						PATCH_SWITCHER_IF(fast_exeditwindow_step)
					#endif
					#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
						PATCH_SWITCHER_IF(PATCH_SWITCH_FAST_SETTINGDIALOG)
					#endif
						
					#ifdef PATCH_SWITCH_CL
						PATCH_SWITCHER_IF(PATCH_SWITCH_CL)
						#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
							PATCH_SWITCHER_IF(PATCH_SWITCH_FAST_RADIATIONALBLUR)
						#endif
						#ifdef PATCH_SWITCH_FAST_POLORTRANSFORM
							PATCH_SWITCHER_IF(PATCH_SWITCH_FAST_POLORTRANSFORM)
						#endif
					#endif
				#endif
			}
		}

		int store(HANDLE hFile, int level) {
			const auto value_n = value_count();
			if (!value_n) return 0;

			WriteLevel(hFile, level++);
			WriteKey(hFile, name);
			WriteBlockBegin(hFile);

			size_t value_i = 0;

			to_json tj{ hFile, level, value_i, value_n };
			#ifdef PATCH_SWITCH_ACCESS_KEY
				PATCH_SWITCHER_STORE(PATCH_SWITCH_ACCESS_KEY)
			#endif
			#ifdef PATCH_SWITCH_SUSIE_LOAD
				PATCH_SWITCHER_STORE(PATCH_SWITCH_SUSIE_LOAD)
			#endif
			#ifdef PATCH_SWITCH_SPLASH
				PATCH_SWITCHER_STORE(PATCH_SWITCH_SPLASH)
			#endif
			#ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
				PATCH_SWITCHER_STORE(PATCH_SWITCH_TRA_AVIUTL_FILTER)
			#endif
			#ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
				PATCH_SWITCHER_STORE(PATCH_SWITCH_EXO_AVIUTL_FILTER)
			#endif
			#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
				PATCH_SWITCHER_STORE(PATCH_SWITCH_EXO_TRACK_MINUSVAL)
			#endif
			#ifdef PATCH_SWITCH_EXO_SCENEIDX
				PATCH_SWITCHER_STORE(PATCH_SWITCH_EXO_SCENEIDX)
			#endif
			#ifdef PATCH_SWITCH_EXO_TRACKPARAM
				PATCH_SWITCHER_STORE(PATCH_SWITCH_EXO_TRACKPARAM)
			#endif
			#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
				PATCH_SWITCHER_STORE(PATCH_SWITCH_EXO_SPECIALCOLORCONV)
			#endif
			#ifdef PATCH_SWITCH_TEXT_OP_SIZE
				PATCH_SWITCHER_STORE(PATCH_SWITCH_TEXT_OP_SIZE)
			#endif
			#ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
				PATCH_SWITCHER_STORE(PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET)
			#endif
			#ifdef PATCH_SWITCH_FONT_DIALOG
				PATCH_SWITCHER_STORE(PATCH_SWITCH_FONT_DIALOG)
			#endif
			#ifdef PATCH_SWITCH_SCROLL_OBJDLG
				PATCH_SWITCHER_STORE(PATCH_SWITCH_SCROLL_OBJDLG)
			#endif
			#ifdef PATCH_SWITCH_ALPHA_BG
				PATCH_SWITCHER_STORE(PATCH_SWITCH_ALPHA_BG)
			#endif
			#ifdef PATCH_SWITCH_HELPFUL_MSGBOX
				PATCH_SWITCHER_STORE(PATCH_SWITCH_HELPFUL_MSGBOX)
			#endif
			#ifdef PATCH_SWITCH_THEME_CC
				PATCH_SWITCHER_STORE(PATCH_SWITCH_THEME_CC)
			#endif
			#ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
				PATCH_SWITCHER_STORE(PATCH_SWITCH_EXEDITWINDOW_SIZING)
			#endif
			#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
				PATCH_SWITCHER_STORE(PATCH_SWITCH_SETTINGDIALOG_MOVE)
			#endif

			#ifdef PATCH_SWITCH_UNDO
				PATCH_SWITCHER_STORE(PATCH_SWITCH_UNDO)
				#ifdef PATCH_SWITCH_UNDO_REDO
					PATCH_SWITCHER_STORE(PATCH_SWITCH_UNDO_REDO)
					#ifdef PATCH_SWITCH_UNDO_REDO_SHIFT
						PATCH_SWITCHER_STORE(PATCH_SWITCH_UNDO_REDO_SHIFT)
					#endif
				#endif
			#endif
			#ifdef PATCH_SWITCH_CONSOLE
				PATCH_SWITCHER_STORE(PATCH_SWITCH_CONSOLE)
				PATCH_SWITCHER_STORE(console_escape)
				PATCH_SWITCHER_STORE(console_input)
				#ifdef PATCH_SWITCH_DEBUGSTRING
					PATCH_SWITCHER_STORE(console_debug_string)
					PATCH_SWITCHER_STORE(console_debugstring_time)
				#endif
			#endif
			#ifdef PATCH_SWITCH_LUA
				PATCH_SWITCHER_STORE(PATCH_SWITCH_LUA)
				#ifdef PATCH_SWITCH_LUA_ENV
					PATCH_SWITCHER_STORE(PATCH_SWITCH_LUA_ENV)
				#endif
				#ifdef PATCH_SWITCH_LUA_RAND
					PATCH_SWITCHER_STORE(PATCH_SWITCH_LUA_RAND)
				#endif
				#ifdef PATCH_SWITCH_LUA_RANDEX
					PATCH_SWITCHER_STORE(PATCH_SWITCH_LUA_RANDEX)
				#endif
				#ifdef PATCH_SWITCH_LUA_GETVALUE
					PATCH_SWITCHER_STORE(PATCH_SWITCH_LUA_GETVALUE)
				#endif
				#ifdef PATCH_SWITCH_LUA_PATH
					PATCH_SWITCHER_STORE(PATCH_SWITCH_LUA_PATH)
				#endif
			#endif
			#ifdef PATCH_SWITCH_FAST
				PATCH_SWITCHER_STORE(PATCH_SWITCH_FAST)
				#ifdef PATCH_SWITCH_CL
					PATCH_SWITCHER_STORE(PATCH_SWITCH_CL)
				#endif
				#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
					PATCH_SWITCHER_STORE(PATCH_SWITCH_FAST_EXEDITWINDOW)
					PATCH_SWITCHER_STORE(fast_exeditwindow_step)
				#endif
				#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
					PATCH_SWITCHER_STORE(PATCH_SWITCH_FAST_SETTINGDIALOG)
				#endif
				#ifdef PATCH_SWITCH_CL
					#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
						PATCH_SWITCHER_STORE(PATCH_SWITCH_FAST_RADIATIONALBLUR)
					#endif
					#ifdef PATCH_SWITCH_FAST_POLORTRANSFORM
						PATCH_SWITCHER_STORE(PATCH_SWITCH_FAST_POLORTRANSFORM)
					#endif
				#endif
			#endif
			WriteBlockEnd(hFile, --level);

			return value_n;
		}

		constexpr size_t value_count() const {
			int i = 0;

			#ifdef PATCH_SWITCH_ACCESS_KEY
				i++;
			#endif
			#ifdef PATCH_SWITCH_SUSIE_LOAD
				i++;
			#endif
			#ifdef PATCH_SWITCH_SPLASH
				i++;
			#endif
			#ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
				i++;
			#endif
			#ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
				i++;
			#endif
			#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
				i++;
			#endif
			#ifdef PATCH_SWITCH_EXO_SCENEIDX
				i++;
			#endif
			#ifdef PATCH_SWITCH_EXO_TRACKPARAM
				i++;
			#endif
			#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
				i++;
			#endif
			#ifdef PATCH_SWITCH_TEXT_OP_SIZE
				i++;
			#endif
			#ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
				i++;
			#endif
			#ifdef PATCH_SWITCH_FONT_DIALOG
				i++;
			#endif
			#ifdef PATCH_SWITCH_SCROLL_OBJDLG
				i++;
			#endif
			#ifdef PATCH_SWITCH_ALPHA_BG
				i++;
			#endif
			#ifdef PATCH_SWITCH_HELPFUL_MSGBOX
				i++;
			#endif
			#ifdef PATCH_SWITCH_THEME_CC
				i++;
			#endif
			#ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
				i++;
			#endif
			#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
				i++;
			#endif
			#ifdef PATCH_SWITCH_UNDO
				i++;
				#ifdef PATCH_SWITCH_UNDO_REDO
					i++;
					#ifdef PATCH_SWITCH_UNDO_REDO_SHIFT
						i++;
					#endif
				#endif
			#endif
			#ifdef PATCH_SWITCH_CONSOLE
				i++;
				i++;
				i++;
				#ifdef PATCH_SWITCH_DEBUGSTRING
					i++;
					i++;
				#endif
			#endif
			#ifdef PATCH_SWITCH_LUA
				i++;
				#ifdef PATCH_SWITCH_LUA_ENV
					i++;
				#endif
				#ifdef PATCH_SWITCH_LUA_RAND
					i++;
				#endif
				#ifdef PATCH_SWITCH_LUA_RANDEX
					i++;
				#endif
				#ifdef PATCH_SWITCH_LUA_GETVALUE
					i++;
				#endif
				#ifdef PATCH_SWITCH_LUA_PATH
					i++;
				#endif
			#endif

			#ifdef PATCH_SWITCH_FAST
				i++;
				#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
					i++;
					i++;
				#endif
				#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
					i++;
				#endif
				#ifdef PATCH_SWITCH_CL
					i++;
					#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
						i++;
					#endif
					#ifdef PATCH_SWITCH_FAST_POLORTRANSFORM
						i++;
					#endif
				#endif
			#endif
			return i;
		}

	} switcher;

	bool invalid_json = false;
	void load(std::wstring_view path) {
		auto hFile = CreateFileW(path.data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) return;
		DWORD ignore;
		auto size_low = GetFileSize(hFile, &ignore);
		auto file = std::make_unique<byte[]>(size_low);
		if (ReadFile(hFile, file.get(), size_low, &ignore, NULL) == FALSE) {
			patch_resource_message_stack.emplace_back(new patch_resource_message_class_w(PATCH_RS_PATCH_FAILED_TO_LOAD_SETTING, MB_TASKMODAL | MB_ICONEXCLAMATION));
			return;
		}
		CloseHandle(hFile);

		json_value_s* root = nullptr;
		BOOST_SCOPE_EXIT_ALL(&root) { free(root); };

		root = json_parse(file.get(), size_low);
		if (root == nullptr) {
			patch_resource_message_stack.emplace_back(new patch_resource_message_class_w(PATCH_RS_PATCH_INVALID_SETTING_JSON, MB_TASKMODAL | MB_ICONEXCLAMATION));
			invalid_json = true;
			return;
		}

		auto obj = json_value_as_object(root);
		if (obj == nullptr) return;

		for (auto elm = obj->start; elm != nullptr; elm = elm->next) {
			const auto key = elm->name->string;
			if (0);
#ifdef PATCH_SWITCH_CONSOLE
			else if (key_equal(elm, console.name)) console.load(elm->value);
#endif
#ifdef PATCH_SWITCH_THEME_CC
			else if (key_equal(elm, theme_cc.name)) theme_cc.load(elm->value);
#endif
			else if (key_equal(elm, switcher.name)) switcher.load(elm->value);
		}
	}

	void store(std::wstring_view path) {
		if (invalid_json)return;

		auto hFile = CreateFileW(path.data(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			patch_resource_message_w(PATCH_RS_PATCH_FAILED_TO_SAVE_SETTING, MB_TASKMODAL | MB_ICONEXCLAMATION);
			return;
		}
		BOOST_SCOPE_EXIT_ALL(&hFile) {
			CloseHandle(hFile);
		};

		int level = 0;

		WriteBlockBegin(hFile);

		const auto value_n = value_count();
		size_t value_i = 0;

		to_json tj{ hFile, level, value_i, value_n };

#ifdef PATCH_SWITCH_CONSOLE
		tj(console);
#endif
#ifdef PATCH_SWITCH_THEME_CC
		tj(theme_cc);
#endif
		tj(switcher);

		WriteBlockEnd(hFile, 0);
	}

	size_t value_count() const {
		return
#ifdef PATCH_SWITCH_CONSOLE
			console.value_count() +
#endif
#ifdef PATCH_SWITCH_THEME_CC
			theme_cc.value_count() + 
#endif
			switcher.value_count();
	}
};
