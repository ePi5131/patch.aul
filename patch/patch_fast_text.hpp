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
#include "macro.h"

#ifdef PATCH_SWITCH_FAST_TEXT

#include <compare>
#include <map>
#include <mutex>
#include <memory>
#include <algorithm>
#include <Windows.h>

#include "cryptostring.hpp"
#include "util_magic.hpp"
#include "timer.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	namespace text_detail {
		inline auto gettime() {
			return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		using FontKey = LOGFONTW;

		struct GlyphKey {
			UINT uChar;
			UINT fuFormat;
		};
		inline bool operator<(const GlyphKey& l, const GlyphKey& r) {
			return
				std::tie(l.uChar, l.fuFormat) <
				std::tie(r.uChar, r.fuFormat);
		}

		struct GlyphValue {
			DWORD size = 0;
			time_t last_use = 0;
			GLYPHMETRICS gm;
			std::shared_ptr<byte[]> data;
			GlyphValue(DWORD size_, time_t time_, LPGLYPHMETRICS lpgm_, void* data_) :size(size_), last_use(time_) {
				memcpy(&gm, lpgm_, sizeof(GLYPHMETRICS));
				if (size == GDI_ERROR)return;
				data = std::make_unique<byte[]>(size);
				if (data_) {
					memcpy(data.get(), data_, size_);
				}
			}
		};

		struct FontValue {
			HFONT font;
			time_t last_use;
			std::map<GlyphKey, GlyphValue> glyph_map;

			FontValue(HFONT font_) : font(font_), last_use(gettime()), glyph_map() {}
			~FontValue() {
				DeleteObject(font);
			}
		};

		inline std::strong_ordering operator<=>(const FontKey& a, const FontKey& b) {
			#define comp(member) if (auto cmp = a.member <=> b.member; cmp != 0) do { return cmp; } while(0)
			#define compb(member) \
			if(a.member == TRUE) { \
				if(b.member != TRUE) \
					return std::strong_ordering::less; /*a==TRUE b==FALSE */ \
			} else { \
				if (b.member == TRUE) \
					return std::strong_ordering::greater; /*a==FALSE b==TRUE */ \
			}

			comp(lfHeight);
			comp(lfWidth);
			comp(lfEscapement);
			comp(lfOrientation);
			comp(lfWeight);
			compb(lfItalic);
			compb(lfUnderline);
			compb(lfStrikeOut);
			comp(lfCharSet);
			comp(lfOutPrecision);
			comp(lfClipPrecision);
			comp(lfQuality);
			comp(lfPitchAndFamily);

			#undef comp
			#undef compb

			auto cmp = wcscmp(a.lfFaceName, b.lfFaceName);
			if (cmp < 0)return std::strong_ordering::less;
			if (cmp > 0)return std::strong_ordering::greater;
			return std::strong_ordering::equal;
		}
		struct comp_fontkey {
			bool operator()(const FontKey& a, const FontKey& b) const {
				return (a <=> b) < 0;
			}
		};
	}

	inline class text_t {
		using FontKey = text_detail::FontKey;
		using FontValue = text_detail::FontValue;
		using GlyphKey = text_detail::GlyphKey;
		using GlyphValue = text_detail::GlyphValue;
	public:

		static HFONT __cdecl MyCreateFont(const char* fontname, int height, int weight, BOOL italic, BOOL high_precision, BOOL vertical);
		static HFONT WINAPI CreateFontIndirectW(const LOGFONTW* lplf);
		static BOOL WINAPI DeleteObject(HGDIOBJ ho);
		static DWORD WINAPI GetGlyphOutlineW(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm, DWORD cjBuffer, LPVOID pvBuffer, CONST MAT2* lpmat2);

		void collect(int elapse);

	private:
		std::map<FontKey, FontValue, text_detail::comp_fontkey> map;
		std::mutex mtx;

		bool enabled = true;
		bool enabled_i;

		int release_time = 120;

		inline static const char key[] = "fast.text";

		inline static const char key_release_time[] = "release_time";

	public:
		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			ExchangeFunction(GLOBAL::exedit_hmod, cstr_gdi32_dll.get(), cstr_GetGlyphOutlineW.get(), &GetGlyphOutlineW);
			ExchangeFunction(GLOBAL::exedit_hmod, cstr_gdi32_dll.get(), cstr_DeleteObject.get(), &DeleteObject);
			ExchangeFunction(GLOBAL::exedit_hmod, cstr_gdi32_dll.get(), cstr_CreateFontIndirectW.get(), &CreateFontIndirectW);
			ReplaceFunction(GLOBAL::exedit_base + 0x8a720, &MyCreateFont);

			timer.set([this]() {
				collect(release_time);
			}, release_time * 1000);
		}

		void switching(bool flag) { enabled = flag; }

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }

		void config_load(ConfigReader& cr) {
			cr.regist(key_release_time, [this](json_value_s* value) {
				ConfigReader::load_variable(value, release_time);
			});
		}

		void config_store(ConfigWriter& cw) {
			cw.append(key_release_time, release_time);
		}

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}

	} text;

}
#endif
