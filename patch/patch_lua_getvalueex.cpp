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

#include "patch_lua_getvalueex.hpp"
#ifdef PATCH_SWITCH_LUA_GETVALUE

#include "offset_address.hpp"

namespace patch {
	int __cdecl lua_getvalueex_t::lua_getvalue_override(lua_State* L) {
		//lua_t::states.push_back(L);
		int layer_idx = 0;
		int target_i = 0;

		auto efpip = load_i32<ExEdit::FilterProcInfo*>(GLOBAL::exedit_base + OFS::ExEdit::efpip_g);
		auto ScriptProcessingFilter = load_i32<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::ScriptProcessingFilter);

		int n = lua_gettop(L);
		int frame;
		int subframe;
		if (n < 2) {
			frame = efpip->frame_num;
			subframe = efpip->subframe;
		}
		else {
			double time = luaL_checknumber(L, 2);
			double frame_before_round = time * efpip->framerate_nu / efpip->framerate_de;
			frame = static_cast<int>(floor(frame_before_round));
			subframe = static_cast<int>(floor((frame_before_round - frame) * 100.0));
			if (n < 3) {
				frame += ScriptProcessingFilter->frame_start_chain;
			}
			else {
				int section = luaL_checkinteger(L, 3);
				if (section < 0) {
					frame += ScriptProcessingFilter->frame_end_chain;
				}
				else {
					//exfunc_10;
					auto ps = reinterpret_cast<ExEdit::ObjectFilterIndex(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_10)(ScriptProcessingFilter->processing);
					while (section) {
						//exfunc_08
						ps = reinterpret_cast<ExEdit::ObjectFilterIndex(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_08)(ps);
						section--;
					}
					if (is_valid(ps)) {
						// func_0x047ad0
						frame += reinterpret_cast<ExEdit::Object*(__cdecl*)(ExEdit::ObjectFilterIndex, int)>(GLOBAL::exedit_base + OFS::ExEdit::func_0x047ad0)(ps, 0)->frame_begin;
					}
					else {
						frame += ScriptProcessingFilter->frame_end_chain;
					}
				}
			}
		}
		int track_idx;
		int result_nu, result_de;
		if (lua_isnumber(L, 1)) {
			int target = luaL_checkinteger(L, 1);
			if (target < 0)return 0;
			if (ScriptProcessingFilter->track_n <= target)return 0;
			reinterpret_cast<BOOL(__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int*, int)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_64)(ScriptProcessingFilter->processing, frame, subframe, &result_nu, target + 1);
			result_de = 100;
		}
		else {
			const char* target = luaL_checkstring(L, 1);
			if (lstrcmpiA(target, "scenechange") == 0) {
				lua_pushnumber(L, load_i32<int>(GLOBAL::exedit_base + OFS::ExEdit::scenechange_progress_times4096) / 4096.0);
				return 1;
			}
			ExEdit::ObjectFilterIndex ps;
			
			if (strncmp(target, "layer", 5) == 0) {
				target += 5;
				int layer_idx = 0;
				while (*target != '.') {
					uint8_t i = (uint8_t)*target - (uint8_t)'0';
					if (i > 9) {
						layer_idx = -1; break;
					}
					layer_idx = layer_idx * 10 + i;
					target++;
				}
				if (layer_idx < 1 && 100 < layer_idx) return 0;
				target++;
				// ef0x1c
				ps = reinterpret_cast<ExEdit::ObjectFilterIndex(__cdecl*)(int, int, int, int, unsigned int)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_1c)(frame, layer_idx - 1, ScriptProcessingFilter->scene_set, 0, 0x10);
			}
			else {
				// GetCurrentProcessing
				ps = reinterpret_cast<ExEdit::ObjectFilterIndex(__cdecl*)(ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + OFS::ExEdit::GetCurrentProcessing)(efpip);
			}
			if (!is_valid(ps))return 0;

			int a;
			// FUN_10047ad0
			auto eop = reinterpret_cast<ExEdit::Object*(__cdecl*)(ExEdit::ObjectFilterIndex, int*)>(GLOBAL::exedit_base + OFS::ExEdit::func_0x047ad0)(ps, &a);
			auto efp = reinterpret_cast<ExEdit::Filter**>(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable)[eop->filter_param[a].id];
			if (efp->track_gui == nullptr)return 0;

#if 1
			char target2[7];
			{
				int i = 0;
				do{
					if(target[i] == '\0') {
						target2[i] = '\0';
						goto BREAK;
					}
					target2[i] = target[i] | 0b10'0000; // to lower
					i++;
				} while(i < 7);
				return 0;
				BREAK:
				;
			}

			switch(target2[0]) {
			case 'a': // alpha aspect
				if(*(int*)(target2 + 1) == 'ahpl') {
					if(target2[5] != '\0') return 0;
					// alpha

					result_de = 100;
					track_idx = efp->track_gui->alpha;
					if (track_idx < 0) return lua_pushnumber(L, 0), 1;
					
					//exfunc_64
					reinterpret_cast<BOOL(__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int*, int)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_64)(ps, frame, subframe, &result_nu, track_idx + 1);
					lua_pushnumber(L, 1. - result_nu * 0.001);
				
					return 1;
				}
				if(*(int*)(target2 + 1) == 'ceps') {
					if(*(short*)(target2 + 5) != '\0t') return 0;
					// aspect

					result_de = 1000;
					track_idx = efp->track_gui->aspect;
					if (track_idx < 0) return lua_pushnumber(L, 0), 1;

					break;
				}
				return 0;
			case 'c': // cx cy cz
				switch(*(short*)(target2 + 1)) {
					case '\0x':

						result_de = 10;
						track_idx = efp->track_gui->cx;
						if (track_idx < 0) return lua_pushnumber(L, 0), 1;

						break;
					case '\0y':

						result_de = 10;
						track_idx = efp->track_gui->cy;
						if (track_idx < 0) return lua_pushnumber(L, 0), 1;

						break;
					case '\0z':

						result_de = 10;
						track_idx = efp->track_gui->cz;
						if (track_idx < 0) return lua_pushnumber(L, 0), 1;

						break;
					default:
						return 0;
				}
				break;
			case 't': // time
				if(*(int*)(target2 + 1) != '\0emi') return 0;

				if (auto leader = eop->index_midpt_leader; leader >= 0) {
					eop = load_i32<ExEdit::Object*>(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer) + leader;
				}
				lua_pushnumber(L, (subframe * 0.01 + (double)(frame - eop->frame_begin))* (double)efpip->framerate_de / (double)efpip->framerate_nu);
				//lua_pushnumber(L, (subframe * 0.01 + (double)(frame - ScriptProcessingFilter->frame_start_chain)) * (double)efpip->framerate_de / (double)efpip->framerate_nu);
				return 1;
			case 'r': // rx ry rz
				switch(*(short*)(target2 + 1)) {
					case '\0x':

						result_de = 100;
						track_idx = efp->track_gui->rx;
						if (track_idx < 0) return lua_pushnumber(L, 0), 1;

						break;
					case '\0y':

						result_de = 100;
						track_idx = efp->track_gui->ry;
						if (track_idx < 0) return lua_pushnumber(L, 0), 1;

						break;
					case '\0z':

						result_de = 100;
						track_idx = efp->track_gui->rz;
						if (track_idx < 0) return lua_pushnumber(L, 0), 1;

						break;
					default:
						return 0;
				}
				break;
			case 'x':
				if(target2[1] != '\0') return 0;

				result_de = 10;
				track_idx = efp->track_gui->bx;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;

				break;
			case 'y':
				if(target2[1] != '\0') return 0;

				result_de = 10;
				track_idx = efp->track_gui->by;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;

				break;
			case 'z': // z zoom
				if(target2[1] == '\0') {
					// z

					result_de = 10;
					track_idx = efp->track_gui->bz;
					if (track_idx < 0) return lua_pushnumber(L, 0), 1;

					break;
				}
				else if(*(int*)(target2 + 1) == '\0moo') {
					//zoom

					result_de = 100;
					track_idx = efp->track_gui->zoom;
					if (track_idx < 0) return lua_pushnumber(L, 0), 1;

					break;
				}
				return 0;
			default:
				return 0;
			}
#else
			if (lstrcmpiA(target, "x") == 0) {
				result_de = 10;
				track_idx = efp->track_gui->bx;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "y") == 0) {
				result_de = 10;
				track_idx = efp->track_gui->by;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "z") == 0) {
				result_de = 10;
				track_idx = efp->track_gui->bz;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "zoom") == 0) {
				result_de = 100;
				track_idx = efp->track_gui->zoom;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "rx") == 0) {
				result_de = 100;
				track_idx = efp->track_gui->rx;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "ry") == 0) {
				result_de = 100;
				track_idx = efp->track_gui->ry;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "rz") == 0) {
				result_de = 100;
				track_idx = efp->track_gui->rz;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "cx") == 0) {
				result_de = 10;
				track_idx = efp->track_gui->cx;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "cy") == 0) {
				result_de = 10;
				track_idx = efp->track_gui->cy;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "cz") == 0) {
				result_de = 10;
				track_idx = efp->track_gui->cz;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "alpha") == 0) {
				result_de = 100;
				track_idx = efp->track_gui->alpha;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
				
				//exfunc_64
				reinterpret_cast<BOOL(__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int*, int)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_64)(ps, frame, subframe, &result_nu, track_idx + 1);
				lua_pushnumber(L, 1. - result_nu * 0.001);
				
				return 1;
			}
			else if (lstrcmpiA(target, "aspect") == 0) {
				result_de = 1000;
				track_idx = efp->track_gui->aspect;
				if (track_idx < 0) return lua_pushnumber(L, 0), 1;
			}
			else if (lstrcmpiA(target, "time") == 0) {
				lua_pushnumber(L, (subframe * 0.01 + (double)(frame - ScriptProcessingFilter->frame_start_chain)) * (double)efpip->framerate_de / (double)efpip->framerate_nu);
				return 1;
			}
			else return 0;
#endif

			reinterpret_cast<BOOL(__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int*, int)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_64)(ps, frame, subframe, &result_nu, track_idx + 1);
		}
		lua_pushnumber(L, static_cast<double>(result_nu) / static_cast<double>(result_de));
		return 1;
	}

	int __cdecl lua_getvalueex_t::lua_getvalueex_main(lua_State* L) {
		

		return 1;
	}
} // namespace patch
#endif // #ifdef PATCH_SWITCH_GETVALUEEX
