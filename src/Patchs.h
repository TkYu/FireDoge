#pragma once
#pragma region Hook
BOOL WINAPI GetCurrentProcessName(LPWSTR lpstrName, DWORD wlen)
{
    size_t i = 0;
    WCHAR lpFullPath[MAX_PATH + 1] = { 0 };
    if (GetModuleFileNameW(NULL, lpFullPath, MAX_PATH)>0)
    {
        for (i = wcslen(lpFullPath); i>0; i--)
        {
            if (lpFullPath[i] == L'\\')
                break;
        }
        if (i > 0)
        {
            i = _snwprintf(lpstrName, wlen, L"%ls", lpFullPath + i + 1);
        }
    }
    return (i>0);
}
LPWSTR stristrW(LPCWSTR Str, LPCWSTR Pat)
{
    WCHAR *pptr, *sptr, *start;

    for (start = (WCHAR *)Str; *start != '\0'; start++)
    {
        for (; ((*start != '\0') && (toupper(*start) != toupper(*Pat))); start++);
        if ('\0' == *start) return NULL;
        pptr = (WCHAR *)Pat;
        sptr = (WCHAR *)start;
        while (toupper(*sptr) == toupper(*pptr))
        {
            sptr++;
            pptr++;
            if ('\0' == *pptr) return (start);
        }
    }
    return NULL;
}
BOOL WINAPI is_specialdll(UINT_PTR callerAddress, LPCWSTR dll_file)
{
    BOOL	ret = FALSE;
    HMODULE hCallerModule = NULL;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)callerAddress, &hCallerModule))
    {
        WCHAR szModuleName[MAX_PATH + 1] = { 0 };
        if (GetModuleFileNameW(hCallerModule, szModuleName, MAX_PATH))
        {
            if (StrChrW(dll_file, L'*') || StrChrW(dll_file, L'?'))
            {
                if (PathMatchSpecW(szModuleName, dll_file))
                {
                    ret = TRUE;
                }
            }
            else if (stristrW(szModuleName, dll_file))
            {
                ret = TRUE;
            }
        }
    }
    return ret;
}
BOOL is_thunderbird()
{
    WCHAR	process_name[MAX_PATH];
    GetCurrentProcessName(process_name, MAX_PATH);
    return (_wcsicmp(process_name, L"thunderbird.exe") == 0);
}

BOOL is_browser()
{
    WCHAR	process_name[MAX_PATH];
    GetCurrentProcessName(process_name, MAX_PATH);
    return (!(_wcsicmp(process_name, L"Iceweasel.exe") &&
        _wcsicmp(process_name, L"firefox.exe") &&
        _wcsicmp(process_name, L"lawlietfox.exe"))
        );
}

BOOL WINAPI get_mozilla_profile(LPCWSTR app, LPWSTR in_dir, size_t len)
{
    BOOL  sz_name = is_thunderbird();
    in_dir[0] = L'\0';
    if (sz_name)
    {
        _snwprintf(in_dir, len, L"%ls%ls", app, L"\\Thunderbird\\profiles.ini");
    }
    else if (is_browser())
    {
        _snwprintf(in_dir, len, L"%ls%ls", app, L"\\Mozilla\\Firefox\\profiles.ini");
        sz_name = TRUE;
    }
    return sz_name;
}

BOOL DirectoryExists(const wchar_t* dirName) {
    const auto attribs = ::GetFileAttributesW(dirName);
    if (attribs == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return attribs & FILE_ATTRIBUTE_DIRECTORY;
}

void init_global_env(const wchar_t *iniPath)
{
    if (appdata_path[1] != L':')
    {
        wchar_t profileFolder[256];
        GetPrivateProfileStringW(L"General", L"FolderName", L"", profileFolder, MAX_PATH, iniPath);
        if (wcscmp(profileFolder, L"") == 0)wcscpy(profileFolder, L"Profile");

        wcscpy(appdata_path, iniPath);
        PathRemoveFileSpecW(appdata_path);
        //wcscat(appdata_path, L"\\AppData");
        wcscat(appdata_path, L"\\");
        wcscat(appdata_path, profileFolder);
        //DebugLog(L"appdata_path = %s", appdata_path);
        if (!DirectoryExists(appdata_path))
            SHCreateDirectoryExW(NULL, appdata_path, NULL);
        WCHAR profile_orgi[MAX_PATH + 1];
        if (!get_mozilla_profile(appdata_path, profile_orgi, MAX_PATH))
            return;
        if (PathFileExistsW(profile_orgi))
        {
            if (is_thunderbird())
            {
                WritePrivateProfileStringW(L"Profile0", L"Path", L"../", profile_orgi);
            }
            else
            {
                WritePrivateProfileStringW(L"Profile0", L"Path", L"../../", profile_orgi);
            }
        }
        else
        {
            LPWSTR szDir;
            if ((szDir = (LPWSTR)malloc(sizeof(profile_orgi))) != NULL)
            {
                wcsncpy(szDir, profile_orgi, MAX_PATH);
                PathRemoveFileSpecW(szDir);
                SHCreateDirectoryExW(NULL, szDir, NULL);
                free(szDir);
                WritePrivateProfileSectionW(L"General", L"StartWithLastProfile=1\r\n\0", profile_orgi);
                if (is_thunderbird())
                {
                    WritePrivateProfileSectionW(L"Profile0", L"Name=default\r\nIsRelative=1\r\nPath=../\r\nDefault=1\r\n\0", profile_orgi);
                }
                else
                {
                    WritePrivateProfileSectionW(L"Profile0", L"Name=default\r\nIsRelative=1\r\nPath=../../\r\nDefault=1\r\n\0", profile_orgi);
                }
            }
        }
    }

    if (localdata_path[1] != L':')
    {
        //wcscpy(localdata_path, iniPath);
        //PathRemoveFileSpecW(localdata_path);
        wcscpy(localdata_path, appdata_path);
        wcscat(localdata_path, L"\\Cache");
        //DebugLog(L"localdata_path = %s", localdata_path);
        if (!DirectoryExists(localdata_path))
            SHCreateDirectoryExW(NULL, localdata_path, NULL);
    }

}

typedef unsigned nsresult;
/*
nsresult GetString(const char* aSection, const char* aKey,
nsACString& aResult);*/
typedef nsresult(__fastcall *pINIGetString)(void *self, const char* aSection, const char* aKey, void* aResult);

pINIGetString RawINIGetString = NULL;

typedef HRESULT(WINAPI *_NtSHGetFolderPathW)(HWND hwndOwner,
    int nFolder,
    HANDLE hToken,
    DWORD dwFlags,
    LPWSTR pszPath);
typedef HRESULT(WINAPI *_NtSHGetSpecialFolderLocation)(HWND hwndOwner,
    int nFolder,
    LPITEMIDLIST *ppidl);
typedef BOOL(WINAPI *_NtSHGetSpecialFolderPathW)(HWND hwndOwner,
    LPWSTR lpszPath,
    int csidl,
    BOOL fCreate);
typedef HRESULT(WINAPI *_NtSHGetKnownFolderPath)(REFKNOWNFOLDERID rfid,
    DWORD /* KNOWN_FOLDER_FLAG */ dwFlags,
    HANDLE hToken,
    LPWSTR *ppszPath);// free *ppszPath with CoTaskMemFree

static _NtSHGetFolderPathW				RawSHGetFolderPathW = NULL;
static _NtSHGetSpecialFolderLocation	RawSHGetSpecialFolderLocation = NULL;
static _NtSHGetSpecialFolderPathW		RawSHGetSpecialFolderPathW = NULL;
static _NtSHGetKnownFolderPath		    RawSHGetKnownFolderPath = NULL;

HRESULT WINAPI MySHGetSpecialFolderLocation(HWND hwndOwner,
    int nFolder,
    LPITEMIDLIST *ppidl)
{
    int folder = nFolder & 0xff;
    //DebugLog(L"进入 MySHGetSpecialFolderLocation %d", folder);
    if (CSIDL_APPDATA == folder || CSIDL_LOCAL_APPDATA == folder)
    {
        LPITEMIDLIST pidlnew = NULL;
        HRESULT result = S_FALSE;
        switch (folder)
        {
        case CSIDL_APPDATA:
        {
            if (appdata_path[0] != L'\0')
            {
                result = SHILCreateFromPath(appdata_path, &pidlnew, NULL);
            }
            break;
        }
        case CSIDL_LOCAL_APPDATA:
        {
            if (localdata_path[0] != L'\0')
            {
                result = SHILCreateFromPath(localdata_path, &pidlnew, NULL);
            }
            break;
        }
        default:
            break;
        }
        if (result == S_OK)
        {
            *ppidl = pidlnew;
            return result;
        }
    }
    return RawSHGetSpecialFolderLocation(hwndOwner, nFolder, ppidl);
}


HRESULT WINAPI MySHGetFolderPathW(HWND hwndOwner, int nFolder, HANDLE hToken,
    DWORD dwFlags, LPWSTR pszPath)
{
    int			folder = nFolder & 0xff;
    HRESULT     ret = E_FAIL;
    //https://github.com/xunxun1982/pcxfirefox/blob/master/tmemutil-3rd/src/portable.c#L258
    UINT_PTR	dwCaller = (UINT_PTR)_ReturnAddress();
    if(is_specialdll(dwCaller, L"*\\xul.dll") || is_specialdll(dwCaller, L"*\\npswf*.dll"))
    {
        int	 num = 0;
        switch (folder)
        {
        case CSIDL_APPDATA:
        {
            if (PathIsDirectoryW(appdata_path))
            {
                num = _snwprintf(pszPath, MAX_PATH, L"%ls", appdata_path);
                //DebugLog(L"Hook APPDATA %s , num = %d", appdata_path, num);
                ret = S_OK;
            }
            break;
        }
        case CSIDL_LOCAL_APPDATA:
        {
            if (PathIsDirectoryW(localdata_path))
            {
                num = _snwprintf(pszPath, MAX_PATH, L"%ls", localdata_path);
                //DebugLog(L"Hook LOCALDATA %s , num = %d", localdata_path, num);
                ret = S_OK;
            }
            break;
        }
        default:
            break;
        }
    }
    if (S_OK != ret)
    {
        ret = RawSHGetFolderPathW(hwndOwner, nFolder, hToken, dwFlags, pszPath);
    }
    return ret;
}

BOOL WINAPI MySHGetSpecialFolderPathW(HWND hwndOwner, LPWSTR lpszPath, int csidl, BOOL fCreate)
{
    UINT_PTR	dwCaller = (UINT_PTR)_ReturnAddress();
    if (is_specialdll(dwCaller, L"*\\xul.dll") || is_specialdll(dwCaller, L"*\\npswf*.dll"))
    {
        switch (csidl)
        {
        case CSIDL_APPDATA:
            //DebugLog(L"Hook SHGetSpecialFolderPathW APPDATA %s", appdata_path);
            _snwprintf(lpszPath, MAX_PATH, L"%ls", appdata_path);
            return true;
        case CSIDL_LOCAL_APPDATA:
            //DebugLog(L"Hook SHGetSpecialFolderPathW LOCALDATA %s", localdata_path);
            _snwprintf(lpszPath, MAX_PATH, L"%ls", localdata_path);
            return true;
        default:
            break;
        }
    }
    return RawSHGetSpecialFolderPathW(hwndOwner, lpszPath, csidl, fCreate);
}

HRESULT WINAPI MySHGetKnownFolderPath(REFKNOWNFOLDERID rfid,
    DWORD /* KNOWN_FOLDER_FLAG */ dwFlags,
    HANDLE hToken, LPWSTR *ppszPath)
{
    UINT_PTR	dwCaller = (UINT_PTR)_ReturnAddress();
    if (is_specialdll(dwCaller, L"*\\xul.dll") || is_specialdll(dwCaller, L"*\\npswf*.dll"))
    {
        *ppszPath = NULL;
        if (IsEqualGUID(rfid, FOLDERID_RoamingAppData))
        {
            //DebugLog(L"重定向 APPDATA 到 %s", appdata_path);
            *ppszPath = static_cast<LPWSTR>(CoTaskMemAlloc((wcslen(appdata_path) + 1) * sizeof(WCHAR)));
            //_snwprintf(*ppszPath, MAX_PATH, L"%ls", appdata_path);
            if (!*ppszPath)
                return E_OUTOFMEMORY;
            wcscpy(*ppszPath, appdata_path);
            PathRemoveBackslashW(*ppszPath);
            return S_OK;
        }
        if (IsEqualGUID(rfid, FOLDERID_LocalAppData))
        {
            //DebugLog(L"重定向 LOCALDATA 到 %s", localdata_path);
            *ppszPath = static_cast<LPWSTR>(CoTaskMemAlloc((wcslen(localdata_path) + 1) * sizeof(WCHAR)));
            //_snwprintf(*ppszPath, MAX_PATH, L"%ls", localdata_path);
            if (!*ppszPath)
                return E_OUTOFMEMORY;
            wcscpy(*ppszPath, localdata_path);
            PathRemoveBackslashW(*ppszPath);
            return S_OK;
        }
    }
    const auto ret = RawSHGetKnownFolderPath(rfid, dwFlags, hToken, ppszPath);
    if(S_OK == ret)
    {
        //OLECHAR* guidString;
        //StringFromCLSID(rfid, &guidString);
        //DebugLog(L"进入 MySHGetKnownFolderPath ppszPath=%s dwFlags=%d , rfid=%s", *ppszPath, dwFlags, guidString);
        //::CoTaskMemFree(guidString);
    }
    else
    {
        DebugLog(L"RawSHGetKnownFolderPath Failed");
    }
    return ret;
}

nsresult __fastcall MyINIGetString(void *self, const char* aSection, const char* aKey, void* aResult)
{
    DebugLog(L"%S %S", aSection, aKey);
    return RawINIGetString(self, aSection, aKey, aResult);
}

#pragma endregion

void MakePortable(const wchar_t *iniPath)
{
    if (GetPrivateProfileIntW(L"General", L"Portable", 0, iniPath) == 1)
    {
        init_global_env(iniPath);
        HMODULE hShell32 = GetModuleHandleW(L"shell32.dll");
        if (hShell32 == NULL)
        {
            DebugLog(L"Load shell32.dll failed");
            return;
        }

        PBYTE SHGetFolderPathW = (PBYTE)GetProcAddress(hShell32, "SHGetFolderPathW");
        PBYTE SHGetSpecialFolderPathW = (PBYTE)GetProcAddress(hShell32, "SHGetSpecialFolderPathW");
        PBYTE SHGetSpecialFolderLocation = (PBYTE)GetProcAddress(hShell32, "SHGetSpecialFolderLocation");
        PBYTE SHGetKnownFolderPath = (PBYTE)GetProcAddress(hShell32, "SHGetKnownFolderPath");

        //SHGetFolderPathW
        MH_STATUS status = MH_CreateHook(SHGetFolderPathW, MySHGetFolderPathW, (LPVOID*)&RawSHGetFolderPathW);
        if (status == MH_OK)
        {
            MH_EnableHook(SHGetFolderPathW);
        }
        else
        {
            DebugLog(L"MH_CreateHook SHGetFolderPathW failed:%d", status);
        }

        status = MH_CreateHook(SHGetSpecialFolderPathW, MySHGetSpecialFolderPathW, (LPVOID*)&RawSHGetSpecialFolderPathW);
        if (status == MH_OK)
        {
            MH_EnableHook(SHGetSpecialFolderPathW);
        }
        else
        {
            DebugLog(L"MH_CreateHook SHGetSpecialFolderPathW failed:%d", status);
        }

        //貌似新版已经弃用
        status = MH_CreateHook(SHGetSpecialFolderLocation, MySHGetSpecialFolderLocation, (LPVOID*)&RawSHGetSpecialFolderLocation);
        if (status == MH_OK)
        {
            MH_EnableHook(SHGetSpecialFolderLocation);
        }
        else
        {
            DebugLog(L"MH_CreateHook SHGetSpecialFolderLocation failed:%d", status);
        }

        status = MH_CreateHook(SHGetKnownFolderPath, MySHGetKnownFolderPath, (LPVOID*)&RawSHGetKnownFolderPath);
        if (status == MH_OK)
        {
            MH_EnableHook(SHGetKnownFolderPath);
        }
        else
        {
            DebugLog(L"MH_CreateHook SHGetKnownFolderPath failed:%d", status);
        }

    }

    if (GetPrivateProfileIntW(L"General", L"Portable", 0, iniPath) == 10086)
    {
        BYTE search_call[] = { 0xBA, 0x14, 0x00, 0x00, 0x00, 0x41, 0xB8, 0x80, 0x01, 0x00, 0x00 };
        uint8_t *GetString = SearchModule(L"xul.dll", search_call, sizeof(search_call));
        if (GetString && *(GetString - 0x11) == 0xE8)
        {
            int *CALL_OFFSET = (int *)(GetString - 0x11 + 1);
            uint8_t *INIGetString = GetString - 0x11 + *CALL_OFFSET + 5;

            MH_STATUS status = MH_CreateHook(INIGetString, MyINIGetString, (LPVOID*)&RawINIGetString);
            if (status == MH_OK)
            {
                MH_EnableHook(INIGetString);
            }
            else
            {
                DebugLog(L"MH_CreateHook INIGetString failed:%d", status);
            }
        }
    }
}
