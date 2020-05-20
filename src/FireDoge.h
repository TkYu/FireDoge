#ifndef __MAIN_H__
#define __MAIN_H__

#define WINVER _WIN32_WINNT_WINXP
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#define _GDI32_

#include <SDKDDKVer.h>

#include <vector>
#include <string>

#define _MMSYSTEM_H
#define _INC_MMSYSTEM
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <psapi.h>
#include <oleacc.h>
#include <process.h>
#include <thread>
#include <Knownfolders.h>

#include <GdiPlus.h>
using namespace Gdiplus;

#pragma warning(disable: 4838)
#pragma warning(disable: 4302)

#define _WTL_NO_CSTRING
#include <atlbase.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlctrlx.h>

// 定义MWORD为机器字长
#include <stdint.h>
#ifdef _WIN64
typedef uint64_t MWORD;
#else
typedef uint32_t MWORD;
#endif

void FireDoge(LPWSTR command_line);

HMODULE hInstance;

#include "..\version.h"

#include "hijack.h"
#include "SharedConfig.h"
#include "util.h"

#include "..\3rd\minhook\include\MinHook.h"

#include "..\3rd\mongoose\mongoose.h"

#include "Patchs.h"
//#include "PatchResourcesPak.h"
//#include "GetParent.h"
#include "Loader.h"
//#include "ModifyLnk.h"
//#include "hosts.h"

#include "gesture\GestureMatch.h"
#include "gesture\GesturePoint.h"
#include "gesture\AddWindow.h"
#include "gesture\GestureWindow.h"
#include "gesture\GestureMgr.h"
#include "TabBookmark.h"

#include "SettingWeb.h"

#include "Bosskey.h"
#include "CustomCommand.h"


#endif // __MAIN_H__
