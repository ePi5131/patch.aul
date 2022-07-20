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

//#define PATCH_INTERNAL

#define PATCH_VERSION_NAME "patch.aul " PATCH_VERSION_STR

#ifdef PATCH_INTERNAL
#define PATCH_VERSION_STR "internal build:" __DATE__ " " __TIME__ ""

#define PATCH_SWITCH_EXCEPTION_LOG
#define PATCH_SWITCH_SYSINFO_MODIFY
#define PATCH_SWITCH_TRANSRATE
#define PATCH_SWITCH_SETTING_GUI


#define PATCH_SWITCH_ACCESS_KEY access_key
#define PATCH_SWITCH_SUSIE_LOAD susie_load
#define PATCH_SWITCH_SPLASH splash
#define PATCH_SWITCH_TRA_AVIUTL_FILTER tra_aviutl_filter
#define PATCH_SWITCH_EXO_AVIUTL_FILTER exo_aviutl_filter
#define PATCH_SWITCH_EXO_TRACK_MINUSVAL exo_track_minusval
#define PATCH_SWITCH_EXO_SCENEIDX exo_sceneidx
#define PATCH_SWITCH_EXO_TRACKPARAM exo_trackparam
#define PATCH_SWITCH_EXO_SPECIALCOLORCONV exo_specialcolorconv
#define PATCH_SWITCH_TEXT_OP_SIZE text_op_size
#define PATCH_SWITCH_GET_FONTNAME get_fontname
#define PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET ignore_media_param_reset
#define PATCH_SWITCH_FONT_DIALOG font_dialog
#define PATCH_SWITCH_SCROLL_OBJDLG scroll_objdlg
#define PATCH_SWITCH_ALPHA_BG alpha_bg
#define PATCH_SWITCH_HELPFUL_MSGBOX helpful_msgbox
#define PATCH_SWITCH_THEME_CC theme_cc
#define PATCH_SWITCH_EXEDITWINDOW_SIZING exeditwindow_sizing
#define PATCH_SWITCH_SETTINGDIALOG_MOVE settingdialog_move
#define PATCH_SWITCH_SETTINGDIALOG_EXCOLORCONFIG settingdialog_excolorconfig

#define PATCH_SWITCH_UNDO undo
#ifdef PATCH_SWITCH_UNDO
	#define PATCH_SWITCH_UNDO_REDO undo_redo
#endif

#define PATCH_SWITCH_CONSOLE console
#ifdef PATCH_SWITCH_CONSOLE

#define PATCH_SWITCH_DEBUGSTRING console_debug_string

#endif // ifdef PATCH_SWITCH_CONSOLE


#define PATCH_SWITCH_LUA lua
#ifdef PATCH_SWITCH_LUA

	#define PATCH_SWITCH_LUA_ENV lua_env
	#define PATCH_SWITCH_LUA_GETVALUE lua_getvalueex
	#define PATCH_SWITCH_LUA_RAND lua_rand
	#define PATCH_SWITCH_LUA_RANDEX lua_randex
	#define PATCH_SWITCH_LUA_PATH lua_path

	//#define PATCH_SWITCH_LUA_COPYBUFFER_SMEM lua_copybuffer_smem


#endif // ifdef PATCH_SWITCH_LUA


#define PATCH_SWITCH_FAST fast
#ifdef PATCH_SWITCH_FAST
	
	#define PATCH_SWITCH_FAST_GETPUTPIXELDATA fast_getputpixeldata
	#define PATCH_SWITCH_FAST_SETTINGDIALOG fast_settingdialog
	#define PATCH_SWITCH_FAST_EXEDITWINDOW fast_exeditwindow

	#define PATCH_SWITCH_CL cl
	#ifdef PATCH_SWITCH_CL
		#define PATCH_SWITCH_FAST_POLORTRANSFORM fast_polortransform
		#define PATCH_SWITCH_FAST_RADIATIONALBLUR fast_radiationalblur
		#define PATCH_SWITCH_FAST_FLASH fast_flash

	#endif // define PATCH_SWITCH_CL

#endif //define PATCH_SWITCH_FAST

#else // ifdef PATCH_INTERNAL
#define PATCH_VERSION_STR "r41"

#define PATCH_SWITCH_EXCEPTION_LOG
#define PATCH_SWITCH_SYSINFO_MODIFY

#define PATCH_SWITCH_ACCESS_KEY access_key
#define PATCH_SWITCH_TRA_AVIUTL_FILTER tra_aviutl_filter
#define PATCH_SWITCH_EXO_AVIUTL_FILTER exo_aviutl_filter
#define PATCH_SWITCH_EXO_TRACK_MINUSVAL exo_track_minusval
#define PATCH_SWITCH_EXO_SCENEIDX exo_sceneidx
#define PATCH_SWITCH_EXO_TRACKPARAM exo_trackparam
#define PATCH_SWITCH_EXO_SPECIALCOLORCONV exo_specialcolorconv
#define PATCH_SWITCH_TEXT_OP_SIZE text_op_size
#define PATCH_SWITCH_GET_FONTNAME get_fontname
#define PATCH_SWITCH_IGNORE_MEDIA_PARAM_RESET ignore_media_param_reset
#define PATCH_SWITCH_THEME_CC theme_cc
#define PATCH_SWITCH_EXEDITWINDOW_SIZING exeditwindow_sizing
#define PATCH_SWITCH_SETTINGDIALOG_MOVE settingdialog_move
#define PATCH_SWITCH_SETTINGDIALOG_EXCOLORCONFIG settingdialog_excolorconfig
#define PATCH_SWITCH_CANCEL_BOOST_CONFLICT
#define PATCH_SWITCH_WARNING_OLD_LSW
#define PATCH_SWITCH_OBJ_LENSBLUR obj_lensblur

#define PATCH_SWITCH_UNDO undo
#ifdef PATCH_SWITCH_UNDO
	#define PATCH_SWITCH_UNDO_REDO undo_redo
	#ifdef PATCH_SWITCH_UNDO_REDO
		#define PATCH_SWITCH_UNDO_REDO_SHIFT undo_redo_shift
	#endif
#endif

#define PATCH_SWITCH_CONSOLE console
#ifdef PATCH_SWITCH_CONSOLE
	#define PATCH_SWITCH_DEBUGSTRING console_debug_string
#endif // ifdef PATCH_SWITCH_CONSOLE

#define PATCH_SWITCH_LUA lua
#ifdef PATCH_SWITCH_LUA
	#define PATCH_SWITCH_LUA_ENV lua_env
	#define PATCH_SWITCH_LUA_GETVALUE lua_getvalue
	#define PATCH_SWITCH_LUA_RAND lua_rand
	#define PATCH_SWITCH_LUA_RANDEX lua_randex
	#define PATCH_SWITCH_LUA_PATH lua_path

	//#define PATCH_SWITCH_LUA_COPYBUFFER_SMEM lua_copybuffer_smem

#endif // ifdef PATCH_SWITCH_LUA

#define PATCH_SWITCH_FAST fast
#ifdef PATCH_SWITCH_FAST
	
	#define PATCH_SWITCH_FAST_SETTINGDIALOG fast_settingdialog
	#define PATCH_SWITCH_FAST_EXEDITWINDOW fast_exeditwindow
	#define PATCH_SWITCH_FAST_TEXT fast_text
	#define PATCH_SWITCH_FAST_BORDER fast_border

	#define PATCH_SWITCH_CL cl
	#ifdef PATCH_SWITCH_CL
		#define PATCH_SWITCH_FAST_POLORTRANSFORM fast_polortransform
		#define PATCH_SWITCH_FAST_RADIATIONALBLUR fast_radiationalblur
		#define PATCH_SWITCH_FAST_FLASH fast_flash
		#define PATCH_SWITCH_FAST_DIRECTIONALBLUR fast_directionalblur
		#define PATCH_SWITCH_FAST_LENSBLUR fast_lensblur

	#endif // define PATCH_SWITCH_CL

#endif //define PATCH_SWITCH_FAST

#endif // ifdef PATCH_INTERNAL

#define PATCH_SWITCHER_DEFINE_STRING(s) #s
#define PATCH_SWITCHER_DEFINE_STRINGX(s) PATCH_SWITCHER_DEFINE_STRING(s)
#define PATCH_SWITCHER_DEFINE_VALUE(s) s
#define PATCH_SWITCHER_KEY_JOINNER(a, b) a ## b
#define PATCH_SWITCHER_KEY_JOINNERX(a, b) PATCH_SWITCHER_KEY_JOINNER(a, b)
#define PATCH_SWITCHER_DEFINE(name, def) bool name = def; inline static constexpr const char PATCH_SWITCHER_KEY_JOINNER(key_, name)[] = PATCH_SWITCHER_DEFINE_STRING(name);
#define PATCH_SWITCHER_DEFINE_EX(name, def, key) bool name = def; inline static constexpr const char PATCH_SWITCHER_KEY_JOINNER(key_, name)[] = PATCH_SWITCHER_DEFINE_STRING(key);
#define PATCH_SWITCHER_IF(name) if (load_variable(elm, PATCH_SWITCHER_KEY_JOINNER(key_, name), name)) continue;
#define PATCH_SWITCHER_STORE(name) tj(PATCH_SWITCHER_KEY_JOINNER(key_, name), name);
#define PATCH_SWITCHER_MEMBER(name) GLOBAL::config.switcher.name
