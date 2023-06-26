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

#include "patch_shared_cache.hpp"

static constexpr int calc_cache_size(int w, int h, int bitcount) {
    // ceil(ceil(w * bitcount / 8) / 4) * 4 * h + 16
    return (((w * bitcount + 31) >> 5) << 2) * h + 16;
}

#ifdef PATCH_SWITCH_SHARED_CACHE
namespace patch {

    void* __cdecl SharedCache_t::GetOrCreateSharedCache_BeforeClipping(ExEdit::ObjectFilterIndex ofi, int w, int h, int bitcount, int v_func_id, int* old_cache_exists) {
        auto a_exfunc = reinterpret_cast<AviUtl::ExFunc*>(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);

        int size = calc_cache_size(w, h, bitcount);
        if (void* smem = a_exfunc->get_shared_mem(std::bit_cast<int32_t>(&GetOrCreateSharedCache_BeforeClipping) + v_func_id, size, nullptr)) {
            *old_cache_exists = 1;
            return smem;
        }
        *old_cache_exists = 0;
        return a_exfunc->create_shared_mem(std::bit_cast<int32_t>(&GetOrCreateSharedCache_BeforeClipping) + v_func_id, size, size, nullptr);
    }

    void* __cdecl SharedCache_t::GetOrCreateSharedCache(ExEdit::ObjectFilterIndex ofi, int w, int h, int bitcount, int v_func_id, int* old_cache_exists) {
        auto a_exfunc = reinterpret_cast<AviUtl::ExFunc*>(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        auto e_exfunc = reinterpret_cast<ExEdit::Exfunc*>(GLOBAL::exedit_base + OFS::ExEdit::exfunc);

        if (0 <= static_cast<int>(ofi)) {
            ofi = e_exfunc->get_start_idx(ofi);
        }

        CHAR name[64];
        wsprintfA(name, "*cache%08x[%d]_%dx%d@%d", ofi, v_func_id, w, h, bitcount);

        int dummy;
        void* cacheptr = GetSharedCache(name, &dummy, &dummy, &dummy);
        if (cacheptr == nullptr) {
            cacheptr = CreateSharedCache(w, h, bitcount, name);
            if (old_cache_exists != nullptr) {
                *old_cache_exists = 0;
            }
        }
        else if (old_cache_exists != nullptr) {
            *old_cache_exists = 1;
        }
        return cacheptr;
    }

    void* __cdecl SharedCache_t::GetSharedCache(char* name, int* w, int* h, int* bitcount) {
        auto a_exfunc = reinterpret_cast<AviUtl::ExFunc*>(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        SharedCacheInfo* sci = GetOrCreateSharedCacheInfo();
        for (int i = 0; i < cache_count; i++) {
            if (sci[i].name[0] != '\0' && lstrcmpiA(name, sci[i].name) == 0) {
                if (shared_mem_info[i] == nullptr) { // 1.10側で破棄されている状態
                    sci[i].name[0] = '\0';
                    sci[i].priority = 0;
                    return get_cache(name, w, h, bitcount);
                }
                sci[i].priority = priority_count++;
                *w = sci[i].w;
                *h = sci[i].h;
                *bitcount = sci[i].bitcount;
                
                if (void* ptr = a_exfunc->get_shared_mem(std::bit_cast<int32_t>(&shared_mem_info[i]), 0, shared_mem_info[i])) return ptr;

                *w = 0;
                *h = 0;
                *bitcount = 0;
                sci = GetOrCreateSharedCacheInfo();
                if (sci != nullptr) {
                    sci[i].name[0] = '\0';
                    sci[i].priority = 0;
                }
                return get_cache(name, w, h, bitcount);
            }
        }
        return get_cache(name, w, h, bitcount);
    }

    void* __cdecl SharedCache_t::CreateSharedCache(int w, int h, int bitcount, char* name) {
        auto a_exfunc = reinterpret_cast<AviUtl::ExFunc*>(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        SharedCacheInfo* sci = GetOrCreateSharedCacheInfo();
        if (sci == nullptr) {
            return create_cache(w, h, bitcount, name);
        }

        unsigned int priority_min = 0xFFFFFFFF;
        int new_id = 0;
        int i;
        for (i = 0; i < cache_count; i++) {
            if (sci[i].name[0] != '\0' && lstrcmpiA(name, sci[i].name) == 0) {
                new_id = i;
                break;
            }
            else if (0 < priority_min) {
                if (shared_mem_info[i] == nullptr) {
                    priority_min = 0;
                    new_id = i;
                }
                else if (sci[i].priority < priority_min) {
                    priority_min = sci[i].priority;
                    new_id = i;
                }
            }
        }
        if (i == cache_count && cache_count < SHARECACHEINFO_N) {
            new_id = i;
            cache_count++;
        }

        lstrcpyA(sci[new_id].name, name);
        sci[new_id].w = w;
        sci[new_id].h = h;
        sci[new_id].bitcount = bitcount;
        sci[new_id].priority = priority_count++;
        if (shared_mem_info[new_id] != nullptr) {
            a_exfunc->delete_shared_mem(std::bit_cast<int32_t>(&shared_mem_info[new_id]), shared_mem_info[new_id]);
        }

        if (void* ptr = a_exfunc->create_shared_mem(std::bit_cast<int32_t>(&shared_mem_info[new_id]), 0, calc_cache_size(w, h, bitcount), &shared_mem_info[new_id])) return ptr;

        sci = GetOrCreateSharedCacheInfo();
        if (sci != nullptr) {
            sci[new_id].name[0] = '\0';
            sci[new_id].priority = 0;
        }
        return create_cache(w, h, bitcount, name);
    }


} // namespace patch
#endif // ifdef PATCH_SWITCH_SHARED_CACHE
