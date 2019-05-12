class GesturePoint
{
public:
    void init_config(const std::wstring &inifile)
    {
        gm.SetConfigFile(inifile.c_str());
        gm.LoadGestures();
    }
    void init(int x, int y)
    {
        gm.LoadGestures();
        points.clear();
        points.push_back({ (double)x, (double)y });
    }
    void move(int x, int y)
    {
        points.push_back({ (double)x, (double)y });
    }
    const points_t get_points()
    {
        return points;
    }
    std::wstring result(const points_t &pts)
    {
        std::wstring name;
        bool result = gm.GestureRecognized(pts, name);
        if (result)
        {
            return name;
        }
        return L"";
    }
    std::wstring GetName(const points_t &pts)
    {
        return gm.GenerateGestureName(pts);
    }

    std::wstring result()
    {
        return result(points);
    }

    void AddGesture(const points_t &pts)
    {
        gm.AddGesture(pts);
    }

    void GetThumbnail(Image* img, int w, int h)
    {
        Color color = gm.GetColor();
        return GestureThumbnail(points, w, h, img, color);
    }
    const gesture_lists_t &GetList()
    {
        return gm.GetAvailableGestures();
    }
    bool Del(const std::wstring &name)
    {
        return gm.DeleteGesture(name);
    }
private:
    points_t points;
    GestureManager gm;
} gesture_point;