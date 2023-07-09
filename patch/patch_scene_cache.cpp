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

#include "patch_scene_cache.hpp"

#ifdef PATCH_SWITCH_SCENE_CACHE

namespace {
	constexpr int state_shift = 6;

	int32_t make_key1(int scene_idx) {
		return GLOBAL::exedit_base + OFS::ExEdit::get_scene_image
			+ scene_idx
			+ ((*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::is_saving) & 1) << state_shift)
			+ ((*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::fast_process) & 1) << (state_shift + 1));
	}
	int32_t make_key2(int frame, int subframe) {
		return frame * 100 + subframe;
	}

	struct SceneCacheHeader {
		int w, h;
		int _padding[2];

		void* get_img() {
			return this + 1;
		}

		std::pair<int, int> get_wh() const {
			return std::make_pair(w, h);
		}

		void set_wh(int w, int h) {
			this->w = w;
			this->h = h;
		}

		static int get_img_bytesize(int yc_size, int line, int h) {
			return yc_size * line * h;
		}
	};
}

namespace patch {

	void* __cdecl scene_cache_t::get_scene_image_wrap(ExEdit::ObjectFilterIndex ofi, ExEdit::FilterProcInfo* efpip, int scene_idx, int frame, int subframe, int* w, int* h) {
		if (*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::SceneDisplaying) != 0) {
			return get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		}
		
		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);

		auto key1 = make_key1(scene_idx);
		auto key2 = make_key2(frame, subframe);
		
		if (auto cache_header = static_cast<SceneCacheHeader*>(a_exfunc->get_shared_mem(key1, key2, nullptr))) {
			std::tie(*w, *h) = cache_header->get_wh();
			return cache_header->get_img();
		}

		auto t0 = std::chrono::system_clock::now();
		void* img_ptr = get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		if (img_ptr == nullptr) return nullptr;

		if (std::chrono::milliseconds{ threshold_time_ms } < std::chrono::system_clock::now() - t0) {
			int yc_size;
			if (reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::scene_has_alpha)(scene_idx)) {
				yc_size = 8;
			} else {
				yc_size = 6;
			}
			const auto img_size = SceneCacheHeader::get_img_bytesize(yc_size, efpip->scene_line, *h);
			auto cache_header = static_cast<SceneCacheHeader*>(a_exfunc->create_shared_mem(key1, key2, sizeof(SceneCacheHeader) + img_size, nullptr));
			if (cache_header == nullptr) return img_ptr;
			cache_header->set_wh(*w, *h);
			std::memcpy(cache_header->get_img(), img_ptr, img_size);
		}
		return img_ptr;
	}

	void* __cdecl scene_cache_t::get_scene_image_mask_wrap(ExEdit::ObjectFilterIndex ofi, ExEdit::FilterProcInfo* efpip, int scene_idx, int frame, int subframe, int* w, int* h) {
		if (*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::SceneDisplaying) != 0) {
			return get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		}

		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);

		auto key1 = make_key1(scene_idx);
		auto key2 = make_key2(frame, subframe);
		
		if (auto cache_header = static_cast<SceneCacheHeader*>(a_exfunc->get_shared_mem(key1, key2, nullptr))) {
			if (reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::scene_has_alpha)(scene_idx)) {
				if (void* ptr = reinterpret_cast<void*(__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateSceneBufYCA)(ofi, cache_header->w, cache_header->h, efpip->v_func_idx + 1, 1)) {
					std::memcpy(ptr, cache_header->get_img(), SceneCacheHeader::get_img_bytesize(8, efpip->obj_line, cache_header->h));
					return cache_header->get_img();
				}
			} else {
				if (void* ptr = reinterpret_cast<void*(__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateSceneBufYC)(ofi, cache_header->w, cache_header->h, efpip->v_func_idx + 1, 1)) {
					std::memcpy(ptr, cache_header->get_img(), SceneCacheHeader::get_img_bytesize(6, efpip->obj_line, cache_header->h));
					return cache_header->get_img();
				}
			}
		}

		auto t0 = std::chrono::system_clock::now();
		void* img_ptr = get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		if (img_ptr == nullptr) return nullptr;
		if (std::chrono::milliseconds{ threshold_time_ms } < std::chrono::system_clock::now() - t0) {
			int yc_size;
			if (reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::scene_has_alpha)(scene_idx)) {
				yc_size = 8;
			} else {
				yc_size = 6;
			}
			const auto img_size = SceneCacheHeader::get_img_bytesize(yc_size, efpip->obj_line, *h);
			auto cache_header = static_cast<SceneCacheHeader*>(a_exfunc->create_shared_mem(key1, key2, sizeof(SceneCacheHeader) + img_size, nullptr));
			if (cache_header == nullptr) return img_ptr;
			cache_header->set_wh(*w, *h);
			std::memcpy(cache_header->get_img(), img_ptr, img_size);
		}
		return img_ptr;
	}

	void scene_cache_t::delete_scene_cache() {
		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
		for (int i = 1; i < 50; i++) {
			for (int state = 0; state < 3; state++) {
				const auto key1 = GLOBAL::exedit_base + OFS::ExEdit::get_scene_image
					+ i
					+ (state << state_shift);
				a_exfunc->delete_shared_mem(key1, nullptr);
			}
		}
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SCENE_CACHE
