#pragma once
#pragma data_seg(".FDSHARED")

wchar_t config_path[MAX_PATH] = {0};

wchar_t appdata_path[MAX_PATH] = { 0 };
wchar_t localdata_path[MAX_PATH] = { 0 };

bool DoubleClickCloseTab = false;
bool RightClickCloseTab = false;
bool HoverActivateTab = false;
int HoverTime = HOVER_DEFAULT;
// bool KeepLastTab = false;
bool HoverTabSwitch = false;
bool RightTabSwitch = false;
// bool BookMarkNewTab = false;
// bool OpenUrlNewTab = false;
// bool NotBlankTab = false;
// bool FrontNewTab = false;
// bool HandleMenu = false;
int DPI = 100;

bool MouseGesture = false;
int Sensitive = 50;

// bool BlankNewTab = false;
// bool RemoveUpdateError = false;
// bool RemoveDevelopError = false;
bool StopWeb = false;

bool AddGestureFlag = false;

bool KeyTransform = false;
bool KeyRepeat = false;
bool KeyIgnoreEdit = false;
bool DebugKeyPress = false;

// bool HostsSwitch = false;
// wchar_t hosts_file[MAX_PATH] = {0};

int ACP = 0;

DWORD BossID = 0;

#ifdef _WIN64
const bool IsX64 = true;
#else
const bool IsX64 = false;
#endif

#pragma data_seg()
#pragma comment(linker, "/section:.FDSHARED,RWS")

void ReadConfig(const wchar_t *iniPath)
{
    DoubleClickCloseTab = GetPrivateProfileInt(L"UIEnhancements", L"DoubleClickCloseTab", 1, iniPath) == 1;
    RightClickCloseTab = GetPrivateProfileInt(L"UIEnhancements", L"CloseTabByRightClick", 0, iniPath) == 1;
    HoverActivateTab = GetPrivateProfileInt(L"UIEnhancements", L"ActivateTabByHover", 0, iniPath) == 1;
    HoverTime = GetPrivateProfileInt(L"UIEnhancements", L"HoverDelay", HOVER_DEFAULT, iniPath);
   // KeepLastTab = GetPrivateProfileInt(L"UIEnhancements", L"KeepLastTab", 1, iniPath) == 1;
    HoverTabSwitch = GetPrivateProfileInt(L"UIEnhancements", L"SwitchTabByHover", 1, iniPath) == 1;
    RightTabSwitch = GetPrivateProfileInt(L"UIEnhancements", L"SwitchTabByHoldRightButton", 1, iniPath) == 1;
    //BookMarkNewTab = GetPrivateProfileInt(L"UIEnhancements", L"新标签打开书签", 1, iniPath) == 1;
    //OpenUrlNewTab = GetPrivateProfileInt(L"UIEnhancements", L"新标签打开网址", 0, iniPath) == 1;
    //NotBlankTab = GetPrivateProfileInt(L"UIEnhancements", L"新标签页不生效", 1, iniPath) == 1;
    //FrontNewTab = GetPrivateProfileInt(L"UIEnhancements", L"前台打开新标签", 1, iniPath) == 1;
    //HandleMenu = GetPrivateProfileInt(L"UIEnhancements", L"处理弹出菜单", 1, iniPath) == 1;
    DPI = GetPrivateProfileInt(L"UIEnhancements", L"DPI", 100, iniPath);

    MouseGesture = GetPrivateProfileInt(L"Gesture", L"Enabled", 1, iniPath) == 1;
    Sensitive = GetPrivateProfileInt(L"Gesture", L"Threshold", 50, iniPath);

    //BlankNewTab = GetPrivateProfileIntW(L"General", L"空白新标签页面", 0, iniPath) == 1;
    //RemoveUpdateError = GetPrivateProfileIntW(L"General", L"移除更新错误", 0, iniPath) == 1;
    //RemoveDevelopError = GetPrivateProfileIntW(L"General", L"移除开发者模式警告", 0, iniPath) == 1;
    StopWeb = GetPrivateProfileInt(L"General", L"WebUIDisabled", 0, iniPath) == 1;
    
    KeyTransform = GetPrivateProfileInt(L"KeyTransformation", L"Enabled", 0, iniPath) == 1;
    KeyRepeat = GetPrivateProfileInt(L"KeyTransformation", L"DuplicateKeyAccepted", 1, iniPath) == 1;
    KeyIgnoreEdit = GetPrivateProfileInt(L"KeyTransformation", L"IgnoreAtEditBox", 1, iniPath) == 1;
    DebugKeyPress = GetPrivateProfileInt(L"KeyTransformation", L"Debug", 0, iniPath) == 1;

    //HostsSwitch = GetPrivateProfileIntW(L"hosts", L"Enabled", 1, iniPath) == 1;
    //GetPrivateProfileStringW(L"hosts", L"保存位置", L"%appdata%\\gc-hosts.txt", hosts_file, MAX_PATH, iniPath);
}