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

#include <stdio.h>
#include <time.h>
#include "AutoPatch.hpp"

typedef const char* (*FUNC_AUTOPATCH_DATE)();
typedef const char* (*FUNC_AUTOPATCH_TGTFILE)();
typedef void        (*FUNC_AUTOPATCH_CMD)();

static unsigned long getDate(const char *ascdate) {
// Convert __DATE__ format to a long in the form yyyymmdd  
  char *tmpdate = _strdup(ascdate);
  char *tmpdd = tmpdate + 4;
  char *tmpyyyy = tmpdate + 7;
  *(tmpdate + 3) = '\0';
  *(tmpdate + 6) = '\0';
  const char* mm[] = {"   ", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  unsigned long res = 0;
  unsigned long m = 0;
  for (m = 0; m < 12; m++) { if (!strcmp(tmpdate, mm[m])) break; };
  res += (*(tmpyyyy) - '0') * 1000; tmpyyyy++;
  res += (*tmpyyyy - '0') * 100;  tmpyyyy++;
  res += (*tmpyyyy - '0') * 10; tmpyyyy++;
  res += (*tmpyyyy - '0');
  res *= 10000;
  res += m * 100;
  if (*tmpdd != ' ') res += (*tmpdd - '0') * 10;
  res += *++tmpdd - '0';
  return res;
}

static void TagFile(char* fp) {
  // Look for _APyyyymmddhhmmss tag on file, and apply if missing
  char wkgfp[MAX_PATH + 1];
  strcpy_s(wkgfp, MAX_PATH, fp);
  char *p = wkgfp, *lp = NULL;
  while ((p = strstr(p, "_AP20")), p != NULL) {
    lp = p++;
  }
  if (lp) { // check we have 12 numerics after _AP20
    char *c = lp + 5;
    while (*c) {
      if (*c < '0' || *c > '9') break;
      c++;
    }
    if (c == lp + 17) return;
  }
  SYSTEMTIME st;
  GetLocalTime(&st);
  char bt[128];
  sprintf_s(bt, 128, "%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
  char buf[MAX_PATH + 1];
  sprintf_s(buf, MAX_PATH, "_AP%s.dll", bt);
  p = wkgfp + strlen(wkgfp) - 4;
  strcpy_s(p, 128, buf);
  if (MoveFile(fp, wkgfp)) {
    strcpy_s(fp, MAX_PATH, wkgfp);
  }
  return;
}

static void MoveFileToOld(const char *sfp, const char *sf) {
  char wkgp[MAX_PATH + 1];
  sprintf_s(wkgp, MAX_PATH, ".\\Modules\\AutoPatch\\Old\\%s", sf);
  MoveFile(sfp, wkgp);
  TagFile(wkgp);
  return;
}

static void CopyFileToTarget(const char *sfp, const char *tfp, FUNC_AUTOPATCH_CMD fCmd) {
  if (CopyFile(sfp, tfp, true)) {
    if (fCmd) (*fCmd)(); // After Copy to Target, call the source's command function if defined (e.g. to move other files)
  }
  return;
}


extern "C" _declspec(dllexport) void AutoPatch_Main() {
// Only needed in the AutoPatcher itself ... core of the AutoPatch logic

  WIN32_FIND_DATA fileData;
  HANDLE hFile;

  // Find all *.dll files in the AutoPatch directory (FindFirstFile / FindNextFile)
  hFile = FindFirstFile(".\\Modules\\AutoPatch\\*.dll", &fileData);
  while (hFile != INVALID_HANDLE_VALUE) {
    char sf[MAX_PATH + 1]; // source file
    strcpy_s(sf, MAX_PATH, fileData.cFileName);

    char sfp[MAX_PATH+1]; // source file path
    sprintf_s(sfp, MAX_PATH, ".\\Modules\\AutoPatch\\%s", sf);
    TagFile(sfp);

    char tfp[MAX_PATH + 1]; // target file path
    const char* sd = NULL;
    const char* tf = NULL;

    HMODULE hDLL = LoadLibraryA(sfp);  // NULL if the dll cannot load for whatever reason
    if (hDLL != NULL) {
      FUNC_AUTOPATCH_DATE fSrcDate = (FUNC_AUTOPATCH_DATE)GetProcAddress(hDLL, "AutoPatch_Date"); // NULL if the function is missing
      FUNC_AUTOPATCH_TGTFILE fTgtFile = (FUNC_AUTOPATCH_TGTFILE)GetProcAddress(hDLL, "AutoPatch_TgtFile"); // NULL if the function is missing
      FUNC_AUTOPATCH_CMD fCmd = (FUNC_AUTOPATCH_CMD)GetProcAddress(hDLL, "AutoPatch_Cmd"); // NULL if the function is missing

      if (fSrcDate != NULL && fTgtFile != NULL) { // we insist on Date and TgtFile. Cmd is optional.  
        sd = (*fSrcDate)();
        tf = (*fTgtFile)();

        if (!strstr(tf, "..")) { // Ban ",," in targetfile, to ensure we stay inside the Orbiter\modules directory
          sprintf_s(tfp, MAX_PATH, ".\\Modules\\%s", tf);

          HMODULE hTarget = LoadLibraryA(tfp);
          if (hTarget != NULL) {
            FUNC_AUTOPATCH_DATE fTgtDate = (FUNC_AUTOPATCH_DATE)GetProcAddress(hTarget, "AutoPatch_Date");
            if (fTgtDate != NULL) { // look for target's Date
              const char* td = (*fTgtDate)();
              if (getDate(td) < getDate(sd)) {
                MoveFileToOld(tfp, tf);
                CopyFileToTarget(sfp, tfp, fCmd);                // Replacing older target
              } else if (getDate(td) > getDate(sd)) {
                MoveFileToOld(sfp, sf);                          // Removing old source (no longer latest)
              }
              FreeLibrary(hTarget);
            } else {
              CopyFileToTarget(sfp, tfp, fCmd);                  // Replacing target with no date tags
            }
          } else {
            CopyFileToTarget(sfp, tfp, fCmd);                    // Adding new target
          }
        }
      }
    } 
    FreeLibrary(hDLL);
    if (!FindNextFile(hFile, &fileData)) hFile = INVALID_HANDLE_VALUE;
  }
  return;
}




void Hello() {
  return;
}

AUTOPATCH_TGT("AutoPatch.dll")
AUTOPATCH_CMD(Hello)
