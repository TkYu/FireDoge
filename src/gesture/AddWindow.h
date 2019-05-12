
const int thumbnail_size = 128;
const int window_width = 350;
const int padding = 10;
const int window_hight = thumbnail_size + padding * 2;

#define IDC_DONE_BUTTON 1
#define IDC_CANCEL_BUTTON 2
#define IDC_ORDER_COMBO 3

std::vector <std::wstring> action_names = {
    L"SendKeys",
    L"Window",
    L"OpenUrl",
    L"RunApp",
    L"CopyToClipboard",
};

class AddWindow :
    public ATL::CWindowImpl<AddWindow>
{
public:
    DECLARE_WND_CLASS(L"FireDogeGesture")

    BEGIN_MSG_MAP(AddWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT(OnPaint)

        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        COMMAND_HANDLER(IDC_DONE_BUTTON, BN_CLICKED, OnDoneClicked)
        COMMAND_HANDLER(IDC_CANCEL_BUTTON, BN_CLICKED, OnCancelClicked)
        COMMAND_HANDLER(IDC_ORDER_COMBO, CBN_SELCHANGE, OnOrderChange)
    END_MSG_MAP()
public:
    void RefreshThumbnail(const std::wstring &path)
    {
        SetWindowText(Lang(L"AddNew").c_str());
        ini_path = path;
        points = gesture_point.get_points();
        gesture_point.GetThumbnail(thumbnail, thumbnail->GetWidth(), thumbnail->GetHeight());

        std::wstring result = gesture_point.result(points);
        if (!result.empty())
        {
            SetWindowText(Lang(L"AlreadyExist").c_str());
        }
    }
    LRESULT OnDoneClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        CString name_text;
        CString option_text;

        name.GetWindowText(name_text);
        option.GetWindowText(option_text);

        int action_choose = action.GetCurSel();
        int window_choose = window_option.GetCurSel();

        if (name_text.IsEmpty())
        {
            MessageBox(Lang(L"NameRequire").c_str());
            return 0;
        }
        if (action_choose != 1)
        {
            if (option_text.IsEmpty())
            {
                MessageBox(Lang(L"ParamRequire").c_str());
                return 0;
            }
        }

        std::wstring action;
        action += name_text;
        action += L"|";
        action += action_names[action_choose];
        action += L"|";
        if (action_choose != 1)
        {
            action += option_text;
        }
        else
        {
            action += ('0' + window_choose);
        }

        std::wstring result = gesture_point.GetName(points);
        ::WritePrivateProfileString(L"GestureList", result.c_str(), action.c_str(), ini_path.c_str());
        gesture_point.AddGesture(points);
        OnCancel();
        return 0;
    }
    LRESULT OnCancelClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        OnCancel();
        return 0;
    }
    LRESULT OnOrderChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (action.GetCurSel() == 1)
        {
            window_option.ShowWindow(SW_SHOW);
            option.ShowWindow(SW_HIDE);
        }
        else if (action.GetCurSel() == 4)
        {
            option.ShowWindow(SW_SHOW);
            window_option.ShowWindow(SW_HIDE);
            CString option_text = CString("{CURRENT_URL}");
            option.SetWindowText(option_text);
        }
        else
        {
            option.ShowWindow(SW_SHOW);
            window_option.ShowWindow(SW_HIDE);
            CString option_text = CString("");
            option.SetWindowText(option_text);
        }
        return 0;
    }

private:
    void OnCancel()
    {
        ShowWindow(SW_HIDE);
        CenterWindow();
        name.SetWindowTextW(L"");
        action.SetCurSel(0);
        window_option.SetCurSel(0);
        option.SetWindowTextW(L"");
        option.ShowWindow(SW_SHOW);
        window_option.ShowWindow(SW_HIDE);
    }
    LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        ResizeClient(window_width, window_hight + 20);
        /*if(!IsProcessDPIAware())
        {
        DebugLog(L"SetProcessDPIAware");
        SetProcessDPIAware();
        }
        if(dpiScale == 0)
        {
        HDC screen = GetDC();
        double hPixelsPerInch = GetDeviceCaps(screen, LOGPIXELSX);
        double vPixelsPerInch = GetDeviceCaps(screen, LOGPIXELSY);
        ReleaseDC(screen);
        dpiScale = (hPixelsPerInch + vPixelsPerInch) * 0.5 / 96;
        DebugLog(L"h = %f,v = %f, result = %f%%", hPixelsPerInch, vPixelsPerInch, dpiScale * 100);
        }*/
        InitFont();
        CenterWindow();

        thumbnail = new Bitmap(thumbnail_size * 3 / 2, thumbnail_size * 3 / 2);

        int x = thumbnail_size + padding;
        int y = 10 + padding;
        int width = 40;

        RECT pos = { x, y, x + width, y + hight };
        text1.Create(m_hWnd, pos, Lang(L"Name:").c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL);
        text1.SetFont(font);

        y += hight;
        y += 15;
        pos = { x, y, x + width, y + hight };
        text2.Create(m_hWnd, pos, Lang(L"Action:").c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL);
        text2.SetFont(font);

        y += hight;
        y += 15;
        pos = { x, y, x + width, y + hight };
        text3.Create(m_hWnd, pos, Lang(L"Param:").c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL);
        text3.SetFont(font);

        //
        width = window_width - thumbnail_size - width - padding * 2;
        x = window_width - width - padding;
        y = 10 + padding;

        pos = { x, y, x + width, y + hight };
        name.Create(m_hWnd, pos, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, WS_EX_CLIENTEDGE);
        name.SetFont(font);

        y += hight;
        y += 15;
        pos = { x, y, x + width, y + hight };
        action.Create(m_hWnd, pos, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 0, IDC_ORDER_COMBO);
        action.SetFont(font);

        y += hight;
        y += 15;
        pos = { x, y, x + width, y + hight };
        window_option.Create(m_hWnd, pos, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP);
        window_option.SetFont(font);

        option.Create(m_hWnd, pos, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, WS_EX_CLIENTEDGE);
        option.SetFont(font);

        //
        width = 96;
        hight += 3;
        x = window_width - width - padding;
        y = window_hight - padding - hight + 20;
        pos = { x, y, x + width, y + hight };
        done.Create(m_hWnd, pos, Lang(L"OK").c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 0, IDC_DONE_BUTTON);
        done.SetFont(font);

        x -= width;
        x -= padding;
        pos = { x, y, x + width, y + hight };
        cancel.Create(m_hWnd, pos, Lang(L"Cancel").c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 0, IDC_CANCEL_BUTTON);
        cancel.SetFont(font);

        action.AddString(Lang(L"SendKeys").c_str());
        action.AddString(Lang(L"Window").c_str());
        action.AddString(Lang(L"OpenUrl").c_str());
        action.AddString(Lang(L"RunApp").c_str());
        action.AddString(Lang(L"CopyToClipboard").c_str());
        action.SetCurSel(0);

        window_option.AddString(Lang(L"Min").c_str());
        window_option.AddString(Lang(L"MaxOrRestore").c_str());
        window_option.AddString(Lang(L"TopMost").c_str());
        window_option.SetCurSel(0);

        return 0;
    }

    void OnPaint(HDC dc)
    {
        PAINTSTRUCT  ps;
        CDC hdc = BeginPaint(&ps);
        Graphics graphics(hdc);

        graphics.DrawImage(thumbnail, 2, 15, thumbnail_size, thumbnail_size);

        EndPaint(&ps);
    }

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        delete thumbnail;
        return 0;
    }

    LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        OnCancel();
        return 0;
    }
private:
    LRESULT OnCtlColorStatic(UINT, WPARAM, LPARAM, BOOL&)
    {
        return (LRESULT)::GetStockObject(WHITE_BRUSH);
    }
    void InitFont()
    {
        LOGFONT lf = { 0 };
        NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, false);
        lf = ncm.lfMessageFont;
        lf.lfHeight = -12;
        hight = -lf.lfHeight * 2;

        ATLVERIFY(font.CreateFontIndirect(&lf));
    }
    Image* thumbnail;

    CStatic text1;
    CStatic text2;
    CStatic text3;

    CEdit name;
    CComboBox action;
    CComboBox window_option;
    CEdit option;

    CButton cancel;
    CButton done;

    int hight;
    CFont font;
    points_t points;
    std::wstring ini_path;
};
