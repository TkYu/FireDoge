// 构造新命令行
std::wstring GetCommand(const wchar_t *iniPath, LPWSTR commandLine)
{
    std::vector <std::wstring> command_line;

    int nArgs;
    LPWSTR *szArglist = CommandLineToArgvW(commandLine, &nArgs);
    for(int i=0; i<nArgs; i++)
    {
        // 保留原来参数
        command_line.push_back( QuoteSpaceIfNeeded(szArglist[i]) );

        if(i==0) //在进程路径后面追加参数
        {
            command_line.push_back(L"-with-firedoge");

            auto contents = GetSection(L"AppendParams", iniPath);
            for (auto &parameter_str : contents)
            {
                std::size_t equal = parameter_str.find(L"=");
                if (equal != std::wstring::npos)
                {
                    // 含有等号
                    std::wstring parameter = parameter_str.substr(0, equal);

                    // 扩展变量
                    std::wstring parameter_path = ExpandPath(parameter_str.substr(equal + 1));

                    // 组合参数
                    command_line.push_back( parameter + L"=" + QuoteSpaceIfNeeded(parameter_path) );
                }
                else
                {
                    //添加到参数
                    command_line.push_back( parameter_str );
                }
            }
        }
    }
    LocalFree(szArglist);

    std::wstring my_command_line;
    for(auto str : command_line)
    {
        my_command_line += str;
        my_command_line += L" ";
    }

    return my_command_line;
}


void LaunchAtStart(const wchar_t *iniPath, std::vector <HANDLE> &program_handles)
{
    auto contents = GetSection(L"RunAtStartup", iniPath);
    for (auto &content : contents)
    {
        // 扩展变量
        std::wstring program = ExpandPath(content);
        
        // 运行程序
        HANDLE program_handle = RunExecute(program.c_str());
        if (program_handle)
        {
            //DebugLog(L"运行程序：%s %ld", program.c_str(), program_handle);
            program_handles.push_back(program_handle);
        }
    }
}

void LaunchUpdater(const wchar_t *iniPath)
{
    if (::GetPrivateProfileIntW(L"检查更新", L"检查更新", 1, iniPath) == 1)
    {
        // 检查更新
        wchar_t updater_path[MAX_PATH];
        GetPrivateProfileString(L"检查更新", L"更新器地址", L"", updater_path, MAX_PATH, iniPath);
        if (updater_path[0])
        {
            // 扩展变量
            std::wstring updater = ExpandPath(updater_path);

            wchar_t check_version[MAX_PATH];
            GetPrivateProfileString(L"检查更新", L"检查版本", L"", check_version, MAX_PATH, iniPath);

            if (check_version[0])
            {
                std::wstring parameters = QuoteSpaceIfNeeded(updater) + L" " + QuoteSpaceIfNeeded(GetExePath()) + L" " + check_version;

                // 运行程序
                RunExecute(parameters.c_str());
            }
        }
    }
}

// 退出时运行额外程序
void LaunchAtEnd(const wchar_t *iniPath)
{
    auto contents = GetSection(L"RunAtEnd", iniPath);
    for (auto &content : contents)
    {
        // 扩展变量
        std::wstring program = ExpandPath(content);

        // 运行程序
        RunExecute(program.c_str(), SW_HIDE);
    }
}

// 结束附加启动程序
void KillAtEnd(const wchar_t *iniPath, std::vector <HANDLE> &program_handles)
{
    if(GetPrivateProfileInt(L"General", L"AutoTerminate", 1, iniPath)==1)
    {
        //DebugLog(L"结束时杀掉启动时运行的程序");
        for(auto &program_handle : program_handles)
        {
            //DebugLog(L"结束id：%ld", program_handle);
            TerminateProcess(program_handle, 0);
        }
    }
}

// 自定义启动参数
void CustomCommand(const wchar_t *iniPath, const wchar_t *exePath, LPWSTR command_line)
{
    std::vector <HANDLE> program_handles;

    HANDLE mutex = CreateMutexW(NULL, TRUE, L"{BC6575EA-D548-405C-AD33-5B80307C4ECE}");
    bool first_run = (GetLastError() == ERROR_SUCCESS);
    if (first_run)
    {
        // 启动更新器
        LaunchUpdater(iniPath);

        // 启动时运行
        LaunchAtStart(iniPath, program_handles);
    }

    // 启动进程
    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(STARTUPINFO);

    // 根据配置文件插入额外的命令行参数
    std::wstring my_command_line = GetCommand(iniPath, command_line);
    
    if (CreateProcessW(exePath, (LPWSTR)my_command_line.c_str(), NULL, NULL, false, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE, NULL, 0, &si, &pi))
    {
        if(first_run)
        {
            //DebugLog(L"Before WaitForSingleObject");
            WaitForSingleObject(pi.hProcess, INFINITE);
            //DebugLog(L"After WaitForSingleObject");
            // 释放句柄
            CloseHandle(mutex);

            // 结束时杀掉启动时运行的程序
            KillAtEnd(iniPath, program_handles);

            // 结束时运行
            LaunchAtEnd(iniPath);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        ExitProcess(0);
    }
    else
    {
        DebugLog(L"CreateProcessW failed:%d", GetLastError());
    }
}