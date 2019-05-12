#pragma once
#pragma data_seg(".FDSHARED")

wchar_t config_path[MAX_PATH] = {0};

wchar_t appdata_path[MAX_PATH] = { 0 };
wchar_t localdata_path[MAX_PATH] = { 0 };

bool DoubleClickCloseTab = false;
bool RightClickCloseTab = false;
bool HoverActivateTab = false;
int HoverTime = HOVER_DEFAULT;
bool HoverTabSwitch = false;
bool RightTabSwitch = false;
int DPI = 100;

bool MouseGesture = false;
int Sensitive = 50;

bool StopWeb = false;

bool AddGestureFlag = false;

bool KeyTransform = false;
bool KeyRepeat = false;
bool KeyIgnoreEdit = false;
bool DebugKeyPress = false;


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
    HoverTabSwitch = GetPrivateProfileInt(L"UIEnhancements", L"SwitchTabByHover", 1, iniPath) == 1;
    RightTabSwitch = GetPrivateProfileInt(L"UIEnhancements", L"SwitchTabByHoldRightButton", 1, iniPath) == 1;
    DPI = GetPrivateProfileInt(L"UIEnhancements", L"DPI", 100, iniPath);

    MouseGesture = GetPrivateProfileInt(L"Gesture", L"Enabled", 1, iniPath) == 1;
    Sensitive = GetPrivateProfileInt(L"Gesture", L"Threshold", 50, iniPath);
    StopWeb = GetPrivateProfileInt(L"General", L"WebUIDisabled", 0, iniPath) == 1;
    
    KeyTransform = GetPrivateProfileInt(L"KeyTransformation", L"Enabled", 0, iniPath) == 1;
    KeyRepeat = GetPrivateProfileInt(L"KeyTransformation", L"DuplicateKeyAccepted", 1, iniPath) == 1;
    KeyIgnoreEdit = GetPrivateProfileInt(L"KeyTransformation", L"IgnoreAtEditBox", 1, iniPath) == 1;
    DebugKeyPress = GetPrivateProfileInt(L"KeyTransformation", L"Debug", 0, iniPath) == 1;
}