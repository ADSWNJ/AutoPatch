// ==============================================================
// ORBITER AUTOPATCH UTILITY
//
// http://www.github.com/ADSWNJ/AutoPatch
//
// Allow Orbiter modules to intelligently patch their dependencies
// and tidy up after themselves. 
//
// Copyright  (C) 2018 Andrew "ADSWNJ" Stokes
//
//                         All rights reserved
//
// AutoPatch is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// AutoPatch is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with ModuleMessagingExt. If not, see
// <http://www.gnu.org/licenses/>
// ==============================================================

#include "AutoPatch.hpp"

typedef const char* (*FUNC_AUTOPATCH_DATE)();
typedef const char* (*FUNC_AUTOPATCH_TARGET)();
typedef void        (*FUNC_AUTOPATCH_COMMAND)();

extern "C" _declspec(dllexport) void AutoPatch_Main() {
  return;
}

bool Hello() {
  return true;
}

AUTOPATCH_TGT("AutoPatch_X.dll")
AUTOPATCH_CMD(Hello)
