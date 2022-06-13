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

#include "util_others.hpp"

#include <Shlwapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif

#include <aviutl.hpp>

#include "global.hpp"

void save_project(HWND hwnd_owner) {
	std::string path;
	path.resize(512);
	OPENFILENAMEA ofna{
		.lStructSize = sizeof(OPENFILENAMEA),
		.hwndOwner = hwnd_owner,
		.hInstance = GLOBAL::patchaul_hinst,
		.lpstrFilter = "ProjectFile (*.aup)\0*.aup\0AllFile (*.*)\0*.*\0",
		.lpstrCustomFilter = NULL,
		.nMaxCustFilter = 0,
		.nFilterIndex = 0,
		.lpstrFile = path.data(),
		.nMaxFile = path.size(),
		.lpstrFileTitle = NULL,
		.nMaxFileTitle = NULL,
		.lpstrInitialDir = NULL,
		.lpstrTitle = NULL,
		.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_ENABLESIZING,
		.lpstrDefExt = "aup",
		.lCustData = 0,
		.lpfnHook = NULL,
		.lpTemplateName = NULL,
		.pvReserved = NULL,
		.dwReserved = NULL,
		.FlagsEx = NULL
	};

	auto editp = load_i32<AviUtl::EditHandle*>(GLOBAL::aviutl_base + OFS::AviUtl::edit_handle_ptr);
	std::string dir(editp->project_filename);
	if (dir.size() > 0) {
		dir.erase(PathFindFileNameA(dir.data()) - dir.data());
		ofna.lpstrInitialDir = dir.c_str();
	}

	if (GetSaveFileNameA(&ofna)) {
		((BOOL(__fastcall*)(AviUtl::EditHandle*, LPCSTR))(GLOBAL::aviutl_base + OFS::AviUtl::saveProjectFile))(
			editp,
			path.c_str()
		);
	}
}
