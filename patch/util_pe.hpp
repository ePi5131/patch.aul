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
#include <vector>
#include <set>
#include <unordered_map>
#include <optional>
#include <string>

#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include "util_int.hpp"

/// <summary>
/// reloc領域から指定アドレスが使われているアドレスを探す
/// </summary>
/// <param name="hModule">モジュール</param>
/// <param name="target_ofs">探すアドレスのオフセット</param>
/// <returns>アドレスが入った配列 順序の保証は特にない</returns>
inline auto search_reloc(HMODULE hModule, i32 target_ofs) noexcept {
	std::vector<void*> ret;

	auto base = reinterpret_cast<DWORD>(hModule);

	auto target = base + target_ofs;

	ULONG size;
	auto reloc = static_cast<PIMAGE_BASE_RELOCATION>(ImageDirectoryEntryToData(hModule, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &size));
	if (!reloc) return ret;

	auto end_of_directory = reinterpret_cast<void*>(reinterpret_cast<ULONG_PTR>(reloc) + size);

	while (reloc < end_of_directory && reloc->SizeOfBlock) {
		auto end_of_block = reinterpret_cast<void*>(reinterpret_cast<ULONG_PTR>(reloc) + reloc->SizeOfBlock);
		auto TypeOffset = reinterpret_cast<WORD*>(reinterpret_cast<ULONG_PTR>(reloc) + sizeof(IMAGE_BASE_RELOCATION));

		while (TypeOffset < end_of_block) {
			auto type = (*TypeOffset >> 12) & 0xf;
			auto offset = *TypeOffset & 0xfff;

			if (type == IMAGE_REL_BASED_HIGHLOW) {
				auto adr = reinterpret_cast<PULONG_PTR>(base + reloc->VirtualAddress + offset);
				if (*adr == target) {
					ret.push_back(adr);
				}
			}

			TypeOffset++;
		}

		reloc = reinterpret_cast<PIMAGE_BASE_RELOCATION>(end_of_block);
	}

	return ret;
}

/// <summary>
/// search_reloc(HMODULE, i32) を複数オフセットアドレスで同時に行う
/// </summary>
/// <param name="hModule">モジュール</param>
/// <param name="target_ofs">探すオフセットアドレスのset</param>
/// <returns> unordered_map[オフセットアドレス] = オフセットアドレスが使われているアドレスの配列 </returns>
inline auto search_reloc(HMODULE hModule, const std::set<i32>& target_ofs) {
	std::unordered_map<i32, std::vector<void*>> ret;

	auto base = reinterpret_cast<DWORD>(hModule);

	ULONG size;
	auto reloc = static_cast<PIMAGE_BASE_RELOCATION>(ImageDirectoryEntryToData(hModule, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &size));
	if (!reloc) return ret;

	auto end_of_directory = reinterpret_cast<void*>(reinterpret_cast<ULONG_PTR>(reloc) + size);

	while (reloc < end_of_directory && reloc->SizeOfBlock) {
		auto end_of_block = reinterpret_cast<void*>(reinterpret_cast<ULONG_PTR>(reloc) + reloc->SizeOfBlock);
		auto TypeOffset = reinterpret_cast<WORD*>(reinterpret_cast<ULONG_PTR>(reloc) + sizeof(IMAGE_BASE_RELOCATION));

		while (TypeOffset < end_of_block) {
			auto type = (*TypeOffset >> 12) & 0xf;
			auto offset = *TypeOffset & 0xfff;

			if (type == IMAGE_REL_BASED_HIGHLOW) {
				auto adr = reinterpret_cast<PULONG_PTR>(base + reloc->VirtualAddress + offset);
				i32 ofs = *adr - base;

				if (target_ofs.contains(ofs)) {
					if (auto itr = ret.find(ofs); itr != ret.end()) {
						itr->second.push_back(adr);
					}
					else {
						ret.try_emplace(ofs, std::vector<void*>{ adr });
					}
				}
			}

			TypeOffset++;
		}

		reloc = reinterpret_cast<PIMAGE_BASE_RELOCATION>(end_of_block);
	}

	return ret;
}

/// <summary>
/// 序数を関数名に変換する
/// </summary>
/// <param name="hMod">モジュール</param>
/// <param name="ordinal">序数</param>
/// <returns>関数の名前 見つからなければstd::nullopt</returns>
inline std::optional<std::string> ordinal_to_name(HMODULE hMod, DWORD ordinal) noexcept {
	auto base = reinterpret_cast<DWORD>(hMod);
	if (!base)return std::nullopt;

	ULONG size;
	auto iedp = static_cast<PIMAGE_EXPORT_DIRECTORY>(ImageDirectoryEntryToData(hMod, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size));
	auto names = reinterpret_cast<DWORD*>(base + iedp->AddressOfNames);
	auto ordinals = reinterpret_cast<WORD*>(base + iedp->AddressOfNameOrdinals);
	for (DWORD i = 0; i < iedp->NumberOfFunctions; i++) {
		for (DWORD j = 0; j < iedp->NumberOfNames; j++) {
			if (ordinals[j] != i)continue;
			if (ordinals[j] == ordinal) {
				return reinterpret_cast<char*>(base + names[j]);
			}
		}
	}
	return std::nullopt;
}

/// <summary>
/// 関数名を序数に変換する
/// </summary>
/// <param name="hMod">モジュール</param>
/// <param name="name">関数の名前</param>
/// <returns>序数 関数が無ければ-1</returns>
inline DWORD name_to_ordinal(HMODULE hMod, std::string_view name) noexcept {
	if (!hMod)return -1;
	auto base = reinterpret_cast<DWORD>(hMod);

	ULONG size;
	auto iedp = static_cast<PIMAGE_EXPORT_DIRECTORY>(ImageDirectoryEntryToData(hMod, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size));
	auto names = reinterpret_cast<DWORD*>(base + iedp->AddressOfNames);
	auto ordinals = reinterpret_cast<WORD*>(base + iedp->AddressOfNameOrdinals);
	for (DWORD i = 0; i < iedp->NumberOfFunctions; i++) {
		for (DWORD j = 0; j < iedp->NumberOfNames; j++) {
			if (ordinals[j] != i)continue;
			if (lstrcmpiA(reinterpret_cast<char*>(base + names[j]), name.data()) == 0) {
				return ordinals[j];
			}
		}
	}
	return -1;
}

/// <summary>
/// 指定モジュールのIATから関数のあるポインタを探し出す
/// </summary>
/// <param name="target">モジュール</param>
/// <param name="mod_name">探す関数のあるDLLの名前</param>
/// <param name="func_name">探す関数の名前</param>
/// <returns>見つけたポインタ 見つからなければnullptr</returns>
inline void* search_import(HMODULE target, std::string_view mod_name, std::string_view func_name) noexcept {
	auto base = reinterpret_cast<DWORD>(target);
	ULONG size;
	auto iidp = static_cast<PIMAGE_IMPORT_DESCRIPTOR>(ImageDirectoryEntryToData(target, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size));
	if (!iidp)return nullptr;

	auto funcidx = name_to_ordinal(GetModuleHandleA(mod_name.data()), func_name);

	for (; iidp->Name; iidp++) {
		if (lstrcmpiA(reinterpret_cast<char*>(base + iidp->Name), mod_name.data()) != 0) continue;

		auto itdp = reinterpret_cast<PIMAGE_THUNK_DATA>(base + iidp->FirstThunk);
		auto oitdp = reinterpret_cast<PIMAGE_THUNK_DATA>(base + iidp->OriginalFirstThunk);
		for (; itdp->u1.Function; itdp++, oitdp++) {
			if (IMAGE_SNAP_BY_ORDINAL(oitdp->u1.Ordinal)) {
				if (IMAGE_ORDINAL(oitdp->u1.Ordinal) == funcidx)
					return &itdp->u1.Function;
			}
			else {
				auto iibnp = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(base + oitdp->u1.AddressOfData);
				if (lstrcmpiA(iibnp->Name, func_name.data()) == 0) {
					return &itdp->u1.Function;
				}
			}
		}
	}
	return nullptr;
}
/// <summary>
/// 指定モジュールのIATから関数のあるポインタを探し出す
/// </summary>
/// <param name="target">モジュール</param>
/// <param name="mod_name">探す関数のあるDLLの名前</param>
/// <param name="func_name">探す関数の名前</param>
/// <param name="func_ordinal">探す関数の序数</param>
/// <returns>見つけたポインタ 見つからなければnullptr</returns>
inline void* search_import(HMODULE hModule, std::string_view mod_name, std::string_view func_name, unsigned int func_ordinal) noexcept {
	auto base = reinterpret_cast<DWORD>(hModule);
	ULONG size;
	auto iidp = static_cast<PIMAGE_IMPORT_DESCRIPTOR>(ImageDirectoryEntryToData(hModule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size));
	if (!iidp)return nullptr;
	for (; iidp->Name; iidp++) {
		if (lstrcmpiA(reinterpret_cast<char*>(base + iidp->Name), mod_name.data()) != 0) continue;

		auto itdp = reinterpret_cast<PIMAGE_THUNK_DATA>(base + iidp->FirstThunk);
		auto oitdp = reinterpret_cast<PIMAGE_THUNK_DATA>(base + iidp->OriginalFirstThunk);
		for (; itdp->u1.Function; itdp++, oitdp++) {
			if (IMAGE_SNAP_BY_ORDINAL(oitdp->u1.Ordinal)) {
				if (IMAGE_ORDINAL(oitdp->u1.Ordinal) == func_ordinal)
					return static_cast<void*>(&itdp->u1.Function);
			}
			else {
				auto iibnp = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(base + oitdp->u1.AddressOfData);
				if (lstrcmpiA(iibnp->Name, func_name.data()) == 0) {
					return static_cast<void*>(&itdp->u1.Function);
				}
			}
		}
	}
	return nullptr;
}
