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

#include "patch_exception_log.hpp"

#include "patch_exception_history.hpp"
#include "moduledata.hpp"
#include "util_resource.hpp"

#ifdef PATCH_SWITCH_EXCEPTION_LOG
namespace patch {

	BOOL __stdcall exception_log_t::override_exception_catch(EXCEPTION_POINTERS* pExp, const char* plugin_name, const char* func_name) {
		// [esp - 128, esp + 256)
		uint32_t stack[96];
		{
			OverWriteOnProtectHelper h(pExp->ContextRecord->Esp - 128, 384);
			for (int i = 0; i < 96; i++) {
				stack[95 - i] = load_i32<uint32_t>(pExp->ContextRecord->Esp - i * 4 + 252);
			}
		}

		auto [log_name_a, log_name_w] = get_log_name();
		
		std::unique_lock lock(ModulesData::mtx);
		ModulesData::update();
		auto& modulesdata = ModulesData::get();
		lock.unlock();
		

		auto detail = format_detail(pExp, plugin_name, func_name, modulesdata);
		write_logfile(log_name_w, detail, pExp, modulesdata, stack);

		try {
			window_gate.wait();

			if (exception_history.check({
				.address = reinterpret_cast<uint32_t>(pExp->ExceptionRecord->ExceptionAddress),
				.code = static_cast<uint32_t>(pExp->ExceptionRecord->ExceptionCode)
			})) return TRUE;

 			exception_log_dialog_window_t::Param param = {
				.link = L"<a>log/{}</a>"_fmt(log_name_w),
				.info_dir = log_dir,
				.info_path = log_name_w,
				.detail = detail,
				.pExp = pExp
			};
			exception_log_dialog_window_t window(&param);
			
			window_gate.open();
		}
		catch (const std::runtime_error& e) {
			patch_resource_message_a(
				PATCH_RS_PATCH_FAILED_TO_CREATE_EXCEPTION_DIALOG, MB_ICONERROR | MB_TASKMODAL,
				e.what(),
				log_name_a,
				detail
			);
		}

		return TRUE;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_EXCEPTION_LOG
