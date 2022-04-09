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
#ifdef PATCH_SWITCH_LUA_COPYBUFFER_SMEM

#include <string>
#include <string_view>
#include <memory>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <variant>
#include <optional>
#include <compare>

#include <aviutl.hpp>
#include <exedit.hpp>

#include "mylua.hpp"

namespace patch {
	using namespace std::literals::string_literals;

	// init at exedit load
	// obj.copybufferによるキャッシュを共有メモリに確保する
	inline struct copybuffer_smem_t {
	private:

		struct YCABuffer {
			int w, h;
			ExEdit::PixelYCA buf[];
		};

		inline static std::unordered_map<std::string, std::unique_ptr<YCABuffer>> smem;

		struct BufferBasic{};
		struct BufferNamed : public BufferBasic {
			std::string name;
		};
		struct BufferObj : public BufferBasic {};
		struct BufferFrm : public BufferBasic {};
		struct BufferTmp : public BufferBasic {};
		struct BufferCache : public BufferNamed {};
		struct BufferImage : public BufferNamed {};
		using Buffer = std::variant<std::monostate, BufferObj, BufferFrm, BufferTmp, BufferCache, BufferImage>;

		enum class BufferId {
			Obj, Tmp, Frm, Cache, Image,
			Invalid = -1
		};

		inline static Buffer getBufferIdAll(std::string_view name) {
			if (auto cmp = ::strnicmp(name.data(), "image:", 6) <=> 0; cmp < 0) {
				if (auto cmp = ::stricmp(name.data(), "frm") <=> 0; cmp < 0) {
					if (auto cmp = ::strnicmp(name.data(), "cache:", 6) <=> 0; cmp != 0) return std::monostate{};
					return BufferCache{ BufferNamed{ .name = { name.begin() + 6, name.end() } } };
				}
				else if(cmp == 0){
					// frm
					return BufferFrm{};
				}
			}
			else if(cmp == 0) {
				// image:
				return BufferImage{ BufferNamed{ .name = { name.begin() + 6, name.end() } } };
			}
			else {
				if (auto cmp = ::stricmp(name.data(), "obj") <=> 0; cmp == 0) {
					// obj
					return BufferObj{};
				}
				else {
					// tmp
					if (auto cmp = ::stricmp(name.data(), "tmp") <=> 0; cmp != 0) return std::monostate{};
					return BufferTmp{}; 
				}
			}
			return std::monostate{};
		}

		inline static Buffer getBufferIdDst(std::string_view name) {
			if (auto cmp = ::stricmp(name.data(), "obj") <=> 0; cmp < 0) {
				// cache:
				if (auto cmp = ::strnicmp(name.data(), "cache:", 6) <=> 0; cmp != 0) return std::monostate{};
				return BufferCache{ BufferNamed{ .name = { name.begin() + 6, name.end() } } };
			}
			else if(cmp == 0) {
				// obj:
				return BufferObj{};
			}
			else {
				// tmp
				if (auto cmp = ::stricmp(name.data(), "tmp") <=> 0; cmp != 0) return std::monostate{};
				return BufferTmp{};
			}
			return std::monostate{};
		}

		static Buffer getBufferIdFromReadonly(std::string_view name) {
			if (auto cmp = ::stricmp(name.data(), "obj") <=> 0; cmp == 0) {
				// obj:
				return BufferObj{};
			}
			else {
				// tmp
				if (auto cmp = ::stricmp(name.data(), "tmp") <=> 0; cmp != 0) return std::monostate{};
				return BufferTmp{};
			}
			return std::monostate{};
		}

		struct VisitorSrc {
			std::string_view dst_name;
			bool operator()(std::monostate) { return false; }
			bool operator()(BufferObj) {
				auto dst = getBufferIdDst(dst_name);
				if(std::holds_alternative<std::monostate>(dst)) return false;
				if(std::holds_alternative<BufferObj>(dst)) return true;

				return true;
			}
			bool operator()(BufferTmp) {
				auto dst = getBufferIdDst(dst_name);
				if(std::holds_alternative<std::monostate>(dst)) return false;
				if(std::holds_alternative<BufferTmp>(dst)) return true;

				return true;
			}
			bool operator()(BufferFrm) {
				auto dst = getBufferIdFromReadonly(dst_name);
				if(std::holds_alternative<std::monostate>(dst)) return false;

				return true;
			}
			bool operator()(BufferCache src) {
				if(src.name.empty()) return false;
				auto dst = getBufferIdDst(dst_name);
				if(std::holds_alternative<std::monostate>(dst)) return false;
				if(std::holds_alternative<BufferCache>(dst)) {
					return std::get<BufferCache>(dst).name == src.name;
				}

				return true;
			}
			bool operator()(BufferImage src) {
				if(src.name.empty()) return false;
				auto dst = getBufferIdFromReadonly(dst_name);
				if(std::holds_alternative<std::monostate>(dst)) return false;
				
				return true;
			}
		};

		static int copybuffer_override(lua_State* L) {
			auto src = getBufferIdAll(lua_tostring(L, 2));

			lua_pushboolean(L, std::visit(VisitorSrc{lua_tostring(L, 1)}, src));

			return 1;
		}

	public:
		void operator()() {

		}
	} copybuffer_smem;
} // namespace patch
#endif // ifdef PATCH_SWITCH_LUA_COPYBUFFER_SMEM
