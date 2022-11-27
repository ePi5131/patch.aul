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

#include "patch_add_extension.hpp"

#ifdef PATCH_SWITCH_ADD_EXTENSION
namespace patch {

	int __cdecl add_extension_t::wsprintfA_wrap2701a(void* ebp, LPSTR dst, LPCSTR src) {

		int* flag = (int*)((int)ebp - 1308); // 1:VideoFile 2:AudioFile
		int* new_ext_num = (int*)((int)ebp - 1288); // new_ext_bufに追加された拡張子の数（最初の時点では0）
		char* new_ext_buf = (char*)((int)ebp - 1284); // ".mp4\0.acb\0.xyz\0"のように'\0'区切りで書いていく .aviのみ別枠として確定で追加されるのでここには追加しないこと

		*new_ext_num = 0;

		char* str_movie_audio_file;
		if (*flag == 1) {
			str_movie_audio_file = (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_DOUGAFILE); // "動画ファイル"
		} else { // *flag == 2
			str_movie_audio_file = (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_ONSEIFILE); // "音声ファイル"
		}
		char* ptr = (char*)(GLOBAL::exedit_base + OFS::ExEdit::ini_extension_buf);
		unsigned int endptr = (unsigned int)ptr + 2047; // 読み込み時に字数カウントしていればその条件にする
		while ((unsigned int)ptr < endptr) {
			if (*ptr == '.') {
				int cnt = 1;
				while (ptr[cnt] != '\0') { // 次の区切り文字までの字数カウントついでに拡張子の大文字は小文字にしてしまう
					if ((unsigned int)ptr + cnt >= endptr) {
						return wsprintfA(dst, src);
					}
					if ('A' <= ptr[cnt] && ptr[cnt] <= 'Z') {
						ptr[cnt] += 'a' - 'A';
					}
					cnt++;
				}
				cnt++;

				if (lstrcmpiA(&ptr[cnt], str_movie_audio_file) == 0) {
					if (lstrcmpiA(ptr, (LPCSTR)(GLOBAL::aviutl_base + OFS::AviUtl::str_dot_avi)) != 0) { // ".avi"
						lstrcpyA(new_ext_buf, ptr);
						new_ext_buf += cnt;
						(*new_ext_num)++;
					}
				}

				ptr += cnt;
			}
			while (*ptr != '\0') {
				ptr++;
			}
			ptr++;
		}
		return wsprintfA(dst, src);
	}



} // namespace patch
#endif // ifdef PATCH_ADD_EXTENSION