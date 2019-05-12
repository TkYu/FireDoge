#include "LayeredWindowUtils.h"

#define WM_USER_HWND (WM_USER + 1)
#define WM_USER_SHOW (WM_USER + 2)
#define WM_USER_UPDATE (WM_USER + 3)
#define WM_USER_END (WM_USER + 4)

class GestureWindow :
    public ATL::CWindowImpl<GestureWindow>,
    public LayeredWindowUtils<GestureWindow>
{
public:
    DECLARE_WND_CLASS(L"FireDogeGesture")

    BEGIN_MSG_MAP(GestureWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_TIMER(OnTimer)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_USER_HWND, OnUserHwnd)
        MESSAGE_HANDLER(WM_USER_SHOW, OnUserShow)
        MESSAGE_HANDLER(WM_USER_UPDATE, Update)
        MESSAGE_HANDLER(WM_USER_END, End)
    END_MSG_MAP()

    byte my_color(byte c, int delta)
    {
        int color = (int)c + delta;
        if (color>255) color = 255;
        if (color<0) color = 0;
        return color;
    }

    void DrawGestureTrack(Graphics &graphics)
    {
        auto points = gesture_point.get_points();
        if (points.size()<2) return;

        std::vector<Point> pnts;
        point_t last_point = { -100, -100 };
        for (auto &point : points)
        {
            if (GetDistance(last_point, point)<3.5f) continue;
            last_point = point;
            POINT pt = { (int)point.x, (int)point.y };
            ScreenToClient(&pt);
            pnts.push_back({ pt.x, pt.y });
        }
        if (pnts.size()<2) return;

        INT minimum_x = pnts[0].X;
        INT minimum_y = pnts[0].Y;
        INT maximum_x = pnts[0].X;
        INT maximum_y = pnts[0].Y;
        for (size_t i = 1; i < pnts.size(); i++)
        {
            if (pnts[i].X < minimum_x)
                minimum_x = pnts[i].X;
            if (pnts[i].Y < minimum_y)
                minimum_y = pnts[i].Y;
            if (pnts[i].X > maximum_x)
                maximum_x = pnts[i].X;
            if (pnts[i].Y > maximum_y)
                maximum_y = pnts[i].Y;
        }

        int adjust = max(thick, 5);
        minimum_x -= adjust;
        minimum_y -= adjust;
        maximum_x += adjust;
        maximum_y += adjust;

        LinearGradientBrush gradient_brush(Rect(minimum_x, minimum_y, maximum_x - minimum_x, maximum_y - minimum_y),
            Color(a, my_color(r, 30), my_color(g, 30), my_color(b, 30)),
            Color(a, my_color(r, -10), my_color(g, -10), my_color(b, -10)),
            LinearGradientModeForwardDiagonal);

        //graphics.FillRectangle(&gradient_brush, minimum_x, minimum_y, maximum_x - minimum_x, maximum_y - minimum_y);
        Pen pen(Color(a, r, g, b), (Gdiplus::REAL)thick);
        pen.SetBrush(&gradient_brush);

        if (show_arrow)
        {
            pen.SetStartCap(Gdiplus::LineCapRoundAnchor);
            //pen.SetEndCap(Gdiplus::LineCapArrowAnchor);

            AdjustableArrowCap cap(4, 3, true);
            cap.SetBaseCap(Gdiplus::LineCapSquare);
            pen.SetCustomEndCap(&cap);
        }
        pen.SetLineJoin(LineJoinBevel);


        graphics.DrawLines(&pen, &pnts[0], (INT)pnts.size());
    }

    void DrawGestureTips(Graphics &graphics)
    {
        std::wstring command = gesture_point.result();

        RECT rect;
        ::GetWindowRect(main_hwnd, &rect);

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        std::wstring command_name = GetGestureName(command);

        FontFamily fontFamily(font_name.c_str());

        StringFormat stringformat;
        stringformat.SetAlignment(StringAlignmentCenter);
        stringformat.SetLineAlignment(StringAlignmentCenter);

        GraphicsPath path;
        path.AddString(command_name.c_str(), command_name.length(), &fontFamily, FontStyleRegular, 28, Gdiplus::Point(width / 2, height - 100), &stringformat);

        for (int i = 1; i<8; ++i)
        {
            Pen pen(Color(a * 100 / 300, r, g, b), (Gdiplus::REAL)i);
            pen.SetLineJoin(LineJoinRound);
            graphics.DrawPath(&pen, &path);
        }

        SolidBrush brush(Color(255, 255, 255));
        graphics.FillPath(&brush, &path);
    }

    void DoLayeredPaint(WTL::CDCHandle dc, RECT rcclient)
    {
        Graphics graphics(dc);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        //graphics.Clear(0x99345678);

        if (show_track)
        {
            DrawGestureTrack(graphics);
        }
        if (show_operate)
        {
            DrawGestureTips(graphics);
        }
    }

    LRESULT OnUserHwnd(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        main_hwnd = GetTopWnd((HWND)wParam);

        POINT *pt = (POINT *)lParam;
        gesture_point.init(pt->x, pt->y);
        return 0;
    }

    LRESULT OnUserShow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        if (GetForegroundWindow() != main_hwnd)
        {
            SetForegroundWindow(main_hwnd);
        }
        ReadConfig();

        RECT rect;
        ::GetWindowRect(main_hwnd, &rect);
        ::SetWindowPos(m_hWnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER);

        DoUpdateWindow();

        ShowWindow(SW_SHOW);
        return 0;
    }

    LRESULT Update(UINT /*uMsg*/, WPARAM x, LPARAM y, BOOL& /*bHandled*/)
    {
        gesture_point.move((int)x, (int)y);
        drawing = true;
        return 0;
    }
    void OnTimer(UINT_PTR nTimeId)
    {
        if (drawing)
        {
            drawing = false;
            DoUpdateWindow();
        }
    }

    LRESULT End(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        drawing = false;
        std::wstring command = gesture_point.result();
        ShowWindow(SW_HIDE);

        DoAction(command);
        return 0;
    }

private:
    std::wstring inVaild = Lang(L"InVailed Gesture");
    std::wstring addNew = Lang(L"Add New Gesture");
    std::wstring unKnow = Lang(L"UnConfigured Gesture");

    static void DelaySendkeys(std::wstring action)
    {
        Sleep(100);
        SendKeys(action);
    }
    HWND GetTopWnd(HWND hwnd)
    {
        while (::GetParent(hwnd) && ::IsWindowVisible(::GetParent(hwnd)))
        {
            hwnd = ::GetParent(hwnd);
        }
        return hwnd;
    }
    LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        LOGFONT lf = { 0 };
        NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, false);
        font_name = ncm.lfMessageFont.lfFaceName;

        // exe路径
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);

        // exe所在文件夹
        wchar_t exeFolder[MAX_PATH];
        wcscpy(exeFolder, exePath);
        PathRemoveFileSpecW(exeFolder);

        // 获得默认ini文件
        ReleaseIni(exeFolder, ini_path);

        gesture_point.init_config(ini_path);

        addwindow.Create(NULL, CRect(0, 0, 500, 348), L"", WS_OVERLAPPED | WS_POPUP | WS_CAPTION | WS_SYSMENU, WS_EX_TOOLWINDOW | WS_EX_TOPMOST);

        SetTimer(0, 1000 / 40);
        drawing = false;
        return 0;
    }

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        ::PostQuitMessage(0);
        return 0;
    }

    void WindowAction(std::wstring &option)
    {
        if (option == L"0")
        {
            //最小化
            ::ShowWindow(main_hwnd, SW_MINIMIZE);
        }
        else if (option == L"1")
        {
            if (::IsZoomed(main_hwnd))
            {
                //还原
                ::ShowWindow(main_hwnd, SW_RESTORE);
            }
            else
            {
                //最大化
                ::ShowWindow(main_hwnd, SW_MAXIMIZE);
            }
        }
        else if (option == L"2")
        {
            if (::GetWindowLong(main_hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
            {
                //取消置顶
                ::SetWindowPos(main_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
            else
            {
                //置顶
                ::SetWindowPos(main_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
        }
    }
    void Dispatch(std::wstring &action, std::wstring &option)
    {
        if (action == L"SendKeys")
        {
            std::thread th(DelaySendkeys, option);
            th.detach();
        }
        else if (action == L"OpenUrl")
        {
            if (option.find(L"about:config") == 0)
            {
                std::thread th([](std::wstring option) {
                    Sleep(50);
                    SendKeys(L"Ctrl+L");
                    Sleep(50);
                    SendStr(option);
                    Sleep(50);
                    SendKeys(L"Enter");
                }, option);
                th.detach();
            }
            else
            {
                std::thread th([](std::wstring option) {

                    wchar_t exePath[MAX_PATH];
                    GetModuleFileNameW(NULL, exePath, MAX_PATH);

                    std::wstring command_line = QuoteSpaceIfNeeded(exePath);
                    command_line += L" ";
                    command_line += QuoteSpaceIfNeeded(ExpandCommand(option));
                    RunExecute(command_line.c_str());
                }, option);
                th.detach();
            }
        }
        else if (action == L"Window")
        {
            WindowAction(option);
        }
        else if (action == L"RunApp")
        {
            std::thread th([](std::wstring option) {
                RunExecute(ExpandCommand(option).c_str());
            }, option);
            th.detach();
        }
        else if (action == L"CopyToClipboard")
        {
            std::thread th([](std::wstring option) {
                CopyToClipboard(NULL, ExpandCommand(option));
            }, option);
            th.detach();
        }
    }

    void DoAction(std::wstring &command)
    {
        wchar_t buffer[1024];
        GetPrivateProfileStringW(L"GestureList", command.c_str(), L"", buffer, 1024, ini_path);
        if (!AddGestureFlag && buffer[0])
        {
            std::wstring config = buffer;
            std::size_t action = config.find(L"|");
            if (action != std::wstring::npos)
            {
                std::size_t option = config.find(L"|", action + 1);
                if (option != std::wstring::npos)
                {
                    std::wstring actions = config.substr(action + 1, option - action - 1);
                    std::wstring options = config.substr(option + 1);
                    Dispatch(actions, options);
                    return;
                }
            }
        }

        if (AddGestureFlag || training_mode)
        {
            if (gesture_point.get_points().size() > 1)
            {
                addwindow.RefreshThumbnail(ini_path);
                addwindow.Invalidate();
                addwindow.ShowWindow(SW_SHOW);
                SetForegroundWindow(addwindow);
            }
            AddGestureFlag = false;
        }
        else
        {
            if (command.empty()) return;

            //MessageBox(L"配置文件解析错误");
        }
    }

    std::wstring GetGestureName(std::wstring &command)
    {
        if (command.empty() || AddGestureFlag)
        {
            if (!AddGestureFlag && (!training_mode || gesture_point.get_points().size() <= 1))
            {
                return inVaild;
            }
            return addNew;
        }

        wchar_t buffer[1024];
        GetPrivateProfileStringW(L"GestureList", command.c_str(), L"", buffer, 1024, ini_path);
        if (buffer[0])
        {
            std::wstring config = buffer;
            std::size_t action = config.find(L"|");
            if (action != std::wstring::npos)
            {
                std::size_t option = config.find(L"|", action + 1);
                if (option != std::wstring::npos)
                {
                    return config.substr(0, action);
                }
            }
        }
        return unKnow;
    }
    void ReadConfig()
    {
        show_operate = ::GetPrivateProfileIntW(L"Gesture", L"ShowResult", 1, ini_path) == 1;
        show_track = ::GetPrivateProfileIntW(L"Gesture", L"ShowTrail", 1, ini_path) == 1;
        show_arrow = ::GetPrivateProfileIntW(L"Gesture", L"ShowArrow", 1, ini_path) == 1;

        wchar_t color[MAX_PATH];
        GetPrivateProfileStringW(L"Gesture", L"Colour", L"C8337AB7", color, MAX_PATH, ini_path);

        swscanf(color, L"%02X%02X%02X%02X", &a, &r, &g, &b);

        thick = ::GetPrivateProfileIntW(L"Gesture", L"Weight", 3, ini_path);

        training_mode = ::GetPrivateProfileIntW(L"Gesture", L"TrainMode", 0, ini_path) == 1;
    }
private:
    // ini配置每次开始绘制时读取一次就够了
    bool show_operate;
    bool show_track;
    bool show_arrow;
    int a, r, g, b;
    int thick;
    bool training_mode;
    AddWindow addwindow;
    bool drawing;
private:
    std::wstring font_name;
    HWND main_hwnd;
    wchar_t ini_path[MAX_PATH];
};