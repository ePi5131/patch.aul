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

#include "macro.h"

#ifdef PATCH_SWITCH_SETTING_GUI
namespace patch {
	// patch.aul の設定用ダイアログ
	inline class setting_gui_t {

	public:
		void operator()() {

		}
	} setting_gui;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTING_GUI
