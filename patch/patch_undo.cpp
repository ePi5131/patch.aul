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

#include "patch_undo.hpp"

namespace patch {
#ifdef PATCH_SWITCH_UNDO

	void __cdecl undo_t::set_undo_wrap_3e037(unsigned int object_idx, unsigned int flag) {
		auto exists_movable_playback_pos = [](unsigned int object_idx) {
			auto& exdata_buffer = *exdata_buffer_ptr;
			auto eop = &(*ObjectArrayPointer_ptr)[object_idx];
			for (int i = 0; i < 12; i++) {
				auto fparam = &eop->filter_param[i];
				switch (fparam->id) {
				case FILTER_ID_MOVIE:
					if (eop->track_mode[fparam->track_begin].num == 0) {
						if (eop->check_value[fparam->check_begin] == 0) {
							auto exdata = reinterpret_cast<ExEdit::Exdata::efMovieFile*>(reinterpret_cast<uintptr_t>(exdata_buffer) + 4 + eop->exdata_offset + fparam->exdata_offset);
							if (exdata->frame_n > 0) {
								return true;
							}
						}
					}
					break;
				case FILTER_ID_MOVIE_MIX:
					if (eop->track_mode[fparam->track_begin].num == 0) {
						if (eop->check_value[fparam->check_begin] == 0) {
							auto exdata = reinterpret_cast<ExEdit::Exdata::efMovieSynthesis*>(reinterpret_cast<uintptr_t>(exdata_buffer) + 4 + eop->exdata_offset + fparam->exdata_offset);
							if (exdata->frame_n > 0) {
								return true;
							}
						}
					}
					break;
				case FILTER_ID_AUDIO:
					if (eop->track_mode[fparam->track_begin].num == 0) {
						if (eop->check_value[fparam->check_begin] == 0 && eop->check_value[fparam->check_begin + 1] == 0) {
							auto exdata = reinterpret_cast<ExEdit::Exdata::efAudioFile*>(reinterpret_cast<uintptr_t>(exdata_buffer) + 4 + eop->exdata_offset + fparam->exdata_offset);
							if (exdata->frame_n > 0) {
								return true;
							}
						}
					}
					break;
				case FILTER_ID_WAVEFORM:
					if (eop->track_mode[fparam->track_begin].num == 0) {
						if (eop->check_value[fparam->check_begin + 3] == 0) {
							auto exdata = reinterpret_cast<ExEdit::Exdata::efWaveForm*>(reinterpret_cast<uintptr_t>(exdata_buffer) + 4 + eop->exdata_offset + fparam->exdata_offset);
							if (exdata->frame_n > 0) {
								return true;
							}
						}
					}
					break;
				case FILTER_ID_SCENE:
					if (eop->track_mode[fparam->track_begin].num == 0) {
						if (eop->check_value[fparam->check_begin] == 0) {
							auto exdata = reinterpret_cast<ExEdit::Exdata::efScene*>(reinterpret_cast<uintptr_t>(exdata_buffer) + 4 + eop->exdata_offset + fparam->exdata_offset);
							if (scene_setting[exdata->scene].max_frame > 0) {
								return true;
							}
						}
					}
					break;
				case FILTER_ID_SCENE_AUDIO:
					if (eop->track_mode[fparam->track_begin].num == 0) {
						if (eop->check_value[fparam->check_begin] == 0 && eop->check_value[fparam->check_begin + 1] == 0) {
							auto exdata = reinterpret_cast<ExEdit::Exdata::efSceneAudio*>(reinterpret_cast<uintptr_t>(exdata_buffer) + 4 + eop->exdata_offset + fparam->exdata_offset);
							if (scene_setting[exdata->scene].max_frame > 0) {
								return true;
							}
						}
					}
					break;
				case -1:
					return false;
				}
			}
			return false;
		};

		if (*timeline_obj_click_mode_ptr == 2 || (*timeline_obj_click_mode_ptr == 3 && (*timeline_edit_both_adjacent_ptr & 1))) { // 左端 || (右端 && 環境設定の隣接するオブジェクトも選択がON )
			auto eop = &(*ObjectArrayPointer_ptr)[object_idx];
			int obj_idx = eop->index_midpt_leader;
			if (obj_idx == -1) {
				if (exists_movable_playback_pos(object_idx)) {
					set_undo(object_idx, 0);
					return;
				}
			} else if (obj_idx == object_idx) {
				if (exists_movable_playback_pos(obj_idx)) {
					while (0 <= obj_idx) {
						set_undo(obj_idx, 0);
						obj_idx = NextObjectIdxArray[obj_idx];
					}
					return;
				}
			}
		}
		set_undo(object_idx, flag);
	}

	int __cdecl undo_t::efDraw_func_WndProc_wrap_06e2b4(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp) {
		auto ret = efDraw_func_WndProc(hwnd, message, wparam, lparam, editp, efp);
		if (ret) return ret;
		if (LOWORD(wparam) == 7708) {
			AddUndoCount();
			set_undo(object(efp->processing) - 1, 1);
		}
		return ret;
	}

	int __stdcall undo_t::f8b97f(HWND hwnd, ExEdit::Filter* efp, WPARAM wparam, LPARAM lparam) {
		interval_set_undo(object(efp->processing) - 1, 1);
		return SendMessageA(hwnd, CB_GETLBTEXT, wparam, lparam);
	}

	int __stdcall undo_t::f8ba87_8bad5(ExEdit::Filter* efp, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
		int ret = SendMessageA(hwnd, message, wparam, lparam);
		if (ret != -1) {
			AddUndoCount();
			set_undo(object(efp->processing) - 1, 1);
		}
		return ret;
	}

	int __stdcall undo_t::f8bb4d_8bbcc(int value, int8_t* exdata, int offset, ExEdit::Filter* efp) {
		if (value < -100) value = -100;
		else if (100 < value) value = 100;
		if (exdata[offset] != value) {
			AddUndoCount();
			set_undo(object(efp->processing) - 1, 1);
		}
		return value;
	}

	int* __stdcall undo_t::f59e27(WPARAM wparam, LPARAM lparam, ExEdit::Filter* efp, UINT message) {
		if ((message != 0x702) || (wparam != 0x651e24)) return 0;

		int* exdata_layer_num = (int*)efp->exdata_ptr;
		int new_layer_num = *exdata_layer_num - *(int*)(lparam + 0x10);

		if (new_layer_num < 0) new_layer_num = 0;
		else if (99 < new_layer_num) new_layer_num = 99;

		if (new_layer_num != *exdata_layer_num) {
			AddUndoCount();
			set_undo(object(efp->processing) - 1, 1);
		}

		return exdata_layer_num;
	}

	int __stdcall undo_t::f8b9f0(ExEdit::Filter* efp, HWND hWnd, LPWSTR lpString, int nMaxCount) {
		interval_set_undo(object(efp->processing) - 1, 1);
		return GetWindowTextW(hWnd, lpString, nMaxCount);
	}

	int __stdcall undo_t::f875ef(ExEdit::Filter* efp, HWND hWnd, LPWSTR lpString) {
		return f8b9f0(efp, hWnd, lpString, 0x400);
	}

	int __cdecl undo_t::NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a(int timeline_y) {
		timeline_y = NormalizeExeditTimelineY(timeline_y);
		AddUndoCount();
		set_undo(timeline_y, 0x10);
		return timeline_y;
	}

	int __cdecl undo_t::NormalizeExeditTimelineY_wrap_4253e(int timeline_y) {
		timeline_y = NormalizeExeditTimelineY(timeline_y);

		AddUndoCount();
		BOOL other_flag = FALSE;
		for (int i = 0; i < 100; i++) {
			if (timeline_y != i) {
				if (has_flag((*layer_setting_ofsptr_ptr)[i].flag, ExEdit::LayerSetting::Flag::UnDisp)) {
					other_flag = TRUE;
					break;
				}
			}
		}

		if (has_flag((*layer_setting_ofsptr_ptr)[timeline_y].flag, ExEdit::LayerSetting::Flag::UnDisp)) {
			set_undo(timeline_y, 0x10);
		}

		if (other_flag) {
			for (int i = 0; i < 100; i++) {
				if (timeline_y != i) {
					if (has_flag((*layer_setting_ofsptr_ptr)[i].flag, ExEdit::LayerSetting::Flag::UnDisp)) {
						set_undo(i, 0x10);
					}
				}
			}
		}
		else {
			for (int i = 0; i < 100; i++) {
				if (timeline_y != i) {
					if (!has_flag((*layer_setting_ofsptr_ptr)[i].flag, ExEdit::LayerSetting::Flag::UnDisp)) {
						set_undo(i, 0x10);
					}
				}
			}
		}

		return timeline_y;
	}

	ExEdit::Object* __stdcall undo_t::f42617() {
		AddUndoCount();
		set_undo((*ObjectArrayPointer_ptr)[*ObjDlg_ObjectIndex_ptr].layer_disp, 0x10);
		return *ObjectArrayPointer_ptr;
	}

	void __stdcall undo_t::f4355c(ExEdit::Object* obj) {
		AddUndoCount();
		set_undo(obj - *ObjectArrayPointer_ptr, 0);
		*(int*)&obj->flag ^= 0x200;
	}

	void __stdcall undo_t::f435bd(ExEdit::Object* obj) {
		AddUndoCount();
		set_undo(obj - *ObjectArrayPointer_ptr, 0);
		*(int*)&obj->flag ^= 0x100;
	}

	void __cdecl undo_t::add_track_value_wrap(ExEdit::Filter* efp, int track_id, int add_value) {
		interval_set_undo(object(efp->processing) - 1, 1);
		add_track_value(efp, track_id, add_value);
	}

#endif
}
