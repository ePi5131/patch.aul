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

#include "patch_susie_load.hpp"
#ifdef PATCH_SWITCH_SUSIE_LOAD

namespace patch {

	void __cdecl susie_load_t::LoadSpi(LPCSTR dir) {
		auto loaded_spi_array = reinterpret_cast<ExEdit::structSPI*>(GLOBAL::exedit_base + OFS::ExEdit::loaded_spi_array);

		ZeroMemory(loaded_spi_array, sizeof(ExEdit::structSPI) * 32);
		//MyFindFirstFile
		reinterpret_cast<BOOL(*)(LPCSTR)>(GLOBAL::exedit_base + OFS::ExEdit::MyFindFirstFile)(dir);

		for (int i = 0; i < 32;) {
			char path[260];
			// MyFindNextFile
			if (reinterpret_cast<BOOL(*)(LPSTR)>(GLOBAL::exedit_base + OFS::ExEdit::MyFindNextFile)(path) != TRUE)return;

			if (auto hMod = LoadLibraryA(path); hMod != NULL) {
				auto spi_GetPluginInfo = reinterpret_cast<ExEdit::SpiGetPluginInfo>(GetProcAddress(hMod, "GetPluginInfo"));
				if (!spi_GetPluginInfo) continue;

				loaded_spi_array[i].GetPicture = reinterpret_cast<ExEdit::SpiGetPicture>(GetProcAddress(hMod, "GetPicture"));
				loaded_spi_array[i].hmodule = hMod;

				spi_GetPluginInfo(1, loaded_spi_array[i].information, 256);


				int j = 2;
				auto ext = loaded_spi_array[i].extension;
				auto ext_pos = 0;
				auto ext_size = std::size(loaded_spi_array[i].extension);

				while (true) {
					char buf[256];
					auto ret = spi_GetPluginInfo(j, buf, sizeof(buf));
					if (ret == 0)break;

					const std::string_view view(buf);

					size_t pos_a = 0;
					size_t pos_b;
					while ((pos_b = view.find_first_of(';', pos_a)) != std::string_view::npos) {
						auto ext_pos_new = ext_pos + (pos_b - pos_a) + 1;
						if (ext_pos_new >= ext_size) {
							if (i == 31) goto BREAK_EXT;
							ext[ext_pos - 1] = '\0';

							LoadLibraryA(path);

							i++;
							ext = loaded_spi_array[i].extension;
							ext_pos = 0;
							ext_pos_new = (pos_b - pos_a) + 1;

							loaded_spi_array[i].GetPicture = loaded_spi_array[i - 1].GetPicture;
							loaded_spi_array[i].hmodule = hMod;

							strcpy_s(loaded_spi_array[i].information, loaded_spi_array[i - 1].information);
						}
						strncpy_s(ext + ext_pos, ext_size - ext_pos, buf + pos_a, pos_b - pos_a);

						ext_pos = ext_pos_new;
						ext[ext_pos - 1] = ';';

						pos_a = pos_b + 1;
					}

					if (ret - pos_a > 0) {
						auto ext_pos_new = ext_pos + (ret - pos_a) + 1;
						if (ext_pos_new >= ext_size) {
							if (i == 31) goto BREAK_EXT;
							ext[ext_pos - 1] = '\0';

							LoadLibraryA(path);

							i++;
							ext = loaded_spi_array[i].extension;
							ext_pos = 0;
							ext_pos_new = (pos_b - pos_a) + 1;

							loaded_spi_array[i].GetPicture = loaded_spi_array[i - 1].GetPicture;
							loaded_spi_array[i].hmodule = hMod;

							strcpy_s(loaded_spi_array[i].information, loaded_spi_array[i - 1].information);
						}
						strncpy_s(ext + ext_pos, ext_size - ext_pos, buf + pos_a, ret - pos_a);
						ext_pos = ext_pos_new;
						ext[ext_pos - 1] = ';';
					}

					if (j >= (std::numeric_limits<int>::max)() - 1)break;
					j += 2;
				}
			BREAK_EXT:

				if (ext_pos != 0) ext[ext_pos - 1] = '\0';

				//spi_GetPluginInfo(2, loaded_spi_array[i].extension, 256);

				i++;
			}
		}

	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_SUSIE_LOAD
