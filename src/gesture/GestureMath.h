#include <math.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

typedef struct
{
    double x;
    double y;
} point_t;

typedef std::vector<point_t> points_t;

#define M_PI        3.14159265358979323846

double GetAngularGradient(const point_t &start_point, const point_t &end_point)
{
    return atan2(end_point.y - start_point.y, end_point.x - start_point.x);
}

double GetAngularDelta(double angle1, double angle2)
{
    double ret_value = fabs(angle1 - angle2);

    if (ret_value > M_PI)
        ret_value = M_PI - (ret_value - M_PI);

    return ret_value;
}

double GetProbabilityFromAngularDelta(double AngularDelta)
{
    const double dScale = 31.830988618379067;
    return fabs(AngularDelta * dScale - 100);
}

double GetDegreeFromRadian(double Angle)
{
    return Angle * (180.0 / M_PI);
}

double GetDistance(const point_t &start_point, const point_t &end_point)
{
    return sqrt(pow(end_point.x - start_point.x, 2.0F) + pow(end_point.y - start_point.y, 2.0F));
}

double GetPointArrayLength(const points_t &points)
{
    // Create return variable to hold final calculated length
    double ret_length = 0;

    // Enumerate points in point pattern and get a sum of each line segments distances
    for (size_t i = 1; i < points.size(); i++)
        ret_length += GetDistance(points[i - 1], points[i]);

    // Return calculated length
    return ret_length;
}

point_t GetInterpolatedPoint(const point_t &start_point, const point_t &end_point, double interpolate_position)
{
    // Create return point
    point_t return_point;

    // Calculate x and y of increment point
    return_point.x = (1 - interpolate_position) * start_point.x + interpolate_position * end_point.x;
    return_point.y = (1 - interpolate_position) * start_point.y + interpolate_position * end_point.y;

    // Return new point
    return return_point;
}

std::vector<double> GetPointArrayAngularMargins(const points_t &point_array)
{
    // Create an empty collection of angles
    std::vector<double> angular_margins;

    // Enumerate input point array starting with second point and calculate angular margin
    for (size_t i = 1; i < point_array.size(); i++)
        angular_margins.push_back(GetAngularGradient(point_array[i - 1], point_array[i]));

    // Return angular margins array
    return std::move(angular_margins);
}

points_t GetInterpolatedPointArray(const points_t &points, size_t segments)
{
    // Create an empty return collection to store interpolated points
    points_t interpolated_points;

    // Precalculate desired segment length and define helper variables
    double desired_segment_length = GetPointArrayLength(points) / segments;
    double current_segment_length = 0;	// Initialize to zero

                                        // Add first point in point pattern to return array and save it for use in the interpolation process
    point_t last_test_point = points[0]; // Initialize to first point in point pattern
    interpolated_points.push_back(last_test_point);

    // Enumerate points starting with second point (if any)
    for (size_t i = 1; i < points.size(); i++)
    {
        // Store current index point in helper variable
        point_t currentPoint = points[i];

        // Calculate distance between last added point and current point in point pattern
        // and use calculated length to calculate test segment length for next point to add
        double inc_to_current_length = GetDistance(last_test_point, currentPoint);
        double test_segment_length = current_segment_length + inc_to_current_length;

        // Does the test segment length meet our desired length requirement
        if (test_segment_length < desired_segment_length)
        {
            // Desired segment length has not been satisfied so we don't need to add an interpolated point
            // save this test point and move on to next test point
            current_segment_length = test_segment_length;
            last_test_point = currentPoint;
            continue;
        }

        // Test segment length has met or exceeded our desired segment length
        // so lets calculate how far we overshot our desired segment length and calculate
        // an interpolation position to use to derive our new interpolation point
        double interpolation_position = (desired_segment_length - current_segment_length) * (1 / inc_to_current_length);

        // Use interpolation position to derive our new interpolation point
        point_t interpolatedPoint = GetInterpolatedPoint(last_test_point, currentPoint, interpolation_position);
        interpolated_points.push_back(interpolatedPoint);

        // Sometimes rounding errors cause us to attempt to add more points than the user has requested.
        // If we've reached our segment count limit, exit loop
        if (interpolated_points.size() == segments)
            break;

        // Store new interpolated point as last test point for use in next segment calculations
        // reset current segment length and jump back to the last index because we aren't done with original line segment
        last_test_point = interpolatedPoint;
        current_segment_length = 0;
        i--;
    }

    // Return interpolated point array
    return std::move(interpolated_points);
}
