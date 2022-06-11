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

#include "patch_setting_dialog_excolorconfig.hpp"
#ifdef PATCH_SWITCH_SETTINGDIALOG_EXCOLORCONFIG



namespace patch {

	void __cdecl excolorconfig_t::efExColorConfig_setButtonStr_wrap_17083(ExEdit::Filter* efp) {

		HWND ctrl_hwnd = efp->exfunc->get_hwnd(efp->processing, 7, 2);
		if (ctrl_hwnd) {
			SetWindowPos(ctrl_hwnd, (HWND)0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
		}
		ctrl_hwnd = efp->exfunc->get_hwnd(efp->processing, 7, 1);
		if (ctrl_hwnd) {
			SetWindowPos(ctrl_hwnd, (HWND)0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
		}

		efExColorConfig_setButtonStr(efp);
	}


} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_EXCOLORCONFIG
