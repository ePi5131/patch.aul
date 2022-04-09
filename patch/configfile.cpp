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

#include "configfile.hpp"
#include "patch_console.hpp"

// コンソールウィンドウの情報を渡してあげる
void console_get_setting_rect(std::optional<RECT>& rect) {
	auto hwnd = patch::console.get_console_hwnd();
	if (!IsIconic(hwnd)) {
		rect = RECT{};
		if (GetWindowRect(hwnd, &rect.value()) == FALSE) rect = std::nullopt;
	}
}
