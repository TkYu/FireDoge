

/*
chrome ui tree


browser/ui/views/frame/BrowserRootView
ui/views/window/NonClientView
BrowserView
    TopContainerView
        TabStrip
            Tab
                TabCloseButton
            ImageButton
        ToolbarView
            LocationBarView
                OmniboxViewViews
    BookmarkBarView
        BookmarkButton
*/

std::wstring GetName(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild = CHILDID_SELF)
{
    if (!pAcc)
        return L"";
    CComBSTR bstrName;
    HRESULT hr = pAcc->get_accName(varChild, &bstrName);
    if (FAILED(hr))
        return L"";
    if (!bstrName.m_str)
        return L"<NULL>";
    return bstrName.m_str;
}

std::wstring GetRole(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild = CHILDID_SELF)
{
    if (!pAcc)
        return L"";
    CComVariant varRoleID;
    HRESULT hr = pAcc->get_accRole(varChild, &varRoleID);
    if (FAILED(hr))
        return L"";
    WCHAR sRoleBuff[1024] = { 0 };
    hr = ::GetRoleText(varRoleID.lVal, sRoleBuff, 1024);
    if (FAILED(hr))
        return L"";
    return sRoleBuff;
}

std::wstring GetValue(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild = CHILDID_SELF)
{
    if (!pAcc)
        return L"";
    CComBSTR bstrValue;
    HRESULT hr = pAcc->get_accValue(varChild, &bstrValue);
    if (FAILED(hr))
        return L"";
    if (!bstrValue.m_str)
        return L"<NULL>";
    return bstrValue.m_str;
}

template<typename Function>
void GetAccessibleName(IAccessible *node, Function f)
{
    VARIANT self;
    self.vt = VT_I4;
    self.lVal = CHILDID_SELF;

    BSTR bstr = NULL;
    if( S_OK == node->get_accName(self, &bstr) )
    {
        f(bstr);
        SysFreeString(bstr);
    }
}

template<typename Function>
void GetAccessibleValue(IAccessible *node, Function f)
{
    VARIANT self;
    self.vt = VT_I4;
    self.lVal = CHILDID_SELF;

    BSTR bstr = NULL;
    if( S_OK == node->get_accValue(self, &bstr) )
    {
        f(bstr);
        SysFreeString(bstr);
    }
}

float GetDPI()
{
    /*
    static float dpi = -1;
    if (dpi > 0)
    {
        return dpi;
    }
    if (IsWindows8OrGreater())
    {
        HKEY hKey;
        if (::RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop\\WindowMetrics", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            DWORD AppliedDPI = 0;
            DWORD dwSize = sizeof(DWORD);
            if (::RegQueryValueEx(hKey, L"AppliedDPI", NULL, NULL, (LPBYTE)&AppliedDPI, &dwSize) == ERROR_SUCCESS)
            {
                dpi = (float)AppliedDPI / 96.0f;
                ::RegCloseKey(hKey);
                return dpi;
            }
            ::RegCloseKey(hKey);
        }
    }
    dpi = 1.0;
    */
    return 1.0f * DPI / 100.0f;
}

template<typename Function>
void GetAccessibleSize(IAccessible *node, Function f)
{
    VARIANT self;
    self.vt = VT_I4;
    self.lVal = CHILDID_SELF;

    RECT rect;
    if( S_OK == node->accLocation(&rect.left, &rect.top, &rect.right, &rect.bottom, self) )
    {
        rect.left = (int)((float)rect.left * GetDPI());
        rect.top = (int)((float)rect.top * GetDPI());
        rect.right = (int)((float)rect.right * GetDPI());
        rect.bottom = (int)((float)rect.bottom * GetDPI());

        rect.right += rect.left;
        rect.bottom += rect.top;

        f(rect);
    }
}

long GetAccessibleState(IAccessible *node)
{
    VARIANT self;
    self.vt = VT_I4;
    self.lVal = CHILDID_SELF;

    VARIANT state;
    if( S_OK == node->get_accState(self, &state) )
    {
        if (state.vt == VT_I4)
        {
            return state.lVal;
        }
    }
    return 0;
}

long GetAccessibleRole(IAccessible *node)
{
    VARIANT self;
    self.vt = VT_I4;
    self.lVal = CHILDID_SELF;

    VARIANT role;
    if( S_OK == node->get_accRole(self, &role) )
    {
        if (role.vt == VT_I4)
        {
            return role.lVal;
        }
    }
    return 0;
}

template<typename Function>
void TraversalAccessible(IAccessible *node, Function f)
{
    long childCount = 0;
    if( node && S_OK == node->get_accChildCount(&childCount) && childCount )
    {
        VARIANT* varChildren = (VARIANT*)malloc(sizeof(VARIANT) * childCount);
        if( S_OK == AccessibleChildren(node, 0, childCount, varChildren, &childCount) )
        {
            for(int i=0;i<childCount;i++)
            {
                if( varChildren[i].vt==VT_DISPATCH )
                {
                    IDispatch* dispatch = varChildren[i].pdispVal;
                    IAccessible* child = NULL;
                    if (S_OK == dispatch->QueryInterface(IID_IAccessible, (void**)&child))
                    {
                        if ((GetAccessibleState(child) & STATE_SYSTEM_INVISIBLE) == 0) // 只遍历可见节点
                        {
                            if (f(child))
                            {
                                dispatch->Release();
                                break;
                            }
                            child->Release();
                        }
                    }
                    dispatch->Release();
                }
            }
        }
        free(varChildren);
    }
}



template<typename Function>
void TraversalRawAccessible(IAccessible *node, Function f)
{
    long childCount = 0;
    if (node && S_OK == node->get_accChildCount(&childCount) && childCount)
    {
        VARIANT* varChildren = (VARIANT*)malloc(sizeof(VARIANT) * childCount);
        if (S_OK == AccessibleChildren(node, 0, childCount, varChildren, &childCount))
        {
            for (int i = 0; i<childCount; i++)
            {
                if (varChildren[i].vt == VT_DISPATCH)
                {
                    IDispatch* dispatch = varChildren[i].pdispVal;
                    IAccessible* child = NULL;
                    if (S_OK == dispatch->QueryInterface(IID_IAccessible, (void**)&child))
                    {
                        //if ((GetAccessibleState(child) & STATE_SYSTEM_INVISIBLE) == 0) // 只遍历可见节点
                        {
                            if(f(child))
                            {
                                dispatch->Release();
                                break;
                            }
                            child->Release();
                        }
                    }
                    dispatch->Release();
                }
            }
        }
        free(varChildren);
    }
}

bool GetButtonCount(IAccessible *node, int &count)
{
    count = 0;
    bool flag = false;//标记是否有子节点
    TraversalAccessible(node, [&flag, &count]
    (IAccessible* child) {
        if (GetAccessibleRole(child) == ROLE_SYSTEM_PUSHBUTTON)
        {
            count++;
        }
        flag = true;
        return false;
    });
    return flag;
}

bool GetChildCountR(IAccessible *node, int &count)
{
    count = 0;
    bool flag = false;//标记是否有子节点
    TraversalAccessible(node, [&flag, &count]
    (IAccessible* child) {
        count++;
        flag = true;
        return false;
    });
    return flag;
}

IAccessible* GetChildElement(IAccessible *parent, bool aoto_release, int index)
{
    IAccessible* element = NULL;
    if(parent)
    {
        int i = 1;
        TraversalAccessible(parent, [&element, &i, &index]
            (IAccessible* child){
                if(i==index)
                {
                    element = child;
                }
                i++;
                return element!=NULL;
            });
        if(aoto_release) parent->Release();
    }
    return element;
}

bool CheckAccessibleRole(IAccessible* node, long role)
{
    if(node)
    {
        if(GetAccessibleRole(node)==role) return true;
        node->Release();
    }

    return false;
}

IAccessible* FindChildElement(IAccessible *parent, long role, int skipcount = 0)
{
    IAccessible* element = NULL;
    if (parent)
    {
        int i = 0;
        TraversalAccessible(parent, [&element, &role, &i, &skipcount]
        (IAccessible* child) {
            //DebugLog(L"当前 %d,%d", i, skipcount);
            if (GetAccessibleRole(child) == role)
            {
                if (i == skipcount) {
                    element = child;
                }
                i++;
            }
            return element != NULL;
        });
    }
    return element;
}

// 鼠标是否在标签栏上
bool IsOnTheTab(IAccessible* top, POINT pt)
{
    bool flag = false;
    IAccessible *TabStrip = FindChildElement(top, ROLE_SYSTEM_PAGETABLIST);
    if(TabStrip)
    {
        GetAccessibleSize(TabStrip, [&flag, &pt]
            (RECT rect){
                if(PtInRect(&rect, pt))
                {
                    flag = true;
                }
        });
        TabStrip->Release();
    }
    else
    {
        if (top) DebugLog(L"IsOnTheTab failed");
    }
    return flag;
}

// 鼠标是否在某个标签上
bool IsOnOneTab(IAccessible* top, POINT pt)
{
    bool flag = false;
    IAccessible *TabStrip = FindChildElement(top, ROLE_SYSTEM_PAGETABLIST);
    if(TabStrip)
    {
        TraversalAccessible(TabStrip, [&flag, &pt]
            (IAccessible* child){
                if(GetAccessibleRole(child)==ROLE_SYSTEM_PAGETAB)
                {
                    GetAccessibleSize(child, [&flag, &pt]
                        (RECT rect){
                            if(PtInRect(&rect, pt))
                            {
                                flag = true;
                            }
                        });
                }
                if(flag) child->Release();
                return flag;
            });
        TabStrip->Release();
    }
    else
    {
        if(top) DebugLog(L"IsOnOneTab failed!");
    }
    return flag;
}

// 是否悬停在关闭按钮上
bool IsOnOneInactiveTabCloseButton(IAccessible *node, POINT pt)
{
    bool flag = false;
    TraversalAccessible(node, [&pt, &flag]
    (IAccessible* child) {
        if (GetAccessibleRole(child) == ROLE_SYSTEM_PUSHBUTTON)
        {
            GetAccessibleSize(child, [&flag, &pt]
            (RECT rect) {
                //调整大小，更精确匹配
                rect.left += 4;
                rect.top += 7;
                rect.right -= 16;
                rect.bottom -= 6;
                InflateRect(&rect, 1, 1);
                if (PtInRect(&rect, pt))
                {
                    flag = true;
                }
            });
        }
        return false;
    });
    return flag;
}

// 获取到在第几个tab上
int GetTabIndex(IAccessible *node, POINT pt)
{
    std::vector <RECT> tab_rects;
    TraversalAccessible(node, [&]
    (IAccessible* child) {
        if (GetAccessibleRole(child) == ROLE_SYSTEM_PAGETAB)
        {
            GetAccessibleSize(child, [&]
            (RECT rect) {
                tab_rects.push_back(rect);
            });
        }
        return false;
    });
    std::sort(tab_rects.begin(), tab_rects.end(), [](auto &a, auto &b) {
        return a.left < b.left;
    });

    int index = 0;
    for (auto rect : tab_rects)
    {
        index++;
        if (PtInRect(&rect, pt))
        {
            break;
        }
    }

    if (index >= 9)
    {
        if (index == tab_rects.size())
        {
            index = 9;
        }
        else
        {
            index = 0;
        }

    }
    return index;
}

// 鼠标是否在某个未激活标签上
bool IsOnOneInactiveTab(IAccessible* top, POINT pt, int &index, bool &onclose)
{
    bool flag = false;
    index = 0;
    onclose = false;
    IAccessible *TabStrip = FindChildElement(top, ROLE_SYSTEM_PAGETABLIST);
    if (TabStrip)
    {
        TraversalAccessible(TabStrip, [&]
        (IAccessible* child) {
            if (GetAccessibleRole(child) == ROLE_SYSTEM_PAGETAB)
            {
                if (GetAccessibleState(child) & STATE_SYSTEM_SELECTED)
                {
                    // 跳过已经选中标签
                    return false;
                }
                GetAccessibleSize(child, [&]
                (RECT rect) {
                    if (PtInRect(&rect, pt))
                    {
                        flag = true;
                        onclose = IsOnOneInactiveTabCloseButton(child, pt);
                        index = GetTabIndex(TabStrip, pt);
                    }
                });
            }
            if (flag) child->Release();
            return flag;
        });
        TabStrip->Release();
    }
    else
    {
        if (top) DebugLog(L"IsOnOneTab failed");
    }
    return flag;
}

/*
// 是否只有一个标签
bool IsOnlyOneTab(IAccessible* top)
{
    IAccessible *TabStrip = FindChildElement(top, ROLE_SYSTEM_PAGETABLIST);
    if(TabStrip)
    {
        long tab_count = 0;
        bool closing = false;
        TraversalAccessible(TabStrip, [&tab_count, &closing]
        (IAccessible* child) {
            if (GetAccessibleRole(child) == ROLE_SYSTEM_PAGETAB)
            {
                if (closing)
                {
                    //已经找到一个正在关闭的标签了，其它的都算正常标签
                    tab_count++;
                }
                else
                {
                    //如果有关闭按钮才算标签
                    int count = 0;
                    if (GetButtonCount(child, count))
                    {
                        if (count == 0)
                            closing = true;
                        else
                            tab_count++;
                    }
                    else
                    {
                        //没有子节点，比如固定标签页
                        tab_count++;
                    }
                }
                }
            return false;
        });
        TabStrip->Release();
        return tab_count<=1;
    }
    else
    {
        if (top) DebugLog(L"IsOnlyOneTab failed");
    }
    return false;
}
*/

bool IsOnOneBookmarkInner(IAccessible* parent, POINT pt)
{
    bool flag = false;

    // 寻找书签栏
    IAccessible *BookmarkBarView = NULL;
    if (parent)
    {
        TraversalAccessible(parent, [&BookmarkBarView]
        (IAccessible* child) {
            if (GetAccessibleRole(child) == ROLE_SYSTEM_TOOLBAR)
            {
                IAccessible *group = FindChildElement(child, ROLE_SYSTEM_GROUPING);
                if (group==NULL)
                {
                    BookmarkBarView = child;
                    return true;
                }
                group->Release();
            }
            return false;
        });
    }

    if(BookmarkBarView)
    {
        TraversalAccessible(BookmarkBarView, [&flag, &pt]
            (IAccessible* child){
                if(GetAccessibleRole(child)==ROLE_SYSTEM_PUSHBUTTON)
                {
                    GetAccessibleSize(child, [&flag, &pt]
                        (RECT rect){
                            if(PtInRect(&rect, pt))
                            {
                                flag = true;
                            }
                        });
                }
                if(flag) child->Release();
                return flag;
            });
        BookmarkBarView->Release();
    }
    return flag;
}

// 是否点击书签栏
bool IsOnOneBookmark(IAccessible* top, POINT pt)
{
    bool flag = false;
    if(top)
    {
        // 开启了书签栏长显
        if(IsOnOneBookmarkInner(top, pt)) return true;

        // 未开启书签栏长显
        IDispatch* dispatch = NULL;
        if( S_OK == top->get_accParent(&dispatch) && dispatch)
        {
            IAccessible* parent = NULL;
            if( S_OK == dispatch->QueryInterface(IID_IAccessible, (void**)&parent))
            {
                flag = IsOnOneBookmarkInner(parent, pt);
                parent->Release();
            }
            dispatch->Release();
        }
    }
    return flag;
}

// 当前激活标签是否是新标签
bool IsBlankTab(IAccessible* top)
{
    bool flag = false;
    IAccessible *TabStrip = FindChildElement(top, ROLE_SYSTEM_PAGETABLIST);
    if(TabStrip)
    {
        wchar_t* new_tab_title = NULL;
        TraversalAccessible(TabStrip, [&new_tab_title]
            (IAccessible* child){
                if(GetAccessibleRole(child)==ROLE_SYSTEM_PUSHBUTTON)
                {
                    GetAccessibleName(child, [&new_tab_title]
                        (BSTR bstr){
                            new_tab_title = _wcsdup(bstr);
                    });
                }
                return false;
            });
        if(new_tab_title)
        {
            TraversalAccessible(TabStrip, [&flag, &new_tab_title]
                (IAccessible* child){
                    if(GetAccessibleState(child) & STATE_SYSTEM_SELECTED)
                    {
                        GetAccessibleName(child, [&flag, &new_tab_title]
                            (BSTR bstr){
                                if(wcscmp(bstr, new_tab_title)==0)
                                {
                                    flag = true;
                                }
                        });
                    }
                    return false;
                });
            free(new_tab_title);
        }
        TabStrip->Release();
    }
    return flag;
}

// 是否焦点在地址栏
bool IsOmniboxViewFocus(IAccessible* top)
{
    bool flag = false;

    // 寻找地址栏
    IAccessible *LocationBarView = NULL;
    if (top)
    {
        TraversalAccessible(top, [&LocationBarView]
        (IAccessible* child) {
            if (GetAccessibleRole(child) == ROLE_SYSTEM_TOOLBAR)
            {
                IAccessible *group = FindChildElement(child, ROLE_SYSTEM_GROUPING);
                if (group)
                {
                    LocationBarView = group;
                    child->Release();
                    return true;
                }
            }
            return false;
        });
    }

    if (LocationBarView)
    {
        IAccessible *OmniboxViewViews = FindChildElement(LocationBarView, ROLE_SYSTEM_TEXT);
        if(OmniboxViewViews)
        {
            GetAccessibleValue(OmniboxViewViews, [&OmniboxViewViews, &flag]
                (BSTR bstr){
                    if(bstr[0]!=0) // 地址栏为空直接跳过
                    {
                        if( (GetAccessibleState(OmniboxViewViews) & STATE_SYSTEM_FOCUSED) == STATE_SYSTEM_FOCUSED)
                        {
                            flag = true;
                        }
                    }
            });
            OmniboxViewViews->Release();
        }
        LocationBarView->Release();
    }
    else
    {
        if (top) DebugLog(L"IsOmniboxViewFocus failed");
    }
    return flag;
}
int GetChildCount(IAccessible *node)
{
    int count = 0;
    TraversalAccessible(node, [&count]
    (IAccessible* child) {
        count++;
        return false;
    });
    return count;
}

int GetPMCount(IAccessible *node)
{
    int count = 0;
    TraversalAccessible(node, [&count]
    (IAccessible* child) {
        const auto role = GetAccessibleRole(child);
        //DebugLog(L"类型是:%ld", role);
        if (role == ROLE_SYSTEM_PUSHBUTTON || role == ROLE_SYSTEM_BUTTONMENU)
        {
            count++;
        }
        return false;
    });
    return count;
}

IAccessible* GetApplicationView(IAccessible *top)
{
    IAccessible *Application = NULL;
    if(top)
    {
        Application = GetChildElement(top, true, 3);
        //第三个元素必定是APPLICATION
        if (!CheckAccessibleRole(Application, ROLE_SYSTEM_APPLICATION))
        {
            DebugLog(L"GetApplicationView fail!");
            Application->Release();
            return NULL;
        }
    }
    return Application;
}

IAccessible* GetLocationBarView(IAccessible *top)
{
    IAccessible *LocationBarView = NULL;
    if (top)
    {
        //地址栏是2个combobox中的一个，遍历一下，子项多的那个是地址栏
        TraversalAccessible(top, [&LocationBarView]
        (IAccessible* child) {
            const auto role = GetAccessibleRole(child);
            //DebugLog(L"类型是:%ld", role);
            if (role == ROLE_SYSTEM_COMBOBOX)
            {
                const auto childcount = GetChildCount(child);
                //DebugLog(L"GetLocationBarView %s | %s | %s | %d", GetName(child).c_str(), GetRole(child).c_str(), GetValue(child).c_str(), childcount);
                if(childcount>1)
                {
                    LocationBarView = child;
                    return true;
                }
            }
            return false;
        });
    }
    return LocationBarView;
}
IAccessible* GetNavigatorBar(HWND hwnd)
{
    IAccessible *TopContainerView = NULL;
    wchar_t name[MAX_PATH];
    if (GetClassName(hwnd, name, MAX_PATH) && wcscmp(name, L"MozillaWindowClass") == 0)
    {
        IAccessible *paccMainWindow = NULL;
        if (S_OK == AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_IAccessible, (void**)&paccMainWindow))
        {
            const auto Application = GetApplicationView(paccMainWindow);
            //TraversalAccessible(top, []
            //(IAccessible* child) {

            //	GetAccessibleSize(child, []
            //	(RECT rect) {
            //		DebugLog(L"GetAccessibleSize %d, %d %d, %d", rect.bottom, rect.top, rect.right, rect.left);
            //	});
            //	return false;
            //});
            if(Application)
            {
                //导航栏是第三个toolbar
                TopContainerView = FindChildElement(Application, ROLE_SYSTEM_TOOLBAR, 2);
                Application->Release();
                if (!TopContainerView)
                {
                    DebugLog(L"FindNavigatorBar failed");
                }
                //else
                //{
                //    DebugLog(L"FindNavigatorBar | %s | %s | %s | %d", GetName(TopContainerView).c_str(), GetRole(TopContainerView).c_str(), GetValue(TopContainerView).c_str(), GetChildCount(TopContainerView));
                //}
            }
        }
        else
        {
            DebugLog(L"AccessibleObjectFromWindow failed");
        }
    }
    return TopContainerView;
}
IAccessible* FindTopContainerView(IAccessible *top)
{
    top = GetChildElement(top, true, 3);
    if (!CheckAccessibleRole(top, ROLE_SYSTEM_APPLICATION)) return NULL;

    //TraversalAccessible(top, []
    //(IAccessible* child) {

    //	GetAccessibleSize(child, []
    //	(RECT rect) {
    //		DebugLog(L"GetAccessibleSize %d, %d %d, %d", rect.bottom, rect.top, rect.right, rect.left);
    //	});
    //	return false;
    //});

    top = GetChildElement(top, true, 5);
    if (!CheckAccessibleRole(top, ROLE_SYSTEM_TOOLBAR)) return NULL;

    return top;
}

IAccessible* FindPageTabList(IAccessible *node, int level=0)
{
    IAccessible *PageTabList = NULL;
    if (node && level<5)
    {
        TraversalAccessible(node, [&]
        (IAccessible* child) {
            if (GetAccessibleRole(child) == ROLE_SYSTEM_PAGETABLIST)
            {
                PageTabList = child;
            }
            else
            {
                PageTabList = FindPageTabList(child, level+1);
            }
            return PageTabList;
        });
    }
    return PageTabList;
}


IAccessible* GetParentElement(IAccessible *child)
{
    IAccessible* element = NULL;
    IDispatch* dispatch = NULL;
    if (S_OK == child->get_accParent(&dispatch) && dispatch)
    {
        IAccessible* parent = NULL;
        if (S_OK == dispatch->QueryInterface(IID_IAccessible, (void**)&parent))
        {
            element = parent;
        }
        dispatch->Release();
    }
    return element;
}

IAccessible* GetTopContainerView(HWND hwnd)
{
    IAccessible *TopContainerView = NULL;
    wchar_t name[MAX_PATH];
    if (GetClassName(hwnd, name, MAX_PATH) && wcscmp(name, L"MozillaWindowClass") == 0)
    {
        IAccessible *paccMainWindow = NULL;
        if (S_OK == AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_IAccessible, (void**)&paccMainWindow))
        {
            //const auto Application = GetApplicationView(paccMainWindow);
            //if (Application)
            //{
            //    //Tab栏是第二个Toolbar
            //    TopContainerView = FindChildElement(Application, ROLE_SYSTEM_TOOLBAR, 1);
            //    //Application->Release();
            //    if (!TopContainerView)
            //    {
            //        DebugLog(L"FindTabBar failed");
            //    }
            //    else
            //    {
            //        DebugLog(L"FindTabBar | %s | %s | %s | %d", GetName(TopContainerView).c_str(), GetRole(TopContainerView).c_str(), GetValue(TopContainerView).c_str(), GetChildCount(TopContainerView));
            //    }
            //}

           /* TopContainerView = FindTopContainerView(paccMainWindow);
            if (!TopContainerView)
            {
                DebugLog(L"FindTopContainerView failed");
            }*/

            IAccessible *PageTabList = FindPageTabList(paccMainWindow);
            if (PageTabList)
            {
                TopContainerView = GetParentElement(PageTabList);
            }
            if (!TopContainerView)
            {
                DebugLog(L"FindTopContainerView failed");
            }
        }
        else
        {
            DebugLog(L"AccessibleObjectFromWindow failed!");
        }
    }
    return TopContainerView;
}

/**
 * \brief 比对字符串忽略大小写
 * \param strHaystack 比对项
 * \param strNeedle 被比对项
 * \return 是否一致
 */
bool findStringIC(const std::wstring & strHaystack, const std::wstring & strNeedle)
{
    const auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](wchar_t ch1, wchar_t ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return (it != strHaystack.end());
}
/**
* \brief 获取当前URL
* \return 当前TAB的URL
*/
std::wstring GetCurrentURL()
{
    HWND wnd = FindWindowByProcessIdAndClassName(GetCurrentProcessId(), static_cast<LPWSTR>(L"MozillaWindowClass"));
    IAccessible *top = GetNavigatorBar(wnd);
    if (!top)
    {
        wnd = GetForegroundWindow();
        //DebugLog(L"2当前激活句柄：%p => %d", wnd, wnd);
        top = GetNavigatorBar(wnd);
    }
    std::wstring result = std::wstring();
    IAccessible *LocationBarView = GetLocationBarView(top);
    if (LocationBarView)
    {
        IAccessible *OmniboxViewViews = FindChildElement(LocationBarView, ROLE_SYSTEM_TEXT);
        if (OmniboxViewViews)
        {
            GetAccessibleValue(OmniboxViewViews, [&OmniboxViewViews, &result]
            (BSTR bstr) {
                if (bstr[0] != 0)
                {
                    result.append(bstr);
                }
            });
            OmniboxViewViews->Release();
        }
        LocationBarView->Release();
    }
    if (!result.empty())
    {
        if (!findStringIC(result, std::wstring(L"http://")) && !findStringIC(result, std::wstring(L"https://")) && !findStringIC(result, std::wstring(L"about:")))
        {
            result.insert(0, L"http://");
        }
    }
    if (top)top->Release();
    return result;
}

std::map <HWND, bool> tracking_hwnd;
bool ignore_mouse_event = false;

HHOOK mouse_hook = NULL;
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    static bool close_tab_ing = false;
    static bool wheel_tab_ing = false;
    
    bool close_tab = false;
    bool keep_tab = false;
    bool bookmark_new_tab = false;
    bool menu_bookmark_new_tab = false;

    if (nCode==HC_ACTION)
    {
        PMOUSEHOOKSTRUCT pmouse = (PMOUSEHOOKSTRUCT) lParam;

        if (wParam == WM_RBUTTONUP && wheel_tab_ing)
        {
            wheel_tab_ing = false;
            if (MouseGesture)
            {
                gesture_mgr.OnRButtonUp(pmouse, true);
            }
            return 1;
        }

        // 处理鼠标手势
        if(MouseGesture)
        {
            bool handled = false;

            if (wParam == WM_RBUTTONDOWN || wParam == WM_NCRBUTTONDOWN)
            {
                handled = gesture_mgr.OnRButtonDown(pmouse);
            }
            if (wParam == WM_RBUTTONUP || wParam == WM_NCRBUTTONUP)
            {
                handled = gesture_mgr.OnRButtonUp(pmouse);
            }
            if (wParam == WM_MOUSEMOVE || wParam == WM_NCMOUSEMOVE)
            {
                handled = gesture_mgr.OnMouseMove(pmouse);
            }

            if(handled)
            {
                return 1;
            }
        }

        if (RightTabSwitch)
        {
            if (wParam == WM_RBUTTONDOWN)
            {
                if (!ignore_mouse_event)
                {
                    return 1;
                }
            }
            if (wParam == WM_RBUTTONUP)
            {
                if (!ignore_mouse_event)
                {
                    ignore_mouse_event = true;
                    SendKey(VK_RBUTTON);
                    return 1;
                }
                ignore_mouse_event = false;
            }
        }

        if (HoverActivateTab && ignore_mouse_event)
        {
            if (wParam == WM_LBUTTONDBLCLK)
            {
                ignore_mouse_event = false;
                return 1;
            }
        }

        if (HoverActivateTab && wParam == WM_MOUSEMOVE)
        {
            HWND hwnd = WindowFromPoint(pmouse->pt);
            if (tracking_hwnd.find(hwnd) == tracking_hwnd.end())
            {
                TRACKMOUSEEVENT MouseEvent;
                MouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
                MouseEvent.dwFlags = TME_HOVER | TME_LEAVE;
                MouseEvent.hwndTrack = hwnd;
                MouseEvent.dwHoverTime = HoverTime;
                if (::TrackMouseEvent(&MouseEvent))
                {
                    tracking_hwnd[hwnd] = true;
                }
            }
        }

        // 忽略鼠标移动消息
        if(wParam==WM_MOUSEMOVE || wParam==WM_NCMOUSEMOVE)
        {
            return CallNextHookEx(mouse_hook, nCode, wParam, lParam );;
        }

        IAccessible* TopContainerView = GetTopContainerView(WindowFromPoint(pmouse->pt));

        // 双击关闭
        if(wParam==WM_LBUTTONDBLCLK && DoubleClickCloseTab && IsOnOneTab(TopContainerView, pmouse->pt))
        {
            close_tab = true;
        }

        // 右键关闭（没有按住SHIFT）
        if(wParam==WM_RBUTTONUP && RightClickCloseTab && !IsPressed(VK_SHIFT) && IsOnOneTab(TopContainerView, pmouse->pt))
        {
            close_tab = true;
        }

        // 只有一个标签
        //if(close_tab && KeepLastTab && IsOnlyOneTab(TopContainerView))
        //{
        //    keep_tab = true;
        //}

        if(wParam==WM_MBUTTONUP && IsOnOneTab(TopContainerView, pmouse->pt))
        {
            if(close_tab_ing)
            {
                close_tab_ing = false;
            }
            else
            {
                /*if(KeepLastTab && IsOnlyOneTab(TopContainerView))
                {
                    keep_tab = true;
                    close_tab = true;
                }*/
            }
        }

        if(wParam==WM_MOUSEWHEEL)
        {
            PMOUSEHOOKSTRUCTEX pwheel = (PMOUSEHOOKSTRUCTEX) lParam;
            int zDelta = GET_WHEEL_DELTA_WPARAM(pwheel->mouseData);
            if( HoverTabSwitch && IsOnTheTab(TopContainerView, pmouse->pt) )
            {
                // 切换标签页，发送ctrl+pagedown/pageup
                SendKey(VK_CONTROL, zDelta>0 ? VK_PRIOR : VK_NEXT);
            }
            else if( RightTabSwitch && IsPressed(VK_RBUTTON) )
            {
                // 切换标签页，发送ctrl+pagedown/pageup
                SendKey(VK_CONTROL, zDelta>0 ? VK_PRIOR : VK_NEXT);

                wheel_tab_ing = true;
                if(TopContainerView)
                {
                    TopContainerView->Release();
                }
                return 1;
            }
        }

  //      if(wParam==WM_LBUTTONUP && BookMarkNewTab && !IsPressed(VK_CONTROL) && IsOnOneBookmark(TopContainerView, pmouse->pt) )
  //      {
  //          if(!NotBlankTab || !IsBlankTab(TopContainerView))
  //          {
  //              bookmark_new_tab = true;
  //          }
  //      }
        //if (wParam == WM_LBUTTONUP && BookMarkNewTab && HandleMenu && !IsPressed(VK_CONTROL))
        //{
        //	HWND hwnd = WindowFromPoint(pmouse->pt);
        //	wchar_t name[MAX_PATH];
        //	if (GetClassName(hwnd, name, MAX_PATH) && wcscmp(name, L"Chrome_WidgetWin_2") == 0)
        //	{
        //		menu_bookmark_new_tab = true;
        //	}
        //}

        if(TopContainerView)
        {
            TopContainerView->Release();
        }
    }

    if(keep_tab)
    {
        // 最后一个标签页要关闭，新建一个标签
        // 打开新标签页，发送ctrl+t
        SendKey(VK_CONTROL, 'T');
    }

    if(close_tab)
    {
        // 发送中键消息，关闭标签
        SendKey(VK_MBUTTON);
        close_tab_ing = true;
        return 1;
    }

    if(bookmark_new_tab)
    {
        //if(FrontNewTab)
        //{
        //    // 发送中键消息，新建标签，前台
        //    SendKeys(VK_SHIFT, VK_MBUTTON);
        //}
        //else
        {
            // 发送中键消息，关闭标签
            SendKey(VK_MBUTTON);
        }
        return 1;
    }

    if (menu_bookmark_new_tab)
    {
        //if (FrontNewTab)
        //{
        //	// 新建标签，前台
        //	SendKeys(VK_CONTROL, VK_SHIFT, VK_LBUTTON);
        //}
        //else
        {
            // 新建标签
            SendKey(VK_CONTROL, VK_LBUTTON);
        }
        return 1;
    }

    return CallNextHookEx(mouse_hook, nCode, wParam, lParam );
}

bool IsInputMethodOpen(HWND hwnd)
{
    bool is_open = false;
    HIMC himc = ImmGetContext(hwnd);
    if (ImmGetOpenStatus(himc))
    {
        if (ImmGetCompositionString(himc, GCS_COMPSTR, NULL, 0))
        {
            is_open = true;
        }
    }
    ImmReleaseContext(hwnd, himc);
    return is_open;
}

bool IsInEditor(HWND hwnd)
{
    bool is_editor = false;
    HIMC himc = ImmGetContext(hwnd);
    CANDIDATEFORM cdf;
    if (ImmGetCandidateWindow(himc, 0, &cdf))
    {
        is_editor = true;
    }
    ImmReleaseContext(hwnd, himc);
    return is_editor;
}

HHOOK keyboard_hook = NULL;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    static bool open_url_ing = false;
    static bool close_tab_ing = false;
    if (nCode == HC_ACTION && KeyTransform)
    {
        int KeyDown = (lParam & 0x40000000) == 0x40000000;
        bool transition = (lParam & 0x80000000) == 0x80000000;
        //int scanCode = lParam & 0xFF0000;
        bool extend = (lParam & 0x1000000) == 0x1000000;
        if (!(lParam & 0x80000000) && (KeyRepeat || KeyDown == 0) )
        {
            switch (wParam)
            {
            case VK_CONTROL: // Ctrl
            case VK_MENU:    // Alt
            case VK_SHIFT:   // Shift
            case VK_LWIN:    // Win
            case VK_RWIN:
            case VK_CAPITAL: // Caps Lock
            case VK_NUMLOCK: // Num Lock
            case VK_SCROLL:  // Scroll Lock
                break;
            default:
                wchar_t name[256];
                int scanCode = ::MapVirtualKey(LOBYTE(wParam), MAPVK_VK_TO_VSC) << 16;
                if(extend) scanCode |= 0x1 << 24;
                if (GetKeyNameText(scanCode, name, _countof(name)))
                {
                    if(KeyIgnoreEdit || !IsInEditor(GetFocus()))
                    {
                        std::wstring keys;
                        if (IsPressed(VK_CONTROL))
                        {
                            keys += L"Ctrl+";
                        }
                        if (IsPressed(VK_SHIFT))
                        {
                            keys += L"Shift+";
                        }
                        if (IsPressed(VK_MENU))
                        {
                            keys += L"Alt+";
                        }
                        keys += name;
                        wchar_t transform_keys[256];
                        GetPrivateProfileStringW(L"KeyTransformationList", keys.c_str(), L"", transform_keys, 256, config_path);
                        if(DebugKeyPress) DebugLog(L"%s -> %s", keys.c_str(), transform_keys);
                        if (transform_keys[0])
                        {
                            SendKeys(transform_keys);
                            return 1;
                        }
                    }
                }
            }

        }

    }
    if (nCode==HC_ACTION && !(lParam & 0x80000000)) // pressed
    {
        //if(wParam==VK_RETURN && OpenUrlNewTab)
        //{
        //    if(open_url_ing)
        //    {
        //        open_url_ing = false;
        //        return CallNextHookEx(keyboard_hook, nCode, wParam, lParam );
        //    }
        //    HWND hwnd = GetFocus();
        //    IAccessible* TopContainerView = GetTopContainerView(hwnd);
        //    if( !IsPressed(VK_MENU) && !IsInputMethodOpen(hwnd) && IsOmniboxViewFocus(TopContainerView) )
        //    {
        //        if(!NotBlankTab || !IsBlankTab(TopContainerView))
        //        {
        //            open_url_ing = true;
        //        }
        //    }

        //    if(TopContainerView)
        //    {
        //        TopContainerView->Release();
        //    }

        //    if(open_url_ing)
        //    {
        //        // 在新标签打开url，发送alt+enter
        //        SendKeys(VK_MENU, VK_RETURN);
        //        return 1;
        //    }
        //}

        if (wParam == 'W' && close_tab_ing)
        {
            close_tab_ing = false;
            return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
        }

        if (wParam == VK_F4 && close_tab_ing)
        {
            close_tab_ing = false;
            return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
        }

        //bool keep_tab = false;
        //if (wParam == 'W' && IsPressed(VK_CONTROL) && !IsPressed(VK_SHIFT) && KeepLastTab)
        //{
        //    IAccessible* TopContainerView = GetTopContainerView(GetForegroundWindow());
        //    if (IsOnlyOneTab(TopContainerView))
        //    {
        //        keep_tab = true;
        //    }

        //    if (TopContainerView)
        //    {
        //        TopContainerView->Release();
        //    }
        //}
        //if (wParam == VK_F4 && IsPressed(VK_CONTROL) && KeepLastTab)
        //{
        //    IAccessible* TopContainerView = GetTopContainerView(GetForegroundWindow());
        //    //if (IsOnlyOneTab(TopContainerView))
        //    //{
        //    //    keep_tab = true;
        //    //}

        //    if (TopContainerView)
        //    {
        //        TopContainerView->Release();
        //    }
        //}

        //if (keep_tab)
        //{
        //    // 打开新标签页，发送ctrl+t
        //    SendKeys(VK_CONTROL, 'T');

        //    // 切换到上一个标签页
        //    SendKeys(VK_CONTROL, VK_PRIOR);

        //    // 关闭标签页
        //    close_tab_ing = true;
        //    SendKeys(VK_CONTROL, 'W');
        //    return 1;
        //}
    }
    return CallNextHookEx(keyboard_hook, nCode, wParam, lParam );
}

void SendClick()
{
    ignore_mouse_event = true;
    SendKey(VK_LBUTTON);
    std::thread th([]() {
        Sleep(500);
        ignore_mouse_event = false;
    });
    th.detach();
}

HHOOK message_hook = NULL;
LRESULT CALLBACK MessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        MSG* msg = (MSG*)lParam;


        if (msg->message == WM_MOUSEHOVER)
        {
            HWND hwnd = WindowFromPoint(msg->pt);
            IAccessible* TopContainerView = GetTopContainerView(hwnd);
            int index = 0;
            bool onclose;
            if (IsOnOneInactiveTab(TopContainerView, msg->pt, index, onclose))
            {
                if (GetForegroundWindow() != hwnd)
                {
                    SetForegroundWindow(hwnd);
                }
                if (index >= 1 && index <= 9)
                {
                    // 1到8标签用ctrl+数字直接跳，9为最后一个标签
                    SendKey(VK_CONTROL, '0' + index);
                }
                else
                {
                    if (onclose)
                    {
                        // 移动鼠标位置点击后再移动回来
                        POINT pt;
                        GetCursorPos(&pt);
                        SetCursorPos(pt.x - 30, pt.y);
                        SendClick();
                        SetCursorPos(pt.x, pt.y);
                    }
                    else
                    {
                        SendClick();
                    }
                }
        }
            if (TopContainerView)
            {
                TopContainerView->Release();
            }

            tracking_hwnd.erase(msg->hwnd);
        }
        else if (msg->message == WM_MOUSELEAVE)
        {
            tracking_hwnd.erase(msg->hwnd);
        }
    }
    return CallNextHookEx(message_hook, nCode, wParam, lParam);
}

void TabBookmark()
{
    mouse_hook = SetWindowsHookEx(WH_MOUSE, MouseProc, hInstance, GetCurrentThreadId());
    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, hInstance, GetCurrentThreadId());
    message_hook = SetWindowsHookEx(WH_GETMESSAGE, MessageProc, hInstance, GetCurrentThreadId());

    {
        std::thread th(StartGestureThread);
        th.detach();
    }
}
