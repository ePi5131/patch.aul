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

#define ISOLATION_AWARE_ENABLED 1
#include <Windows.h>
#include "patch_exception_log_dialog.hpp"

#ifdef PATCH_SWITCH_EXCEPTION_LOG

#include <optional>
#include <format>

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "offset_address.hpp"
#include "patch_exception_history.hpp"
#include "util_others.hpp"

namespace patch {

    exception_log_dialog_window_t::exception_log_dialog_window_t(const Param* param) {
        this->param = param;
        INT_PTR ret;
        std::thread th([this, &ret] {
            MessageBeep(MB_ICONEXCLAMATION);
            auto current_hmod = load_i32<HMODULE>(GLOBAL::aviutl_base + OFS::AviUtl::current_resource_hmod);
            const auto RT_DIALOG_W = (LPWSTR)(RT_DIALOG);
            if (FindResourceW(current_hmod, L"PATCH_DIALOG_EXCEPTION", RT_DIALOG_W)) 
                ret = IsolationAwareDialogBoxParamW(current_hmod, L"PATCH_DIALOG_EXCEPTION", NULL, dialog_proc, (LPARAM)this);
            else
                ret = IsolationAwareDialogBoxParamW(GLOBAL::patchaul_hinst, L"PATCH_DIALOG_EXCEPTION", NULL, dialog_proc, (LPARAM)this);
        });
        if (th.joinable()) {
            th.join();
        }
        else {
            throw std::runtime_error("Failed to start a thread.");
        }
        if (ret == 0 || ret == -1) throw std::runtime_error("Failed to create a dialog.");
    }

    // 48 144 2 130
    INT_PTR CALLBACK exception_log_dialog_window_t::dialog_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        auto this_ = reinterpret_cast<exception_log_dialog_window_t*>(GetWindowLong(hwnd, GWL_USERDATA));

        switch (message) {
            case WM_INITDIALOG: {
                SetWindowLong(hwnd, GWL_USERDATA, lparam);
                this_ = reinterpret_cast<exception_log_dialog_window_t*>(lparam);

                this_->hwnd_detail_text = GetDlgItem(hwnd, PATCH_ID_EXCEPTION_DETAIL_TEXT);
                this_->hwnd_link = GetDlgItem(hwnd, PATCH_ID_EXCEPTION_LINK);
                this_->hwnd_button_ok = GetDlgItem(hwnd, PATCH_ID_EXCEPTION_OK);
                this_->hwnd_button_detail = GetDlgItem(hwnd, PATCH_ID_EXCEPTION_DETAIL_BUTTON);
                this_->hwnd_check_stopmes = GetDlgItem(hwnd, PATCH_ID_EXCEPTION_STOPMES);
                this_->hwnd_button_save_project = GetDlgItem(hwnd, PATCH_ID_EXCEPTION_SAVE_PROJECT);
                this_->hwnd_save_text = GetDlgItem(hwnd, PATCH_ID_EXCEPTION_LABEL2);

                RECT rect_detail_text; GetWindowRect(this_->hwnd_detail_text, &rect_detail_text);

                RECT rect_check_stopmes; GetWindowRect(this_->hwnd_check_stopmes, &rect_check_stopmes);

                this_->resize_ofs = rect_check_stopmes.top - rect_detail_text.top;

                SetWindowTextW(this_->hwnd_link, this_->param->link.c_str());
                if (!this_->detail_showed) {
                    ShowWindow(this_->hwnd_detail_text, SW_HIDE);
                    this_->update_window_detail(hwnd, false);
                }
                SetWindowTextA(this_->hwnd_detail_text, this_->param->detail.c_str());

                ShowWindow(hwnd, SW_SHOW);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

                return TRUE;
            }
            case WM_CLOSE:
                EndDialog(hwnd, 1);
                return TRUE;

            case WM_COMMAND: switch(LOWORD(wparam)){
                case PATCH_ID_EXCEPTION_OK:
                    if(SendMessageA(this_->hwnd_check_stopmes, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                        auto record = this_->param->pExp->ExceptionRecord;
                        exception_history.map.insert_or_assign(ExceptionHistoryEntry{
                            .address = reinterpret_cast<uint32_t>(record->ExceptionAddress),
                            .code = static_cast<uint32_t>(record->ExceptionCode)
                        }, std::chrono::system_clock::now());
                    }
                
                    EndDialog(hwnd, 1);
                    return TRUE;

                case PATCH_ID_EXCEPTION_DETAIL_BUTTON:
                    if(this_->detail_showed){
                        this_->update_window_detail(hwnd, false);
                        this_->detail_showed = false;
                    }else{
                        this_->update_window_detail(hwnd, true);
                        this_->detail_showed = true;
                    }
                    return TRUE;
                case PATCH_ID_EXCEPTION_SAVE_PROJECT: {
                    save_project(hwnd);
                }
                return TRUE;
            }break;

            case WM_NOTIFY: switch(const auto& nmhdr = *reinterpret_cast<LPNMHDR>(lparam); nmhdr.code) {
                case NM_CLICK:
                case NM_RETURN:
                switch(nmhdr.idFrom){
                    case PATCH_ID_EXCEPTION_LINK:{
                        STARTUPINFOW si={.cb=sizeof(STARTUPINFOW)};
                        PROCESS_INFORMATION pi;
                        auto commandline = std::format(L"explorer.exe /select,{}{}", this_->param->info_dir, this_->param->info_path);
                        auto ret = CreateProcessW(NULL, commandline.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
                        if(ret){
                            CloseHandle(pi.hProcess);
                            CloseHandle(pi.hThread);
                        }
                        return TRUE;
                    }
                }
                break;
            }break;
        }
        return FALSE;
    }
} // namespace patch

#endif // ifdef PATCH_SWITCH_EXCEPTION_LOG
