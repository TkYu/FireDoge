#include "..\3rd\cjson\cJSON.h"

void ReadList(cJSON *root, const wchar_t *iniPath, const wchar_t *name)
{
    cJSON *list = cJSON_CreateArray();

    auto contents = GetSection(name, iniPath);
    for (auto &content : contents)
    {
        cJSON_AddItemToArray(list, cJSON_CreateString(utf16to8(content.c_str()).c_str()));
    }

    cJSON_AddItemToObject(root, utf16to8(name).c_str(), list);
}

void ReadValue(cJSON *node, const wchar_t *iniPath, const wchar_t *section, const wchar_t *name)
{
    wchar_t value[256];
    GetPrivateProfileStringW(section, name, L"", value, MAX_PATH, iniPath);
    cJSON_AddItemToObject(node, utf16to8(name).c_str(), cJSON_CreateString(utf16to8(value).c_str()));
}

void SendResponse(struct mg_connection *nc, const std::string &text, int code=200)
{
    const char extra_header[] = "Content-Type: application/json; charset=utf-8\r\nAccess-Control-Allow-Origin: *";

    mg_send_head(nc, code, text.length(), extra_header);
    mg_send(nc, text.c_str(), text.length());
}

void SendPlainTextResponse(struct mg_connection *nc, const std::string &text, int code = 200)
{
    const char extra_header[] = "Content-Type: text/plain; charset=utf-8";
    mg_send_head(nc, code, text.length(), extra_header);
    mg_send(nc, text.c_str(), text.length());
}
void BuildMSAATree(cJSON *parentNode, IAccessible* parentAcc)
{
    if (parentAcc)
    {
        TraversalAccessible(parentAcc, [&parentNode]
        (IAccessible* child) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddItemToObject(item, "name", cJSON_CreateString(utf16to8(GetName(child).c_str()).c_str()));
            cJSON_AddItemToObject(item, "role", cJSON_CreateString(utf16to8(GetRole(child).c_str()).c_str()));
            cJSON_AddItemToObject(item, "value", cJSON_CreateString(utf16to8(GetValue(child).c_str()).c_str()));
            cJSON *list = cJSON_CreateArray();
            BuildMSAATree(list, child);
            cJSON_AddItemToObject(item, "child", list);
            cJSON_AddItemToArray(parentNode, item);
            return false;//一直遍历
        });
    }
}

static char* getSettings(const wchar_t *iniPath)
{
    cJSON *root = cJSON_CreateObject();
    ReadList(root, iniPath, L"AppendParams");
    ReadList(root, iniPath, L"RunAtStartup");
    ReadList(root, iniPath, L"RunAtEnd");
    ReadList(root, iniPath, L"KeyTransformationList");

    cJSON *node = cJSON_CreateObject();
    ReadValue(node, iniPath, L"General", L"BossKey");
    /*
    ReadValue(node, iniPath, L"General", L"空白新标签页面");
    ReadValue(node, iniPath, L"General", L"移除更新错误");
    ReadValue(node, iniPath, L"General", L"移除开发者模式警告")
    */;
    ReadValue(node, iniPath, L"General", L"Portable");
    ReadValue(node, iniPath, L"General", L"AutoTerminate");
    cJSON_AddItemToObject(root, utf16to8(L"General").c_str(), node);

    node = cJSON_CreateObject();
    ReadValue(node, iniPath, L"UIEnhancements", L"DoubleClickCloseTab");
    ReadValue(node, iniPath, L"UIEnhancements", L"CloseTabByRightClick");
    ReadValue(node, iniPath, L"UIEnhancements", L"ActivateTabByHover");
    ReadValue(node, iniPath, L"UIEnhancements", L"HoverDelay");
    //ReadValue(node, iniPath, L"UIEnhancements", L"KeepLastTab");
    ReadValue(node, iniPath, L"UIEnhancements", L"SwitchTabByHover");
    ReadValue(node, iniPath, L"UIEnhancements", L"SwitchTabByHoldRightButton");

    /*
    ReadValue(node, iniPath, L"UIEnhancements", L"新标签打开书签");
    ReadValue(node, iniPath, L"UIEnhancements", L"新标签打开网址");
    ReadValue(node, iniPath, L"UIEnhancements", L"新标签页不生效");
    ReadValue(node, iniPath, L"UIEnhancements", L"前台打开新标签");
    ReadValue(node, iniPath, L"UIEnhancements", L"处理弹出菜单");
    */

    ReadValue(node, iniPath, L"UIEnhancements", L"DPI");

    cJSON_AddItemToObject(root, utf16to8(L"UIEnhancements").c_str(), node);

    node = cJSON_CreateObject();
    ReadValue(node, iniPath, L"KeyTransformation", L"Enabled");
    ReadValue(node, iniPath, L"KeyTransformation", L"DuplicateKeyAccepted");
    ReadValue(node, iniPath, L"KeyTransformation", L"IgnoreAtEditBox");
    cJSON_AddItemToObject(root, utf16to8(L"KeyTransformation").c_str(), node);

    /*
    node = cJSON_CreateObject();
    ReadValue(node, iniPath, L"hosts", L"Enabled");
    ReadValue(node, iniPath, L"hosts", L"保存位置");
    ReadValue(node, iniPath, L"hosts", L"下载地址");
    bool status = NeedUpdate(iniPath);
    cJSON_AddItemToObject(node, utf16to8(L"状态").c_str(), cJSON_CreateString(utf16to8(status?L"1":L"0").c_str()));
    cJSON_AddItemToObject(root, utf16to8(L"hosts").c_str(), node);
    */

    node = cJSON_CreateObject();
    ReadValue(node, iniPath, L"Update", L"CheckFD");
    ReadValue(node, iniPath, L"Update", L"CheckFF");
    ReadValue(node, iniPath, L"Update", L"FFBranch");
    cJSON_AddItemToObject(root, utf16to8(L"Update").c_str(), node);


    node = cJSON_CreateObject();
    ReadValue(node, iniPath, L"Gesture", L"Enabled");
    ReadValue(node, iniPath, L"Gesture", L"Colour");
    ReadValue(node, iniPath, L"Gesture", L"ShowResult");
    ReadValue(node, iniPath, L"Gesture", L"ShowTrail");
    ReadValue(node, iniPath, L"Gesture", L"ShowArrow");
    ReadValue(node, iniPath, L"Gesture", L"Weight");
    ReadValue(node, iniPath, L"Gesture", L"TrainMode");
    ReadValue(node, iniPath, L"Gesture", L"Threshold");
    cJSON_AddItemToObject(root, utf16to8(L"Gesture").c_str(), node);

    cJSON_AddItemToObject(root, utf16to8(L"version").c_str(), cJSON_CreateString(RELEASE_VER_STR));

    cJSON_AddItemToObject(root, "chcp", cJSON_CreateNumber(ACP));
    wchar_t current_version[20] = { 0 };
    GetFileVersion(current_version);
    cJSON_AddItemToObject(root, "cversion", cJSON_CreateString(utf16to8(current_version).c_str()));
    cJSON_AddItemToObject(root, "arch", cJSON_CreateBool(IsX64));

    const auto ret = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return ret;
}

static bool http_get(struct mg_connection *nc, struct http_message *hm)
{
    if (mg_vcmp(&hm->uri, "/get_currenturl") == 0)
    {
        SendPlainTextResponse(nc, utf16to8(GetCurrentURL().c_str()).c_str());
    }
    else if (mg_vcmp(&hm->uri, "/get_setting") == 0)
    {

        const auto inipath = static_cast<const wchar_t *>(nc->mgr->user_data);
        const auto str = getSettings(inipath);
        if (str)
        {
            SendResponse(nc, str);
            free(str);
        }
    }
    else if (mg_vcmp(&hm->uri, "/get_diagnostics") == 0)
    {
        cJSON *root = cJSON_CreateObject();

        #pragma region 系统版本及架构
        OSVERSIONINFOEX info;
        ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
        info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&info));
        auto systemVersion = std::string(std::to_string(info.dwMajorVersion));
        systemVersion.append(".");
        systemVersion.append(std::to_string(info.dwMinorVersion));
        systemVersion.append(".");
        systemVersion.append(std::to_string(info.dwBuildNumber));
        cJSON_AddItemToObject(root, "winver", cJSON_CreateString(systemVersion.c_str()));

        //是否x64
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        cJSON_AddItemToObject(root, "arch", cJSON_CreateNumber(si.wProcessorArchitecture));
        //if ((si.wProcessorArchitecture & PROCESSOR_ARCHITECTURE_IA64) || (si.wProcessorArchitecture & PROCESSOR_ARCHITECTURE_AMD64) == 64)
        //    return 1;
        cJSON_AddItemToObject(root, "x64", cJSON_CreateBool(IsX64));
        #pragma endregion

        #pragma region 系统语言环境
        //当前acp
        cJSON_AddItemToObject(root, "chcp", cJSON_CreateNumber(ACP));

        //当前语言
        WCHAR szISOLang[5] = { 0 };
        WCHAR szISOCountry[5] = { 0 };
        WCHAR szLocal[10] = { 0 };
        ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, szISOLang, sizeof szISOLang / sizeof(WCHAR));
        ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, szISOCountry, sizeof szISOCountry / sizeof(WCHAR));
        wcscpy(szLocal, szISOLang);
        wcscat(szLocal, szISOCountry);
        cJSON_AddItemToObject(root, "locale", cJSON_CreateString(utf16to8(szLocal).c_str()));
        wchar_t current_version[20] = { 0 };
        GetFileVersion(current_version);
        cJSON_AddItemToObject(root, "version", cJSON_CreateString(utf16to8(current_version).c_str()));
        #pragma endregion

        #pragma region DPI相关
        cJSON_AddItemToObject(root, "current_w", cJSON_CreateNumber(GetSystemMetrics(SM_CXSCREEN)));
        cJSON_AddItemToObject(root, "current_h", cJSON_CreateNumber(GetSystemMetrics(SM_CYSCREEN)));
        if (info.dwMajorVersion >= 6 && info.dwMinorVersion >= 3)
        {
            HKEY hKey;
            if (::RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop\\WindowMetrics", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
            {
                DWORD AppliedDPI = 0;
                DWORD dwSize = sizeof(DWORD);
                if (::RegQueryValueEx(hKey, L"AppliedDPI", NULL, NULL, (LPBYTE)&AppliedDPI, &dwSize) == ERROR_SUCCESS)
                {
                    auto dpi = (float)AppliedDPI / 96.0f;
                    cJSON_AddItemToObject(root, "scale_factor", cJSON_CreateNumber(dpi));
                    ::RegCloseKey(hKey);
                }
                ::RegCloseKey(hKey);
            }
        }
        else if (info.dwMajorVersion >= 6 && info.dwMinorVersion < 3)
        {
            HINSTANCE hDLL = LoadLibrary(L"User32.dll");
            if (hDLL != NULL)
            {
                typedef BOOL(*SetProcessDPIAwareD)();
                SetProcessDPIAwareD func = reinterpret_cast<SetProcessDPIAwareD>(GetProcAddress(hDLL, "SetProcessDPIAware"));
                func();
                FreeLibrary(hDLL);
                HDC screen = GetDC(NULL);
                int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
                //int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
                auto dpi = (float)dpiX / 96.0f;
                cJSON_AddItemToObject(root, "scale_factor", cJSON_CreateNumber(dpi));
                ReleaseDC(NULL, screen);
            }
        }
        else
        {
            cJSON_AddItemToObject(root, "scale_factor", cJSON_CreateNumber(1));
        }
        //设置的dpi
        cJSON_AddItemToObject(root, "current_dpi", cJSON_CreateNumber(static_cast<float>(DPI) / 100));
        #pragma endregion

        #pragma region MSAA树
        cJSON *list = cJSON_CreateArray();
        const auto hwnd = FindWindowByProcessIdAndClassName(GetCurrentProcessId(), static_cast<LPWSTR>(L"MozillaWindowClass"));
        if (hwnd)
        {
            IAccessible *paccMainWindow = NULL;
            if (S_OK == AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_IAccessible, reinterpret_cast<void**>(&paccMainWindow)))
            {
                BuildMSAATree(list, paccMainWindow);
                if (paccMainWindow)paccMainWindow->Release();
            }
        }
        cJSON_AddItemToObject(root, "msaa_tree", list);
        #pragma endregion

        const auto str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        SendResponse(nc, str);
        free(str);
    }
    else
    {
        SendPlainTextResponse(nc, "work like a charm");
        //return false;
    }

    return true;
}

static bool http_post(struct mg_connection *nc, struct http_message *hm)
{
    const wchar_t *iniPath = (const wchar_t *)nc->mgr->user_data;

    if (mg_vcmp(&hm->uri, "/get_setting") == 0)
    {
        const auto str = getSettings(iniPath);
        if (str)
        {
            SendResponse(nc, str);
            free(str);
        }
    }
    else if (mg_vcmp(&hm->uri, "/set_setting") == 0)
    {
        char section[200];
        char name[200];
        char value[200];
        mg_get_http_var(&hm->body, "section", section, sizeof(section));
        mg_get_http_var(&hm->body, "name", name, sizeof(name));
        mg_get_http_var(&hm->body, "value", value, sizeof(value));
        //DebugLog(L"set_setting %s %s=%s", utf8to16(section).c_str(), utf8to16(name).c_str(), utf8to16(value).c_str());
        if (!::WritePrivateProfileString(utf8to16(section).c_str(), utf8to16(name).c_str(), utf8to16(value).c_str(), iniPath))
        {
            return false;
        }
        ReadConfig(iniPath);
        //CheckDownload(iniPath);
        SendResponse(nc, "{}");
    }
    else if (mg_vcmp(&hm->uri, "/del_setting") == 0)
    {
        char section[200];
        char name[200];
        mg_get_http_var(&hm->body, "section", section, sizeof(section));
        mg_get_http_var(&hm->body, "name", name, sizeof(name));
        //DebugLog(L"del_setting %s %s=%s", utf8to16(section).c_str(), utf8to16(name).c_str());

        if (!::WritePrivateProfileString(utf8to16(section).c_str(), utf8to16(name).c_str(), NULL, iniPath))
        {
            return false;
        }
        SendResponse(nc, "{}");
    }
    else if (mg_vcmp(&hm->uri, "/add_section") == 0)
    {
        char section[200];
        char value[200];
        mg_get_http_var(&hm->body, "section", section, sizeof(section));
        mg_get_http_var(&hm->body, "value", value, sizeof(value));

        auto contents = GetSection(utf8to16(section).c_str(), iniPath);
        contents.push_back(utf8to16(value));
        SetSection(utf8to16(section).c_str(), contents, iniPath);

        SendResponse(nc, "{}");
    }
    else if (mg_vcmp(&hm->uri, "/del_section") == 0)
    {
        char section[200];
        char value[200];
        mg_get_http_var(&hm->body, "section", section, sizeof(section));
        mg_get_http_var(&hm->body, "value", value, sizeof(value));

        auto contents = GetSection(utf8to16(section).c_str(), iniPath);

        int index = 0;
        for (auto &content : contents)
        {
            if (_wcsicmp(content.c_str(), utf8to16(value).c_str()) == 0)
            {
                contents.erase(contents.begin() + index);
                SetSection(utf8to16(section).c_str(), contents, iniPath);
                break;
            }
            index++;
        }

        SendResponse(nc, "{}");
    }
    else if (mg_vcmp(&hm->uri, "/get_gestures") == 0)
    {
        cJSON *root = cJSON_CreateArray();
        gesture_point.init_config(iniPath);
        const auto &gesture_list = gesture_point.GetList();

        for (const auto &gesture : gesture_list)
        {
            cJSON *node = cJSON_CreateObject();

            cJSON_AddItemToObject(node, utf16to8(L"name").c_str(), cJSON_CreateString(utf16to8(gesture.name.c_str()).c_str()));
            cJSON_AddItemToObject(node, utf16to8(L"img").c_str(), cJSON_CreateString(gesture.png.c_str()));

            ReadValue(node, iniPath, L"GestureList", gesture.name.c_str());

            cJSON_AddItemToArray(root, node);
        }

        char *str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);

        SendResponse(nc, str);
        free(str);
    }
    else if (mg_vcmp(&hm->uri, "/del_gesture") == 0)
    {
        char name[200];
        mg_get_http_var(&hm->body, "name", name, sizeof(name));
        //DebugLog(L"del_setting %s %s=%s", utf8to16(section).c_str(), utf8to16(name).c_str());

        if (!::WritePrivateProfileString(L"GestureList", utf8to16(name).c_str(), NULL, iniPath))
        {
            return false;
        }

        gesture_point.Del(utf8to16(name));
        SendResponse(nc, "{}");
    }
    else if (mg_vcmp(&hm->uri, "/add_gesture") == 0)
    {
        if (!MouseGesture || AddGestureFlag) return false;

        AddGestureFlag = true;

        SendResponse(nc, "{}");
    }
     else if (mg_vcmp(&hm->uri, "/restart") == 0)
     {
         std::thread th([](std::wstring option) {
             Sleep(50);
             SendKeys(L"Shift+F2");
             Sleep(50);
             SendStr(option);
             Sleep(50);
             SendKeys(L"Enter");
         }, L"restart");
         th.detach();
         SendResponse(nc, "{}");
     }
    else if (mg_vcmp(&hm->uri, "/openconfig") == 0)
    {
        if (!BrowseToFile(iniPath))
        {
            return false;
        }
        SendResponse(nc, "{}");
    }
    else if (mg_vcmp(&hm->uri, "/update") == 0)
    {
        auto cmd = std::string(R"(cmd /c @SET "fdupdateCLV=)");
        #pragma region 读取变量
        //char lv[20];
        //char ov[20];
        char clv[20];
        char cov[20];
        char lfd[MAX_PATH];
        char sfd[41];
        //mg_get_http_var(&hm->body, "lv", lv, sizeof lv);
        //mg_get_http_var(&hm->body, "ov", ov, sizeof ov);
        mg_get_http_var(&hm->body, "clv", clv, sizeof clv);
        mg_get_http_var(&hm->body, "cov", cov, sizeof cov);
        mg_get_http_var(&hm->body, "lfd", lfd, sizeof lfd);
        mg_get_http_var(&hm->body, "sfd", sfd, sizeof sfd);

        cmd.append(clv);
        cmd.append(R"(" && @SET "fdupdateCOV=)");
        cmd.append(cov);
        cmd.append(R"(" && @SET "fdupdateLFD=)");
        cmd.append(lfd);
        cmd.append(R"(" && @SET "fdupdateSFD=)");
        cmd.append(sfd);
        cmd.append(R"(" && @SET "fdupdatepath={FILENAME}" && @"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -InputFormat None -ExecutionPolicy AllSigned -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://static.pzhacm.org/shuax/static/fd.ps1'))"")");
        //cmd.append(R"(" && @SET "fdupdatepath={FILENAME}" && @"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -InputFormat None -ExecutionPolicy AllSigned -Command "iex ((New-Object System.Net.WebClient).DownloadString('http://localhost/MyPSScript/MyPSScript/fd.ps1'))"")");
        #pragma endregion
        auto cmdWide = utf8to16(cmd.c_str());
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        ReplaceStringInPlace(cmdWide, L"{FILENAME}", exePath);
        //DebugLog(L"xx : %s", cmdWide.c_str());
        RunExecute(cmdWide.c_str(), SW_SHOWNORMAL, true);
        SendResponse(nc, "{}");
    }
    else
    {
        return false;
    }

    return true;
}

void not_found(struct mg_connection *nc)
{
    SendResponse(nc, "Not Found", 404);
}
void server_error(struct mg_connection *nc)
{
    SendResponse(nc, "Server Error", 500);
}

static void http_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    switch (ev)
    {
    case MG_EV_HTTP_REQUEST:
    {
        struct http_message *hm = (struct http_message *) ev_data;
        //DebugLog(L"%.*S %.*S %.*S", (int)hm->method.len, hm->method.p, (int)hm->uri.len, hm->uri.p, (int)hm->body.len, hm->body.p);

        if (mg_vcmp(&hm->method, "POST") == 0)
        {
            if (!http_post(nc, hm))
            {
                server_error(nc);
            }
        }
        else if (mg_vcmp(&hm->method, "GET") == 0)
        {
            if (!http_get(nc, hm))
            {
                server_error(nc);
            }
        }
        if (mg_vcmp(&hm->method, "OPTIONS") == 0)
        {
            const char extra_header[] = "Content-Type: application/json; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Content-Type";
            mg_send_head(nc, 200, 0, extra_header);
            mg_send(nc, "", 0);
        }
        else
        {
            //404
            not_found(nc);
        }
        nc->flags |= MG_F_SEND_AND_CLOSE;
    }
    break;
    default:
        break;
    }
}

void WebThread(const std::wstring iniPath)
{
    Sleep(1000);

    ::CreateMutexW(NULL, TRUE, L"{4ADDA7C7-4F4F-4C9B-AB02-7D8EC921F130}");
    if (GetLastError() != ERROR_SUCCESS)
    {
        return;
    }

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    struct mg_mgr mgr;
    struct mg_connection *nc;
    mg_mgr_init(&mgr, (void *)iniPath.c_str());

    for (int http_port = 20000; http_port < 20010; http_port++)
    {
        char http_address[1024];
        wsprintfA(http_address, "127.0.0.1:%d", http_port);
        if (nc = mg_bind(&mgr, http_address, http_handler))
        {
            break;
        }
    }

    if (nc)
    {
        mg_set_protocol_http_websocket(nc);
        for (;;) {
            mg_mgr_poll(&mgr, 10);
        }
    }
    else
    {
        DebugLog(L"WebThread failed");
    }
    mg_mgr_free(&mgr);
}

void SettingWeb(const wchar_t *iniPath)
{
    if (!StopWeb)
    {
        std::thread th(WebThread, iniPath);
        th.detach();
    }
}
