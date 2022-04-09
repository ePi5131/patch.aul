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

#include "patch_fast_getputpixeldata.hpp"
#ifdef PATCH_SWITCH_FAST_GETPUTPIXELDATA

#include <ppl.h>

#include <exedit.hpp>
#include "multi_threading.hpp"

namespace patch::fast {
	static BOOL BufCpyBGRA2YCA(void* dst, void* src, int w, int h, int line) {


	}

	static BOOL BufCpyYCA2BGRA(void* dst, void* src, int w, int h, int line) {


	}

	int getputpixeldata_t::avx_getpixeldata(lua_State* L) {
		auto n = lua_gettop(L);
		bool is_alloc = false;
		bool is_work = false;
		for (int i = 0; i < n; i++) {
			auto s = lua_tostring(L, i + 1);
			if (lstrcmpiA(s, "alloc") == 0)is_alloc = true;
			if (lstrcmpiA(s, "work") == 0)is_work = true;
		}
		auto efpip_g = load_i32<ExEdit::FilterProcInfo*>(GLOBAL::exedit_base + OFS::ExEdit::efpip_g);

		if (is_work) {
			if (is_alloc) {
				lua_newuserdata(L, efpip_g->obj_w * efpip_g->obj_h * 4 + 16);
			}
			else {
				lua_pushlightuserdata(L, reinterpret_cast<BYTE*>(efpip_g->obj_temp) + load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h_add8) * load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line) * 4);
			}
		}
		else {
			ExEdit::PixelBGRA* ptr;
			if (is_alloc) {
				ptr = static_cast<ExEdit::PixelBGRA*>(lua_newuserdata(L, efpip_g->obj_w * efpip_g->obj_h * 4 + 16));
			}
			else {
				ptr = static_cast<ExEdit::PixelBGRA*>(static_cast<void*>(efpip_g->obj_temp)) + load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h_add8) * load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
				lua_pushlightuserdata(L, ptr);
			}
			
			constexpr static __m256i y_c = { .m256i_i16 = {
				16320, 16320, 16320, 0,
				16320, 16320, 16320, 0,
				16320, 16320, 16320, 0,
				16320, 16320, 16320, 0
			} };

			constexpr static __m256i cb_c = { .m256i_i16 = {
				28919, -5616, 0, 0,
				28919, -5616, 0, 0,
				28919, -5616, 0, 0,
				28919, -5616, 0, 0
			} };

			constexpr static __m256i cr_c = { .m256i_i16 = {
				0, -11655, 22881, 16320,
				0, -11655, 22881, 16320,
				0, -11655, 22881, 16320,
				0, -11655, 22881, 16320
			} };

			//concurrency::parallel_for(0, efpip_g->obj_h, [=](int y) {
			for (int y = 0; y < efpip_g->obj_h; y++) {
				auto src_itr = efpip_g->obj_edit + y * efpip_g->obj_line;
				auto dst_itr = ptr + y * efpip_g->obj_w;
				for (int x = 0; x < efpip_g->obj_w / 4; x++) {
					auto src = _mm256_loadu_epi16(src_itr);
					auto dst = dst_itr;

					auto m_y  = _mm256_shufflehi_epi16(_mm256_shufflelo_epi16(src, 0b1100'0000), 0b1100'0000);
					auto m_cb = _mm256_shufflehi_epi16(_mm256_shufflelo_epi16(src, 0b1101'0101), 0b1101'0101);
					auto m_cr = _mm256_shufflehi_epi16(_mm256_shufflelo_epi16(src, 0b1110'1010), 0b1110'1010);

					m_y  = _mm256_mulhi_epi16(m_y , y_c);
					m_cb = _mm256_mulhi_epi16(m_cb, cb_c);
					m_cr = _mm256_mulhi_epi16(m_cr, cr_c);

					auto result_i16 = _mm256_adds_epi16(m_cb, m_cr);
					result_i16 = _mm256_adds_epi16(result_i16, m_y);
 					result_i16 = _mm256_adds_epi16(result_i16, _mm256_set1_epi16(3));
					result_i16 = _mm256_srai_epi16(result_i16, 2);

					auto result_i8 = _mm_packus_epi16(_mm256_extracti128_si256(result_i16, 0), _mm256_extracti128_si256(result_i16, 1));
					_mm_storeu_epi8(dst, result_i8);

					src_itr += 4;
					dst_itr += 4;
				}
				if (auto mod = efpip_g->obj_w % 4; mod) {
					auto src = _mm256_loadu_epi16(src_itr);
					auto dst = dst_itr;

					auto m_y  = _mm256_shufflehi_epi16(_mm256_shufflelo_epi16(src, 0b1100'0000), 0b1100'0000);
					auto m_cb = _mm256_shufflehi_epi16(_mm256_shufflelo_epi16(src, 0b1101'0101), 0b1101'0101);
					auto m_cr = _mm256_shufflehi_epi16(_mm256_shufflelo_epi16(src, 0b1110'1010), 0b1110'1010);

					m_y  = _mm256_mulhi_epi16(m_y , y_c);
					m_cb = _mm256_mulhi_epi16(m_cb, cb_c);
					m_cr = _mm256_mulhi_epi16(m_cr, cr_c);

					auto result_i16 = _mm256_adds_epi16(m_cb, m_cr);
					result_i16 = _mm256_adds_epi16(result_i16, m_y);
 					result_i16 = _mm256_adds_epi16(result_i16, _mm256_set1_epi16(3));
					result_i16 = _mm256_srai_epi16(result_i16, 2);

					auto result_i8 = _mm_packus_epi16(_mm256_extracti128_si256(result_i16, 0), _mm256_extracti128_si256(result_i16, 1));

					switch (mod) {
					case 1:
						_mm_storeu_si32(dst, result_i8);
						break;
					case 2:
						_mm_storeu_si64(dst, result_i8);
						break;
					case 3:
						_mm_maskstore_epi32(reinterpret_cast<int*>(dst), _mm_set_epi32(0, -1, -1, -1), result_i8);
						break;
					}

				}
			//});
			}
		}
		lua_pushinteger(L, efpip_g->obj_w);
		lua_pushinteger(L, efpip_g->obj_h);
		return 3;
	}

	int getputpixeldata_t::avx_putpixeldata(lua_State* L) {
		if (!lua_isuserdata(L, 1)) return 0;
		auto pixelp = static_cast<ExEdit::PixelBGRA*>(lua_touserdata(L, 1));
		if (pixelp == nullptr)return 0;



		return 0;
	}

#if 0
	int avx_putpixeldata_old(lua_State* L) {
		static constexpr m128_256 y_ef = { .m256 = {
				0.299f / 255.f * 4096.f, 0.587f / 255.f * 4096.f, 0.114f / 255.f * 4096.f, 0.f,
				0.299f / 255.f * 4096.f, 0.587f / 255.f * 4096.f, 0.114f / 255.f * 4096.f, 0.f,
			}
		};
		static constexpr m128_256 cb_ef = { .m256 = {
				-0.169f / 255.f * 4096.f, -0.331f / 255.f * 4096.f, 0.5f / 255.f * 4096.f, 0.f,
				-0.169f / 255.f * 4096.f, -0.331f / 255.f * 4096.f, 0.5f / 255.f * 4096.f, 0.f,
			}
		};
		static constexpr m128_256 cr_ef = { .m256 = {
				0.5f / 255.f * 4096.f, -0.419f / 255.f * 4096.f, -0.081f / 255.f * 4096.f, 0.f,
				0.5f / 255.f * 4096.f, -0.419f / 255.f * 4096.f, -0.081f / 255.f * 4096.f, 0.f,
			}
		};
		static constexpr m128_256 a_ef = { .m256 = {
				0.f,0.f,0.f,1.f / 255.f * 4096.f,
				0.f,0.f,0.f,1.f / 255.f * 4096.f,
			}
		};

		auto n = lua_gettop(L);
		if (auto data = static_cast<ExEdit::PixelBGRA*>(lua_touserdata(L, 1)); data) {
			auto efpip = *reinterpret_cast<ExEdit::FilterProcInfo**>(GLOBAL::exedit_base + OFS::ExEdit::efpip_g);

			multi_threading([data, efpip](size_t i, size_t n) {
				const auto w = efpip->obj_w;
				const auto h = efpip->obj_h;

				const auto y_min = h * i / n;
				const auto y_max = h * (i + 1) / n;

				for (int y = y_min; std::cmp_less(y, h); y++) {
					for (int x = 0; std::cmp_less(x, w - (w & 1)); x += 2) {
						auto p128 = _mm_loadu_si64(data + x + y * efpip->obj_h);
						auto p256i = _mm256_cvtepu8_epi32(p128);
						/*
						auto p256ps = _mm256_cvtepi32_ps(p256i);

						auto ry = _mm256_dp_ps(p256ps, y_ef.m256, 0b01110001);
						auto rcb = _mm256_dp_ps(p256ps, cb_ef.m256, 0b01110010);
						auto rcr = _mm256_dp_ps(p256ps, cr_ef.m256, 0b01110100);
						auto ra = _mm256_dp_ps(p256ps, a_ef.m256, 0b01111000);

						auto rycb = _mm256_or_ps(ry, rcb);
						auto rcra = _mm256_or_ps(rcr, ra);
						auto r256ps = _mm256_or_ps(rycb, rcra);

						r256ps = _mm256_round_ps(r256ps, 0);

						auto r256i = _mm256_cvtps_epi32(r256ps);
						*/
						auto pr = _mm256_shuffle_epi32(p256i, 0b00000000);
						pr = _mm256_mullo_epi32(pr, _mm256_set_epi32(4918, -2775, 8224, 0, 4918, -2775, 8224, 0));
						pr = _mm256_add_epi32(pr, _mm256_set_epi32(354, 240, 256, 0, 354, 240, 256, 0));
						pr = _mm256_srai_epi32(pr, 10);

						auto pg = _mm256_shuffle_epi32(p256i, 0b01010101);
						pg = _mm256_mullo_epi32(pg, _mm256_set_epi32(9655, -5449, -6887, 0, 9655, -5449, -6887, 0));
						pg = _mm256_add_epi32(pg, _mm256_set_epi32(585, 515, 110, 0, 585, 515, 110, 0));
						pg = _mm256_srai_epi32(pg, 10);

						auto pb = _mm256_shuffle_epi32(p256i, 0b10101010);
						pb = _mm256_mullo_epi32(pb, _mm256_set_epi32(1875, 8224, -1337, 0, 1875, 8224, -1337, 0));
						pb = _mm256_add_epi32(pb, _mm256_set_epi32(512, 256, 646, 0, 512, 256, 646, 0));
						pb = _mm256_srai_epi32(pb, 10);

						auto pa = _mm256_shuffle_epi32(p256i, 0b11111111);
						pa = _mm256_mullo_epi32(pa, _mm256_set1_epi32(4096));
						pa = _mm256_div_epi32(pa, _mm256_set1_epi32(255));

						auto r = _mm256_add_epi32(pr, pg);
						r = _mm256_add_epi32(r, pb);
						r = _mm256_blend_epi32(r, pa, 0b10001000);

						auto r128 = _mm256_extracti128_si256(_mm256_packs_epi32(r, r), 0);
						_mm_storeu_si128((__m128i*)(efpip->obj_edit + efpip->obj_line * y + x), r128);
					}
					if (w & 1) {
						auto p128 = _mm_loadu_si64(data + w - 1 + y * efpip->obj_h);
						auto p128i = _mm_cvtepu8_epi32(p128);
						auto p128ps = _mm_cvtepi32_ps(p128i);

						auto ry = _mm_dp_ps(p128ps, y_ef.m128, 0b01110001);
						auto rcb = _mm_dp_ps(p128ps, cb_ef.m128, 0b01110010);
						auto rcr = _mm_dp_ps(p128ps, cr_ef.m128, 0b01110100);
						auto ra = _mm_dp_ps(p128ps, a_ef.m128, 0b01111000);

						auto rycb = _mm_or_ps(ry, rcb);
						auto rcra = _mm_or_ps(rcr, ra);
						auto r128ps = _mm_or_ps(rycb, rcra);

						r128ps = _mm_round_ps(r128ps, 0);

						auto r128i = _mm_cvtps_epi32(r128ps);
						auto r128 = _mm_cvtepi32_epi16(r128i);

						_mm_storeu_si64(efpip->obj_edit + efpip->obj_line * y + w - 1, r128);
					}
				}
			});
		}
		return 0;
	}
	
#endif

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_GETPUTPIXELDATA
