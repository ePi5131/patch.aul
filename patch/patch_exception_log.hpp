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
#ifdef PATCH_SWITCH_EXCEPTION_LOG

#include <unordered_map>
#include <sstream>
#include <fstream>
#include <string>
#include <format>
#include <variant>

#include <Windows.h>

#include <TlHelp32.h>

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <boost/scope_exit.hpp>

#include <winwrap.hpp>

#include "global.hpp"
#include "util.hpp"
#include "resource.h"
#include "hash.hpp"
#include "moduledata.hpp"
#include "gate.hpp"

#include "patch_exception_log_dialog.hpp"

namespace patch {
	// init at dllload
	inline class exception_log_t {
#define exception_code_name_macro(name) { name, #name }
		// 例外コードを文字列にするための連想配列
		static inline const std::unordered_map<DWORD, std::string_view> exception_code_name{
			exception_code_name_macro(STILL_ACTIVE),
			exception_code_name_macro(EXCEPTION_GUARD_PAGE),
			exception_code_name_macro(EXCEPTION_DATATYPE_MISALIGNMENT),
			exception_code_name_macro(EXCEPTION_BREAKPOINT),
			exception_code_name_macro(EXCEPTION_SINGLE_STEP),
			exception_code_name_macro(EXCEPTION_ACCESS_VIOLATION),
			exception_code_name_macro(EXCEPTION_IN_PAGE_ERROR),
			exception_code_name_macro(EXCEPTION_INVALID_HANDLE),
			exception_code_name_macro(EXCEPTION_ILLEGAL_INSTRUCTION),
			exception_code_name_macro(EXCEPTION_NONCONTINUABLE_EXCEPTION),
			exception_code_name_macro(EXCEPTION_INVALID_DISPOSITION),
			exception_code_name_macro(EXCEPTION_ARRAY_BOUNDS_EXCEEDED),
			exception_code_name_macro(EXCEPTION_FLT_DENORMAL_OPERAND),
			exception_code_name_macro(EXCEPTION_FLT_DIVIDE_BY_ZERO),
			exception_code_name_macro(EXCEPTION_FLT_INEXACT_RESULT),
			exception_code_name_macro(EXCEPTION_FLT_INVALID_OPERATION),
			exception_code_name_macro(EXCEPTION_FLT_OVERFLOW),
			exception_code_name_macro(EXCEPTION_FLT_STACK_CHECK),
			exception_code_name_macro(EXCEPTION_FLT_UNDERFLOW),
			exception_code_name_macro(EXCEPTION_INT_DIVIDE_BY_ZERO),
			exception_code_name_macro(EXCEPTION_INT_OVERFLOW),
			exception_code_name_macro(EXCEPTION_PRIV_INSTRUCTION),
			exception_code_name_macro(EXCEPTION_STACK_OVERFLOW),
			exception_code_name_macro(CONTROL_C_EXIT),
			{ 0xe0434352L, "* .Net exception" },
			{ 0xc06d007eL, "* DLL delay load failed" },
			{ 0xe06d7363L, "* C++ exception" },
			{ 0x80000100L, "* Unimplemented function" },
		};
#undef exception_code_name_macro

		// 例外コードごとに何か特別なフォーマットをしたいとき用の関数を置く連想配列
		static inline const std::unordered_map<DWORD, std::string(*)(PEXCEPTION_RECORD)> exception_format{
			{ EXCEPTION_ACCESS_VIOLATION, [](PEXCEPTION_RECORD erp){
				return "read/write : {}\r\naccessed address : 0x{:08x}\r\n"_fmt(
					erp->ExceptionInformation[0],
					erp->ExceptionInformation[1]
				);
			}},
			{ EXCEPTION_IN_PAGE_ERROR, [](PEXCEPTION_RECORD erp) {
				return "read/write : {}\r\naccessed address : 0x{:08x}\r\nNTSTATUS : {:08x}\r\n"_fmt(
					erp->ExceptionInformation[0],
					erp->ExceptionInformation[1],
					erp->ExceptionInformation[2]
				);
			}},
			{ 0xe06d7363L, [](PEXCEPTION_RECORD erp) {
				uint32_t d1 = load_i32(erp->ExceptionInformation[2] + 12);
				uint32_t d2 = load_i32(d1 + 4);
				uint32_t d3 = load_i32(d2 + 4);
				auto class_name = reinterpret_cast<const char*>(d3 + 8);
				auto exception = erp->ExceptionInformation[1];
				return "Information[0] : {:08x}\r\nclass_name : {}\r\nexception ptr : {:08x}\r\n"_fmt(
					erp->ExceptionInformation[0],
					class_name,
					exception
				);
			}},
		};
		
		// デフォルトの例外コードフォーマット
		static std::string exception_format_default(PEXCEPTION_RECORD erp) {
			std::ostringstream ss;
			for(DWORD i = 0; i < erp->NumberParameters; i++){
				format_to_os(ss, "[{}] : {:08x}\r\n", i, erp->ExceptionInformation[i]);
			}
			return ss.str();
		}

		static std::string format_eflags(DWORD eflags) {
			std::ostringstream ss;
			ss << "(";
			int c = 0;
			#define def(shift, name) if (eflags & (1 << shift)) { if(c++) ss << "|"; ss << #name; }
			def(0, CF);
			def(2, PF);
			def(4, AF);
			def(6, ZF);
			def(7, SF);
			def(8, TF);
			def(9, IF);
			def(10, DF);
			def(11, OF);
			#undef def
			if(c++) ss << "|"; ss << "IOPL" << ((eflags >> 12) & 0b11);
			#define def(shift, name) if (eflags & (1 << shift)) {  ss << "|" << #name; }
			def(14, NT);
			def(16, RF);
			def(17, VM);
			def(18, AC);
			def(19, VIF);
			def(20, VIP);
			def(21, ID);
			#undef def
			ss << ")";
			return ss.str();
		}

		static std::string format_registers(PCONTEXT cp) {
			std::ostringstream ss;
			ss << "registers\r\n";
			
			format_to_os(ss, "{} : {:08x}\r\n", "eax", cp->Eax);
			format_to_os(ss, "{} : {:08x}\r\n", "ecx", cp->Ecx);
			format_to_os(ss, "{} : {:08x}\r\n", "edx", cp->Edx);
			format_to_os(ss, "{} : {:08x}\r\n", "ebx", cp->Ebx);
			format_to_os(ss, "{} : {:08x}\r\n", "esi", cp->Esi);
			format_to_os(ss, "{} : {:08x}\r\n", "edi", cp->Edi);

			format_to_os(ss, "{} : {:08x}\r\n", "ebp", cp->Ebp);
			format_to_os(ss, "{} : {:08x}\r\n", "esp", cp->Esp);
			format_to_os(ss, "{} : {:08x}\r\n", "eip", cp->Eip);

			format_to_os(ss, "{} : {:08x} {}\r\n", "flag", cp->EFlags, format_eflags(cp->EFlags));


			format_to_os(ss, "{} : {:08x}\r\n", "cs", cp->SegCs);
			format_to_os(ss, "{} : {:08x}\r\n", "ds", cp->SegDs);
			format_to_os(ss, "{} : {:08x}\r\n", "ss", cp->SegSs);
			format_to_os(ss, "{} : {:08x}\r\n", "es", cp->SegEs);
			format_to_os(ss, "{} : {:08x}\r\n", "fs", cp->SegFs);
			format_to_os(ss, "{} : {:08x}\r\n", "gs", cp->SegGs);
			
			ss << "extend : ";
			for(size_t i = 0; i < sizeof(cp->ExtendedRegisters); i++){
				format_to_os(ss, "{:08x}", cp->ExtendedRegisters[i]);
			}
			ss << '\n' << '\n';

			return ss.str();
		}

		static std::tuple<std::string, DWORD> get_module_address(void* address, const std::vector<ModulesDataEntry>& data) {
			auto addr = reinterpret_cast<uintptr_t>(address);
			ModulesDataEntry d{0u,addr,""};
			auto itr = std::lower_bound(data.begin(),data.end(),d,[](auto a, auto b){ return a.end < b.end; });
			if(itr!=data.end()){
				if(itr->begin <= addr && addr < itr->end) {
					return { itr->name, addr - itr->begin };
				}
			}
			return { "", 0 };
		}

		static inline std::wstring log_dir;

		static auto get_log_name() {
			auto tid = GetCurrentThreadId();
			SYSTEMTIME st;
			GetLocalTime(&st);
			return std::make_tuple(
				"{:04}-{:02}-{:02}_{:02}-{:02}-{:02}-{}_{}.txt"_fmt(
					st.wYear, st.wMonth,  st.wDay,
					st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
					tid
				),
				L"{:04}-{:02}-{:02}_{:02}-{:02}-{:02}-{}_{}.txt"_fmt(
					st.wYear, st.wMonth, st.wDay,
					st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
					tid
				)
			);
		}

		inline static std::string format_detail(EXCEPTION_POINTERS* pExp, const char* plugin_name, const char* func_name, const std::vector<ModulesDataEntry>& data) {
			const auto code = pExp->ExceptionRecord->ExceptionCode;
			std::ostringstream ss;
			ss << "code : ";
			if(auto itr = exception_code_name.find(code); itr != exception_code_name.end()) {
				format_to_os(ss, "{} (0x{:08x})\r\n", itr->second, code);
			}else{
				format_to_os(ss, "unknown (0x{:08x})\r\n", code);
			}
			format_to_os(ss, "address : 0x{:08x}", reinterpret_cast<uintptr_t>(pExp->ExceptionRecord->ExceptionAddress));
			if (data.size()) {
				auto [modname, offset] = get_module_address(pExp->ExceptionRecord->ExceptionAddress, data);
				if(modname.size()) format_to_os(ss, " ({} + 0x{:08x})", modname, offset);
			}
			ss << "\r\n";
			format_to_os(ss, "plugin_name : {}\r\n", plugin_name);
			format_to_os(ss, "func_name : {}\r\n", func_name);

			if (auto itr = exception_format.find(code); itr != exception_format.end()) {
				ss << itr->second(pExp->ExceptionRecord);
			}
			else {
				ss << exception_format_default(pExp->ExceptionRecord);
			}
			return ss.str();
		}

		inline static std::string format_stack(uint32_t start, uint8_t* stack, uint32_t length) {
			auto esp_mod16 = start & 0xf;

			std::ostringstream ss;

			ss << "stack\r\n";

			auto esp = start & ~0xf;
			if (esp_mod16) {
				format_to_os(ss, "{:08x} ", esp);
				for (uint32_t i = 0; i < esp_mod16 * 2; i++) ss << ' ';
				for (uint32_t i = 0; i < 16 - esp_mod16; i++) {
					format_to_os(ss, "{:02x}", stack[i]);
				}
				ss << "\r\n";
				esp += 16;
			}
			for (uint32_t i = 0; i < (length - esp_mod16) / 16; i++) {
				format_to_os(ss, "{:08x} ", esp);
				for (int j = 0; j < 16; j++) format_to_os(ss, "{:02x}", stack[i * 16 + j + esp_mod16]);
				ss << "\r\n";
				esp += 16;
			}
			
			if (esp_mod16) {
				format_to_os(ss, "{:08x} ", esp);
				for (uint32_t i = length - esp_mod16; i < length; i++) {
					format_to_os(ss, "{:02x}", stack[i]);
				}
			}
			ss << "\r\n";

			return ss.str();
		}

		template<size_t N>
		static bool write_logfile(std::wstring& filename, std::string_view detail, EXCEPTION_POINTERS* pExp, const std::vector<ModulesDataEntry>& data, const uint32_t (&stack)[N]) {
			if(!PathFileExistsW(log_dir.c_str())) {
				if(!CreateDirectoryW(log_dir.c_str(), NULL)){
					return false;
				}
			}
			auto filepath = log_dir + filename;
			auto hfile = CreateFileW(filepath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
			if(hfile == INVALID_HANDLE_VALUE) {
				return false;
			}

			const auto code = pExp->ExceptionRecord->ExceptionCode;

			std::ostringstream ss;
			ss << "patch.aul (" PATCH_VERSION_STR ") debug info file\r\n\r\n";

			ss << detail;
			
			ss << "\r\n";

			ss << "modules\r\n";
			if (data.size()) {
				for(const auto& d : data) {
					if (d.hash.has_value()) {
						format_to_os(ss, "{:08x}-{:08x}:{} ({})\r\n", d.begin, d.end, d.name, d.hash->tostring());
					}
					else {
						format_to_os(ss, "{:08x}-{:08x}:{}\r\n", d.begin, d.end, d.name);
					}
				}
			}
			else {
				ss << "(Failed to get modules info)\r\n";
			}
			ss << "\r\n";

			ss << format_registers(pExp->ContextRecord);
			ss << "\r\n";


			ss << format_stack(pExp->ContextRecord->Esp - 128, (uint8_t*)&stack, N * 4);
			ss << "\r\n";


			auto str = ss.str();
			DWORD written;
			WriteFile(hfile, str.c_str(), str.size(), &written, NULL);

			CloseHandle(hfile);
			return true;
			
			// todo: ヒープ情報の出力
		}
		
		inline static Gate window_gate{ true };

		static BOOL __stdcall override_exception_catch(EXCEPTION_POINTERS* pExp, const char* plugin_name, const char* func_name);
		inline static const auto exception_catch_ptr = &override_exception_catch;

		inline static bool dialog_init = false;

	public:
		void operator()() {
			log_dir = WinWrap::Module{}.getFileNameW();
			log_dir.resize(log_dir.find_last_of(L"/\\") + 1);
			log_dir += L"log\\";

			OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x5b8b0, 13);
			h.store_i32(0, { 0xff, 0x74, 0x24, 0x04 }); // push [esp+4]
			h.store_i32(4, { 0x52, 0x51, 0xff, 0x15 }); // push edx ; push ecx ; call [(i32)]
			h.store_i32(8, &exception_catch_ptr);
			h.store_i8(12, '\xc3'); // ret

			dialog_init = exception_log_dialog_window_t::init();

			//if (!dialog_init)return;

		}
	} exception_log;
} // namespace patch

#endif // ifdef PATCH_SWITCH_EXCEPTION_LOG
