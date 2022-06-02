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

#include "patch_fast_text.hpp"

#ifdef PATCH_SWITCH_FAST_TEXT

#include "global.hpp"

namespace patch::fast {

	HFONT __cdecl text_t::MyCreateFont(const char* fontname, int height, int weight, BOOL italic, BOOL high_precision, BOOL vertical) {
		HFONT& currentFont = load_i32<HFONT&>(GLOBAL::exedit_base + 0x236388);

		height = -height;
		if (high_precision) {
			height *= 2;
		}

		char fontname_v[LF_FACESIZE + 1];
		if (vertical) {
			fontname_v[0] = '@';
			strncpy_s(fontname_v + 1, LF_FACESIZE + 1, fontname, LF_FACESIZE);
			fontname = fontname_v;
		}

		auto font = ::CreateFontA(height, 0, 0, 0, weight, italic, 0, 0, 1, 8, 0, 4, 0, fontname);
		if (font == NULL)return currentFont = NULL;
		LOGFONTW lfw;
		::GetObjectW(font, sizeof(LOGFONTW), &lfw);

		std::lock_guard lock(text.mtx);

		auto [val, b] = text.map.try_emplace(lfw, font);
		if (!b) {
			::DeleteObject(font);
		}
		return currentFont = val->second.font;
	}

	HFONT WINAPI text_t::CreateFontIndirectW(const LOGFONTW* lplf) {
		char facename[LF_FACESIZE];
		auto const facename_size = ::WideCharToMultiByte(CP_ACP, 0U, lplf->lfFaceName, -1, nullptr, 0, nullptr, nullptr);
		if (::WideCharToMultiByte(CP_ACP, 0U, lplf->lfFaceName, -1, facename, sizeof(facename), nullptr, nullptr) == 0) {
			return NULL;
		}

		std::lock_guard lock(text.mtx);
		auto& map = text.map;
		if (auto itr = map.find(*lplf); itr != map.end()) {
			return itr->second.font;
		}

		return map.try_emplace(*lplf, ::CreateFontIndirectW(lplf)).first->second.font;
	}


	DWORD WINAPI text_t::GetGlyphOutlineW(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm, DWORD cjBuffer, LPVOID pvBuffer, CONST MAT2* lpmat2) {
		auto font = reinterpret_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));

		LOGFONTW lfw;
		::GetObjectW(font, sizeof(LOGFONTW), &lfw);

		std::lock_guard lock(text.mtx);

		auto itr = text.map.find(lfw);
		if (itr == text.map.end()) {
			return ::GetGlyphOutlineW(hdc, uChar, fuFormat, lpgm, cjBuffer, pvBuffer, lpmat2);
		}
		auto& glyph = itr->second.glyph_map;

		GlyphKey glyphkey{ .uChar = uChar, .fuFormat = fuFormat };
		auto itr2 = glyph.find(glyphkey);
		if (itr2 != glyph.end()) {
			auto& val = itr2->second;
			memcpy(lpgm, &val.gm, sizeof(GLYPHMETRICS));
			if (cjBuffer && pvBuffer) {
				if (val.size == GDI_ERROR)return GDI_ERROR;
				memcpy_s(pvBuffer, cjBuffer, &val.data[0], val.size);
			}
			auto now = text_detail::gettime();
			val.last_use = now;
			itr->second.last_use = now;

			return val.size;
		}
		auto size = ::GetGlyphOutlineW(hdc, uChar, fuFormat, lpgm, 0, nullptr, lpmat2);
		auto& val = glyph.try_emplace(glyphkey, size, text_detail::gettime(), lpgm, nullptr).first->second;
		if (size == GDI_ERROR)return GDI_ERROR;


		val.data.reset(new byte[size]);
		::GetGlyphOutlineW(hdc, uChar, fuFormat, lpgm, size, val.data.get(), lpmat2);

		if (cjBuffer && pvBuffer) {
			if (val.size == GDI_ERROR)return GDI_ERROR;
			memcpy_s(pvBuffer, cjBuffer, &val.data[0], val.size);
		}

		return size;
	}

	BOOL WINAPI text_t::DeleteObject(HGDIOBJ ho) {
		std::lock_guard lock(text.mtx);

		for (const auto& v : text.map) {
			if (v.second.font == ho)return TRUE;
		}
		return ::DeleteObject(ho);
	}

	void text_t::collect(int elapse) {
		std::lock_guard lock(text.mtx);

		auto& font_map = text.map;
		auto now = text_detail::gettime();

	#ifdef _DEBUG
		size_t count = 0;
		for (auto fitr = font_map.begin(); fitr != font_map.end();) {
			if (now - fitr->second.last_use > elapse) {
				printf("collected font\n");
				font_map.erase(fitr++);
			}
			else {
				auto& glyph_map = fitr->second.glyph_map;
				for (auto gitr = glyph_map.begin(); gitr != glyph_map.end();) {
					if (now - gitr->second.last_use > elapse) {
						//printf("collected glyph\n");
						glyph_map.erase(gitr++);
					}
					else {
						count += sizeof GlyphKey;
						count += sizeof GlyphValue;
						count += gitr->second.size;
						gitr++;
					}
				}
				count += sizeof LOGFONTW;
				count += sizeof FontValue;
				fitr++;
			}
		}
		printf("[lighttext]using memory: %dKB\n", count / 1000);

#else
		for (auto fitr = font_map.begin(); fitr != font_map.end();) {
			if (now - fitr->second.last_use > elapse) {
				font_map.erase(fitr++);
			}
			else {
				auto& glyph_map = fitr->second.glyph_map;
				for (auto gitr = glyph_map.begin(); gitr != glyph_map.end();) {
					if (now - gitr->second.last_use > elapse) {
						glyph_map.erase(gitr++);
					}
					else {
						gitr++;
					}
				}
				fitr++;
			}
		}
#endif
	}

}
#endif
