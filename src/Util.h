#include <cctype>
#include <algorithm>
#include <map>
#include "FastSearch.h"

//函数声明
std::wstring GetCurrentURL();

/**
* \brief 打印调试信息
* \param format 格式
* \param ... 参数
*/
void DebugLog(const wchar_t* format, ...)
{
    va_list args;

    va_start(args, format);
    size_t length = _vscwprintf(format, args);
    va_end(args);

    wchar_t* buffer = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
    va_start(args, format);
    _vsnwprintf_s(buffer, length + 1, length, format, args);
    va_end(args);

    OutputDebugStringW(buffer);
    free(buffer);
}
// 处理多国语言
const static std::map<std::wstring, std::vector<std::wstring>> i18n = {
{ L"AddNew",{ L"添加鼠标手势" ,L"新增滑鼠手勢" } },
{ L"AlreadyExist",{ L"添加鼠标手势【注意：已存在相似手势】" ,L"新增滑鼠手勢【注意：已存在相似手勢】" } },
{ L"NameRequire",{ L"必须填写名称" ,L"必須填寫名稱" } },
{ L"ParamRequire",{ L"必须填写参数" ,L"必須填寫參數" } },
{ L"GestureAction",{ L"手势操作" ,L"手勢動作" } },
{ L"Name:",{ L"名称：" ,L"名稱：" } },
{ L"Action:",{ L"命令：" ,L"指令：" } },
{ L"Param:",{ L"参数：" ,L"參數：" } },
{ L"OK",{ L"完成" ,L"確定" } },
{ L"Cancel",{ L"取消" ,L"取消" } },
{ L"SendKeys",{ L"发送按键" ,L"傳送按鍵" } },
{ L"Window",{ L"窗口控制" ,L"視窗控制" } },
{ L"OpenUrl",{ L"打开网址" ,L"開啟網址" } },
{ L"RunApp",{ L"打开程序" ,L"開啟程式" } },
{ L"Min",{ L"最小化" ,L"最小化" } },
{ L"MaxOrRestore",{ L"最大化/还原" ,L"最大化/還原" } },
{ L"TopMost",{ L"置顶/取消置顶" ,L"最上層顯示/取消最上層顯示" } },
{ L"FireDoge Config",{ L"设置 FireDoge" ,L"設定 FireDoge" } },
{ L"InVailed Gesture",{ L"无效手势" ,L"無效手勢" } },
{ L"Add New Gesture",{ L"添加新的手势" ,L"新增滑鼠手勢" } },
{ L"UnConfigured Gesture",{ L"尚未配置" ,L"尚未設定" } },
{ L"CopyToClipboard",{ L"复制到剪贴板" ,L"複製至剪貼簿" } }
};

/**
* \brief 国际化
* \param text 内容
* \return
*/
inline std::wstring Lang(std::wstring text)
{
    if (ACP == 0)
    {
        //ACP = GetSystemDefaultUILanguage();
        //ACP = GetUserDefaultUILanguage();
        ACP = GetACP();
        //DebugLog(L"ACP = %d", ACP);
    }
    const auto it = i18n.find(text);
    if (it != i18n.end())
    {
        //DebugLog(L"I18N %s -> %s", text.c_str(), it->second[ACP == 950 ? 1 : 0].c_str());
        return it->second[ACP == 950 ? 1 : 0];
    }
    //if (i18n.count(text))
    //{
    //    auto item = i18n[text];
    //    DebugLog(L"I18N %s -> %s", text, item[ACP == 950 ? 1 : 0]);
    //    return item[ACP == 950 ? 1 : 0];
    //}
    return text;
}
/**
* \brief 根据类名寻找句柄
* \param pid 进程ID
* \param szWndClassName 类名
* \return 句柄，如果没找到返回NULL
*/
HWND FindWindowByProcessIdAndClassName(DWORD pid, const LPWSTR szWndClassName)
{
    HWND hCurWnd = GetTopWindow(0);
    while (hCurWnd != NULL)
    {
        DWORD cur_pid;
        DWORD dwTheardId = GetWindowThreadProcessId(hCurWnd, &cur_pid);

        if (cur_pid == pid)
        {
            if (IsWindowVisible(hCurWnd) != 0)
            {
                TCHAR szClassName[256];
                GetClassName(hCurWnd, szClassName, 256);
                if (_tcscmp(szClassName, szWndClassName) == 0)
                    return hCurWnd;
            }
        }
        hCurWnd = GetNextWindow(hCurWnd, GW_HWNDNEXT);
    }
    return NULL;
}
// 如果需要给路径加引号
inline std::wstring QuoteSpaceIfNeeded(const std::wstring &str)
{
    if (str.find(L' ') == std::wstring::npos)
        return str;

    std::wstring escaped(L"\"");
    for(auto c : str)
    {
        if (c == L'"')
            escaped += L'"';
        escaped += c;
    }
    return escaped + L'"';
}

// 获取exe所在路径
std::wstring GetExePath()
{
    // exe路径
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    // exe所在文件夹
    wchar_t exeFolder[MAX_PATH];
    wcscpy(exeFolder, exePath);
    PathRemoveFileSpecW(exeFolder);

    return std::wstring(exeFolder);
}

// 展开环境路径比如 %windir%
std::wstring ExpandEnvironmentPath(const std::wstring &path)
{
    std::vector<wchar_t> buffer(MAX_PATH);
    size_t expandedLength = ::ExpandEnvironmentStrings(path.c_str(), &buffer[0], (DWORD)buffer.size());
    if (expandedLength > buffer.size())
    {
        buffer.resize(expandedLength);
        expandedLength = ::ExpandEnvironmentStrings(path.c_str(), &buffer[0], (DWORD)buffer.size());
    }
    return std::wstring(&buffer[0], 0, expandedLength);
}

// 替换字符串
void ReplaceStringInPlace(std::wstring& subject, const std::wstring& search, const std::wstring& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::wstring::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

bool ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
    bool find = false;
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
        find = true;
    }
    return find;
}
/**
* \brief 获取当前时间戳
* \return 转换成毫秒
*/
inline uint64_t get_timestamp()
{
    std::chrono::time_point<std::chrono::system_clock> ts = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(ts.time_since_epoch()).count();
}

/**
* \brief 扩展Command
* \param command cmd
* \return
*/
std::wstring ExpandCommand(const std::wstring command)
{
    std::wstring result = std::wstring(command);
    //DebugLog(L"展开command %s", command.c_str());
    if (command.find(L"{CURRENT_URL}") != command.npos)
    {
        //auto tUrl = GetCurrentURLAsync();
        ReplaceStringInPlace(result, L"{CURRENT_URL}", GetCurrentURL());
    }
    if (command.find(L"{CURRENT_APPDIR}") != command.npos)
    {
        ReplaceStringInPlace(result, L"{CURRENT_APPDIR}", GetExePath());
    }
    if (command.find(L"{CURRENT_TIMESTAMP}") != command.npos)
    {
        ReplaceStringInPlace(result, L"{CURRENT_TIMESTAMP}", std::to_wstring(get_timestamp()));
    }
    return result;
}
/**
* \brief 复制东西到剪贴板里面
* \param owner 默认传NULL
* \param w 内容
*/
void CopyToClipboard(HWND owner, const std::wstring &w)
{
    //DebugLog(L"设置剪贴板：%s", w.c_str());
    if (w.empty())return;
    if (OpenClipboard(owner))
    {
        HGLOBAL hgClipBuffer = nullptr;
        std::size_t sizeInWords = w.size() + 1;
        std::size_t sizeInBytes = sizeInWords * sizeof(wchar_t);
        hgClipBuffer = GlobalAlloc(GHND | GMEM_SHARE, sizeInBytes);
        if (!hgClipBuffer)
        {
            CloseClipboard();
            return;
        }
        wchar_t *wgClipBoardBuffer = static_cast<wchar_t*>(GlobalLock(hgClipBuffer));
        wcscpy_s(wgClipBoardBuffer, sizeInWords, w.c_str());
        GlobalUnlock(hgClipBuffer);
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, hgClipBuffer);
        CloseClipboard();
    }
}
// 扩展路径
std::wstring ExpandPath(const std::wstring &old_path)
{
    //wchar_t path[MAX_PATH];
    //wcscpy(path, old_path.c_str());

    std::wstring exe_path = GetExePath();

    // 转换绝对路径
    //PathCombineW(path, exe_path.c_str(), path);

    // 扩展环境变量
    std::wstring new_path = ExpandEnvironmentPath(old_path);

    // 扩展%app%
    ReplaceStringInPlace(new_path, L"%app%", exe_path);

    return new_path;
}

// 压缩HTML
std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}
std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}
std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}
std::vector<std::string> split(const std::string &text, char sep) {
    std::vector<std::string> tokens;
    std::size_t start = 0, end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        std::string temp = text.substr(start, end - start);
        tokens.push_back(temp);
        start = end + 1;
    }
    std::string temp = text.substr(start);
    tokens.push_back(temp);
    return tokens;
}
void compression_html(std::string& html)
{
    auto lines = split(html, '\n');
    html.clear();
    for ( auto &line : lines)
    {
        html += trim(line);
        html += "\n";
    }
}

// 运行外部程序
HANDLE RunExecute(const wchar_t *command, WORD show = SW_SHOWNORMAL, bool admin=false)
{
    std::vector <std::wstring> command_line;

    int nArgs;
    LPWSTR *szArglist = CommandLineToArgvW(command, &nArgs);
    for (int i = 0; i < nArgs; i++)
    {
        command_line.push_back(QuoteSpaceIfNeeded(szArglist[i]));
    }
    LocalFree(szArglist);

    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    ShExecInfo.lpFile = command_line[0].c_str();
    ShExecInfo.nShow = show;
    ShExecInfo.lpVerb = admin ? L"runas" : L"open";

    std::wstring parameter;
    for (size_t i = 1; i < command_line.size(); i++)
    {
        parameter += command_line[i];
        parameter += L" ";
    }
    if ( command_line.size() > 1 )
    {
        ShExecInfo.lpParameters = parameter.c_str();
    }

    if (ShellExecuteEx(&ShExecInfo))
    {
        return ShExecInfo.hProcess;
    }

    return 0;
}


// 加载资源内容
template<typename Function>
bool LoadFromResource(const char *type, const char *name, Function f)
{
    bool result = false;
    HRSRC res = FindResourceA(hInstance, name, type);
    if (res)
    {
        HGLOBAL header = LoadResource(hInstance, res);
        if (header)
        {
            const char *data = (const char*)LockResource(header);
            DWORD size = SizeofResource(hInstance, res);
            if (data)
            {
                f(data, size);
                result = true;
                UnlockResource(header);
            }
        }
        FreeResource(header);
    }

    return result;
}

bool CheckUACPath(const wchar_t *exePath, wchar_t *iniPath)
{
    bool uac = false;

    // 当前目录路径
    wcscpy(iniPath, exePath);
    wcscat(iniPath, L"\\detect");
    FILE *fp = _wfopen(iniPath, L"wb");
    if (fp)
    {
        fclose(fp);
        DeleteFile(iniPath);

        wcscpy(iniPath, exePath);
        wcscat(iniPath, L"\\FireDoge.ini");
    }
    else
    {
        //当前目录无法写入时，写%appdata%
        std::wstring path = ExpandEnvironmentPath(L"%appdata%\\FireDoge.ini");
        wcscpy(iniPath, path.c_str());
        uac = true;
    }

    return uac;
}

// 释放配置文件
void ReleaseIni(const wchar_t *exePath, wchar_t *iniPath)
{
    if (config_path[0])
    {
        wcscpy(iniPath, config_path);
        return;
    }
    // 获取路径
    bool uac = CheckUACPath(exePath, iniPath);
    wcscpy(config_path, iniPath);

    if (PathFileExistsW(iniPath))
    {
        return;
    }

    // 从资源中释放默认配置文件
    LoadFromResource("INI", uac ? "CONFIG_UAC" : "CONFIG", [&](const char *data, DWORD size)
    {
        FILE *fp = _wfopen(iniPath, L"wb");
        if (fp)
        {
            fwrite(data, size, 1, fp);
            fclose(fp);
        }
        else
        {
            DebugLog(L"ReleaseIni failed %s", iniPath);
        }
    });
}

// 搜索内存
uint8_t* memmem(uint8_t* src, int n, const uint8_t* sub, int m)
{
    return (uint8_t*)FastSearch(src, n, sub, m);
}
/*uint8_t* memmem(uint8_t* src, int n, const uint8_t* sub, int m)
{
    if (m > n)
    {
        return NULL;
    }

    int skip[256];
    for (int i = 0; i < 256; i++)
    {
        skip[i] = m;
    }
    for (int i = 0; i < m - 1; i++)
    {
        skip[sub[i]] = m - i - 1;
    }

    int pos = 0;
    while (pos <= n - m)
    {
        int j = m - 1;
        while (j >= 0 && src[pos + j] == sub[j])
        {
            j--;
        }
        if (j < 0)
        {
            return src + pos;
        }
        pos = pos + skip[src[pos + m - 1]];
    }

    return NULL;
}*/

void GetFileVersion(wchar_t *vinfo)
{

    vinfo[0] = '\0';

    HMODULE exe = GetModuleHandle(NULL);

    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), exe, &mi, sizeof(MODULEINFO));

    wchar_t FileVersion[] = L"FileVersion\0";
    int FileVersionLength = sizeof(FileVersion);
    uint8_t* version = memmem((uint8_t*)exe, mi.SizeOfImage, (const uint8_t*)FileVersion, FileVersionLength);
    if (version)
    {
        wcscpy(vinfo, (wchar_t *)(version + FileVersionLength));
    }
}

// 搜索模块，自动加载
uint8_t* SearchModule(const wchar_t *path, const uint8_t* sub, int m)
{
    HMODULE module = LoadLibraryW(path);

    if(!module)
    {
        // dll存在于版本号文件夹中
        wchar_t version[MAX_PATH];
        GetFileVersion(version);
        wcscat(version, L"/");
        wcscat(version, path);

        module = LoadLibraryW(version);
    }

    if(module)
    {
        uint8_t* buffer = (uint8_t*)module;

        PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)(buffer + ((PIMAGE_DOS_HEADER)buffer)->e_lfanew);
        PIMAGE_SECTION_HEADER section = (PIMAGE_SECTION_HEADER)((char*)nt_header + sizeof(DWORD) +
            sizeof(IMAGE_FILE_HEADER) + nt_header->FileHeader.SizeOfOptionalHeader);

        for(int i=0; i<nt_header->FileHeader.NumberOfSections; i++)
        {
            if(strcmp((const char*)section[i].Name,".text")==0)
            {
                return memmem(buffer + section[i].PointerToRawData, section[i].SizeOfRawData, sub, m);
                break;
            }
        }
    }
    return NULL;
}

template<typename String, typename Char, typename Function>
void StringSplit(String *str, Char delim, Function f)
{
    String *ptr = str;
    while (*str)
    {
        if (*str == delim)
        {
            *str = 0;           // 截断字符串

            if (str - ptr)      // 非空字符串
            {
                f(ptr);
            }

            *str = delim;       // 还原字符串
            ptr = str + 1;      // 移动下次结果指针
        }
        str++;
    }

    if (str - ptr)  // 非空字符串
    {
        f(ptr);
    }
}

void SendKeys(const std::wstring &keys)
{
    std::vector <INPUT> inputs;

    wchar_t *temp = _tcsdup(keys.c_str());
    StringSplit(temp, L'+', [&]
    (wchar_t *key)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;

        WORD vk = 0;

        // 解析控制键
        if (_tcsicmp(key, _T("Shift")) == 0) vk = VK_SHIFT;
        else if (_tcsicmp(key, _T("Ctrl")) == 0) vk = VK_CONTROL;
        else if (_tcsicmp(key, _T("Alt")) == 0) vk = VK_MENU;
        else if (_tcsicmp(key, _T("Win")) == 0) vk = VK_LWIN;
        // 解析方向键
        else if (_tcsicmp(key, _T("←")) == 0) vk = VK_LEFT;
        else if (_tcsicmp(key, _T("→")) == 0) vk = VK_RIGHT;
        else if (_tcsicmp(key, _T("↑")) == 0) vk = VK_UP;
        else if (_tcsicmp(key, _T("↓")) == 0) vk = VK_DOWN;
        // 解析单个字符A-Z、0-9等
        else if (_tcslen(key) == 1)
        {
            if (isalnum(key[0]))
            {
                vk = towupper(key[0]);
            }
            else
            {
                vk = VkKeyScan(key[0]);
                BYTE mod = HIBYTE(vk);
                vk = LOBYTE(vk);
                if (mod & 1)
                {
                    INPUT modifier = { 0 };
                    modifier.type = INPUT_KEYBOARD;
                    modifier.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
                    modifier.ki.wVk = VK_LSHIFT;
                    inputs.push_back(modifier);
                }
            }
        }
        // 解析F1-F24功能键
        else if ((key[0] == 'F' || key[0] == 'f') && isdigit(key[1]))
        {
            WORD FX = (WORD)_wtoi(&key[1]);
            if (FX >= 1 && FX <= 24) vk = VK_F1 + FX - 1;
        }
        // 解析其他按键
        else
        {
            if (_tcsicmp(key, _T("Left")) == 0) vk = VK_LEFT;
            else if (_tcsicmp(key, _T("Right")) == 0) vk = VK_RIGHT;
            else if (_tcsicmp(key, _T("Up")) == 0) vk = VK_UP;
            else if (_tcsicmp(key, _T("Down")) == 0) vk = VK_DOWN;

            else if (_tcsicmp(key, _T("Escape")) == 0) vk = VK_ESCAPE;
            else if (_tcsicmp(key, _T("Esc")) == 0) vk = VK_ESCAPE;
            else if (_tcsicmp(key, _T("Tab")) == 0) vk = VK_TAB;

            else if (_tcsicmp(key, _T("Backspace")) == 0) vk = VK_BACK;
            else if (_tcsicmp(key, _T("Enter")) == 0) vk = VK_RETURN;
            else if (_tcsicmp(key, _T("Space")) == 0) vk = VK_SPACE;

            else if (_tcsicmp(key, _T("PrtSc")) == 0) vk = VK_SNAPSHOT;
            else if (_tcsicmp(key, _T("Scroll")) == 0) vk = VK_SCROLL;
            else if (_tcsicmp(key, _T("Pause")) == 0) vk = VK_PAUSE;
            else if (_tcsicmp(key, _T("Break")) == 0) vk = VK_PAUSE;

            else if (_tcsicmp(key, _T("Insert")) == 0) vk = VK_INSERT;
            else if (_tcsicmp(key, _T("Delete")) == 0) vk = VK_DELETE;
            else if (_tcsicmp(key, _T("Del")) == 0) vk = VK_DELETE;

            else if (_tcsicmp(key, _T("End")) == 0) vk = VK_END;
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

            // 音量快捷键
            else if (_tcsicmp(key, _T("VolumeMute")) == 0) vk = VK_VOLUME_MUTE;
            else if (_tcsicmp(key, _T("VolumeDown")) == 0) vk = VK_VOLUME_DOWN;
            else if (_tcsicmp(key, _T("VolumeUp")) == 0) vk = VK_VOLUME_UP;

            // 多媒体快捷键
            else if (_tcsicmp(key, _T("MediaNext")) == 0) vk = VK_MEDIA_NEXT_TRACK;
            else if (_tcsicmp(key, _T("MediaPrev")) == 0) vk = VK_MEDIA_PREV_TRACK;
            else if (_tcsicmp(key, _T("MediaPause")) == 0) vk = VK_MEDIA_PLAY_PAUSE;
        }

        input.ki.wVk = vk;

        inputs.push_back(input);
    });

    free(temp);

    std::vector <INPUT> reverse_inputs = inputs;
    std::reverse(reverse_inputs.begin(), reverse_inputs.end());
    for (auto &input : reverse_inputs)
    {
        input.ki.dwFlags |= KEYEVENTF_KEYUP;
    }
    inputs.insert(inputs.end(), reverse_inputs.begin(), reverse_inputs.end());

    ::SendInput((UINT)inputs.size(), &inputs[0], sizeof(INPUT));
}

// 发送按键
template<typename ... T>
void SendKey(T ... keys)
{
    std::vector <INPUT> inputs;
    std::vector <int> keys_ = { keys ... };
    for (auto & key : keys_)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
        input.ki.wVk = (WORD)key;

        // 修正鼠标消息
        switch (input.ki.wVk)
        {
        case VK_RBUTTON:
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = ::GetSystemMetrics(SM_SWAPBUTTON) == TRUE ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
            break;
        case VK_LBUTTON:
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = ::GetSystemMetrics(SM_SWAPBUTTON) == TRUE ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
            break;
        case VK_MBUTTON:
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
            break;
        }

        inputs.push_back(input);
    }
    std::reverse(keys_.begin(), keys_.end());
    for (auto & key : keys_)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
        input.ki.wVk = (WORD)key;

        // 修正鼠标消息
        switch (input.ki.wVk)
        {
        case VK_RBUTTON:
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = ::GetSystemMetrics(SM_SWAPBUTTON) == TRUE ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
            break;
        case VK_LBUTTON:
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = ::GetSystemMetrics(SM_SWAPBUTTON) == TRUE ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP;
            break;
        case VK_MBUTTON:
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
            break;
        }

        inputs.push_back(input);
    }
    //for (auto & key : inputs)
    //{
    //    DebugLog(L"%X %X", key.ki.wVk, key.mi.dwFlags);
    //}

    ::SendInput((UINT)inputs.size(), &inputs[0], sizeof(INPUT));
}
void SendStr(std::wstring str)
{
    str += L" ";
    for (auto ch : str)
    {
        std::vector <INPUT> inputs;

        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        input.ki.wScan = ch;

        inputs.push_back(input);

        input.ki.dwFlags |= KEYEVENTF_KEYUP;
        inputs.push_back(input);

        ::SendInput((UINT)inputs.size(), &inputs[0], sizeof(INPUT));
    }
    SendKeys(L"\b");
}

//从资源载入图片
bool ImageFromIDResource(const char *name, Image *&pImg)
{
    LoadFromResource("PNG", name, [&](const char *data, DWORD size)
    {
        HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, size);
        BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
        memcpy(pmem, data, size);
        GlobalUnlock(m_hMem);

        IStream* pstm;
        CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);

        pImg = Image::FromStream(pstm);

        pstm->Release();
        GlobalFree(m_hMem);
    });
    return TRUE;
}

bool isEndWith(const wchar_t *s, const wchar_t *sub)
{
    if (!s || !sub) return false;
    size_t len1 = wcslen(s);
    size_t len2 = wcslen(sub);
    if (len2>len1) return false;
    return !_memicmp(s + len1 - len2, sub, len2 * sizeof(wchar_t));
}

bool isStartWith(const wchar_t *s, const wchar_t *sub)
{
    if (!s || !sub) return false;
    size_t len1 = wcslen(s);
    size_t len2 = wcslen(sub);
    if (len2>len1) return false;
    return !_memicmp(s, sub, len2 * sizeof(wchar_t));
}

// auto section2vector(const wchar_t *section)
// {
//     std::vector <std::wstring> list;
//     const wchar_t *str = section;
//     while (str && *str)
//     {
//         list.push_back(str);
//         str += wcslen(str) + 1;
//     }
//     return list;
// }

// auto vector2section(const std::vector <std::wstring> list)
// {
//     std::wstring section;

//     for (auto &str : list)
//     {
//         section.append(str);
//         section.append(L"\0", 1);
//     }
//     section.append(L"\0", 1);
//     return section;
// }

auto GetSection(const wchar_t *name, const wchar_t *iniPath)
{
    std::vector <std::wstring> list;
    FILE * fp = _wfopen(iniPath, L"rb");
    if (fp)
    {
        std::vector <std::wstring> contents;

        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        fseek(fp, 2, SEEK_SET);

        size_t size = (file_size - 2) / sizeof(wchar_t);

        std::wstring text;
        text.resize(size);

        fread(&text[0], sizeof(wchar_t), size, fp);

        std::size_t start = 0;
        while (true)
        {
            std::size_t end = text.find(L"\r\n", start);
            if (end == std::string::npos) break;

            contents.push_back(text.substr(start, end - start));

            start = end + 2;
        }
        contents.push_back(text.substr(start));

        fclose(fp);

        int index = 0;
        int begin = 0;
        int end = (int)contents.size() - 1;

        std::wstring section_name(L"[");
        section_name += name;
        section_name += L"]";
        for (auto &content : contents)
        {
            if (begin == 0)
            {
                if (_wcsicmp(content.c_str(), section_name.c_str()) == 0)
                {
                    begin = index + 1;
                }
            }
            else
            {
                if (content[0] == '[' && content[content.length() - 1] == ']')
                {
                    end = index - 1;
                    break;
                }
            }
            index++;
        }

        if (begin == 0)
        {
            contents.push_back(L"");
            contents.push_back(section_name);
            begin = contents.size();
            contents.push_back(L";");
            contents.push_back(L"");

            end = contents.size() - 1;
        }

        for (int i = begin; i <= end; i++)
        {
            if (contents[i][0] == ';' || contents[i][0] == '#' || contents[i].length() == 0)
            {
                //注释或空行跳过
                continue;
            }
            list.push_back(contents[i]);
        }
    }

    return list;
}

void SetSection(const wchar_t *name, std::vector <std::wstring> &values, const wchar_t *iniPath)
{
    FILE * fp = _wfopen(iniPath, L"rb");
    if (fp)
    {
        std::vector <std::wstring> contents;

        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        fseek(fp, 2, SEEK_SET);

        size_t size = (file_size - 2) / sizeof(wchar_t);

        std::wstring text;
        text.resize(size);

        fread(&text[0], sizeof(wchar_t), size, fp);

        std::size_t start = 0;
        while (true)
        {
            std::size_t end = text.find(L"\r\n", start);
            if (end == std::string::npos) break;

            contents.push_back(text.substr(start, end - start));

            start = end + 2;
        }
        contents.push_back(text.substr(start));

        fclose(fp);

        int index = 0;
        int begin = 0;
        int end = (int)contents.size() - 1;

        std::wstring section_name(L"[");
        section_name += name;
        section_name += L"]";
        for (auto &content : contents)
        {
            if (begin == 0)
            {
                if (_wcsicmp(content.c_str(), section_name.c_str()) == 0)
                {
                    begin = index + 1;
                }
            }
            else
            {
                if (content[0] == '[' && content[content.length() - 1] == ']')
                {
                    end = index - 1;
                    break;
                }
            }
            index++;
        }

        if (begin == 0)
        {
            contents.push_back(L"");
            contents.push_back(section_name);
            begin = contents.size();
            contents.push_back(L";");
            contents.push_back(L"");

            end = contents.size() - 1;
        }

        //  跳过空行
        for (int i = end; i > begin; i--)
        {
            if (contents[i].length()==0)
            {
                end = i - 1;
            }
            else
            {
                break;
            }
        }

        std::map <std::wstring, bool> value_map;
        for (auto &value : values)
        {
            value_map.insert({ value, false });
        }

        std::vector <int> need_delete;
        for (int i = begin; i <= end; i++)
        {
            if (contents[i][0] == ';' || contents[i][0] == '#' || contents[i].length() == 0)
            {
                //注释或空行跳过
            }
            else
            {
                if (value_map.find(contents[i]) != value_map.end())
                {
                    if (value_map[contents[i]]==false)
                    {
                        //尚未找到
                        value_map[contents[i]] = true;
                    }
                    else
                    {
                        //已经有相同的值，删除
                        need_delete.push_back(i);
                    }
                }
                else
                {
                    //删除不存在的值
                    need_delete.push_back(i);
                }
            }
        }

        //删除需要删除的行
        for (auto it = need_delete.rbegin(); it != need_delete.rend(); it++)
        {
            contents.erase(contents.begin() + *it);
            end--;
        }

        //插入尚未找到的内容
        for (auto &value : values)
        {
            if (value_map[value] == false)
            {
                end++;
                contents.insert(contents.begin() + end, value);
            }
        }

        fp = _wfopen(iniPath, L"w, ccs=UNICODE");
        if (fp)
        {
            int write_index = 0;
            for (auto &content : contents)
            {
                write_index++;
                fputws(content.c_str(), fp);

                if (write_index != contents.size())
                {
                    fputws(L"\n", fp);
                }
            }
            fclose(fp);
        }
    }
}

// 获得系统语言
std::string GetDefaultLanguage()
{
    char language[MAX_PATH];
    if (!GetLocaleInfoA(GetUserDefaultUILanguage(), LOCALE_SISO639LANGNAME, language, MAX_PATH))
    {
        return "zh-CN";
    }
    char country[MAX_PATH];
    if (!GetLocaleInfoA(GetUserDefaultUILanguage(), LOCALE_SISO3166CTRYNAME, country, MAX_PATH))
    {
        return std::string(language);
    }

    return std::string(language) + "-" + country;
}

// 编码转换
std::wstring utf8to16(const char* src)
{
    std::vector<wchar_t> buffer;
    buffer.resize(MultiByteToWideChar(CP_UTF8, 0, src, -1, 0, 0));
    MultiByteToWideChar(CP_UTF8, 0, src, -1, &buffer[0], (int)buffer.size());
    return std::wstring(&buffer[0]);
}

// 编码转换
std::string utf16to8(const wchar_t* src)
{
    std::vector<char> buffer;
    buffer.resize(WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL));
    WideCharToMultiByte(CP_UTF8, 0, src, -1, &buffer[0], (int)buffer.size(), NULL, NULL);
    return std::string(&buffer[0]);
}

#define KEY_PRESSED 0x8000
bool IsPressed(int key)
{
    return key && (::GetKeyState(key) & KEY_PRESSED) != 0;
}

bool BrowseToFile(LPCTSTR filename)
{
    ITEMIDLIST *pidl = (ITEMIDLIST*)ILCreateFromPath(filename);
    if (pidl)
    {
        SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
        ILFree(pidl);
        return true;
    }
    return false;
}
