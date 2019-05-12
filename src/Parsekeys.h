﻿#pragma warning(disable: 4996)

bool IsWin7OrLater()
{
    DWORD version = GetVersion();
    DWORD major = (DWORD)(LOBYTE(LOWORD(version)));
    DWORD minor = (DWORD)(HIBYTE(LOWORD(version)));

    return (major > 6) || ((major == 6) && (minor >= 1));
}

UINT ParseHotkeys(const TCHAR* keys)
{
    UINT mo = 0;
    UINT vk = 0;

    TCHAR *temp = _tcsdup(keys);
    StringSplit(temp, '+', [&mo,&vk]
        (TCHAR *key)
        {
            //解析控制键
            if(_tcsicmp(key, _T("Shift"))==0) mo |= MOD_SHIFT;
            else if(_tcsicmp(key, _T("Ctrl"))==0) mo |= MOD_CONTROL;
            else if(_tcsicmp(key, _T("Alt"))==0) mo |= MOD_ALT;
            else if(_tcsicmp(key, _T("Win"))==0) mo |= MOD_WIN;

            else if(_tcsicmp(key, _T("←"))==0) vk = VK_LEFT;
            else if(_tcsicmp(key, _T("→"))==0) vk = VK_RIGHT;
            else if(_tcsicmp(key, _T("↑"))==0) vk = VK_UP;
            else if(_tcsicmp(key, _T("↓"))==0) vk = VK_DOWN;

            else if (_tcslen(key)==1)            //解析单个字符A-Z、0-9等
            {
                if(isalnum(key[0])) vk = toupper(key[0]);
                else vk = LOBYTE(VkKeyScan(key[0]));
            }
            //解析F1-F24功能键
            else if ( (key[0]=='F'||key[0]=='f') && isdigit(key[1]) )
            {
                int FX = _ttoi(&key[1]);
                if(FX>=1&&FX<=24) vk = VK_F1 + FX - 1;
            }
            // 解析其他按键
            else
            {
                if(_tcsicmp(key, _T("Left"))==0) vk = VK_LEFT;
                else if (_tcsicmp(key, _T("Right"))==0) vk = VK_RIGHT;
                else if (_tcsicmp(key, _T("Up"))==0) vk = VK_UP;
                else if (_tcsicmp(key, _T("Down"))==0) vk = VK_DOWN;

                else if (_tcsicmp(key, _T("Esc"))==0) vk = VK_ESCAPE;
                else if (_tcsicmp(key, _T("Tab"))==0) vk = VK_TAB;

                else if (_tcsicmp(key, _T("Backspace"))==0) vk = VK_BACK;
                else if (_tcsicmp(key, _T("Enter"))==0) vk = VK_RETURN;
                else if (_tcsicmp(key, _T("Space"))==0) vk = VK_SPACE;

                else if (_tcsicmp(key, _T("PrtSc"))==0) vk = VK_SNAPSHOT;
                else if (_tcsicmp(key, _T("Scroll"))==0) vk = VK_SCROLL;
                else if (_tcsicmp(key, _T("Pause"))==0) vk = VK_PAUSE;
                else if (_tcsicmp(key, _T("Break")) == 0) vk = VK_PAUSE;

                else if (_tcsicmp(key, _T("Insert"))==0) vk = VK_INSERT;
                else if (_tcsicmp(key, _T("Delete"))==0) vk = VK_DELETE;

                else if (_tcsicmp(key, _T("End"))==0) vk = VK_END;
                else if (_tcsicmp(key, _T("Home")) == 0) vk = VK_HOME;

                else if (_tcsicmp(key, _T("PageUp")) == 0) vk = VK_PRIOR;
                else if (_tcsicmp(key, _T("PageDown")) == 0) vk = VK_NEXT;
                else if (_tcsicmp(key, _T("PgUp")) == 0) vk = VK_PRIOR;
                else if (_tcsicmp(key, _T("PgDn")) == 0) vk = VK_NEXT;

                else if (_tcsicmp(key, _T("Plus")) == 0) vk = VK_OEM_PLUS;
                else if (_tcsicmp(key, _T("Minus")) == 0) vk = VK_OEM_MINUS;

                // 浏览器快捷键
                else if (_tcsicmp(key, _T("Back")) == 0) vk = VK_BROWSER_BACK;
                else if (_tcsicmp(key, _T("Forward")) == 0) vk = VK_BROWSER_FORWARD;
                else if (_tcsicmp(key, _T("Refresh")) == 0) vk = VK_BROWSER_REFRESH;

                // 音量相关快捷键
                else if (_tcsicmp(key, _T("VolumeMute")) == 0) vk = VK_VOLUME_MUTE;
                else if (_tcsicmp(key, _T("VolumeDown")) == 0) vk = VK_VOLUME_DOWN;
                else if (_tcsicmp(key, _T("VolumeUp")) == 0) vk = VK_VOLUME_UP;
            }

        }
    );

    free(temp);

    #define MOD_NOREPEAT 0x4000
    if (IsWin7OrLater()) mo |= MOD_NOREPEAT;

    return MAKELPARAM(mo, vk);
}
