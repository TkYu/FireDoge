points_t ScaleGesture(const points_t &points, int width, int height, SIZE &scaled_size)
{
    // Create generic list of points to hold scaled stroke
    points_t scaled_points;

    // Get total width and height of gesture
    double minimum_x = points[0].x;
    double minimum_y = points[0].y;
    double maximum_x = points[0].x;
    double maximum_y = points[0].y;
    for (size_t i = 1; i < points.size(); i++)
    {
        if (points[i].x < minimum_x)
            minimum_x = points[i].x;
        if (points[i].y < minimum_y)
            minimum_y = points[i].y;
        if (points[i].x > maximum_x)
            maximum_x = points[i].x;
        if (points[i].y > maximum_y)
            maximum_y = points[i].y;
    }
    double gesture_width = maximum_x - minimum_x;
    double gesture_height = maximum_y - minimum_y;

    // Get each scale ratio
    double scale = 1;
    if (gesture_width >= gesture_height)
        scale = (double)width / gesture_width;
    else
        scale = (double)height / gesture_height;

    for (auto &pt : points)
    {
        scaled_points.push_back({
            (pt.x - minimum_x) * scale,
            (pt.y - minimum_y) * scale
        });
    }

    // Calculate new gesture width and height
    scaled_size.cx = (LONG)(gesture_width * scale);
    scaled_size.cy = (LONG)(gesture_height * scale);

    return scaled_points;
}

void GestureThumbnail(const points_t &points, int width, int height, Image* image, const Color& color)
{
    Graphics graphics(image);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.Clear(0);

    if (points.size()<2) return;

    Pen pen(color, 4);
    pen.SetLineJoin(LineJoinRound);
    pen.SetStartCap(Gdiplus::LineCapRoundAnchor);
    //pen.SetEndCap(Gdiplus::LineCapArrowAnchor);

    AdjustableArrowCap end_cap(4, 3.5);
    pen.SetCustomEndCap(&end_cap);

    SIZE scaled_size;
    points_t scaled_points = ScaleGesture(points, width - 16, height - 16, scaled_size);

    int left_offset = (width - scaled_size.cx) / 2;
    int top_offset = (height - scaled_size.cy) / 2;

    std::vector<PointF> pnts;
    for (auto &point : scaled_points)
    {
        pnts.push_back({ (Gdiplus::REAL)point.x + left_offset, (Gdiplus::REAL)point.y + top_offset });
    }

    graphics.DrawLines(&pen, &pnts[0], (INT)pnts.size());
}

#include "base64.h"
std::string GestureThumbnailBase64(const points_t &points, int width, int height, const Color& color)
{
    Image* thumbnail = new Bitmap(width, height);
    GestureThumbnail(points, width, height, thumbnail, color);

    IStream *stream;
    ::CreateStreamOnHGlobal(NULL, TRUE, &stream);

    const CLSID png_clsid = { 0x557cf406, 0x1a04, 0x11d3,{ 0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e } };

    thumbnail->Save(stream, &png_clsid);

    HGLOBAL global = NULL;
    GetHGlobalFromStream(stream, &global);
    LPBYTE bytes = (LPBYTE)GlobalLock(global);
    size_t size = GlobalSize(global);

    static std::string encoded;
    static std::string decoded;

    encoded.resize(Base64::EncodedLength(size));
    Base64::Encode((const char*)bytes, size, &encoded[0], encoded.size());
    Base64::StripPadding(&encoded);

    GlobalUnlock(global);
    stream->Release();
    delete thumbnail;

    return encoded;
}