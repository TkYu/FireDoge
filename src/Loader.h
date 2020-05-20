
typedef int (*Startup) ();
Startup FirefoxMain = NULL;

//
int Loader()
{
    LPWSTR command_line = GetCommandLineW();
	//DebugLog(L"Loader %s", command_line);

	FireDoge(command_line);

    //返回到FF
    return FirefoxMain();
}

void InstallLoader()
{
    //获取程序入口点
    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &mi, sizeof(MODULEINFO));
    PBYTE entry = (PBYTE)mi.EntryPoint;

    // 入口点跳转到Loader
    MH_STATUS status = MH_CreateHook(entry, Loader, (LPVOID*)&FirefoxMain);
    if (status == MH_OK)
    {
        MH_EnableHook(entry);
    }
    else
    {
        DebugLog(L"MH_CreateHook InstallLoader failed:%d", status);
    }
}
