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

#ifdef PATCH_SWITCH_PLAYBACK_SPEED

#include <exedit.hpp>
#include "config_rw.hpp"

#include "global.hpp"

namespace patch {

	// init at exedit load
	// n番目の中間点で再生速度を変化させるとnフレーム遅れて反映されるのを修正
	inline class playback_speed_t {
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "playback_speed";
	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			{ // movie_file
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x005fd9, 1);
				h.store_i8(0, '\x90');
			}
			{ // audio_file
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08faab, 1);
				h.store_i8(0, '\x90');
			}

			{ // scene
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0836b3, 1);
				h.store_i8(0, '\x90');
			}
			{ // scene_audio
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x084297, 1);
				h.store_i8(0, '\x90');
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
	} playback_speed;
} // namespace patch

#endif // ifdef PATCH_SWITCH_PLAYBACK_SPEED
