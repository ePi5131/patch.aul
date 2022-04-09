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
#ifdef PATCH_SWITCH_THEME_CC

#include "global.hpp"
#include "util.hpp"

namespace patch {
	// init at exedit load
	// ConstChanger相当のテーマ機能
	inline class theme_cc_t {
		inline static void* LayerLockBorder_mod_jmp_adr;
		inline static void* LayerLockBorder_mod_jmp_ret_adr;

		inline static uint32_t* LayerLockBorder_ptr;
	public:
		inline static bool enabled() { return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_THEME_CC); }

		void operator()() {
			if (!enabled()) return;

			{
				auto ptr = GLOBAL::executable_memory_cursor;
				GLOBAL::executable_memory_cursor += 17;
				const auto adr = GLOBAL::exedit_base + OFS::ExEdit::LayerLockBorder_mod;

				LayerLockBorder_mod_jmp_ret_adr = reinterpret_cast<void*>(adr + 8);
				LayerLockBorder_mod_jmp_adr = ptr;

				store_i32(ptr, load_i32(adr)); // CALL SelectObject
				store_i32(ptr + 4, load_i32(adr + 4));
				store_i16(ptr + 6, '\x68'); // PUSH (i32)
				store_i32(ptr + 7, 0); LayerLockBorder_ptr = reinterpret_cast<decltype(LayerLockBorder_ptr)>(ptr + 7);
				store_i16(ptr + 11, '\xff\x25'); // jmp [i32]
				store_i32(ptr + 13, &LayerLockBorder_mod_jmp_ret_adr);
				
				OverWriteOnProtectHelper h(adr, 6);
				
				h.store_i16(0, '\xff\x25'); // jmp [i32]
				h.store_i32(2, &LayerLockBorder_mod_jmp_adr);
			}

			{
				auto& layer = GLOBAL::config.theme_cc.layer;
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::layer_height_array, 12);
					auto store = [&h](size_t adr_ofs, std::optional<int>& val) {
						auto in_range = [](size_t x) { return 6 <= x && x <= 60; };
						if (val && in_range(*val)) {
							h.store_i32(adr_ofs, *val);
						}
						else {
							val = h.load_i32<int>(adr_ofs);
						}
					};
					store(0, layer.height_large);
					store(4, layer.height_medium);
					store(8, layer.height_small);
				}
				{
					auto store = [](i32 adr_border, i32 adr_center, Config::ColorBGR2_Opt& val) {
						OverWriteOnProtectHelper hb(adr_border, 4);
						OverWriteOnProtectHelper hc(adr_center, 4);
						if (val.has_value()) {
							hb.store_i32(0, val.ary[0].to_col_rgb());
							if(val.ary[1].is_valid())
								hc.store_i32(0, val.ary[1].to_col_rgb());
							else
								hc.store_i32(0, val.ary[0].to_col_rgb());
						}
						else {
							const auto cb = Config::ColorBGR::from_rgb(hb.load_i32<uint32_t>(0));
							const auto cc = Config::ColorBGR::from_rgb(hc.load_i32<uint32_t>(0));
							if (cb == cc)
								val = { cb, {} };
							else
								val = { cb,cc };
						}
					};
					
					store(
						GLOBAL::exedit_base + OFS::ExEdit::LayerClippingBorder,
						GLOBAL::exedit_base + OFS::ExEdit::LayerClippingCenter,
						layer.clipping_col
					);
					store(
						GLOBAL::exedit_base + OFS::ExEdit::LayerLinkBorder,
						GLOBAL::exedit_base + OFS::ExEdit::LayerLinkCenter,
						layer.link_col
					);
					
				}
				
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::LayerLockCenter, 4);
					if (auto& o = layer.lock_col; o.has_value()) {
						*LayerLockBorder_ptr = o.ary[0].to_col_rgb();
						h.store_i32(0, o.ary[1].to_col_rgb());
					}
					else {
						o = {
							Config::ColorBGR::from_rgb(0),
							Config::ColorBGR::from_rgb(h.load_i32<uint32_t>(0))
						};
					}
				}
				
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::LayerHideAlpha, 8);
					if (auto& o = layer.hide_alpha) {
						h.store_i64(0, std::bit_cast<uint64_t>(*o));
					}
					else {
						o = h.load_i64<double>(0);
					}
				}
				/*
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::LayerNameRectWidth, 1);
					if (auto& o = layer.name_height) {
						h.store_i8(0, static_cast<int8_t>(*o));
					}
					else {
						o = h.load_i8<uint8_t>(0);
					}

				}
				*/
			}

			{
				auto& object = GLOBAL::config.theme_cc.object;
				{
					auto store = [](uint32_t adr, Config::ColorBGR3& col) {
						OverWriteOnProtectHelper h(adr, 36);
						if (col.has_value()) {
							h.store_i32(0 , col.ary[0].r);
							h.store_i32(4 , col.ary[0].g);
							h.store_i32(8 , col.ary[0].b);
							h.store_i32(12, col.ary[1].r);
							h.store_i32(16, col.ary[1].g);
							h.store_i32(20, col.ary[1].b);
							h.store_i32(24, col.ary[2].r);
							h.store_i32(28, col.ary[2].g);
							h.store_i32(32, col.ary[2].b);
						}
						else {
							col = {
								Config::ColorBGR{
									h.load_i32<int>(8),
									h.load_i32<int>(4),
									h.load_i32<int>(0)
								},
								Config::ColorBGR{
									h.load_i32<int>(20),
									h.load_i32<int>(16),
									h.load_i32<int>(12)
								},
								Config::ColorBGR{
									h.load_i32<int>(32),
									h.load_i32<int>(28),
									h.load_i32<int>(24)
								}
							};
						}
					};

					store(GLOBAL::exedit_base + OFS::ExEdit::ObjectColorControl, object.control_col);
					store(GLOBAL::exedit_base + OFS::ExEdit::ObjectColorMedia, object.media_col);
					store(GLOBAL::exedit_base + OFS::ExEdit::ObjectColorMFilter, object.mfilter_col);
					store(GLOBAL::exedit_base + OFS::ExEdit::ObjectColorSound, object.audio_col);
					store(GLOBAL::exedit_base + OFS::ExEdit::ObjectColorSFilter, object.afilter_col);
					store(GLOBAL::exedit_base + OFS::ExEdit::ObjectColorInactive, object.inactive_col);
				}
				{
					OverWriteOnProtectHelper hb(GLOBAL::exedit_base + OFS::ExEdit::ObjectClippingColorB, 5);
					//OverWriteOnProtectHelper hg(GLOBAL::exedit_base + OFS::ExEdit::ObjectClippingColorG, 1);
					//OverWriteOnProtectHelper hr(GLOBAL::exedit_base + OFS::ExEdit::ObjectClippingColorR, 1);

					if (auto& o = object.clipping_col; o.is_valid()) {
						hb.store_i8(0, o.b);
						hb.store_i8(2, o.g);
						hb.store_i8(4, o.r);
					}
					else {
						o = {
							hb.load_i8<uint8_t>(0),
							hb.load_i8<uint8_t>(2),
							hb.load_i8<uint8_t>(4)
						};
					}
				}

				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::ObjectClippingHeight, 1);
					if (auto& o = object.clipping_height) 
						h.store_i8(0, static_cast<int8_t>(*o));
					else
						o = h.load_i8<int8_t>(0);
				}

				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::MidPointSize, 12);
					if (auto& o = object.midpt_size; o) {
						h.store_i32(0, static_cast<int>(o.value()[0]));
						h.store_i32(4, static_cast<int>(o.value()[1]));
						h.store_i32(8, static_cast<int>(o.value()[2]));
					}
					else {
						o.emplace();
						o.value()[0] = h.load_i32(0);
						o.value()[1] = h.load_i32(4);
						o.value()[2] = h.load_i32(8);
					}
				}

				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::ObjectNameColor, 8 );
					if (auto& o = object.name_col; o.has_value()) {
						h.store_i32(0, o.ary[0].to_col());
						h.store_i32(4, o.ary[1].to_col());
					}
					else {
						o.ary[0] = h.load_i32<uint32_t>(0);
						o.ary[1] = h.load_i32<uint32_t>(4);
					}
				}
			}

			{
				auto& timeline = GLOBAL::config.theme_cc.timeline;

				{
					OverWriteOnProtectHelper hf(GLOBAL::exedit_base + OFS::ExEdit::ScaleColorForeGround, 4);
					OverWriteOnProtectHelper hb(GLOBAL::exedit_base + OFS::ExEdit::ScaleColorBackGround, 4);
					
					if (auto& val = timeline.scale_col; val.has_value()) {
						hf.store_i32(0, val.ary[0].to_col_rgb());
						hb.store_i32(0, val.ary[1].to_col_rgb());
					}
					else {
						val = {
							Config::ColorBGR::from_rgb(hf.load_i32<uint32_t>(0)),
							Config::ColorBGR::from_rgb(hb.load_i32<uint32_t>(0))
						};
					}
				};

				{
					OverWriteOnProtectHelper hm(GLOBAL::exedit_base + OFS::ExEdit::BPMGridColorMeasure, 4);
					OverWriteOnProtectHelper hb(GLOBAL::exedit_base + OFS::ExEdit::BPMGridColorBeat, 4);

					if (auto& val = timeline.bpm_grid_col; val.has_value()) {
						hm.store_i32(0, val.ary[0].to_col());
						hb.store_i32(0, val.ary[1].to_col());
					}
					else {
						val = { hm.load_i32<uint32_t>(0),hb.load_i32<uint32_t>(0) };
					}
				}
			}
		}
	} theme_cc;
} // namespace patch
#endif // ifdef PATCH_SWITCH_THEME_CC
