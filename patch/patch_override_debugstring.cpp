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

#include "patch_override_debugstring.hpp"
#ifdef PATCH_SWITCH_DEBUGSTRING

#include <chrono>
#include <format>

namespace patch {

	void __stdcall override_debugstring_t::debug_print_override(LPCSTR lpOutputString) {
		if (!lpOutputString)return;
		console.setConsoleTextAttribute(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		if (GLOBAL::config.switcher.console_debugstring_time) console.writeConsole("[{}]\t"_fmt(get_local_time_string()));
		console.writeConsole(lpOutputString);
		console.setConsoleTextAttribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		console.writeConsole("\n");
	}

	void __stdcall override_debugstring_t::exedit_lua_error_override(LPCSTR lpOutputString) {
		if (!lpOutputString)return;
		console.setConsoleTextAttribute(FOREGROUND_RED | FOREGROUND_INTENSITY);
		if (GLOBAL::config.switcher.console_debugstring_time) console.writeConsole("[{}]\t"_fmt(get_local_time_string()));
		console.writeConsole(lpOutputString);
		console.setConsoleTextAttribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		console.writeConsole("\n");
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_DEBUGSTRING
