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
#ifdef PATCH_SWITCH_UNDO

#include <exedit.hpp>
#include "util_magic.hpp"
#include "global.hpp"
#include "offset_address.hpp"
#include "config_rw.hpp"

// ty saunazo

namespace patch {

    // init at exedit load
    inline class undo_t {

        inline static ExEdit::Object** ObjectArrayPointer_ptr;
        inline static int* NextObjectIdxArray;
        inline static ExEdit::LayerSetting** layer_setting_ofsptr_ptr;
        inline static void** exdata_buffer_ptr;
        inline static int* timeline_obj_click_mode_ptr;
        inline static int* ObjDlg_ObjectIndex_ptr;
        inline static int* timeline_edit_both_adjacent_ptr;
        inline static int* UndoInfo_current_id_ptr;

        inline static ExEdit::SceneSetting* scene_setting;

        inline static void(__cdecl*set_undo)(unsigned int, unsigned int);
        inline static void(__cdecl*AddUndoCount)();
        inline static int(__cdecl*efDraw_func_WndProc)(HWND, UINT, WPARAM, LPARAM, AviUtl::EditHandle*, ExEdit::Filter*);
        inline static int(__cdecl*NormalizeExeditTimelineY)(int);
        inline static void(__cdecl *add_track_value)(ExEdit::Filter*, int, int);
        
        inline constexpr static int UNDO_INTERVAL = 1000;

		static void __cdecl set_undo_wrap_42878(unsigned int object_idx, int layer_id) {
			if (layer_id < (*ObjectArrayPointer_ptr)[object_idx].layer_disp) {
				set_undo(object_idx, 8);
			}
		}

		static void __cdecl set_undo_wrap_40e5c(unsigned int object_idx, unsigned int flag) {
			// select_idx_list
			set_undo(reinterpret_cast<int*>(GLOBAL::exedit_base + 0x179230)[object_idx], flag);
		}


        inline constexpr static int FILTER_ID_MOVIE = 0; // track 0 check 0 exdata 268 = maxframe
        inline constexpr static int FILTER_ID_AUDIO = 2; // track 0 check 0,1 exdata 268 = maxframe
        inline constexpr static int FILTER_ID_WAVEFORM = 6; // track 0 check 3 exdata 268 = maxframe
        inline constexpr static int FILTER_ID_SCENE = 7; // track 0 check 0 exdata 0 = sceneid
        inline constexpr static int FILTER_ID_SCENE_AUDIO = 8; // track 0 check 0,1 exdata 0 = sceneid
        inline constexpr static int FILTER_ID_MOVIE_MIX = 82; // track 0 check 0 exdata 268 = maxframe

        static void __cdecl set_undo_wrap_3e037(unsigned int object_idx, unsigned int flag);

        static int __cdecl efDraw_func_WndProc_wrap_06e2b4(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp);

        static int __stdcall f8b97f(HWND hwnd, ExEdit::Filter* efp, WPARAM wparam, LPARAM lparam);

        static int __stdcall f8ba87_8bad5(ExEdit::Filter* efp, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

        static int __stdcall f8bb4d_8bbcc(int value, int8_t* exdata, int offset, ExEdit::Filter* efp);

        static int* __stdcall f59e27(WPARAM wparam, LPARAM lparam, ExEdit::Filter* efp, UINT message);

        static int __stdcall f8b9f0(ExEdit::Filter* efp, HWND hWnd, LPWSTR lpString, int nMaxCount);
        
        static int __stdcall f875ef(ExEdit::Filter* efp, HWND hWnd, LPWSTR lpString);

        static int __cdecl NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a(int timeline_y);

        static int __cdecl NormalizeExeditTimelineY_wrap_4253e(int timeline_y);

        static ExEdit::Object* __stdcall f42617();

        static void __stdcall f4355c(ExEdit::Object* obj);

        static void __stdcall f435bd(ExEdit::Object* obj);

        static void __cdecl add_track_value_wrap(ExEdit::Filter* efp, int track_id, int add_value);

        static void interval_set_undo(int object_idx, int flag) {
            static ULONGLONG pretime = 0;
            static int pre_undo_id = 0;
            int& UndoInfo_current_id = *UndoInfo_current_id_ptr;
            ULONGLONG time = GetTickCount64();
            if (pretime < time - UNDO_INTERVAL || pre_undo_id != UndoInfo_current_id) {
                AddUndoCount();
                set_undo(object_idx, flag);
            }
            pretime = time;
            pre_undo_id = UndoInfo_current_id;
        }

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "undo";

    public:

		void init() {
            enabled_i = enabled;
            if (!enabled_i) return;

            ObjectArrayPointer_ptr = reinterpret_cast<decltype(ObjectArrayPointer_ptr)>(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
            NextObjectIdxArray = reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::NextObjectIdxArray);
            layer_setting_ofsptr_ptr = reinterpret_cast<decltype(layer_setting_ofsptr_ptr)>(GLOBAL::exedit_base + 0x0a4058);
            exdata_buffer_ptr = reinterpret_cast<void**>(GLOBAL::exedit_base + 0x1e0fa8);
            timeline_obj_click_mode_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0x177a24);
            ObjDlg_ObjectIndex_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0x177a10);
            timeline_edit_both_adjacent_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0x14ea00);
            scene_setting = reinterpret_cast<decltype(scene_setting)>(GLOBAL::exedit_base + 0x177a50);
            UndoInfo_current_id_ptr = reinterpret_cast<decltype(UndoInfo_current_id_ptr)>(GLOBAL::exedit_base + 0x244e14);
			
            set_undo = reinterpret_cast<decltype(set_undo)>(GLOBAL::exedit_base + 0x08d290);
            AddUndoCount = reinterpret_cast<decltype(AddUndoCount)>(GLOBAL::exedit_base + 0x08d150);
            efDraw_func_WndProc = reinterpret_cast<decltype(efDraw_func_WndProc)>(GLOBAL::exedit_base + 0x01b550);
            NormalizeExeditTimelineY = reinterpret_cast<decltype(NormalizeExeditTimelineY)>(GLOBAL::exedit_base + 0x032c10);
            add_track_value = reinterpret_cast<decltype(add_track_value)>(GLOBAL::exedit_base + 0x01c0f0);


			// レイヤー削除→元に戻すで他シーンのオブジェクトが消える
			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x042875, 2);
				h.store_i8(0, '\x56'); // push esi=layer_id
				h.store_i8(1, '\x90'); // nop
				ReplaceNearJmp(GLOBAL::exedit_base + 0x042879, &set_undo_wrap_42878);
			}

			// Ctrlで複数オブジェクトを選択しながら設定ダイアログのトラックバーを動かすと一部オブジェクトが正常に戻らない
			ReplaceNearJmp(GLOBAL::exedit_base + 0x040e5d, &set_undo_wrap_40e5c);

			// オブジェクトの左端をつまんで動かすと再生位置パラメータが変わるが、それが元に戻らない
			ReplaceNearJmp(GLOBAL::exedit_base + 0x03e038, &set_undo_wrap_3e037);
			
			// 一部フィルタのファイル参照を変更→元に戻すで設定ダイアログが更新されない(音声波形など)
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x08d50e, 4).store_i32(0, '\x0f\x1f\x40\x00'); // nop

            // 部分フィルタのマスクの種類を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x06e2b5, &efDraw_func_WndProc_wrap_06e2b4);

            // テキストオブジェクトのフォントを変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08b97c, 8);
                h.store_i32(0, '\x90\x57\x51\xe8'); // nop; push edi=efp; push ecx; call (rel32)
                h.replaceNearJmp(4, &f8b97f);
            }

            // テキストオブジェクトの影付き・縁付きを変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08ba86, 2);
                h.store_i16(0, '\x57\xe8'); // push edi=efp; call (rel32)
                ReplaceNearJmp(GLOBAL::exedit_base + 0x08ba88, &f8ba87_8bad5);
            }

            // テキストオブジェクトの文字配置(左寄せ[上]など)を変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08bad4, 6);
                h.store_i16(0, '\x57\xe8'); // push edi=efp; call (rel32)
                h.replaceNearJmp(2, &f8ba87_8bad5);
            }

            // テキストオブジェクトの字間を変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08bb48, 10);
                h.store_i32(0, '\x57\x6a\x05\x56'); // push edi=efp; push 5; push esi=exdata
                h.store_i16(4, '\x50\xe8'); //  push eax=value; call rel32
                h.replaceNearJmp(6, &f8bb4d_8bbcc);
            }

            // テキストオブジェクトの行間を変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08bbc7, 10);
                h.store_i32(0, '\x57\x6a\x06\x56'); // push edi=efp; push 6; push esi=exdata
                h.store_i16(4, '\x50\xe8'); //  push eax=value; call rel32
                h.replaceNearJmp(6, &f8bb4d_8bbcc);
            }

            // グループ制御とかの対象レイヤー数を変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x059e1b, 20);
                const char patch[] = {
                    "\x51" // push ecx=message
                    "\x50" // push eax=efp
                    "\x8b\x4c\x24\x28" // mov ecx, dword ptr[esp + 0x28]=lparam
                    "\x51" // push ecx
                    "\x8b\x4c\x24\x28" // mov ecx, dword ptr[esp + 0x28]=wparam
                    "\x51" // push ecx
                    "\xe8XXXX" // call rel32
                    "\x85\xc0" // test eax, eax
                    "\x74" /* 0x6e */ // JZ +0x6e
                };
                memcpy(reinterpret_cast<void*>(h.address()), patch, sizeof(patch) - 1);
                h.replaceNearJmp(13, &f59e27);
            }

            // テキストを変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08b9ef, 6);
                h.store_i16(0, '\x57\xe8'); // push edi; call (rel32)
                h.replaceNearJmp(2, &f8b9f0);
            }

            // スクリプト制御・カメラスクリプトを変更してもUndoデータが生成されない
            {
                // 100875e7 68 00 04 00 00    PUSH       0x400
                // 100875ec 56                PUSH       ESI
                // 100875ed 51                PUSH       ECX
                // 100875ee ff 15 54 a2 09 10 CALL       dword ptr [->USER32.DLL::GetWindowTextW] = 0009bc30
                // ↓
                // 100875e7 8b 54 24 38       MOV EDX, DWORD PTR [ESP+38H]
                // 100875eb 90                NOP
                // 100875ec 56                PUSH ESI
                // 100875ed 51                PUSH ECX
                // 100875ee 52                PUSH EDX
                // 100875ef e8 XXXX           CALL rel32
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0875e7, 13);
                h.store_i32(0, '\x8b\x54\x24\x38');
                h.store_i32(4, '\x90\x56\x51\x52');
                h.store_i8(8, '\xe8');
                h.replaceNearJmp(9, &f875ef);
            }


            // 左クリックよりレイヤーの表示状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x03c8fb, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューよりレイヤーの表示状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x04262a, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューよりレイヤーのロック状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x042663, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューよりレイヤーの座標のリンク状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x042925, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューより上クリッピング状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x042a0b, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューより他のレイヤーを全表示/非表示を押してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x04253f, &NormalizeExeditTimelineY_wrap_4253e);

            // ショートカットよりレイヤーの表示状態を変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x042617, 5);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, &f42617);
            }

            // カメラ制御の対象 を切り替えてもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x04355b, 6);
                h.store_i16(0, '\x51\xe8'); // push ecx, call (rel32)
                h.replaceNearJmp(2, &f4355c);
            }
            // 上のオブジェクトでクリッピング を切り替えてもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0435ba, 8);
                h.store_i32(0, '\x90\x90\x50\xe8'); // nop, push eax, call (rel32)
                h.replaceNearJmp(4, &f435bd);
            }

            // テンキー2468+-*/ Ctrl+テンキー2468 で(座標XY 回転 拡大率 中心XY)トラックバーを変えてもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b611, &add_track_value_wrap); // テンキー4座標X-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b646, &add_track_value_wrap); // Ctrl+テンキー6中心X+
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b674, &add_track_value_wrap); // テンキー6座標X+
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b6ab, &add_track_value_wrap); // Ctrl+テンキー8中心Y-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b6db, &add_track_value_wrap); // テンキー8座標Y-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b710, &add_track_value_wrap); // Ctrl+テンキー2中心Y+, Ctrl+テンキー4中心X-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b73e, &add_track_value_wrap); // テンキー2座標Y+
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b765, &add_track_value_wrap); // テンキー-拡大率-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b78c, &add_track_value_wrap); // テンキー+拡大率+
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b7b0, &add_track_value_wrap); // テンキー/回転-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b7d4, &add_track_value_wrap); // テンキー*回転+


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
	} undo;
}


#endif
