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
#include <cstdint>
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace patch {
    // ExceptionHistory の要素型
    // オフセットアドレスと例外コード
    struct ExceptionHistoryEntry {
        uint32_t address;
        uint32_t code;
        struct Hash {
            using result_type = uint32_t;

            result_type operator()(const ExceptionHistoryEntry& history) const {
                return history.address ^ history.code;
            }
        };

        bool operator==(const ExceptionHistoryEntry& x) const {
            return this->address == x.address && this->code == x.code;
        }
    };

    // しばらく同種のメッセージの表示を停止する のためのもの
    // 過去に出た例外のコードとオフセットアドレスを連想配列に記憶する
    inline struct ExceptionHistory {
        std::mutex mtx;
        std::unordered_map<ExceptionHistoryEntry, std::chrono::system_clock::time_point, ExceptionHistoryEntry::Hash> map;

        // 過去10分以内に出たことのある例外ならtrueを返す
        [[nodiscard]] bool check(const ExceptionHistoryEntry& key) {
            using namespace std::chrono_literals;
            
            std::lock_guard lock(mtx);

            if (auto itr = map.find(key); itr != map.end()) {
                if (std::chrono::system_clock::now() - itr->second < std::chrono::system_clock::duration(10min)) {
                    return true;
                }
                map.erase(itr);
            }
            return false;
        }
    } exception_history;
} // namespace patch
