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
#include <utility>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <optional>
#include <mutex>
#include <compare>

#include <Windows.h>

#include "hash.hpp"

struct ModulesDataEntry {
	uintptr_t begin;
	uintptr_t end;
	std::string name;
	std::optional<SHA256> hash;
};

inline static std::pair<std::vector<ModulesDataEntry>, std::string_view> getModulesData() {
	std::vector<ModulesDataEntry> ret;
	auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, NULL);
	if (snapshot != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 entry{ .dwSize = sizeof(MODULEENTRY32) };
		if (Module32First(snapshot, &entry)) {
			do {
				ret.emplace_back(
					reinterpret_cast<uintptr_t>(entry.modBaseAddr),
					reinterpret_cast<uintptr_t>(entry.modBaseAddr) + static_cast<uintptr_t>(entry.modBaseSize),
					entry.szModule,
					SHA256::make_opt(entry.szExePath)
				);
			} while (Module32Next(snapshot, &entry));
			CloseHandle(snapshot);
		}
		else {
			auto err = GetLastError();
			CloseHandle(snapshot);
			return { ret,format("LastError:{}", err) };
		}
	}
	else {
		auto err = GetLastError();
		CloseHandle(snapshot);
		return { ret,format("LastError:{}", err) };
	}
	std::sort(ret.begin(), ret.end(), [](auto a, auto b) { return a.begin < b.begin; });
	return { ret, "" };
}

inline std::weak_ordering operator<=>(const ModulesDataEntry& a, const ModulesDataEntry& b) {
	return a.begin <=> b.begin;
}

class ModulesData {
	inline static std::vector<ModulesDataEntry> modules_data_cache;
	inline static std::set<std::string> modules_data_cache_name;

public:
	inline static std::mutex mtx;

	static void update() {
		auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, NULL);
		if (snapshot != INVALID_HANDLE_VALUE) {
			MODULEENTRY32 entry{ .dwSize = sizeof(MODULEENTRY32) };
			if (Module32First(snapshot, &entry)) {
				do {
					if (!modules_data_cache_name.contains(entry.szModule)) {
						modules_data_cache_name.insert(entry.szModule);
						modules_data_cache.emplace_back(
							reinterpret_cast<uintptr_t>(entry.modBaseAddr),
							reinterpret_cast<uintptr_t>(entry.modBaseAddr) + static_cast<uintptr_t>(entry.modBaseSize),
							entry.szModule,
							SHA256::make_opt(entry.szExePath)
						);
					}
				} while (Module32Next(snapshot, &entry));
				CloseHandle(snapshot);
			}
			else {
				auto err = GetLastError();
				CloseHandle(snapshot);
			}
		}
		else {
			auto err = GetLastError();
			CloseHandle(snapshot);
		}
	}

	static const auto& get() {
		if (!std::is_sorted(modules_data_cache.cbegin(), modules_data_cache.cend(), [](auto a, auto b) { return a.begin < b.begin; })) {
			std::sort(modules_data_cache.begin(), modules_data_cache.end(), [](auto a, auto b) { return a.begin < b.begin; });
		}
		return modules_data_cache;
	}
};
