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

#ifdef PATCH_SWITCH_SHARED_CACHE
namespace patch {

	void* __cdecl SharedCache_t::GetOrCreateSharedCache(ExEdit::ObjectFilterIndex ofi, int w, int h, int bitcount, int v_func_id, int* old_cache_exists) {
		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
		auto e_exfunc = (ExEdit::Exfunc*)(GLOBAL::exedit_base + OFS::ExEdit::exfunc);

		if (0 <= (int)ofi) {
			ofi = e_exfunc->get_start_idx(ofi);
		}

		unsigned int key1 = (int)ofi ^ (v_func_id << 14) ^ (bitcount << 25);
		unsigned int key2 = (w - 1) ^ ((h - 1) << 16);

		void* smem = a_exfunc->get_shared_mem(key1, key2, NULL);
		if (smem != NULL) {
			if (old_cache_exists != NULL) {
				*old_cache_exists = 1;
			}
			return smem;
		}
		if (old_cache_exists != NULL) {
			*old_cache_exists = 0;
		}
		int mem_size = (((w * bitcount + 7) >> 3) + 3 & 0xfffffffc) * h;
		return a_exfunc->create_shared_mem(key1, key2, mem_size, NULL);
	}


    int calc_cache_size(int w, int h, int bitcount) {
        return ((((w * bitcount + 7) >> 3) + 3) & 0xfffffffc) * h + 16;
    }

    SharedCache_t::SharedCacheInfo* SharedCache_t::InitSharedCacheInfo() {
        auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        cache_count = 0;
        priority_count = 1;
        for (int i = 0; i < SHARECACHEINFO_N; i++) {
            shared_mem_info[i] = NULL;
        }
        return (SharedCacheInfo*)a_exfunc->create_shared_mem((int)&cache_count, (int)&cache_count, SHARECACHEINFO_SIZE, NULL);
    }
    SharedCache_t::SharedCacheInfo* SharedCache_t::GetOrCreateSharedCacheInfo() {
        auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        auto sci = (SharedCacheInfo*)a_exfunc->get_shared_mem((int)&cache_count, (int)&cache_count, NULL);
        if (sci == NULL) {
            return InitSharedCacheInfo();
        }
        return sci;
    }

    void* __cdecl SharedCache_t::GetSharedCache(char* name, int* w, int* h, int* bitcount) {
        auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        SharedCacheInfo* sci = GetOrCreateSharedCacheInfo();
        for (int i = 0; i < cache_count; i++) {
            if (sci[i].name[0] != '\0' && lstrcmpiA(name, sci[i].name) == 0) {
                if (shared_mem_info[i] == NULL) { // 1.10側で破棄されている状態
                    sci[i].name[0] = '\0';
                    sci[i].priority = 0;
                    return get_cache(name, w, h, bitcount);
                }
                sci[i].priority = priority_count++;
                *w = sci[i].w;
                *h = sci[i].h;
                *bitcount = sci[i].bitcount;
                void* ptr = a_exfunc->get_shared_mem((int)&shared_mem_info[i], 0, shared_mem_info[i]);
                if (ptr != NULL) {
                    return ptr;
                }
                *w = 0;
                *h = 0;
                *bitcount = 0;
                sci = GetOrCreateSharedCacheInfo();
                if (sci != NULL) {
                    sci[i].name[0] = '\0';
                    sci[i].priority = 0;
                }
                return get_cache(name, w, h, bitcount);
            }
        }
        return get_cache(name, w, h, bitcount);
    }


    void* __cdecl SharedCache_t::CreateSharedCache(int w, int h, int bitcount, char* name) {
        auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        SharedCacheInfo* sci = GetOrCreateSharedCacheInfo();
        if (sci == NULL) {
            return create_cache(w, h, bitcount, name);
        }

        unsigned int priority_min = 0xFFFFFFFF;
        int new_id = 0;
        int i;
        BOOL del_flag = FALSE;
        for (i = 0; i < cache_count; i++) {
            if (sci[i].name[0] != '\0' && lstrcmpiA(name, sci[i].name) == 0) {
                new_id = i;
                if (shared_mem_info[i] != NULL) {
                    del_flag = TRUE;
                }
                break;
            } else if (0 < priority_min) {
                if (shared_mem_info[i] == NULL) {
                    priority_min = 0;
                    new_id = i;
                } else if (sci[i].priority < priority_min) {
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
        if (del_flag) {
            a_exfunc->delete_shared_mem((int)&shared_mem_info[new_id], shared_mem_info[new_id]);
        }

        void* ptr = a_exfunc->create_shared_mem((int)&shared_mem_info[new_id], 0, calc_cache_size(w, h, bitcount), &shared_mem_info[new_id]);
        if (ptr != NULL) {
            return ptr;
        }

        sci = GetOrCreateSharedCacheInfo();
        if (sci != NULL) {
            sci[new_id].name[0] = '\0';
            sci[new_id].priority = 0;
        }
        return create_cache(w, h, bitcount, name);
    }


} // namespace patch
#endif // ifdef PATCH_SWITCH_SHARED_CACHE