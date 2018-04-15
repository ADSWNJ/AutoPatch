#pragma once
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

#include "windows.h"

extern "C" _declspec(dllexport) const char* AutoPatch_Date() { return __DATE__; }
#define AUTOPATCH_TGT(tgt) extern "C" _declspec(dllexport) const char* AutoPatch_Target() {return tgt;}
#define AUTOPATCH_CMD(cmd) extern "C" _declspec(dllexport) bool AutoPatch_Command() {return cmd();}

namespace AutoPatch
{
  typedef void (*FUNC_AUTOPATCH_MAIN)();

  static void Execute() {
    HMODULE hDLL;
    FUNC_AUTOPATCH_MAIN fMain;
    if (!(hDLL = LoadLibraryA(".\\Modules\\AutoPatch.dll"))) return;
    fMain = (FUNC_AUTOPATCH_MAIN)GetProcAddress(hDLL, "AutoPatch_Main");
    if (!fMain) (*fMain)();
    FreeLibrary(hDLL);
  };
}

