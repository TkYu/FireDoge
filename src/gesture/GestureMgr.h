// 识别是否启用手势
class GestureMgr
{
public:
    GestureMgr()
    {
        gesture_window = NULL;
        running_ = false;
        recognition_ = false;
        ignore_mouse_event = false;
    }
    void Init(GestureWindow *wnd)
    {
        gesture_window = wnd;
        running_ = false;
        recognition_ = false;
        ignore_mouse_event = false;
    }

    //鼠标右键按下
    bool OnRButtonDown(PMOUSEHOOKSTRUCT pmouse)
    {
        if (!gesture_window) return false;
        if(!ignore_mouse_event)
        {
            running_ = true;
            init = pmouse->pt;
            gesture_window->SendMessageW(WM_USER_HWND, (WPARAM)pmouse->hwnd, (LPARAM)&init);
            return true;
        }

        return false;
    }

    //鼠标右键弹起
    bool OnRButtonUp(PMOUSEHOOKSTRUCT pmouse, bool ignore = false)
    {
        if (!gesture_window) return false;
        if(!ignore_mouse_event && running_)
        {
            if(recognition_)
            {
                gesture_window->SendMessageW(WM_USER_END);
            }
            else
            {
                if (!ignore)
                {
                    ignore_mouse_event = true;

                    SendKey(VK_RBUTTON);
                }
            }
            running_ = false;
            recognition_ = false;
            return true;
        }

        ignore_mouse_event = false;
        return false;
    }

    // 鼠标移动
    bool OnMouseMove(PMOUSEHOOKSTRUCT pmouse)
    {
        if (!gesture_window) return false;
        if(running_)
        {
            if(recognition_)
            {
                if(IsPressed(VK_RBUTTON))
                {
                    gesture_window->SendMessageW(WM_USER_UPDATE, pmouse->pt.x, pmouse->pt.y);
                }
                else
                {
                    gesture_window->SendMessageW(WM_USER_END);
                    running_ = false;
                    recognition_ = false;
                }
            }
            else
            {
                // 尚未开始识别
                if(GetDistance(init, pmouse->pt)>10)
                {
                    gesture_window->SendMessageW(WM_USER_SHOW);
                    recognition_ = true;
                }
            }
        }
        return false;
    }
private:
    int GetDistance(POINT a, POINT b)
    {
        int s = (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
        return _floor(SquareRootFloat((float)s));
    }
    float SquareRootFloat(float number)
    {
        long i;
        float x, y;
        const float f = 1.5F;
        x = number * 0.5F;
        y = number;
        i = *(long *)&y;
        i = 0x5f375a86 - (i >> 1);   //注意这一行
        y = *(float *)&i;
        y = y * (f - (x * y * y));
        return number * y;
    }
    int _floor(float f)
    {
        int a = *(int*)(&f);
        int sign = (a >> 31);
        int mantissa = (a&((1 << 23) - 1)) | (1 << 23);
        int exponent = ((a & 0x7fffffff) >> 23) - 127;
        int r = ((unsigned int)(mantissa) << 8) >> (31 - exponent);
        return ((r ^ (sign)) - sign) &~(exponent >> 31);
    }
private:
    bool running_;
    bool recognition_;
    bool ignore_mouse_event;
    POINT init;
    GestureWindow *gesture_window;
} gesture_mgr;

void StartGestureThread()
{
    // 初始化gdiplus
    GdiplusStartupInput in;
    ULONG_PTR gdiplus_token;
    GdiplusStartup(&gdiplus_token, &in, NULL);

    // 初始化手势
    GestureWindow wnd;
    wnd.Create(NULL, CRect(0, 0, 0, 0), L"FireDogeGesture", WS_POPUP, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE);

    if (MouseGesture)
    {
        gesture_mgr.Init(&wnd);
    }

    // 消息循环
    WTL::CMessageLoop msgLoop;
    int ret = msgLoop.Run();

    GdiplusShutdown(gdiplus_token);
}
