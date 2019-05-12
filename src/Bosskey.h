#include "Parsekeys.h"

static bool is_hide = false;

static std::vector <HWND> hwnd_list;

BOOL CALLBACK SearchFirefoxWindow(HWND hWnd, LPARAM lParam)
{
    //隐藏
    if(IsWindowVisible(hWnd))
    {
		DWORD pid;
		GetWindowThreadProcessId(hWnd, &pid);
		if (pid == GetCurrentProcessId())
		{
			wchar_t buff[256];
			GetClassNameW(hWnd, buff, 255);
			if (wcscmp(buff, L"MozillaWindowClass") == 0 || wcscmp(buff, L"MozillaDialogClass") == 0)// || wcscmp(buff, L"MozillaDropShadowWindowClass")==0 || wcscmp(buff, L"SysShadow")==0 )
			{
				ShowWindow(hWnd, SW_HIDE);
				hwnd_list.push_back(hWnd);
			}
		}
    }
    return true;
}

void OnBosskey()
{
    if(!is_hide)
    {
        EnumWindows(SearchFirefoxWindow, 0);
    }
    else
    {
        for (auto r_iter = hwnd_list.rbegin(); r_iter != hwnd_list.rend() ; r_iter++)
        {
            ShowWindow(*r_iter, SW_SHOW);
        }
        hwnd_list.clear();
    }
    is_hide = !is_hide;
}

void HotKeyRegister(LPARAM lParam)
{
    Sleep(1000);

    ::CreateMutexW(NULL, TRUE, L"{EF700BF0-7687-42B1-AC78-642E7B8F32BE}");
    if (GetLastError() != ERROR_SUCCESS)
    {
        return;
    }

    RegisterHotKey(NULL, 0, LOWORD(lParam), HIWORD(lParam));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_HOTKEY)
        {
            OnBosskey();
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool Bosskey(const wchar_t *iniPath)
{
    wchar_t keys[256];
    GetPrivateProfileStringW(L"General", L"BossKey", L"", keys, 256, iniPath);
    if(keys[0])
    {
        UINT flag = ParseHotkeys(keys);

        std::thread th(HotKeyRegister, flag);
        th.detach();
    }
    return keys[0]!='\0';
}
