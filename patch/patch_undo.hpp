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

// ty saunazo

namespace patch {

    // init at exedit load
    inline class undo_t {

        inline static ExEdit::Object** ObjectArrayPointer_ptr;
        inline static ExEdit::LayerSetting** layer_setting_ofsptr_ptr;
        inline static void** exdata_buffer_ptr;
        inline static int* timeline_obj_click_mode_ptr;
        inline static int* ObjDlg_ObjectIndex_ptr;
        inline static int* timeline_edit_both_adjacent_ptr;

        inline static ExEdit::SceneSetting* scene_setting;

		inline static void(__cdecl*set_undo)(unsigned int, unsigned int);
        inline static void(__cdecl*AddUndoCount)();
        inline static int(__cdecl*efRadiationalBlur_func_WndProc)(HWND, UINT, WPARAM, LPARAM, AviUtl::EditHandle*, ExEdit::Filter*);
        inline static int(__cdecl*NormalizeExeditTimelineY)(int);
        
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

        static int __cdecl efRadiationalBlur_func_WndProc_wrap_06e2b4(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp);

        static int __stdcall f8b97f(HWND hwnd, ExEdit::Filter* efp, WPARAM wparam, LPARAM lparam);

        static int __stdcall f8ba87_8bad5(ExEdit::Filter* efp, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

        static int __stdcall f8bb4d_8bbcc(int value, int8_t* exdata, int offset, ExEdit::Filter* efp);

        static int* __stdcall f59e27(WPARAM wparam, LPARAM lparam, ExEdit::Filter* efp, UINT message);

        static int __stdcall f8b9f0(ExEdit::Filter* efp, HWND hWnd, LPWSTR lpString, int nMaxCount);
        
        static int __stdcall f875ef(ExEdit::Filter* efp, HWND hWnd, LPWSTR lpString);

        static int __cdecl NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a(int timeline_y);

        static int __cdecl NormalizeExeditTimelineY_wrap_4253e(int timeline_y);

        static ExEdit::Object* __stdcall f42617();

        static bool enable() {
            return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_UNDO);
        }
#ifdef PATCH_SWITCH_UNDO_REDO
        struct UndoData {
            int data_id;
            int object_id; // layer | 0x1000000;
            int data_size;
            DWORD object_flag;
            int object_layer_disp;
            int object_frame_begin;
            int object_frame_end;
            DWORD* data;
        };

        inline static int* UndoInfo_object_num_ptr;
        inline static int* UndoInfo_write_offset_ptr;
        inline static int* UndoInfo_current_id_ptr;
        inline static int* SceneDisplaying_ptr;
        inline static DWORD* UndoInfo_buffer_ptr_ptr;
        inline static int* UndoInfo_buffer_size_ptr;
        inline static int* UndoInfo_limit_mode_ptr;
        inline static int* selecting_obj_num_ptr;
        inline static AviUtl::EditHandle** editp_ptr;
        inline static AviUtl::FilterPlugin** fp_g_ptr;

        inline static UndoData** UndoDataPtrArray;

        inline static void*(__cdecl*exedit_memmove)(void*, void*, size_t);
        inline static void(__cdecl*run_undo)();
        inline static void(__cdecl*change_disp_scene)(int, AviUtl::FilterPlugin*, AviUtl::EditHandle*);

        inline static int pre_scene_idx = 0;
        inline static int UndoInfo_max_id = 0;
        inline static int UndoInfo_object_new = 0;
        inline static int UndoInfo_pre_id = 0;
        inline static bool new_object = true;
        inline static bool running_undo = false;


        static void __cdecl init_undo_patch();


        // レイヤーUndo情報そのままではシーンが分からないため、UndoData.object_layer_dispにscene_idxを入れておくようにする
        static void add_scene_idx() {
            for (int i = UndoInfo_object_new; i < *UndoInfo_object_num_ptr; i++) {
                UndoData* undodata = UndoDataPtrArray[i];
                if (undodata->object_id & 0x1000000) { // レイヤー
                    undodata->object_layer_disp = pre_scene_idx;
                }
            }
        }

        static int get_scene_idx_UndoData(int undo_id) {
            UndoData* undodata = UndoDataPtrArray[undo_id];
            if (undodata->object_id & 0x1000000) {
                return undodata->object_layer_disp;
            }
            else {
                return (*ObjectArrayPointer_ptr)[undodata->object_id].scene_set;
            }
        }

        // undo_bufferから指定idのデータを削除（空白が出来ないようにずらす）
        static void shift_undo_buffer(int id) {
            DWORD UndoInfo_buffer_ptr = *UndoInfo_buffer_ptr_ptr;
            int& UndoInfo_object_num = *UndoInfo_object_num_ptr;
            int& UndoInfo_write_offset = *UndoInfo_write_offset_ptr;

            int size = UndoDataPtrArray[id]->data_size;
            DWORD delptr = (DWORD)UndoDataPtrArray[id];

            DWORD nextptr = delptr + size;
            if (UndoInfo_buffer_ptr + (DWORD)UndoInfo_write_offset > nextptr) {
                exedit_memmove((void*)delptr, (void*)nextptr, UndoInfo_buffer_ptr + (DWORD)UndoInfo_write_offset - nextptr);
            }
            UndoInfo_write_offset -= size;

            for (int i = 0; i < UndoInfo_object_num; i++) {
                if (delptr < (DWORD)UndoDataPtrArray[i]) {
                    UndoDataPtrArray[i] = (UndoData*)((DWORD)UndoDataPtrArray[i] - size);
                }
            }
            UndoInfo_object_num--;
        }

        // 指定idのデータを削除・整理
        static void remove_UndoData(int id) {
            shift_undo_buffer(id);
            for (int i = id; i < *UndoInfo_object_num_ptr; i++) {
                UndoDataPtrArray[i] = UndoDataPtrArray[i + 1];
            }
        }

        // 指定id以上のデータを削除(新しく追加されるデータを除く) 
        static void remove_old_UndoData(int id) {
            for (int i = UndoInfo_object_new - 1; i >= 0; i--) {
                if (id <= UndoDataPtrArray[i]->data_id) {
                    remove_UndoData(i);
                }
            }
        }

        // id1のデータを削除し、id2のデータを置き換える
        static void move_UndoData(int id1, int id2) {
            shift_undo_buffer(id1);
            UndoDataPtrArray[id1] = UndoDataPtrArray[id2];
        }

        // id1とid2の統合(重複部分はid1が優先)
        static void integrate_undodata(int id1, int id2) {
            int& UndoInfo_write_offset = *UndoInfo_write_offset_ptr;

            UndoData* undodata1 = UndoDataPtrArray[id1];
            UndoData* undodata2 = UndoDataPtrArray[id2];

            if (undodata1->object_flag == 0) {
                remove_UndoData(id2);
                return;
            }
            if (undodata2->object_flag == 0) {
                remove_UndoData(id1);
                return;
            }

            if (undodata1->data_size < undodata2->data_size) {

                memcpy((void*)((DWORD)undodata2 + 0xc), (void*)((DWORD)undodata1 + 0xc), undodata1->data_size - 0xc);

                if (undodata1->data_size == 0x1c) {
                    auto undoobj = reinterpret_cast<ExEdit::Object*>(&undodata2->data);
                    undoobj->flag = static_cast<ExEdit::Object::Flag>(undodata1->object_flag);
                    if (undodata1->object_flag) {
                        undoobj->layer_set = undoobj->layer_disp = undodata1->object_layer_disp;
                        undoobj->frame_begin = undodata1->object_frame_begin;
                        undoobj->frame_end = undodata1->object_frame_end;
                    }
                }
                remove_UndoData(id1);
            }
            else {
                remove_UndoData(id2);
            }
        }

        // 新しく追加されたデータに対して、重複があれば統合する
        static void optimize_new_undo_buffer() {
            int& UndoInfo_object_num = *UndoInfo_object_num_ptr;
            UndoData* newdata = UndoDataPtrArray[UndoInfo_object_num - 1];
            UndoData* undodata;
            for (int i = UndoInfo_object_new; i < UndoInfo_object_num - 1; i++) {
                undodata = UndoDataPtrArray[i];
                if (undodata->object_id == newdata->object_id && undodata->data_id == newdata->data_id) {
                    integrate_undodata(i, UndoInfo_object_num - 1);
                    break;
                }
            }
        }

        // 現在のオブジェクトデータと比べて変更が無ければ削除する
        static void remove_emptiness_UndoData() {
            // 指定idのデータと現在のオブジェクト状態を比較 (TRUE:相違点あり FALSE:同じ)
            auto cmp_obj_undo = [](int id) {
                auto& ObjectArrayPointer = *ObjectArrayPointer_ptr;
                auto& layer_setting_ofsptr = *layer_setting_ofsptr_ptr;
                auto& exdata_buffer = *exdata_buffer_ptr;
                auto& UndoInfo_object_num = *UndoInfo_object_num_ptr;
                auto& UndoInfo_write_offset = *UndoInfo_write_offset_ptr;

                UndoData* undodata = UndoDataPtrArray[id];

                void* ptr1;
                void* ptr2;
                int cmpsize;

                if (undodata->object_id & 0x1000000) { // レイヤー
                    ptr1 = (void*)&layer_setting_ofsptr[undodata->object_id & 0xffffff];
                    ptr2 = &undodata->data;
                    cmpsize = 8;
                }
                else {
                    ptr1 = (void*)&ObjectArrayPointer[undodata->object_id];
                    if (undodata->data_size > 0x1c) {
                        ptr2 = &undodata->data;
                        cmpsize = sizeof(ExEdit::Object);

                        auto fix_track_right = [](ExEdit::Object* obj) {
                            int track_num = obj->track_n;
                            for (int i = 0; i < track_num; i++) {
                                if (obj->track_mode[i].num == 0) {
                                    obj->track_value_right[i] = obj->track_value_left[i];
                                }
                            }
                        };

                        fix_track_right(static_cast<ExEdit::Object*>(ptr1)); // 処理順の関係かトラック右の値が変わってしまっていることがあるので直す
                        fix_track_right(static_cast<ExEdit::Object*>(ptr2));

                        void* ptr3 = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(exdata_buffer) + ObjectArrayPointer[undodata->object_id].exdata_offset + 4);
                        void* ptr4 = reinterpret_cast<void*>(reinterpret_cast<DWORD>(&undodata->data) + cmpsize);
                        int excmpsize = undodata->data_size - 0x1c - cmpsize;
                        if (memcmp(ptr3, ptr4, excmpsize))return true;
                    }
                    else {
                        ptr2 = &undodata->object_flag;
                        if (undodata->object_flag == 0) {
                            cmpsize = 4;
                        }
                        else {
                            cmpsize = 0x10;
                        }
                    }
                }

                if (memcmp(ptr1, ptr2, cmpsize))return true;

                return false;
            };

            for (int i = UndoInfo_object_new; i < *UndoInfo_object_num_ptr; i++) {
                if (!cmp_obj_undo(i)) { // 変更なし
                    remove_UndoData(i);
                    i--;
                }
            }
        }

        // run_undo内で生成されたredo用データを置き換える
        static void move_redo_data() {
            int& UndoInfo_object_num = *UndoInfo_object_num_ptr;

            if (UndoInfo_object_new < UndoInfo_object_num) {
                int dataid = UndoDataPtrArray[UndoInfo_object_new]->data_id;
                int objid = UndoDataPtrArray[UndoInfo_object_new]->object_id;

                // 元のUndoデータを削除、UndoDataPtrArrayの置換
                BOOL movef = FALSE;
                for (int i = 0; i < UndoInfo_object_new; i++) {
                    if (UndoDataPtrArray[i]->object_id == objid && UndoDataPtrArray[i]->data_id == dataid) {
                        add_scene_idx();
                        move_UndoData(i, UndoInfo_object_new);
                        movef = TRUE;
                        break;
                    }
                }
                if (!movef) {
                    remove_UndoData(UndoInfo_object_new);
                }
            }
            UndoInfo_object_new = UndoInfo_object_num;
        }

        static void __stdcall AllocUndoBuffer_patch(int num) {
            for (int i = num - 1; i >= 0; i--) {
                UndoInfo_object_new--;
                remove_UndoData(i);
            }
        }

        // 一つ前の操作で作られたundoデータの最適化(状態が確定してから最適化を行う必要があるため一つ前のデータに対して行う)
        static void optimize_undo_data() {
            int& UndoInfo_current_id = *UndoInfo_current_id_ptr;
            int& UndoInfo_object_num = *UndoInfo_object_num_ptr;
            int& UndoInfo_write_offset = *UndoInfo_write_offset_ptr;

            if (new_object) {
                new_object = FALSE;
                UndoInfo_pre_id = UndoInfo_current_id;
                UndoInfo_object_new = UndoInfo_object_num;
            }

            if (UndoInfo_object_new < UndoInfo_object_num) {
                optimize_new_undo_buffer();

                if (UndoInfo_pre_id < UndoInfo_current_id) {
                    remove_emptiness_UndoData();

                    if (UndoInfo_object_new < UndoInfo_object_num) { // 変更があった
                        add_scene_idx();
                        remove_old_UndoData(UndoInfo_pre_id);
                        UndoInfo_pre_id = UndoInfo_current_id;
                        UndoInfo_max_id = UndoInfo_current_id - 1;
                        UndoInfo_object_new = UndoInfo_object_num;
                    }
                    else {
                        UndoInfo_current_id--;
                    }
                }
            }

        }

        // 最新のundoデータの最適化(undo処理を行う直前に、まだの部分の最適化を行う)
        static void optimize_newer_undo_data() {
            int& UndoInfo_current_id = *UndoInfo_current_id_ptr;
            int& UndoInfo_object_num = *UndoInfo_object_num_ptr;

            if (!new_object) {
                if (UndoInfo_object_new < UndoInfo_object_num) {
                    optimize_new_undo_buffer();

                    remove_emptiness_UndoData();
                    if (UndoInfo_object_new < UndoInfo_object_num) { // 変更があった
                        add_scene_idx();
                        remove_old_UndoData(UndoInfo_current_id);
                        UndoInfo_max_id = UndoInfo_current_id;
                    }
                    else {
                        UndoInfo_current_id--;
                    }
                }
            }
            UndoInfo_pre_id = UndoInfo_current_id;
            UndoInfo_object_new = UndoInfo_object_num;
            new_object = TRUE;
        }

        // set_undoの先頭に追加する処理
        static int __cdecl f8d290(void* ret, int object_idx, int flag);

        // run_undoの始めの方に追加する処理
        static int __cdecl pre_run_undo();

        // run_undoの後処理
        static void __cdecl end_run_undo();

        static void __cdecl undo_change_disp_scene();

        // undoと同じ処理でredoをするためには並び順を逆にする必要がある
        static void reverse_UndoDataPtrArray() {
            int& UndoInfo_current_id = *UndoInfo_current_id_ptr;
            int& UndoInfo_object_num = *UndoInfo_object_num_ptr;

            int begin = 0;
            int end = 0;
            for (int i = 0; i < UndoInfo_object_num; i++) {
                if (UndoDataPtrArray[i]->data_id == UndoInfo_current_id) {
                    begin = i;
                    break;
                }
            }
            for (int i = begin; i < UndoInfo_object_num; i++) {
                if (UndoDataPtrArray[i]->data_id == UndoInfo_current_id) {
                    end = i;
                }
            }

            UndoData* tmp;
            while (begin < end) {
                tmp = UndoDataPtrArray[begin];
                UndoDataPtrArray[begin] = UndoDataPtrArray[end];
                UndoDataPtrArray[end] = tmp;
                begin++; end--;
            }
        }



        static bool enable_redo() {
            return PATCH_SWITCHER_MEMBER(PATCH_SWITCH_UNDO_REDO);
        }
#endif

    public:
        static void run_redo() {
            int& UndoInfo_current_id = *UndoInfo_current_id_ptr;

            // この後呼ばれるrun_undo()でも同じ処理が行われることになるが、ここで先に処理してredoが可能かどうかを判定する必要がある
            optimize_newer_undo_data();

            // 上記処理にて新しい操作が行われていたとしたら、現在が最新状態という事になるのでredoはできない
            if (UndoInfo_current_id >= UndoInfo_max_id)return;


            // idを増やしてUndoDataPtrArrayの並びを逆にしてrun_undoを動かせばredoになる
            UndoInfo_current_id++;
            reverse_UndoDataPtrArray();

            running_undo = true;
            run_undo();

            UndoInfo_current_id++;
            UndoInfo_pre_id = UndoInfo_current_id;
            reverse_UndoDataPtrArray();
        }

		void operator()() {
            if (!enable()) return;

            ObjectArrayPointer_ptr = reinterpret_cast<decltype(ObjectArrayPointer_ptr)>(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
            layer_setting_ofsptr_ptr = reinterpret_cast<decltype(layer_setting_ofsptr_ptr)>(GLOBAL::exedit_base + 0x0a4058);
            exdata_buffer_ptr = reinterpret_cast<void**>(GLOBAL::exedit_base + 0x1e0fa8);
            timeline_obj_click_mode_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0x177a24);
            ObjDlg_ObjectIndex_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0x177a10);
            timeline_edit_both_adjacent_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0x14ea00);
            scene_setting = reinterpret_cast<decltype(scene_setting)>(GLOBAL::exedit_base + 0x177a50);
			
            set_undo = reinterpret_cast<decltype(set_undo)>(GLOBAL::exedit_base + 0x08d290);
            AddUndoCount = reinterpret_cast<decltype(AddUndoCount)>(GLOBAL::exedit_base + 0x08d150);
            efRadiationalBlur_func_WndProc = reinterpret_cast<decltype(efRadiationalBlur_func_WndProc)>(GLOBAL::exedit_base + 0x01b550);
            NormalizeExeditTimelineY = reinterpret_cast<decltype(NormalizeExeditTimelineY)>(GLOBAL::exedit_base + 0x032c10);

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
            ReplaceNearJmp(GLOBAL::exedit_base + 0x06e2b5, &efRadiationalBlur_func_WndProc_wrap_06e2b4);

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
                char patch[] = {
                    0x51, // push ecx=message
                    0x50, // push eax=efp
                    0x8b, 0x4c, 0x24, 0x28, // mov ecx, dword ptr[esp + 0x28]=lparam
                    0x51, // push ecx
                    0x8b, 0x4c, 0x24, 0x28, // mov ecx, dword ptr[esp + 0x28]=wparam
                    0x51, // push ecx
                    0xe8, 0, 0, 0, 0, // call rel32
                    0x85, 0xc0, // test eax, eax
                    0x74, /* 0x6e */ // JZ +0x6e
                };
                memcpy(reinterpret_cast<void*>(h.address()), patch, sizeof(patch));
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


            #ifdef PATCH_SWITCH_UNDO_REDO
                if (!enable_redo())return;

                UndoInfo_object_num_ptr = reinterpret_cast<decltype(UndoInfo_object_num_ptr)>(GLOBAL::exedit_base + 0x244e08);
                UndoInfo_write_offset_ptr = reinterpret_cast<decltype(UndoInfo_write_offset_ptr)>(GLOBAL::exedit_base + 0x244e10);
                UndoInfo_current_id_ptr = reinterpret_cast<decltype(UndoInfo_current_id_ptr)>(GLOBAL::exedit_base + 0x244e14);
                SceneDisplaying_ptr = reinterpret_cast<decltype(SceneDisplaying_ptr)>(GLOBAL::exedit_base + 0x1a5310);
                UndoInfo_buffer_ptr_ptr = reinterpret_cast<decltype(UndoInfo_buffer_ptr_ptr)>(GLOBAL::exedit_base + 0x244e0c);
                UndoInfo_buffer_size_ptr = reinterpret_cast<decltype(UndoInfo_buffer_size_ptr)>(GLOBAL::exedit_base + 0x244e18);
                UndoInfo_limit_mode_ptr = reinterpret_cast<decltype(UndoInfo_limit_mode_ptr)>(GLOBAL::exedit_base + 0x244e1c);
                selecting_obj_num_ptr = reinterpret_cast<decltype(selecting_obj_num_ptr)>(GLOBAL::exedit_base + 0x167d88);
                editp_ptr = reinterpret_cast<decltype(editp_ptr)>(GLOBAL::exedit_base + 0x1a532c);
                fp_g_ptr = reinterpret_cast<decltype(fp_g_ptr)>(GLOBAL::exedit_base + 0x14d4b4);

                UndoDataPtrArray = reinterpret_cast<decltype(UndoDataPtrArray)>(GLOBAL::exedit_base + 0x2363a8);

                exedit_memmove = reinterpret_cast<decltype(exedit_memmove)>(GLOBAL::exedit_base + 0x091f60);
                run_undo = reinterpret_cast<decltype(run_undo)>(GLOBAL::exedit_base + 0x08d490);
                change_disp_scene = reinterpret_cast<decltype(change_disp_scene)>(GLOBAL::exedit_base + 0x02ba60);


                /*  initに内容を追加する
                    void __cdecl InitUndo(void) { // 8d140
                        UndoInfo_object_num = -1;
                        UndoInfo_write_offset = -1;
                    }
                */
                ReplaceFunction(GLOBAL::exedit_base + 0x08d140, &init_undo_patch);

                /*  AddUndoCount 8d150 の処理を少し変更した方が良いように見えるけど、今回の実装では変えなくても影響なさそう
                    if ((UndoInfo_write_offset <= 0) || (UndoInfo_object_num <= 0)) {
                        UndoInfo_write_offset = 0;
                        UndoInfo_object_num = 0;
                    }
                    ↓
                    if ((UndoInfo_write_offset <= 0) || (UndoInfo_object_num < 0)) {
                        UndoInfo_write_offset = 0;
                        UndoInfo_object_num = 0;
                    }

                    1008d15a 7e 08           JLE
                    1008d162 7f 0a           JG

                byte jle2jl[1] = { 0x7c };
                exedit_ReplaceData(0x8d15a, jle2jl, 1);

                byte jg2jge[1] = { 0x7d };
                exedit_ReplaceData(0x8d162, jg2jge, 1);
                */


                /*  bufferの最適化処理を行う
                    1008d290 a11c4e2410   MOV        EAX, [UndoInfo_limit_mode]
                    →       e8--------   CALL       f8d290
                */
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08d290, 5);
                    h.store_i8(0, '\xe8'); // call (rel32)
                    h.replaceNearJmp(1, &f8d290);
                }

                /*  undoの前処理
                    1008d4a9 a1084e2410     EAX = UndoInfo_object_num
                    →       e8--------     pre_run_undo();

                */
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08d4a9, 5);
                    h.store_i8(0, '\xe8'); // call (rel32)
                    h.replaceNearJmp(1, &pre_run_undo);
                }


                /*  redoデータ生成を無効にする処理を消す
                    1008d4c5 89151c4e2410   Undoinfo_limit_mode = 1
                    1008d4cb
                    ↓
                    1008d4c5 660f1f440000   nop word ptr [eax+eax+00H]
                    1008d4cb
                */
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08d4c5, 6);
                    char patch[] = {
                        0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00
                    };
                    memcpy(reinterpret_cast<void*>(h.address()), patch, 6);
                }


                /*  後処理
                1008d723 8b0d144e2410       mov     ecx,UndoInfo_current_id
                1008d729 40                 inc     eax++
                1008d72a a3084e2410         mov     UndoInfo_object_num = eax
                1008d72f 8b442428           mov     eax = pre_limit_mode
                1008d733 49                 dec     ecx--
                1008d734 a31c4e2410         mov     UndoInfo_limit_mode = eax
                1008d739 890d144e2410       mov     UndoInfo_current_id = ecx
                ↓
                1008d723 eb15               jmp     1008d73a
                1008d725 144e2410           err
                1008d729 40                 inc     eax++
                1008d72a a3084e2410         mov     UndoInfo_object_num = eax
                1008d72f 8b442428           mov     eax = pre_limit_mode
                1008d733 49                 dec     ecx--
                1008d734 a31c4e2410         mov     UndoInfo_limit_mode = eax
                1008d739 89                 err
                1008d73a e8--------         call    end_run_undo()

                */
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08d723, 2);
                    h.store_i16(0, '\xeb\x15');
                }
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08d73a, 5);
                    h.store_i8(0, '\xe8'); // call (rel32)
                    h.replaceNearJmp(1, &end_run_undo);
                }

                /*  AllocUndoBufferにて削除が行われる場合にUndoInfo_object_newを再計算する必要あり
                1008d22a e8 31 4d 00 00     CALL    exedit_memmove()

                exedit_ReplaceCall(0x8d22b, &redo_8d22b_patch);
                */

                /*  AllocUndoBufferで行われる削除処理がそのままではまずそうなので書きかえ
                    while (true) {
                        if (object_num < 15000 && write_offset + data_size < UndoInfo_buffer_size) {
                            return TRUE;
                        }

                        if (object_num < 1) break;

                        for (i = 1; i < object_num; i++) {
                            if (UndoDataPtrArray[i]->data_id != UndoDataPtrArray[0]->data_id) break;
                        }
                        if (i == object_num) break;

                        // -------------- ここから下を関数に置き換える ---------------------

                        buffer_size = (DWORD)UndoDataPtrArray[i] - (DWORD)buffer_ptr;
                        FUN_10091f60(buffer_ptr, (void)UndoDataPtrArray[i], write_offset - buffer_size); // memmove相当の関数

                        buffer_ptr = UndoInfo_buffer_ptr;
                        object_num = UndoInfo_object_num - i;
                        write_offset = UndoInfo_write_offset - buffer_size;
                        UndoInfo_object_num = object_num;
                        UndoInfo_write_offset = write_offset;

                        offset = 0;
                        for (i = 0; i < object_num; i++) { //
                            UndoDataPtrArray[i] = (UndoData*)((DWORD)buffer_ptr + offset);
                            offset += ((UndoData*)((DWORD)buffer_ptr + offset))->data_size;
                        }
                    }


                    1008d21a 8b04bda8632310  MOV   EAX,dword ptr [EDI*0x4 + 0x102363a8]
                    1008d221
                    ..
                    1008d22f 8b15084e2410    MOV   EDX,dword ptr [UndoInfo_object_num]
                    1008d235 8b2d104e2410    MOV   EBP,dword ptr [UndoInfo_write_offset]
                    ..
                    1008d274 e95dffffff      JMP   1008d1d6


                    ↓
                    1008d21a 57              PUSH  EDI ; == i
                    1008d21b e8--------      CALL  AllocUndoBuffer_patch(i)
                    1008d220 eb0d            JMP   1008d22f
                    1008d222
                    ..
                    1008d22f 8b15084e2410    MOV   EDX,dword ptr [UndoInfo_object_num]
                    1008d235 8b2d104e2410    MOV   EBP,dword ptr [UndoInfo_write_offset]
                    1008d23b eb99            JMP   1008d1d6
                    1008d23d

                */
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08d21a, 6);
                    h.store_i16(0, '\x57\xe8');
                    h.replaceNearJmp(2, &AllocUndoBuffer_patch);
                }
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08d220, 2);
                    h.store_i16(0, '\xeb\x0d'); // jmp +0dh
                }
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08d23b, 2);
                    h.store_i16(0, '\xeb\x99'); // jmp +99h
                }

                // シーン切り替えてもUndoが継続できるようにする
                /*  change_disp_scene内のundo初期化を削除し、別の処理を入れる
                    1002ba84 e8b7160600       CALL   InitUndo()
                    ↓
                    1002ba84 e8--------       CALL   new_function()
                */
                ReplaceNearJmp(GLOBAL::exedit_base + 0x02ba85, &undo_change_disp_scene);

            #endif

		}


	} undo;
}


#endif
