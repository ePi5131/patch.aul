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

#include "patch_exo_trackminusval.hpp"

#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
namespace patch {
	int __cdecl exo_trackminusval_t::exo_trackminusval_insteadfunc(char** strp, ExEdit::Filter* efp, int track_idx) {
		if (!has_flag(efp->flag, ExEdit::Filter::Flag::ExEditFilter) || efp->track_scale == nullptr || efp->track_scale[track_idx] <= 1) {
			auto ret = reinterpret_cast<int(__cdecl*)(char*)>(GLOBAL::exedit_base + OFS::ExEdit::str2int2)(*strp);
			auto itr = *strp;
			while (*itr != '\0' && *itr != ',') itr++;
			*strp = itr;
			return ret;
		}
		else {
			auto itr = *strp;
			bool is_neg = false;
			if (*itr == '-') {
				is_neg = true;
				itr++;
				(*strp)++;
			}
			while (*itr) {
				if (*itr == '.') {
					*itr = '\0';
					itr++;
					break;
				}
				itr++;
			}
			auto scale = efp->track_scale[track_idx];
			auto ret = reinterpret_cast<int(__cdecl*)(char*)>(GLOBAL::exedit_base + OFS::ExEdit::str2int2)(*strp) * scale;
			while (scale /= 10, '0' <= *itr && *itr <= '9') {
				ret += (*itr - '0') * scale;
				itr++;
			}
			if (is_neg)ret = -ret;
			while (*itr != '\0' && *itr != ',') itr++;
			*strp = itr;
			return ret;
		}
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
