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

#include "patch_yc_rgb_conv.hpp"


#ifdef PATCH_SWITCH_YC_RGB_CONV
namespace patch {

    void __cdecl yc_rgb_conv_t::do_multi_thread_func_wrap(AviUtl::MultiThreadFunc func,BOOL flag) {
        if (0 < *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yc_conv_w_loop_count)) {
            reinterpret_cast<void(__cdecl*)(AviUtl::MultiThreadFunc, BOOL)>(GLOBAL::exedit_base + OFS::ExEdit::do_multi_thread_func)(func, flag);
        }
    }
} // namespace patch
#endif // ifdef PATCH_SWITCH_YC_RGB_CONV
