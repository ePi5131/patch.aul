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

#include "init.hpp"


#include "add_dll_ref.hpp"
#include "config.hpp"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	// static bool loading_self = false;
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);

		if (add_dll_ref.dllmain_if_break())break;

		GLOBAL::init_minimum(hinstDLL);

		init.InitAtDllMain();

		break;
	case DLL_PROCESS_DETACH:
		//GLOBAL::config.store(GLOBAL::patchaul_config_path);
		config2.store(GLOBAL::patchaul_config_path);
		#ifdef PATCH_SWITCH_CONSOLE
			patch::console.exit();
		#endif
		break;
	}
	return TRUE;
}
