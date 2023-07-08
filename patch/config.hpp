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

#include <Windows.h>

#include "scope_exit.hpp"

#include "macro.h"
#include "util_resource.hpp"
#include "config_rw.hpp"

#include "patch.hpp"


class Config2 {
	bool invalid_json;

public:
	void load(const std::wstring& path) {
		auto hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) return;
		DWORD ignore;
		auto size_low = GetFileSize(hFile, &ignore);
		auto file = std::make_unique<byte[]>(size_low);
		if (ReadFile(hFile, file.get(), size_low, &ignore, NULL) == FALSE) {
			patch_resource_message_stack.emplace_back(new patch_resource_message_class_w(PATCH_RS_PATCH_FAILED_TO_LOAD_SETTING, MB_TASKMODAL | MB_ICONEXCLAMATION));
			return;
		}
		CloseHandle(hFile);

		json_value_s* root = nullptr;
		SCOPE_EXIT_AUTO{[root]{ free(root); }};

		root = json_parse(file.get(), size_low);
		if (root == nullptr) {
			patch_resource_message_stack.emplace_back(new patch_resource_message_class_w(PATCH_RS_PATCH_INVALID_SETTING_JSON, MB_TASKMODAL | MB_ICONEXCLAMATION));
			invalid_json = true;
			return;
		}

		ConfigReader cr(root);

		cr.regist("switch", [](json_value_s* value) {
			ConfigReader cr(value);
			
			#ifdef PATCH_SWITCH_KEYCONFIG
				patch::KeyConfig.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_ACCESS_KEY
				patch::access_key.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_COLORPALETTE_CACHE
				patch::colorpalette_cache.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_FILEINFO
				patch::fileinfo.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SUSIE_LOAD
				patch::susie_load.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SPLASH
				patch::splash.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_AUP_SCENE_SETTING
				patch::aup_scene_setting.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_AUP_LAYER_SETTING
				patch::aup_layer_setting.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
				patch::exo_aviutlfilter.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_EXO_SCENEIDX
				patch::exo_sceneidx.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_EXO_TRACKPARAM
				patch::exo_trackparam.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
				patch::exo_trackminusval.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_EXO_MIDPT_AND_TRA
				patch::exo_midpt_and_tra.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
				patch::exo_specialcolorconv.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_EXO_FOLD_GUI
				patch::exo_fold_gui.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
				patch::tra_aviutlfilter.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_TRA_CHANGE_DRAWFILTER
				patch::tra_change_drawfilter.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_TRA_SPECIFIED_SPEED
				patch::tra_specified_speed.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SETTING_NEW_PROJECT
				patch::setting_new_project.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_TEXT_OP_SIZE
				patch::text_op_size.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
				patch::ignore_media_param_reset.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_FONT_DIALOG
				patch::font_dialog.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SCROLL_OBJDLG
				patch::scroll_objdlg.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_ALPHA_BG
				patch::alpha_bg.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_HELPFUL_MSGBOX
				patch::helpful_msgbox.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX
				patch::failed_sjis_msgbox.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_FAILED_LONGER_PATH
				patch::failed_longer_path.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_FAILED_FILE_DROP
				patch::failed_file_drop.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_THEME_CC
				patch::theme_cc.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
				patch::exeditwindow_sizing.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
				patch::setting_dialog_move.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_OBJ_COLORCORRECTION
				patch::ColorCorrection.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_OBJ_GLOW
				patch::Glow.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_OBJ_LENSBLUR
				patch::LensBlur.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_OBJ_IMAGELOOP
				patch::ImageLoop.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_OBJ_NOISE
				patch::Noise.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_OBJ_SPECIALCOLORCONV
				patch::obj_specialcolorconv.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SETTINGDIALOG_EXCOLORCONFIG
				patch::excolorconfig.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_RCLICKMENU_SPLIT
				patch::rclickmenu_split.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_RCLICKMENU_DELETE
				patch::rclickmenu_delete.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_BLEND
				patch::blend.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_ADD_EXTENSION
				patch::add_extension.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_DIALOG_NEW_FILE
				patch::dialog_new_file.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_PLAYBACK_SPEED
				patch::playback_speed.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SCENE_CACHE
				patch::scene_cache.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SCRIPT_SORT_PATCH
				patch::patch_script_sort.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_WARNING_DUPLICATE_PLUGINS
				patch::WarningDuplicate.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_SHARED_CACHE
				patch::SharedCache.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_YC_RGB_CVT
				patch::yc_rgb_cvt.switch_load(cr);
			#endif
			#ifdef PATCH_SWITCH_GROUP_CAMERA_SCENE
				patch::group_camera_scene.switch_load(cr);
			#endif
		
			#ifdef PATCH_SWITCH_UNDO
				patch::undo.switch_load(cr);
				#ifdef PATCH_SWITCH_UNDO_REDO
					patch::redo.switch_load(cr);
				#endif
			#endif

			#ifdef PATCH_SWITCH_CONSOLE
				patch::console.switch_load(cr);
			#endif
				
			#ifdef PATCH_SWITCH_LUA
				patch::lua.switch_load(cr);
				#ifdef PATCH_SWITCH_LUA_GETVALUE
					patch::lua_getvalueex.switch_load(cr);
				#endif

				#ifdef PATCH_SWITCH_LUA_RAND
					patch::lua_rand.switch_load(cr);
				#endif

				#ifdef PATCH_SWITCH_LUA_RANDEX
					patch::lua_randex.switch_load(cr);
				#endif
			#endif

			#ifdef PATCH_SWITCH_FAST
				patch::fast::fast.switch_load(cr);
				
				#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
					patch::fast_exeditwindow.switch_load(cr);
				#endif
				#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
					patch::fast_setting_dialog.switch_load(cr);
				#endif
				#ifdef PATCH_SWITCH_FAST_TEXT
					patch::fast::text.switch_load(cr);
				#endif
				#ifdef PATCH_SWITCH_FAST_CREATE_FIGURE
					patch::fast::create_figure.switch_load(cr);
				#endif
				#ifdef PATCH_SWITCH_FAST_BORDER
					patch::fast::Border.switch_load(cr);
				#endif
				#ifdef PATCH_SWITCH_FAST_GLOW
					patch::fast::Glow.switch_load(cr);
				#endif
		
				#ifdef PATCH_SWITCH_CL
					patch::fast::cl.switch_load(cr);
					
					#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
						patch::fast::RadiationalBlur.switch_load(cr);
					#endif
					#ifdef PATCH_SWITCH_FAST_POLARTRANSFORM
						patch::fast::PolarTransform.switch_load(cr);
					#endif
					#ifdef PATCH_SWITCH_FAST_DISPLACEMENTMAP
						patch::fast::DisplacementMap.switch_load(cr);
					#endif
					#ifdef PATCH_SWITCH_FAST_FLASH
						patch::fast::Flash.switch_load(cr);
					#endif
					#ifdef PATCH_SWITCH_FAST_DIRECTIONALBLUR
						patch::fast::DirectionalBlur.switch_load(cr);
					#endif
					#ifdef PATCH_SWITCH_FAST_LENSBLUR
						patch::fast::LensBlur.switch_load(cr);
					#endif
				#endif
			#endif

			cr.load();
		});

#ifdef PATCH_SWITCH_CONSOLE
		cr.regist("console", [](json_value_s* value) {
			ConfigReader cr(value);
			patch::console.config_load(cr);
			cr.load();
		});
#endif
#ifdef PATCH_SWITCH_THEME_CC
		cr.regist("theme_cc", [](json_value_s* value) {
			ConfigReader cr(value);
			patch::theme_cc.config_load(cr);
			cr.load();
		});
#endif
#ifdef PATCH_SWITCH_UNDO_REDO
		cr.regist("redo", [](json_value_s* value) {
			ConfigReader cr(value);
			patch::redo.config_load(cr);
			cr.load();
		});
#endif
#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
		cr.regist("fast_exeditwindow", [](json_value_s* value) {
			ConfigReader cr(value);
			patch::fast_exeditwindow.config_load(cr);
			cr.load();
		});
#endif
#ifdef PATCH_SWITCH_FAST_TEXT
		cr.regist("fast_text", [](json_value_s* value) {
			ConfigReader cr(value);
			patch::fast::text.config_load(cr);
			cr.load();
		});
#endif
#ifdef PATCH_SWITCH_SCENE_CACHE
		cr.regist("scene_cache", [](json_value_s* value) {
			ConfigReader cr(value);
			patch::scene_cache.config_load(cr);
			cr.load();
		});
#endif

		cr.load();
	}

	void store(const std::wstring& path) {
		if (invalid_json)return;

		auto hFile = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			patch_resource_message_w(PATCH_RS_PATCH_FAILED_TO_SAVE_SETTING, MB_TASKMODAL | MB_ICONEXCLAMATION);
			return;
		}
		SCOPE_EXIT_AUTO{[hFile]{
			CloseHandle(hFile);
		}};

		int level = 0;
		ConfigWriter cw(level);

#ifdef PATCH_SWITCH_CONSOLE
		{
			ConfigWriter console(++level);

			patch::console.config_store(console);

			std::stringstream ss;
			console.write(ss);

			cw.append("console", ss.str());
			--level;
		}
#endif

#ifdef PATCH_SWITCH_THEME_CC
		{
			ConfigWriter theme_cc(++level);

			patch::theme_cc.config_store(theme_cc);

			std::stringstream ss;
			theme_cc.write(ss);

			cw.append("theme_cc", ss.str());
			--level;
		}
#endif

#ifdef PATCH_SWITCH_UNDO_REDO
		{
			ConfigWriter redo(++level);

			patch::redo.config_store(redo);

			std::stringstream ss;
			redo.write(ss);

			cw.append("redo", ss.str());
			--level;
		}
#endif

#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
		{
			ConfigWriter fast_exeditwindow(++level);
			
			patch::fast_exeditwindow.config_store(fast_exeditwindow);

			std::stringstream ss;
			fast_exeditwindow.write(ss);

			cw.append("fast_exeditwindow", ss.str());
			--level;
		}
#endif

#ifdef PATCH_SWITCH_FAST_TEXT
		{
			ConfigWriter fast_text(++level);

			patch::fast::text.config_store(fast_text);

			std::stringstream ss;
			fast_text.write(ss);

			cw.append("fast_text", ss.str());
			--level;
		}
#endif

#ifdef PATCH_SWITCH_SCENE_CACHE
		{
			ConfigWriter scene_cache(++level);

			patch::scene_cache.config_store(scene_cache);

			std::stringstream ss;
			scene_cache.write(ss);

			cw.append("scene_cache", ss.str());
			--level;
		}
#endif

		{
			ConfigWriter switch_(++level);
			
			#ifdef PATCH_SWITCH_KEYCONFIG
				patch::KeyConfig.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_ACCESS_KEY
				patch::access_key.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_COLORPALETTE_CACHE
				patch::colorpalette_cache.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_FILEINFO
				patch::fileinfo.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SUSIE_LOAD
				patch::susie_load.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SPLASH
				patch::splash.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_AUP_SCENE_SETTING
				patch::aup_scene_setting.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_AUP_LAYER_SETTING
				patch::aup_layer_setting.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_EXO_AVIUTL_FILTER
				patch::exo_aviutlfilter.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_EXO_SCENEIDX
				patch::exo_sceneidx.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_EXO_TRACKPARAM
				patch::exo_trackparam.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_EXO_TRACK_MINUSVAL
				patch::exo_trackminusval.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_EXO_MIDPT_AND_TRA
				patch::exo_midpt_and_tra.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_EXO_SPECIALCOLORCONV
				patch::exo_specialcolorconv.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_EXO_FOLD_GUI
				patch::exo_fold_gui.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_TRA_AVIUTL_FILTER
				patch::tra_aviutlfilter.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_TRA_CHANGE_DRAWFILTER
				patch::tra_change_drawfilter.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_TRA_SPECIFIED_SPEED
				patch::tra_specified_speed.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SETTING_NEW_PROJECT
				patch::setting_new_project.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_TEXT_OP_SIZE
				patch::text_op_size.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET
				patch::ignore_media_param_reset.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_FONT_DIALOG
				patch::font_dialog.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SCROLL_OBJDLG
				patch::scroll_objdlg.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_ALPHA_BG
				patch::alpha_bg.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_HELPFUL_MSGBOX
				patch::helpful_msgbox.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX
				patch::failed_sjis_msgbox.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_FAILED_LONGER_PATH
		patch::failed_longer_path.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_FAILED_FILE_DROP
		patch::failed_file_drop.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_THEME_CC
				patch::theme_cc.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_EXEDITWINDOW_SIZING
				patch::exeditwindow_sizing.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SETTINGDIALOG_MOVE
				patch::setting_dialog_move.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_OBJ_COLORCORRECTION
				patch::ColorCorrection.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_OBJ_GLOW
				patch::Glow.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_OBJ_LENSBLUR
				patch::LensBlur.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_OBJ_IMAGELOOP
				patch::ImageLoop.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_OBJ_NOISE
				patch::Noise.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_OBJ_SPECIALCOLORCONV
				patch::obj_specialcolorconv.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SETTINGDIALOG_EXCOLORCONFIG
				patch::excolorconfig.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_RCLICKMENU_SPLIT
				patch::rclickmenu_split.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_RCLICKMENU_DELETE
				patch::rclickmenu_delete.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_BLEND
				patch::blend.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_RENDERING
				patch::Rendering.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_ADD_EXTENSION
				patch::add_extension.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_DIALOG_NEW_FILE
				patch::dialog_new_file .switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_PLAYBACK_SPEED
				patch::playback_speed.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SCENE_CACHE
				patch::scene_cache.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SCRIPT_SORT_PATCH
				patch::patch_script_sort.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_WARNING_DUPLICATE_PLUGINS
				patch::WarningDuplicate.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_SHARED_CACHE
				patch::SharedCache.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_YC_RGB_CVT
				patch::yc_rgb_cvt.switch_store(switch_);
			#endif
			#ifdef PATCH_SWITCH_GROUP_CAMERA_SCENE
				patch::group_camera_scene.switch_store(switch_);
			#endif

			#ifdef PATCH_SWITCH_UNDO
				patch::undo.switch_store(switch_);
				#ifdef PATCH_SWITCH_UNDO_REDO
					patch::redo.switch_store(switch_);
				#endif
			#endif

			#ifdef PATCH_SWITCH_CONSOLE
				patch::console.switch_store(switch_);
			#endif
				
			#ifdef PATCH_SWITCH_LUA
				patch::lua.switch_store(switch_);
				#ifdef PATCH_SWITCH_LUA_GETVALUE
					patch::lua_getvalueex.switch_store(switch_);
				#endif

				#ifdef PATCH_SWITCH_LUA_RAND
					patch::lua_rand.switch_store(switch_);
				#endif

				#ifdef PATCH_SWITCH_LUA_RANDEX
					patch::lua_randex.switch_store(switch_);
				#endif
			#endif

			#ifdef PATCH_SWITCH_FAST
				patch::fast::fast.switch_store(switch_);
				
				#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
					patch::fast_exeditwindow.switch_store(switch_);
				#endif
				#ifdef PATCH_SWITCH_FAST_SETTINGDIALOG
					patch::fast_setting_dialog.switch_store(switch_);
				#endif
				#ifdef PATCH_SWITCH_FAST_TEXT
					patch::fast::text.switch_store(switch_);
				#endif
				#ifdef PATCH_SWITCH_FAST_CREATE_FIGURE
					patch::fast::create_figure.switch_store(switch_);
				#endif
				#ifdef PATCH_SWITCH_FAST_BORDER
					patch::fast::Border.switch_store(switch_);
				#endif
				#ifdef PATCH_SWITCH_FAST_GLOW
					patch::fast::Glow.switch_store(switch_);
				#endif
		
				#ifdef PATCH_SWITCH_CL
					patch::fast::cl.switch_store(switch_);
					
					#ifdef PATCH_SWITCH_FAST_RADIATIONALBLUR
						patch::fast::RadiationalBlur.switch_store(switch_);
					#endif
					#ifdef PATCH_SWITCH_FAST_POLARTRANSFORM
						patch::fast::PolarTransform.switch_store(switch_);
					#endif
					#ifdef PATCH_SWITCH_FAST_DISPLACEMENTMAP
						patch::fast::DisplacementMap.switch_store(switch_);
					#endif
					#ifdef PATCH_SWITCH_FAST_FLASH
						patch::fast::Flash.switch_store(switch_);
					#endif
					#ifdef PATCH_SWITCH_FAST_DIRECTIONALBLUR
						patch::fast::DirectionalBlur.switch_store(switch_);
					#endif
					#ifdef PATCH_SWITCH_FAST_LENSBLUR
						patch::fast::LensBlur.switch_store(switch_);
					#endif
				#endif
			#endif

			std::stringstream ss;
			switch_.write(ss);

			cw.append("switch", ss.str());
			--level;
		}
		std::stringstream ss;
		cw.write(ss);
		auto s = ss.str();

		DWORD ignore;
		WriteFile(hFile, s.c_str(), s.size(), &ignore, nullptr);
	}
};
inline Config2 config2;
