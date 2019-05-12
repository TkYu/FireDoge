#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "GestureMath.h"

typedef struct
{
    points_t points;
    std::vector<double> angles;
} gesture_t;

typedef struct
{
    std::wstring name;
    double probability;
} match_result_t;

typedef std::map<std::wstring, gesture_t> patterns_t;
typedef std::vector<match_result_t> match_results_t;

typedef struct
{
    std::wstring name;
    std::string png;
} gesture_info_t;

typedef std::vector<gesture_info_t> gesture_lists_t;

#include "GestureThumbnail.h"

const int precision = 100;

class GestureManager
{
public:
    void SetConfigFile(const wchar_t *file);
    Color GetColor();
    bool LoadGestures();
    bool SaveGestures();
public:
    const gesture_lists_t & GetAvailableGestures();
    bool GestureExists(std::wstring name);
public:
    std::wstring AddGesture(const points_t &points, bool save);
    bool DeleteGesture(std::wstring name);
public:
    bool GestureRecognized(const points_t &points, std::wstring &name);
    std::wstring GenerateGestureName(const points_t &points);
private:
    std::wstring SerializePoints(const points_t &points);
    match_results_t GetMatchResults(const points_t &points);
private:
    patterns_t patterns;
    gesture_lists_t lists;
    std::wstring inifile;
};

void GestureManager::SetConfigFile(const wchar_t *file)
{
    inifile = file;
}

Color GestureManager::GetColor()
{
    if (inifile.empty()) return Color(200, 51, 122, 183);

    wchar_t color[MAX_PATH];
    int a, r, g, b;
    GetPrivateProfileStringW(L"Gesture", L"Colour", L"C8337AB7", color, MAX_PATH, inifile.c_str());
    swscanf(color, L"%02X%02X%02X%02X", &a, &r, &g, &b);
    return Color(a, r, g, b);
}

bool GestureManager::LoadGestures()
{
    if (inifile.empty()) return false;

    patterns.clear();
    lists.clear();

    auto contents = GetSection(L"GestureData", inifile.c_str());

    for (auto &text : contents)
    {
        points_t points;

        std::size_t start = 0;
        while (true)
        {
            std::size_t end = text.find(L"|", start);
            if (end == std::string::npos) break;

            std::wstring temp = text.substr(start, end - start);

            point_t point;
            swscanf(temp.c_str(), L"%lf,%lf", &point.x, &point.y);
            points.push_back(point);

            start = end + 1;
        }

        AddGesture(points, false);
    }

    return true;
}

bool GestureManager::SaveGestures()
{
    if (inifile.empty()) return false;

    std::vector <std::wstring> values;
    for (auto &pattern : patterns)
    {
        values.push_back(SerializePoints(pattern.second.points));
    }

    SetSection(L"GestureData", values, inifile.c_str());

    return true;
}

const gesture_lists_t &GestureManager::GetAvailableGestures()
{
    Color color = GetColor();
    for (auto &gesture : lists)
    {
        gesture.png = GestureThumbnailBase64(patterns[gesture.name].points, 96, 96, color);
    }
    return lists;
}

bool GestureManager::GestureExists(std::wstring name)
{
    return patterns.find(name) != patterns.end();
}

std::wstring GestureManager::AddGesture(const points_t &points, bool save = true)
{
    std::wstring name = GenerateGestureName(points);
    patterns[name] = {points, GetPointArrayAngularMargins(GetInterpolatedPointArray(points, precision))};

    lists.push_back({ name });

    if(save) SaveGestures();
    return name;
}

bool GestureManager::DeleteGesture(std::wstring name)
{
    bool result = patterns.erase(name) != 0;
    if (result)
    {
        lists.erase(std::remove_if(lists.begin(), lists.end(), [&](const gesture_info_t & item) {
            return item.name == name;
        }));


        SaveGestures();
    }
    return result;
}

bool GestureManager::GestureRecognized(const points_t &points, std::wstring &name)
{
    if (points.size()<2) return false;

    match_results_t results = GetMatchResults(points);
    if (results.size() == 0) return false;

    std::sort(results.begin(), results.end(), [](const match_result_t &a, const match_result_t &b) {
        return a.probability > b.probability;
    });

    name = results.begin()->name;
    return true;
}

std::wstring GestureManager::GenerateGestureName(const points_t &points)
{
    uint32_t sign = 0;
    for (auto &point : points)
    {
        sign ^= (((uint32_t)point.x << 16) | (uint32_t)point.y);
        sign = sign & 1 ? (sign >> 1) ^ 0XEDB88320 : sign >> 1;
        sign = sign & 1 ? (sign >> 1) ^ 0XEDB88320 : sign >> 1;
        sign = sign & 1 ? (sign >> 1) ^ 0XEDB88320 : sign >> 1;
        sign = sign & 1 ? (sign >> 1) ^ 0XEDB88320 : sign >> 1;
        sign = sign & 1 ? (sign >> 1) ^ 0XEDB88320 : sign >> 1;
        sign = sign & 1 ? (sign >> 1) ^ 0XEDB88320 : sign >> 1;
        sign = sign & 1 ? (sign >> 1) ^ 0XEDB88320 : sign >> 1;
        sign = sign & 1 ? (sign >> 1) ^ 0XEDB88320 : sign >> 1;
    }
    wchar_t buf[16];
    swprintf_s(buf, L"%08X", sign);
    return buf;
}

std::wstring GestureManager::SerializePoints(const points_t &points)
{
    std::wstring str;
    /*
    double minimum_x = points[0].x;
    double minimum_y = points[0].y;
    for (size_t i = 1; i < points.size(); i++)
    {
        if (points[i].x < minimum_x)
        {
            minimum_x = points[i].x;
        }
        if (points[i].y < minimum_y)
        {
            minimum_y = points[i].y;
        }
    }
    */
    for (auto &point : points)
    {
        wchar_t buf[64];
        swprintf_s(buf, L"%d,%d|", (int)(point.x), (int)(point.y));
        str += buf;
    }

    return str;
}

match_results_t GestureManager::GetMatchResults(const points_t &points)
{
    std::vector<double> angles = GetPointArrayAngularMargins(GetInterpolatedPointArray(points, precision));

    match_results_t results;
    for (auto &pattern : patterns)
    {
        double deltas = 0;
        for (size_t i = 0; i < pattern.second.angles.size(); i++)
        {
            deltas += GetAngularDelta(pattern.second.angles[i], angles[i]);
        }

        match_result_t result;
        result.name = pattern.first;
        result.probability = GetProbabilityFromAngularDelta(deltas / precision);
        if(result.probability > 55.0f + 40.f * (1.0f * Sensitive / 100) )
        {
            results.push_back(result);
        }
    }

    return results;
}