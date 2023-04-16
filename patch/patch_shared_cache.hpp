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

#pragma once
#include "macro.h"

#ifdef PATCH_SWITCH_SHARED_CACHE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 画像データのキャッシュの部分で、共有メモリ使用に置き換えても大丈夫な部分を置き換えていく（全てを置き換えてしまうと0.93rc1バグを再現する）

    inline class SharedCache_t {

        struct SharedCacheInfo { // size = 276
            char name[260];
            int w;
            int h;
            int bitcount;
            unsigned int priority;
        };

        static constexpr auto SHARECACHEINFO_N = 1024;
        static constexpr auto SHARECACHEINFO_SIZE = (SHARECACHEINFO_N * sizeof(SharedCacheInfo));

        inline static AviUtl::SharedMemoryInfo* shared_mem_info[SHARECACHEINFO_N];
        inline static int cache_count = 0;
        inline static int priority_count = 1;

        static void* __cdecl GetOrCreateSharedCache(ExEdit::ObjectFilterIndex ofi, int w, int h, int bitcount, int v_func_id, int* old_cache_exists);
        static SharedCacheInfo* GetOrCreateSharedCacheInfo();
        static SharedCacheInfo* InitSharedCacheInfo();
        static void* __cdecl GetSharedCache(char* name, int* w, int* h, int* bitcount);
        static void* __cdecl CreateSharedCache(int w, int h, int bitcount, char* name);


        inline static void*(__cdecl* get_cache)(char* name, int* w, int* h, int* bitcount);
        inline static void*(__cdecl* create_cache)(int w, int h, int bitcount, char* name);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "shared_cache";
    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            get_cache = reinterpret_cast<decltype(get_cache)>(GLOBAL::exedit_base + OFS::ExEdit::GetCache);
            create_cache = reinterpret_cast<decltype(create_cache)>(GLOBAL::exedit_base + OFS::ExEdit::CreateCache);

            // 上のオブジェクトでクリッピング関係
            ReplaceNearJmp(GLOBAL::exedit_base + 0x049a4c, &GetOrCreateSharedCache);

            // 音声ディレイ
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01c1ea, &GetOrCreateSharedCache);

            // copybuffer cache
            ReplaceNearJmp(GLOBAL::exedit_base + 0x060099, &GetSharedCache);
            ReplaceNearJmp(GLOBAL::exedit_base + 0x060306, &GetSharedCache);
            ReplaceNearJmp(GLOBAL::exedit_base + 0x060530, &CreateSharedCache);
            
        }

        void switching(bool flag) {
            enabled = flag;
        }

        bool is_enabled() { return enabled; }
        bool is_enabled_i() { return enabled_i; }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
            });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }
    } SharedCache;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SHARED_CACHE
