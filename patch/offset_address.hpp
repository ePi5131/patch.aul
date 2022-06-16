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
#include "util_int.hpp"

namespace OFS {
	namespace AviUtl {
		constexpr i32 InitAuf = 0x02c930;
		constexpr i32 VersionString = 0x07425c;
		constexpr i32 default_resource_hmod = 0x2c525c;
		constexpr i32 getsys_versionstr_arg = 0x022187;
		constexpr i32 current_resource_hmod = 0x2d910c;
		constexpr i32 edit_handle_ptr = 0x08717c;
		constexpr i32 saveProjectFile = 0x024160;
	}

	namespace ExEdit {
		constexpr i32 ConvertFilter2Exo_TrackScaleJudge_RangeBegin = 0x028a8d;
		constexpr i32 ConvertFilter2Exo_TrackScaleJudge_Overwrite1 = 0x028a84;
		constexpr i32 ConvertFilter2Exo_TrackScaleJudge_Overwrite2 = 0x0289cb;
		constexpr i32 ConvertFilter2Exo_TrackScaleJudge_Overwrite3 = 0x028a00;

		constexpr i32 efSceneAudio_exdatause_idx_type = 0x0aeb5c;
		constexpr i32 efSceneAudio_exdatause_idx_name = 0x0aeb60;
		constexpr i32 efScene_exdatause_idx_name = 0x0ae9b0;

		constexpr i32 exo_readtrack = 0x029090;
		constexpr i32 str2int2 = 0x0918ab;

		constexpr i32 exo_trackparam_overwrite = 0x0299d1;

		constexpr i32 efpip_g = 0x1b2b20;

		constexpr i32 getpixeldata = 0x09a65c;
		constexpr i32 rgb2yc = 0x06fed0;

		constexpr i32 exedit_max_h_add8 = 0x135c64;
		constexpr i32 exedit_buffer_line = 0x135c68;

		constexpr i32 exedit_max_w = 0x196748;
		constexpr i32 exedit_max_h = 0x1920e0;

		constexpr i32 memory_ptr = 0x01a5328;

		constexpr i32 efBorder_func_proc_var_ptr = 0x0a5d28;
		constexpr i32 efBorder_func_proc_ptr = 0x0515d0;
		constexpr i32 efBorder_var_ptr = 0x1b1e30;
		constexpr i32 efPolorTransform_func_proc = 0x0748a0;
		constexpr i32 efPolorTransform_func_proc_ptr = 0x0add30;
		constexpr i32 efRadiationalBlur_func_proc = 0x00b310;
		constexpr i32 efRadiationalBlur_func_proc_ptr = 0x09fdb0;
		constexpr i32 efRadiationalBlur_var_ptr = 0x0d75a8;
		constexpr i32 efFlash_func_proc = 0x04e560;
		constexpr i32 efFlash_var_ptr = 0x1a6b7c;
		constexpr i32 efFlash_func_proc_ptr = 0x0a5a28;
		constexpr i32 efDirectionalBlur_func_proc_ptr = 0x0a00a0;
		constexpr i32 efDirectionalBlur_var_ptr = 0x0d75cc;
		constexpr i32 efDirectionalBlur_Filter_mt_func_call = 0x00cae6;
		constexpr i32 efLensBlur_Media_mt_func_call = 0x012761;
		constexpr i32 efLensBlur_Filter_mt_func_call = 0x012786;
		constexpr i32 efLensBlur_var_ptr = 0x11ec5c;

		constexpr i32 ScriptProcessingFilter = 0x1b2b10;

		constexpr i32 exfunc_10 = 0x04abe0;
		constexpr i32 exfunc_08 = 0x04ab40;
		constexpr i32 func_0x047ad0 = 0x047ad0;
		constexpr i32 exfunc_64 = 0x04d040;
		constexpr i32 scenechange_progress_times4096 = 0x230c60;
		constexpr i32 exfunc_1c = 0x04ade0;
		constexpr i32 GetCurrentProcessing = 0x047ba0;
		constexpr i32 LoadedFilterTable = 0x187c98;
		constexpr i32 TraScript_ProcessingObjectIndex = 0x1b2b04;
		constexpr i32 TraScript_ProcessingTrackBarIndex = 0x1b21f4;
		constexpr i32 TraScript_Time = 0x1b28c8;

		constexpr i32 LoadLua = 0x0646e0;
		constexpr i32 hmodule_lua = 0x1bac9c;
		constexpr i32 luaL_Reg_global_table = 0x09a680;
		constexpr i32 luaL_Reg_obj_table = 0x09a5c0;
		constexpr i32 exeditdir = 0x1b2b18;
		constexpr i32 sScriptFolderName = 0x1b2b4c;

		constexpr i32 l_effect = 0x05d0a0;
		constexpr i32 l_load = 0x05ef50;
		constexpr i32 l_draw = 0x05e250;
		constexpr i32 l_drawpoly = 0x05e680;
		constexpr i32 l_setanchor = 0x0625e0;

		constexpr i32 GetOrCreateLuaState = 0x064660;
		constexpr i32 LuaUnload = 0x064e90;
		constexpr i32 SetLuaPathAndCpath = 0x064550;
		constexpr i32 DoScriptInit = 0x0641a0;
		constexpr i32 DoScriptExit = 0x064250;
		constexpr i32 luastateidx = 0x1baccc;

		constexpr i32 lua_pop_nop = 0x064d15;
		constexpr i32 lua_set_nop = 0x06442a;
		constexpr i32 lua_tonumber_arg = 0x064d64;
		constexpr i32 lua_tostring_calling1 = 0x064d44;
		constexpr i32 lua_tostring_calling2 = 0x06448b;
		constexpr i32 lua_tostring_calling3 = 0x064449;

		constexpr i32 OutputDebugString_calling_err1 = 0x064453;
		constexpr i32 OutputDebugString_calling_err2 = 0x064495;
		constexpr i32 OutputDebugString_calling_err3 = 0x064d4e;
		constexpr i32 OutputDebugString_calling_dbg = 0x05d099;

		constexpr i32 ignore_media_param_reset_mov = 0x00674d;
		constexpr i32 ignore_media_param_reset_aud = 0x090116;

		constexpr i32 ExtendedFilter_wndcls = 0x02e872;

		constexpr i32 loaded_spi_array = 0x2321f0;

		constexpr i32 MyFindFirstFile = 0x04e220;
		constexpr i32 MyFindNextFile = 0x04e270;
		constexpr i32 LoadSpi = 0x08a210;

		constexpr i32 text_op_logfont_size = 0x050716;
		constexpr i32 specialcolorconv_status2 = 0x0a14f4;

		constexpr i32 ObjectArrayPointer = 0x1e0fa4;

		constexpr i32 ScaleColorBackGround = 0x0a4048;
		constexpr i32 ScaleColorForeGround = 0x0a404c;
		constexpr i32 LayerNameRectWidth = 0x037d1a;
		constexpr i32 layer_height_array = 0x0a3e08;
		
		//constexpr i32 LayerLockBorder = 0x037d78; // push char されているので安直には無理
		constexpr i32 LayerLockBorder_mod = 0x037d71;
		constexpr i32 LayerLockCenter = 0x037d81;
		constexpr i32 LayerClippingBorder = 0x037e96;
		constexpr i32 LayerClippingCenter = 0x037ea2;
		constexpr i32 LayerLinkBorder = 0x037dfd;
		constexpr i32 LayerLinkCenter = 0x037e09;
		constexpr i32 LayerHideAlpha = 0x09a570;

		constexpr i32 ObjectColorMedia = 0x0a40e0;
		constexpr i32 ObjectColorMFilter = 0x0a4104;
		constexpr i32 ObjectColorSound = 0x0a4128;
		constexpr i32 ObjectColorSFilter = 0x0a414c;
		constexpr i32 ObjectColorControl = 0x0a4170;
		constexpr i32 ObjectColorUnknown = 0x0a4194;
		constexpr i32 ObjectColorInactive = 0x0a41b8;

		constexpr i32 ObjectNameColor = 0x0a4040;
		constexpr i32 ObjectNameColorShow = 0x0a4040;
		constexpr i32 ObjectNameColorHide = 0x0a4044;
		constexpr i32 MidPointSize = 0x0a3e14;
		constexpr i32 ObjectClippingColorB = 0x0375d7;
		constexpr i32 ObjectClippingColorG = 0x0375d9;
		constexpr i32 ObjectClippingColorR = 0x0375db;
		constexpr i32 ObjectClippingHeight = 0x0375e2;

		constexpr i32 BPMGridColorBeat = 0x0a4050;
		constexpr i32 BPMGridColorMeasure = 0x0a4054;

	}
}
