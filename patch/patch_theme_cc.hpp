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
#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// ConstChanger相当のテーマ機能
	inline class theme_cc_t {
		inline static void* LayerLockBorder_mod_jmp_adr;
		inline static void* LayerLockBorder_mod_jmp_ret_adr;

		inline static uint32_t* LayerLockBorder_ptr;

		using ColorBGR = config_type::ColorBGR;
		using ColorBGR2 = config_type::ColorBGR2;
		using ColorBGR2_Opt = config_type::ColorBGR2_Opt;
		using ColorBGR3 = config_type::ColorBGR3;

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "theme_cc";

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

			void load(ConfigReader& cr) {
				cr.regist(key_large, [this](json_value_s* value) {
					ConfigReader::load_variable(value, height_large);
				});
				cr.regist(key_medium, [this](json_value_s* value) {
					ConfigReader::load_variable(value, height_medium);
				});
				cr.regist(key_small, [this](json_value_s* value) {
					ConfigReader::load_variable(value, height_small);
				});
				cr.regist(key_link_col, [this](json_value_s* value) {
					ConfigReader::load_variable(value, link_col);
				});
				cr.regist(key_clipping_col, [this](json_value_s* value) {
					ConfigReader::load_variable(value, clipping_col);
				});
				cr.regist(key_lock_col, [this](json_value_s* value) {
					ConfigReader::load_variable(value, lock_col);
				});
				cr.regist(key_hide_alpha, [this](json_value_s* value) {
					ConfigReader::load_variable(value, hide_alpha);
				});
			}

			void store(ConfigWriter& cw) {
				cw.append(key_large, height_large);
				cw.append(key_medium, height_medium);
				cw.append(key_small, height_small);
				cw.append(key_link_col, link_col);
				cw.append(key_clipping_col, clipping_col);
				cw.append(key_lock_col, lock_col);
				cw.append(key_hide_alpha, hide_alpha);
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

			void load(ConfigReader& cr) {
				cr.regist(key_media_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, media_col);
				});
				cr.regist(key_mfilter_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, mfilter_col);
				});
				cr.regist(key_audio_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, audio_col);
				});
				cr.regist(key_afilter_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, afilter_col);
				});
				cr.regist(key_control_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, control_col);
				});
				cr.regist(key_inactive_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, inactive_col);
				});
				cr.regist(key_clipping_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, clipping_col);
				});
				cr.regist(key_clipping_height, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, clipping_height);
				});
				cr.regist(key_midpt_size, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, midpt_size);
				});
				cr.regist(key_name_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, name_col);
				});
			}

			void store(ConfigWriter& cw) {
				cw.append(key_media_col, media_col);
				cw.append(key_mfilter_col, mfilter_col);
				cw.append(key_audio_col, audio_col);
				cw.append(key_afilter_col, afilter_col);
				cw.append(key_control_col, control_col);
				cw.append(key_inactive_col, inactive_col);
				cw.append(key_clipping_col, clipping_col);
				cw.append(key_clipping_height, clipping_height);
				cw.append(key_midpt_size, midpt_size);
				cw.append(key_name_col, name_col);
			}

		} object;

		struct {
			inline static const char name[] = "timeline";

			ColorBGR2 scale_col;
			ColorBGR2 bpm_grid_col;

			inline static const char key_scale_col[] = "scale_col";
			inline static const char key_bpm_grid_col[] = "bpm_grid_col";


			void load(ConfigReader& cr) {
				cr.regist(key_scale_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, scale_col);
				});
				cr.regist(key_bpm_grid_col, [this](json_value_s* jv) {
					ConfigReader::load_variable(jv, bpm_grid_col);
				});
			}

			void store(ConfigWriter& cw) {
				cw.append(key_scale_col, scale_col);
				cw.append(key_bpm_grid_col, bpm_grid_col);
			}
		} timeline;
	public:
		void init() {
			enabled_i = enabled;
			if (!enabled_i) return;

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
				store_i16(ptr + 11, { 0xff, 0x25 }); // jmp [i32]
				store_i32(ptr + 13, &LayerLockBorder_mod_jmp_ret_adr);
				
				OverWriteOnProtectHelper h(adr, 6);
				
				h.store_i16(0, { 0xff, 0x25 }); // jmp [i32]
				h.store_i32(2, &LayerLockBorder_mod_jmp_adr);
			}

			{
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
					auto store = [](i32 adr_border, i32 adr_center, config_type::ColorBGR2_Opt& val) {
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
							const auto cb = config_type::ColorBGR::from_rgb(hb.load_i32<uint32_t>(0));
							const auto cc = config_type::ColorBGR::from_rgb(hc.load_i32<uint32_t>(0));
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
							config_type::ColorBGR::from_rgb(0),
							config_type::ColorBGR::from_rgb(h.load_i32<uint32_t>(0))
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
				{
					auto store = [](uint32_t adr, config_type::ColorBGR3& col) {
						OverWriteOnProtectHelper h(adr, 36);
						if (col.has_value()) {
							h.store_i32(0 , col.ary[0].val.r);
							h.store_i32(4 , col.ary[0].val.g);
							h.store_i32(8 , col.ary[0].val.b);
							h.store_i32(12, col.ary[1].val.r);
							h.store_i32(16, col.ary[1].val.g);
							h.store_i32(20, col.ary[1].val.b);
							h.store_i32(24, col.ary[2].val.r);
							h.store_i32(28, col.ary[2].val.g);
							h.store_i32(32, col.ary[2].val.b);
						}
						else {
							col = {
								config_type::ColorBGR{
									h.load_i32<int>(8),
									h.load_i32<int>(4),
									h.load_i32<int>(0)
								},
								config_type::ColorBGR{
									h.load_i32<int>(20),
									h.load_i32<int>(16),
									h.load_i32<int>(12)
								},
								config_type::ColorBGR{
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
						hb.store_i8(0, o.val.b);
						hb.store_i8(2, o.val.g);
						hb.store_i8(4, o.val.r);
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
				{
					OverWriteOnProtectHelper hf(GLOBAL::exedit_base + OFS::ExEdit::ScaleColorForeGround, 4);
					OverWriteOnProtectHelper hb(GLOBAL::exedit_base + OFS::ExEdit::ScaleColorBackGround, 4);
					
					if (auto& val = timeline.scale_col; val.has_value()) {
						hf.store_i32(0, val.ary[0].to_col_rgb());
						hb.store_i32(0, val.ary[1].to_col_rgb());
					}
					else {
						val = {
							config_type::ColorBGR::from_rgb(hf.load_i32<uint32_t>(0)),
							config_type::ColorBGR::from_rgb(hb.load_i32<uint32_t>(0))
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

		void switching(bool flag) {
			enabled = flag;
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
			});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}

		void config_load(ConfigReader& cr) {
			cr.regist(layer.name, [this](json_value_s* value) {
				ConfigReader cr(value);
				layer.load(cr);
				cr.load();
			});
			cr.regist(object.name, [this](json_value_s* value) {
				ConfigReader cr(value);
				object.load(cr);
				cr.load();
			});
			cr.regist(timeline.name, [this](json_value_s* value) {
				ConfigReader cr(value);
				timeline.load(cr);
				cr.load();
			});
		}

		void config_store(ConfigWriter& cw) {
			{
				ConfigWriter cw_layer(cw.get_level() + 1);
				layer.store(cw_layer);
				std::stringstream ss;
				cw_layer.write(ss);
				cw.append(layer.name, ss.str());
			}
			{
				ConfigWriter cw_object(cw.get_level() + 1);
				object.store(cw_object);
				std::stringstream ss;
				cw_object.write(ss);
				cw.append(object.name, ss.str());
			}
			{
				ConfigWriter cw_timeline(cw.get_level() + 1);
				timeline.store(cw_timeline);
				std::stringstream ss;
				cw_timeline.write(ss);
				cw.append(timeline.name, ss.str());
			}
		}
	} theme_cc;
} // namespace patch
#endif // ifdef PATCH_SWITCH_THEME_CC
