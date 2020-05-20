#include "FireDoge.h"

void FireDoge(LPWSTR command_line)
{
    // exe路径
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    // exe所在文件夹
    wchar_t exeFolder[MAX_PATH];
    wcscpy(exeFolder, exePath);
    PathRemoveFileSpecW(exeFolder);

    // 生成默认ini文件
    wchar_t iniPath[MAX_PATH];
    ReleaseIni(exeFolder, iniPath);

    // 读取配置
    ReadConfig(iniPath);

    // 父进程不是FF的话，则需要启动追加参数功能
 //   bool with_firedoge = wcsstr(command_line, L"-with-firedoge") != 0;
 //   if (!with_firedoge)
 //   {
 //       //DebugLog(L"with_firedoge %d %s", with_firedoge, command_line);
 //       CustomCommand(iniPath, exePath, command_line);
 //   }
    //else
    {
        if (BossID == 0)
        {
            BossID = ::GetCurrentProcessId();
            //DebugLog(L"大哥进来了，大哥id %ld", BossID);
            CustomCommand(iniPath, exePath, command_line);
        }
        //else
        //{
        //    DebugLog(L"子进程启动，id %ld", ::GetCurrentProcessId());
        //}
        // 打造便携
        MakePortable(iniPath);

        // 标签页，书签，地址栏增强
        TabBookmark();

        // 启动设置页面
        SettingWeb(iniPath);

        // 启动老板键
        Bosskey(iniPath);
    }
}

EXTERNC BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID pv)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        //if (BossID == 0)
        //{
        //    BossID = ::GetCurrentProcessId();
        //    DebugLog(L"大哥进来了，大哥id %ld", BossID);
        //}
        //else
        //{
        //    DebugLog(L"子进程启动，id %ld", ::GetCurrentProcessId());
        //}
        hInstance = hModule;

        // 保持系统dll原有功能
        LoadSysDll(hModule);

        // 初始化HOOK库成功以后安装加载器
        MH_STATUS status = MH_Initialize();
        if (status == MH_OK)
        {
            InstallLoader();
        }
        else
        {
            DebugLog(L"MH_Initialize failed:%d", status);
        }
    }
    //if (dwReason == DLL_PROCESS_DETACH)
    //{
    //    const auto current = ::GetCurrentProcessId();
    //    if(BossID == current)
    //    {
    //        DebugLog(L"大哥结束，id %ld", current);
    //    }
    //    else
    //    {
    //        DebugLog(L"子进程结束，id %ld", current);
    //    }
    //}
    return TRUE;
}
