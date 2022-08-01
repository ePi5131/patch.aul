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

#include "patch_tra_change_drawfilter.hpp"

#ifdef PATCH_SWITCH_TRA_CHANGE_DRAWFILTER
namespace patch {

	int __cdecl tra_change_drawfilter_t::switch_drawfilter_trackdata_to_mem(int* data, int object_idx, int track_begin, int track_id) {
		if (0 <= track_id) {
			auto& eop = (*ObjectArrayPointer_ptr)[object_idx];
			int tr = track_begin + track_id;
			data[0] = eop.track_value_left[tr];
			data[1] = eop.track_value_right[tr];
			data[2] = *(int*)&eop.track_mode[tr];
			data[3] = eop.track_param[tr];
			return 4;
		}
		return 0;
	}

	int __cdecl tra_change_drawfilter_t::switch_drawfilter_mem_to_trackdata(int* data, int object_idx, int track_begin, int pre_track_exists, int track_id) {
		if (0 <= pre_track_exists) {
			if (0 <= track_id) {
				auto& eop = (*ObjectArrayPointer_ptr)[object_idx];
				int tr = track_begin + track_id;
				eop.track_value_left[tr] = data[0];
				eop.track_value_right[tr] = data[1];
				*(int*)&eop.track_mode[tr] = data[2];
				eop.track_param[tr] = data[3];
			}
			return 4;
		}
		return 0;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_TRA_CHANGE_DRAWFILTER