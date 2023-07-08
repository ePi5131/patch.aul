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

#include "patch_redo.hpp"

#ifdef PATCH_SWITCH_UNDO_REDO

namespace patch {

void __cdecl redo_t::init_undo_patch() {
	*UndoInfo_object_num_ptr = -1;
	*UndoInfo_write_offset_ptr = -1;
	*UndoInfo_current_id_ptr = 0;

	UndoInfo_max_id = 0;
	UndoInfo_object_new = 0;
	UndoInfo_pre_id = 0;
	new_object = true;
	running_undo = false;

	pre_scene_idx = *SceneDisplaying_ptr;
}

int __cdecl redo_t::f8d290(void* ret, int object_idx, int flag) {
	int& UndoInfo_object_num = *UndoInfo_object_num_ptr;
	int& UndoInfo_write_offset = *UndoInfo_write_offset_ptr;
	int& UndoInfo_limit_mode = *UndoInfo_limit_mode_ptr;

	// データが生成されない条件をそのまま追加
	if ((UndoInfo_limit_mode && (flag & 2) == 0) || UndoInfo_write_offset < 0 || UndoInfo_object_num < 0) {
		return UndoInfo_limit_mode;
	}
	if ((flag & 0x10) == 0) {
		auto ObjectArrayPointer = *ObjectArrayPointer_ptr;
		if (!has_flag(ObjectArrayPointer[object_idx].flag, ExEdit::Object::Flag::Exist) && ((flag & 2) == 0)) {
			return UndoInfo_limit_mode;
		}
		if ((flag & 1) && 0 <= ObjectArrayPointer[object_idx].index_midpt_leader) {
			return UndoInfo_limit_mode;
		}
	}
	// ----------------------------------

	if (running_undo) {
		move_redo_data();
	}
	else {
		optimize_undo_data();
	}

	return UndoInfo_limit_mode;
}

int __cdecl redo_t::pre_run_undo() {

	// shift + ctrl + zでredoを実行させるためのコード(元に戻せない状態からでも使えます)


#ifdef PATCH_SWITCH_UNDO_REDO_SHIFT
	if (redo.enabled_i && redo.c_shift) {
		if (!running_undo) {
			if (GetKeyState(VK_SHIFT) < 0) {
				run_redo();
				return 0;
			}
		}
	}
#endif


	int& UndoInfo_write_offset = *UndoInfo_write_offset_ptr;
	int& UndoInfo_current_id = *UndoInfo_current_id_ptr;
	int& UndoInfo_object_num = *UndoInfo_object_num_ptr;
	int& selecting_obj_num = *selecting_obj_num_ptr;

	// optimize_undo_data()で処理していない最新のデータを処理する(元に戻す操作をしているため状態は確定しているはず)
	optimize_newer_undo_data();


	if (UndoInfo_current_id <= 0 || UndoInfo_write_offset <= 0 || UndoInfo_object_num <= 0)return 0;

	// Ctrlを押しながら複数選択した状態で元に戻すをしてオブジェクトが消えると幽霊オブジェクトが出現するのを修正 (選択状態を強制解除)
	selecting_obj_num = 0;

	running_undo = true;

	int i;
	// 元々はobject_numを返す。redoデータが残ることでobject_numではなくdata_id > current_idとなるタイミングのnumを返すことでrun_undo内の処理との辻褄が合う
	for (i = 1; i < UndoInfo_object_num; i++) {
		if (UndoDataPtrArray[i]->data_id > UndoInfo_current_id) {
			break;
		}
	}

	// 必要であればシーン切り替えを行う
	if (UndoDataPtrArray[i - 1]->data_id == UndoInfo_current_id) {
		int scene_idx = get_scene_idx_UndoData(i - 1);
		if (scene_idx != *SceneDisplaying_ptr) {
			//void*& editpm = *(void**)uintptr_t(gp_editp);
			change_disp_scene(scene_idx, *fp_g_ptr, *editp_ptr);
		}
	}

	return i;
}

void __cdecl redo_t::end_run_undo() {
	int& UndoInfo_current_id = *UndoInfo_current_id_ptr;
	move_redo_data();
	running_undo = false;
	UndoInfo_current_id--;
	UndoInfo_pre_id = UndoInfo_current_id;
}

void __cdecl redo_t::undo_change_disp_scene() {
	optimize_newer_undo_data();
	pre_scene_idx = *SceneDisplaying_ptr;
}

} // namespace patch

#endif
